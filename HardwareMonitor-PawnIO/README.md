# HardwareMonitor-PawnIO：把硬件监控从 WinRing0 迁移到 PawnIO

## 这个目录解决什么问题

TrafficMonitor 的硬件监控（CPU 温度 / 主板温度）依赖第三方库 **LibreHardwareMonitor**。
2025 年以前的 LibreHardwareMonitor 会内嵌 **WinRing0 1.2.0.5** 驱动，在运行时释放成 `<进程名>.sys`
（装到 `D:\TrafficMonitor` 时就叫 `TrafficMonitor.sys`），然后加载到内核里读 MSR / I/O 端口。

这个驱动有公开漏洞 **CVE-2020-14979**（CVSS 7.8）：设备对象 `\Device\WinRing0_1_2_0` 对任意本地用户开放，
可以通过 IOCTL 读写 MSR、I/O 端口、PCI 配置空间、映射物理内存 → 低权限进程可提权到 SYSTEM。
微软已把它列入「易受攻击驱动程序」名单，Defender 检测名为 `VulnerableDriver:WinNT/Winring0`，
火绒 / 360 等国产安全软件同样拦截。**所以不是误报，任何加白名单的做法都是把后门留在系统里。**

## 解决思路

LibreHardwareMonitor 上游已经把 WinRing0 **完全移除**，改用 **PawnIO**
（签名驱动、兼容 HVCI / 内存完整性、不在任何黑名单里）。PawnIO 的设计是「只提供受限的、白名单化的
寄存器读写模块」，每个模块（IntelMSR / RyzenSMU / LpcIO …）以 `.bin` 形式由 LHM 在运行时加载，
不再是「任意内核读写」。这也是目前 FanControl、HWiNFO 等同类软件的共同迁移方向。

本目录就是把**新版 LibreHardwareMonitor**（0.9.6，net472，x64）连同它需要的依赖打成一个可部署包，
替换掉旧的 `LibreHardwareMonitorLib.dll`，从而：

- 不再释放任何自己的 `.sys` 文件（WinRing0 已被彻底删除，字符串层面为 0 命中）；
- 不再触发 `VulnerableDriver:WinNT/Winring0` 拦截；
- **CPU 温度 / 主板温度照常可用**（前提是安装了 PawnIO）。

## 为什么不需要重新编译插件

`LibreHardwareMonitorLib` **没有强名称签名**（CLR 头 flags=ILONLY，StrongNameSignature 长度 0），
.NET Framework 对非强名称程序集只按简单名称绑定，不校验版本。所以可以直接替换 DLL，
`plugins\HardwareMonitor.dll`（C++/CLI）和 `TrafficMonitor.exe` 都无需改动、无需重编译。

已核对接口兼容性：新版 `IVisitor` 仍然只有 `VisitComputer / VisitHardware / VisitSensor / VisitParameter`
四个方法（没有新增 `VisitController`），与插件里 `UpdateVisitor` 的实现完全一致；
`Computer.IsCpuEnabled / IsGpuEnabled / IsStorageEnabled / IsMotherboardEnabled`、
`IHardware.SubHardware / Sensors`、`ISensor.SensorType / Name / Value`、`SensorType` / `HardwareType` 枚举均未变化。

## 文件清单

`files\` 下的 DLL 全部来自 NuGet 官方包，请一并复制到 **TrafficMonitor.exe 所在目录**（不是 plugins 目录）：

| 文件 | 来源包 | 包版本 | **程序集版本**（关键） |
|---|---|---|---|
| LibreHardwareMonitorLib.dll | LibreHardwareMonitorLib | 0.9.6（`runtimes/win-x64/lib/net472`） | 0.9.6.0 |
| DiskInfoToolkit.dll | DiskInfoToolkit | 1.1.2 | 1.1.2.0 |
| BlackSharp.Core.dll | BlackSharp.Core | 1.0.7 | 1.0.7.0 |
| HidSharp.dll | HidSharp | 2.6.4（netstandard2.0） | 2.6.4.0 |
| RAMSPDToolkit-NDD.dll | RAMSPDToolkit-NDD | 1.4.2 | 1.4.2.0 |
| System.Memory.dll | System.Memory | 4.6.3 | 4.0.5.0 |
| System.Buffers.dll | System.Buffers | **4.6.1** | **4.0.5.0** |
| System.Numerics.Vectors.dll | System.Numerics.Vectors | **4.6.1** | **4.1.6.0** |
| System.Runtime.CompilerServices.Unsafe.dll | System.Runtime.CompilerServices.Unsafe | **6.1.2** | **6.0.3.0** |
| System.Threading.AccessControl.dll | System.Threading.AccessControl | 10.0.3 | 10.0.0.3 |
| System.Security.AccessControl.dll | System.Security.AccessControl | 6.0.0 | 6.0.0.0 |
| System.Security.Principal.Windows.dll | System.Security.Principal.Windows | 5.0.0 | 5.0.0.0 |

> **为什么必须看「程序集版本」而不是「NuGet 包版本」**：`System.*` 这几个包是微软强名称签名的，
> CLR 绑定强名称程序集时**要求版本号逐位相等**，否则抛 `FileLoadException`
> （`0x80131040`，提示 “找到的程序集清单定义与程序集引用不匹配”）。
> 例如 LHM 0.9.6 编译时引用的是 `System.Runtime.CompilerServices.Unsafe 6.0.3.0`，
> 而 NuGet 上**没有** 6.0.3 这个包——它对应的是包 **6.1.2**。同理 `System.Buffers 4.6.1 → 4.0.5.0`、
> `System.Numerics.Vectors 4.6.1 → 4.1.6.0`。用 4.5.1 / 4.5.0 / 6.0.0 这三个常见版本会连环报错。

> **`System.Management` / `System.CodeDom` 不放进本目录**：LHM 引用的是
> `System.Management 4.0.0.0`，而 .NET Framework 4.x 的 GAC 里本来就有同名同版本（`b03f5f7f11d50a3a`），
> 直接走 GAC 即可。反过来，放 NuGet 包里的 `System.Management 10.0.2`（程序集 4.0.0.0 的 facade）
> 会额外要求 `System.CodeDom 10.0.0.0`，而 10.0.2 包里的程序集是 10.0.0.2 ——
> 又是一个 0x80131040。少放两个文件，少两个坑。

> 三方的 `LibreHardwareMonitorLib` / `DiskInfoToolkit` / `RAMSPDToolkit-NDD` / `BlackSharp.Core` / `HidSharp`
> 都**没有强名称签名**，CLR 只按简单名绑定、不校验版本，所以 `RAMSPDToolkit` 引用
> `BlackSharp.Core 1.0.4.0` 而实际放 1.0.7.0 是安全的（已实测验证）。
> 缺任何一个仍会在 `Computer.Open()` 里抛 `FileNotFoundException`，所以必须**整套**复制。

**注意架构**：`LibreHardwareMonitorLib.dll` 这里提供的是 **x64** 版本（PE Machine=0x8664）。
如果你的 TrafficMonitor 是 32 位版本，请改用 NuGet 包里 `runtimes/win-x86/lib/net472/` 下的同名文件，
其余依赖两个架构通用（ILONLY/AnyCPU）。

## 前置条件：安装 PawnIO

LHM 0.9.6 通过 `\\?\GLOBALROOT\Device\PawnIO` 设备直接与 PawnIO 驱动通信（不再需要 PawnIOLib.dll）。
未安装 PawnIO 时**不会崩溃、不会被拦截**，只是 CPU 温度 / 主板温度读不到（`LoadModuleFromResource`
在设备打不开时返回无效句柄，LHM 静默跳过）。GPU 温度 / 利用率、硬盘信息不受影响。

安装方式任选其一：

```powershell
winget install --id namazso.PawnIO --exact --accept-package-agreements --accept-source-agreements
```

或从 LibreHardwareMonitor 官方仓库获取 `PawnIO_setup.exe`：
`https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/blob/master/LibreHardwareMonitor.Windows.Forms/Resources/PawnIO_setup.exe`

安装后确认：`Get-Service PawnIO` 显示 Status = Running，注册表
`HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\PawnIO` 的 DisplayVersion 存在（本机实测 2.2.0.0）。

> TrafficMonitor 需要**以管理员身份运行**才能打开 PawnIO 设备读取 CPU 温度（非管理员下设备访问被拒绝），
> 这一点和原来用 WinRing0 时一样。

## 部署

```powershell
# 以你自己的 TrafficMonitor 安装目录为参数；脚本会先备份旧 DLL
powershell -ExecutionPolicy Bypass -File .\deploy.ps1 -Target "D:\TrafficMonitor"
```

部署脚本会：
1. 把现有的 `LibreHardwareMonitorLib.dll` 备份到目标目录下 `backup-winring0\`；
2. 复制 `files\*.dll`（12 个）到目标目录；
3. 删除可能残留的 `System.Management.dll` / `System.CodeDom.dll`（改用 GAC 版本）；
4. **逐个校验程序集版本号**，任何一项不符会给出 WARN（这是最容易踩的坑）；
5. 检查 PawnIO 是否安装并给出提示。

**部署前必须先退出 TrafficMonitor**（托盘图标右键 → 退出），否则 DLL 被占用无法覆盖。

## 验证

1. 启动 TrafficMonitor（以管理员身份），观察程序目录：**不应再出现任何 `.sys` 文件**；
2. 安全软件不再弹出「驱动存在漏洞 / 已阻止」；
3. 任务栏或主窗口的 **CPU 温度** 有正常读数（不是 `--`）；
4. 显卡温度、硬盘温度、CPU 利用率、网速照常。

也可以直接对本目录的文件做静态检查：

```powershell
# 期望输出：0（表示已不含 WinRing0）
(Select-String -Path .\files\LibreHardwareMonitorLib.dll -Pattern 'WinRing0' -AllMatches).Matches.Count
```

## 回滚

把 `备份目录\LibreHardwareMonitorLib.dll` 复制回目标目录，并删除本包新增的那 12 个依赖 DLL 即可
（旧版本只依赖 `LibreHardwareMonitorLib.dll` 和 `libMonoPosixHelper.dll`）。

注意回滚等于恢复被拦截的 WinRing0，安全软件会重新报警。

## 已知限制

- **x86 构建**需要换成 `win-x86` 版的 LibreHardwareMonitorLib.dll（见上文）；
- PawnIO 需要单独安装，未安装时温度类指标不可用（但没有安全风险，也不会被拦截）；
- 如果安全软件仍然拦截 `PawnIO.sys`（个别激进策略），那就只能退回到「不做 CPU 温度」的方案；
- 新版 LHM 的传感器命名可能略有差异（例如多了 `CPU Package`、`Core Max` 等），
  插件里按名称匹配的项目（`Core Average`、`GPU Core`、`Total Activity`）在 0.9.6 中仍然存在，
  但个别平台可能显示为其它名字。

## 校验工具

`verify_closure.ps1` 用来在部署前后确认依赖版本没有对错：

```powershell
powershell -ExecutionPolicy Bypass -File .\verify_closure.ps1 -Dir "D:\TrafficMonitor"
# 期望最后一行：结论: 闭包完整，无版本冲突。
```

它只用 `ReflectionOnlyLoadFrom` + `GetReferencedAssemblies()` 读取元数据（**不执行任何被检查程序集的代码**），
把「引用版本 / 实际部署版本 / GAC 版本」三方对齐后报告。改动依赖列表后请务必跑一次。

## 第三方组件与许可

`files\` 下的文件均为官方 NuGet 包的原始二进制（未修改），来源与许可：

| 组件 | 项目 / 许可 |
|---|---|
| LibreHardwareMonitorLib | https://github.com/LibreHardwareMonitor/LibreHardwareMonitor — MPL-2.0 |
| DiskInfoToolkit / RAMSPDToolkit-NDD / BlackSharp.Core | 作者 Blacktempel 的 NuGet 包 — MIT |
| HidSharp | https://github.com/IntergatedCircuits/HidSharp — Apache-2.0 |
| System.* | Microsoft .NET 运行时库 — MIT |

各包版权归原作者所有，此处仅作运行时依赖分发。
