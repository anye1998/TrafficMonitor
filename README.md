**简体中文 | [English](./README_en-us.md)**

[![Badge](https://img.shields.io/badge/link-996.icu-%23FF4D5B.svg?style=flat-square)](https://996.icu/#/en_US)
[![LICENSE](https://img.shields.io/badge/license-Anti%20996-blue.svg?style=flat-square)](https://github.com/996icu/996.ICU/blob/master/LICENSE)
[![GitHub Workflow Status](https://img.shields.io/github/actions/workflow/status/zhongyang219/TrafficMonitor/main.yml?branch=master&label=Release%20CI&logo=github&style=flat-square)](https://github.com/zhongyang219/TrafficMonitor/actions?query=workflow:"Release+CI")
[![GitHub release](https://img.shields.io/github/release/zhongyang219/TrafficMonitor.svg?style=flat-square)](https://github.com/zhongyang219/TrafficMonitor/releases/latest)

<a href="https://hellogithub.com/repository/5ef48af2b2794d4798b17d6539ec7305" target="_blank"><img src="https://api.hellogithub.com/v1/widgets/recommend.svg?rid=5ef48af2b2794d4798b17d6539ec7305&claim_uid=CeVqou2T1dIvfQP" alt="Featured｜HelloGitHub" style="width: 250px; height: 54px;" width="250" height="54" /></a>

# TrafficMonitor 简介

Traffic Monitor是一款用于Windows平台的网速监控悬浮窗软件，可以显示当前网速、CPU及内存利用率，支持嵌入到任务栏显示，支持更换皮肤、历史流量统计等功能。

---

**本仓库是 [zhongyang219/TrafficMonitor](https://github.com/zhongyang219/TrafficMonitor) 的个人 Fork，包含 AI 辅助的本地修改。**

修复了「硬件监控驱动被安全软件拦截」的问题：`LibreHardwareMonitorLib.dll` 内嵌的 **WinRing0 1.2.0.5**（漏洞 CVE-2020-14979）已被微软列入易受攻击驱动名单，安全软件会在程序启动时拦截它释放出来的 `TrafficMonitor.sys`。现已替换为使用 **PawnIO** 后端的 LibreHardwareMonitor 0.9.6，并加固了后端不可用时的异常处理。

- 改动原因与明细：[本 Fork 修改记录](#本-fork-修改记录)
- 新增功能：右键菜单在“帮助”与“退出”之间多了**重启**，一键重启程序，方便调试插件（[详见](#2026-11-01--右键菜单新增重启)）
- 部署包：[`HardwareMonitor-PawnIO/`](./HardwareMonitor-PawnIO)（替换文件 + `deploy.ps1` + 版本校验脚本）
- 下载：[Releases](https://github.com/anye1998/TrafficMonitor/releases) —— 提供 GitHub Actions 编译的
  **标准版**（含内置硬件监控，需管理员）与 **Lite 版**（无内置监控、不弹 UAC，适合只用插件的人）两个 x64 包，
  以及仅替换 DLL 的部署包

> 本次修改由 **AI 助手（WorkBuddy）** 分析、实现并提交，人工仅负责验证运行结果。本仓库与上游项目无关，相关问题请提到本仓库的 Issues，不要打扰原作者。

---

# 相关链接：

请[点击此处](https://github.com/zhongyang219/TrafficMonitor/releases/latest)下载TrafficMonitor的最新版本。

备用链接：[百度网盘下载](https://pan.baidu.com/s/15PMt7s-ASpyDwtS__4cUhg) 提取码：`ou0m`

国内用户如果遇到Github下载缓慢的问题，可以[点击此处](https://gitee.com/zhongyang219/TrafficMonitor)转到此项目在Gitee上的页面。

如果遇到问题，请[点击此处](./Help.md)查看常见问题。

你也可以[点击此处](https://github.com/zhongyang219/TrafficMonitor/actions?query=workflow:"Release+CI")下载TrafficMonitor的预发行构建版本。

从1.80版本开始，TrafficMonitor加入了温度监控功能，如果你不需要温度监控功能，并且在使用1.80以上版本中遇到了问题，建议下载不含温度监控的版本（Lite版本）。（在Release页面找到文件名包含`Lite`的版本。）

TrafficMonitor依赖于Microsoft Visual C++ 运行环境，如果程序启动时提示“找不到MSVC*.dll”，请点击以下链接下载并安装Microsoft Visual C++ 运行环境。

[最新支持的 Visual C++ 可再发行程序包下载 | Microsoft Docs](https://docs.microsoft.com/zh-CN/cpp/windows/latest-supported-vc-redist?view=msvc-170)

# 版本说明

TrafficMonitor提供了标准版和Lite版两种版本可用。标准版包含了所有的功能，Lite版本则不包含温度监控、显卡利用率、硬盘利用率等硬件监控功能。标准版运行需要管理员权限，而Lite版本则不需要。

如果没有监控温度等硬件信息的需要，建议使用Lite版。

以下是两个版本功能对比。

| 功能                          | 标准版 | Lite版 |
| ----------------------------- | ------ | ------ |
| 网速监控                      | ✔      | ✔      |
| CPU、内存利用率          | ✔      | ✔      |
| CPU、显卡、硬盘、主板温度监控  | ✔      | ❌      |
| CPU频率监控 | ✔ | ✔ |
| 显卡利用率监控                | ✔      | ✔     |
| 硬盘利用率监控                | ✔      | ✔     |
| 网络详细信息                  | ✔      | ✔      |
| 插件系统                      | ✔      | ✔      |
| 主窗口更换皮肤                | ✔      | ✔      |
| 需要管理员权限                | 是     | 否     |

注：从1.86版本开始，TrafficMonitor Lite版也提供了显卡和硬盘利用率监控功能，相比标准版仅少了温度监控功能。后续TrafficMonitor中的温度监控功能将不再维护，TrafficMonitor将不再提供标准版，仅提供Lite版。温度监控功能已经迁移到了[硬件监控插件](https://github.com/zhongyang219/TrafficMonitorPlugins/blob/main/download/plugin_download.md#%E7%A1%AC%E4%BB%B6%E7%9B%91%E6%8E%A7%E6%8F%92%E4%BB%B6)，如果需要使用温度监控功能，请下载硬件监控插件。

# 主要特性

* 显示当前实现网络传输速率、CPU和内存占用率
* 如果电脑有多个网卡，支持自动和手动选择网络连接
* 查看网络详细信息
* 支持嵌入到任务栏显示
* 支持更换皮肤和自定义皮肤
* 历史流量统计
* 硬件信息监控
* 插件系统
# 使用说明

**[点击这里](https://github.com/zhongyang219/TrafficMonitor/wiki)转到Wiki页面查看关于TrafficMonitor的详细说明文档。**

# 截图

主悬浮窗：
![](./Screenshots/main1.png)

右键菜单：
![](./Screenshots/main.png)

任务栏窗口：

![](./Screenshots/taskbar.PNG)

多彩皮肤：
<img src="./Screenshots/skins.PNG" style="zoom:80%;" />

# 如何使用
程序启动后在会在屏幕中显示一个显示网速的悬浮窗。在悬浮窗上点击鼠标右键可以弹出右键菜单。

TrafficMonitor支持将信息显示到任务栏。但是TrafficMonitor默认只显示主窗口（悬浮窗），如果需要让它嵌入到任务栏显示，请在右键菜单中选择“显示任务栏窗口”命令。

任务栏窗口支持自定义显示项目，默认情况下只显示网速，如果需要显示CPU和内存利用率等其他信息，请在任务栏窗口右键菜单中选择“显示设置”，在弹出的“显示设置”对话框中勾选需要显示的项目，如下图所示：

<img src="./Screenshots/taskbar_item_settings.png" style="zoom:80%;" />

# 自定义皮肤
<img src="./Screenshots/selecte_skin.png" style="zoom:80%;" />

在主窗口或通知区图标右键菜单上选择“其他功能”——“更换皮肤”可以打开更换皮肤界面。[点击此处](https://github.com/zhongyang219/TrafficMonitorSkin/blob/master/皮肤下载.md)可以下载更多皮肤。用户还可以根据自己的需要编辑自己的皮肤。

皮肤文件放在程序所在目录的`skins`目录下，每个皮肤被放到单独的文件夹下，文件夹的名称就是皮肤的名称。

其中`background.bmp`和`background_l.bmp`是背景图片，`skin.ini`是皮肤的配置文件，可以通过`skin.ini`指定文本颜色、字体、皮肤作者、每个项目的大小和位置等信息。

从1.80版本开始增加了xml格式的皮肤配置文件`skin.xml`，只有xml格式的皮肤配置文件才支持温度和显卡使用率显示。

从1.85版本开始增加了对png格式背景图片的支持，你可以使用png格式来制作带透明背景的皮肤，背景图片的文件名为`background.png`和`background_l.png`。

详细的皮肤制作教程请点击以下链接：

[皮肤制作教程 · zhongyang219/TrafficMonitor Wiki (github.com)](https://github.com/zhongyang219/TrafficMonitor/wiki/皮肤制作教程)

# 选项设置
<img src="./Screenshots/option.jpg" style="zoom:80%;" />

在右键菜单选择“选项...”可以进入选项设置。在选项设置对话框中，可以单独设置主窗口和任务栏窗口的文本颜色、字体、背景颜色、网速单位、显示的文本等。

在“常规设置”选项卡中，可以设置是否在程序时自动检查更新，以及是否需要在开机是自动运行。可以设置在什么时候需要发出消息通知。

从1.72版本开始，支持每个项目文本颜色单独设置。勾选“指定每个项目的颜色”后，点击“文本颜色”右边的颜色框，会弹出详细颜色设置的对话框，可以在这里单独指定每个项目的颜色。

# 插件系统

从1.82版本开始增加了插件系统，插件dll必须放在“TrafficMonitor.exe”同级目录的“plugins”目录下。程序启动后，插件会自动加载。你可以在右键菜单“更多功能”——“插件管理”中查看并管理已加载的插件。

关于如何开发TrafficMonitor的说明，请参见[插件开发指南 · zhongyang219/TrafficMonitor Wiki (github.com)](https://github.com/zhongyang219/TrafficMonitor/wiki/插件开发指南)。

要下载TrafficMonitor插件，请[点击这里](https://github.com/zhongyang219/TrafficMonitorPlugins/blob/main/download/plugin_download.md)。

# 关于硬件监控功能

从1.80版本开始，TrafficMonitor加入了硬件监控功能（包括温度监控和显卡使用率监控、CPU频率监控），它使用了第三方开源库[LibreHardwareMonitor](https://github.com/LibreHardwareMonitor/LibreHardwareMonitor)。如果你在使用温度监控功能时遇到了问题，请[点击这里](./Help.md#13-关于trafficmonitor温度监控的问题)。

需要注意的是，温度监控功能默认是关闭的，如果你要使用TrafficMonitor的温度监控功能，请到[“选项设置”-“常规设置”-“硬件监控”](https://github.com/zhongyang219/TrafficMonitor/wiki/选项设置#硬件监控)中开启。

**注意：硬件监控功能（包括温度监控和显卡使用率监控）可能存在一些问题，它可能会占用更多的CPU和内存。据部分用户反馈，开启温度功能后会导致程序崩溃和系统死机等问题，请在知晓以上风险后再决定开启硬件监控功能。否则，请不要使用硬件监控功能。**

# 本 Fork 修改记录

| 日期 | 版本标记 | 修改内容 | 实施方 |
|---|---|---|---|
| 2026-10-25 | `V1.86-pawnio` | 硬件监控驱动从 WinRing0 迁移到 PawnIO，修复被安全软件拦截 + 加固异常处理 | AI 助手（WorkBuddy） |
| 2026-10-25 | `V1.86-pawnio` | 新增标准版（含硬件监控）编译流水线，解决上游 CI 只编 Lite 版、拿不到带温度的构建产物 | AI 助手（WorkBuddy） |
| 2026-10-25 | `V1.86-pawnio` | 增加 Lite 版编译任务：无内置硬件监控、清单 `asInvoker`（不弹 UAC），硬件数据交给 `HardwareMonitor` 插件 | AI 助手（WorkBuddy） |
| 2026-11-01 | `V1.86-pawnio-r2` | 右键菜单在“帮助”与“退出”之间新增“重启”，一键重启程序，方便调试插件（含配套新图标与 18 种语言文本） | AI 助手（WorkBuddy） |

## 2026-10-25 · 硬件监控驱动迁移到 PawnIO

**性质**：本 Fork 的本地修改，**由 AI 助手（WorkBuddy）完成分析与实现**，未提交到上游项目。

### 1. 问题

安全软件（微软 Defender / 火绒 / 360 等）拦截由本程序释放的内核驱动：

```
驱动文件路径：D:\TrafficMonitor\TrafficMonitor.sys
SHA1: D25340AE8E92A6D29F599FEF426A2BC1B5217299
操作结果：已阻止
释放进程ID：9224
释放进程：D:\TrafficMonitor\TrafficMonitor.exe
```

表现为：开启「CPU 温度 / 主板温度」后安全软件报警，或插件初始化失败、温度全部显示 `--`。

### 2. 根因

- 该 `.sys` **不在本仓库源码里**。它是旧版 `LibreHardwareMonitorLib.dll` 内嵌的 `WinRing0x64.gz`
  在 `Computer.Open()` 时解压释放到程序目录，文件名取 `<进程名>.sys`，因此显示为 `TrafficMonitor.exe` 释放了 `TrafficMonitor.sys`。
  解出该资源的 SHA1 与安全软件日志**逐字节一致**，版本为 **WinRing0 1.2.0.5**（设备 `\Device\WinRing0_1_2_0`）。
- 该驱动存在公开漏洞 **CVE-2020-14979**（CVSS 7.8）：设备对象对任意本地用户开放 IOCTL，
  可读写 MSR、I/O 端口、PCI 配置空间、映射物理内存 → 低权限进程可提权到 SYSTEM。
  微软 Defender 的检测名为 `VulnerableDriver:WinNT/Winring0`。
- 驱动是**预编译二进制**，本仓库没有它的源码；即便自行硬化也没有 EV 签名，Windows 不会加载。
  **所以改本项目的 C++ 代码无法修掉这个漏洞，只能换成不含该驱动的后端。**
  （把驱动加入安全软件白名单等于主动保留提权漏洞，不可取。）

### 3. 改动

| 文件 | 改动 |
|---|---|
| `OpenHardwareMonitorApi/LibreHardwareMonitorLib.dll` | 替换为 LibreHardwareMonitor **0.9.6**（`runtimes/win-x64/lib/net472`）。上游已彻底移除 WinRing0，改用 **PawnIO**（签名驱动、兼容 HVCI / 内存完整性）。二进制内 `WinRing0` 字符串命中 **0**，`PawnIO` 命中 4。 |
| `OpenHardwareMonitorApi/LibreHardwareMonitorLib.xml` | 同步为 0.9.6 的文档。 |
| `OpenHardwareMonitorApi/OpenHardwareMonitorImp.cpp` | `SetCpuEnable` / `SetGpuEnable` / `SetHddEnable` / `SetMainboardEnable` 与 `MonitorGlobal::UnInit()` 共 5 处增加托管异常捕获。原来这 5 处没有任何保护，内核后端不可用时会抛出托管异常，**直接把主程序带崩**；现在记录到 `error_message` 后降级处理。 |
| `HardwareMonitor-PawnIO/`（新增） | 可复用的部署包：12 个依赖 DLL + `deploy.ps1`（备份/回滚/版本校验）+ `verify_closure.ps1`（只读元数据校验依赖闭包）+ README。 |
| `.github/workflows/build-full-pawnio.yml`（新增） | 编译流水线，产出两个变体：`build_x64` 编 `TrafficMonitor.sln` 的 `Release\|x64`（标准版），`build_x64_lite` 编 `TrafficMonitor_Lite.sln` 的 `Release (lite)\|x64`（Lite 版）。上传前校验：无 WinRing0 / PawnIO 存在 / 依赖程序集版本逐位匹配 / 标准版必须导入 `OpenHardwareMonitorApi.dll`、Lite 版必须不导入且清单为 `asInvoker`。 |

> **依赖版本的坑（重要）**：`LibreHardwareMonitorLib` 本身没有强名称签名，可以直接替换；但它依赖的
> `System.*` 是微软签名的程序集，CLR 要求**程序集版本逐位相等**，而 NuGet 包版本 ≠ 程序集版本。
> 例如 LHM 需要的 `System.Runtime.CompilerServices.Unsafe 6.0.3.0` 对应的是包 **6.1.2**（不是 6.0.0），
> `System.Buffers 4.0.5.0` 对应包 **4.6.1**，`System.Numerics.Vectors 4.1.6.0` 对应包 **4.6.1**。
> 用常见版本会连环报 `0x80131040`（找到的程序集清单定义与程序集引用不匹配）。完整对照表见
> [`HardwareMonitor-PawnIO/README.md`](./HardwareMonitor-PawnIO/README.md)。

### 4. 使用

两种方式，任选其一。

**方式 A：直接用 Release 里编译好的包（x64）**

Release 里有两个变体，**用哪个取决于你要不要程序自带的硬件监控**：

| | `TrafficMonitor_V1.86-pawnio-r2_x64.zip`<br>（标准版） | `TrafficMonitor_V1.86-pawnio-r2_x64_Lite.zip`<br>（Lite 版） |
|---|---|---|
| 内置硬件监控 | **有**（编译了 `OpenHardwareMonitorApi`，直接调 `LibreHardwareMonitorLib`） | **无**（`TrafficMonitor_Lite.sln` 构建，不导入 `OpenHardwareMonitorApi.dll`） |
| 清单权限 | `requireAdministrator` → **每次启动弹 UAC** | `asInvoker` → **不弹 UAC** |
| CPU 温度 / 功耗来源 | 程序内置 | 只能靠插件 `plugins\HardwareMonitor.dll` |
| 适合谁 | 只要一个 exe、不装插件的干净用法 | 已经在用 `HardwareMonitor` 插件看负载/功耗的人（也就是本机现在的配置） |

> 两个包都已经换掉了 WinRing0，`WinRing0` 字符串命中 0、都不再释放 `.sys`。
> **插件 `HardwareMonitor.dll` 不依赖程序内置监控**，所以 Lite 版照样能看到 CPU 温度、功耗、显卡负载——本机实测就是 Lite 版 + 插件。
>
> 文件名中的版本标记对应 Release：`V1.86-pawnio-r2` 为当前版本（含“重启”菜单项）；
> 更早的 `V1.86-pawnio` 内容相同，只是没有“重启”。

1. 装 PawnIO（CPU / 主板温度需要）：

   ```powershell
   winget install --id namazso.PawnIO --exact --accept-package-agreements --accept-source-agreements
   ```

2. 从 [Releases](https://github.com/anye1998/TrafficMonitor/releases) 下载对应 zip，解压到一个**新目录**
   （不要直接覆盖旧安装）。
3. 需要保留设置/皮肤的话，把旧目录的 `config.ini`、`global_cfg.ini`、`skins\`、`plugins\` 拷过来。
4. 运行 `TrafficMonitor.exe`：标准版会弹 UAC（需要管理员权限打开 PawnIO 设备）；
   Lite 版不弹 UAC。然后在显示设置里勾选 CPU 温度等硬件监控项。

**方式 B：在已安装的 TrafficMonitor 上替换文件**

1. 装 PawnIO（同上）。
2. 从托盘图标右键**退出** TrafficMonitor（DLL 被占用时无法覆盖）。
3. 解压 `HardwareMonitor-PawnIO-0.9.6.zip`，以管理员身份运行部署脚本：

   ```powershell
   powershell -ExecutionPolicy Bypass -File .\HardwareMonitor-PawnIO\deploy.ps1 -Target "D:\TrafficMonitor"
   ```

4. 以管理员身份启动 TrafficMonitor，确认 CPU 温度有读数、程序目录**不再出现 `.sys` 文件**。

### 5. 验证

```powershell
powershell -ExecutionPolicy Bypass -File .\HardwareMonitor-PawnIO\verify_closure.ps1 -Dir "D:\TrafficMonitor"
# 期望最后一行：结论: 闭包完整，无版本冲突。
```

本机实测结果（Lite 版 + `HardwareMonitor` 插件）：`D:\TrafficMonitor` 下 **0 个 `.sys`**；安全软件不再拦截；
CPU 温度、功耗、显卡温度/负载、硬盘温度、CPU 占用、网速均正常显示。

### 6. 已知限制

- **Release 里两个 zip 都由 GitHub Actions 编译**，不是本地构建：工作流
  `Build (full, PawnIO)` 使用 `windows-2022` + VS2022（v143）+ MFC + C++/CLI + .NET Framework 4.7.2 目标包。
  本地实施环境没有 MSVC，无法复现该构建；如需自己编，用 VS2022 打开 `TrafficMonitor.sln`（`Release|x64`，标准版）
  或 `TrafficMonitor_Lite.sln`（`Release (lite)|x64`，Lite 版）。
- 该 exe 依赖 **VC++ 2015-2022 运行库**（`mfc140u.dll` / `MSVCP140.dll` / `VCRUNTIME140.dll`），
  缺失会提示「找不到 MSVC*.dll」。
- 未安装 PawnIO 时**不会崩溃、不会被拦截**，只是 CPU / 主板温度读不到。
- 打开 PawnIO 设备需要管理员权限：**标准版**清单为 `requireAdministrator`，启动时自己申请提权；
  **Lite 版**清单为 `asInvoker`，以普通权限运行时拿不到依赖 MSR 的温度数据（功耗/占用这类不需要 MSR 的仍可读）。
- **只提供 x64**：`LibreHardwareMonitorLib.dll` 用的是 NuGet 包的 `win-x64` 版本。
  32 位构建需改用 `runtimes/win-x86/lib/net472` 下的同名文件，并相应调整工作流。

### 7. 回滚

回滚只对**方式 B**（在已安装目录上替换 DLL）有效，因为它需要恢复被覆盖的旧文件；
用方式 A 换整个包的话，直接把旧目录换回去即可（旧目录建议先备份）。

删除 `HardwareMonitor-PawnIO` 新增的 12 个 DLL，并从备份恢复旧的 `LibreHardwareMonitorLib.dll`：

```powershell
powershell -ExecutionPolicy Bypass -File .\HardwareMonitor-PawnIO\deploy.ps1 -Target "D:\TrafficMonitor" -Rollback
```

注意回滚等于恢复被拦截的 WinRing0，安全软件会重新报警。

## 2026-11-01 · 右键菜单新增“重启”

**性质**：本 Fork 的本地功能增强，**由 AI 助手（WorkBuddy）完成实现**，未提交到上游项目。

### 1. 需求

调试插件时，每次替换插件 DLL 都得走一遍：托盘图标右键 → 退出 → 回到程序目录双击 `TrafficMonitor.exe`；
标准版每次启动还要再确认一次 UAC。改动本身只有一行代码，重启流程却要三四步，很耽误时间。

### 2. 效果

- **位置**：主窗口（悬浮窗 / 托盘图标）和任务栏窗口的右键菜单，都在“帮助”子菜单下方、“退出”上方多了**重启**一项。
- **图标**：新画的 ↻ 线描图标，与原菜单图标同一套风格——同样的 `rgb(110,110,110)` 线条颜色、约 2.5px 笔画、
  纯描边无填充，并按 `16/20/24/28/32` 五种尺寸分别渲染，保证菜单里的小图标同样清晰。
- **行为**：点一下即可完成“保存设置 → 卸载插件 → 移除通知区图标 → 进程退出 → 自动重新启动”，
  等同于手动退出再打开，但只需一次点击。
- **多语言**：中文“重启(&R)”、英文 `&Restart`，其余 16 个语言文件同步补了对应译文。

### 3. 实现要点

| 文件 | 改动 |
|---|---|
| `TrafficMonitor/TrafficMonitor.rc` | 菜单 `IDR_MENU1`（主窗口）与 `IDR_TASK_BAR_MENU`（任务栏窗口）在 `TXT_HELP_MENU` 与 `TXT_EXIT` 之间插入 `MENUITEM "TXT_RESTART", ID_RESTART`；新增 `IDI_RESTART ICON "res\menu_icon\restart.ico"` |
| `TrafficMonitor/resource.h` | 新增 `IDI_RESTART`(349)、`ID_RESTART`(33666)，并同步 `_APS_NEXT_*` |
| `TrafficMonitor/TrafficMonitor.cpp` | 消息映射新增 `ON_COMMAND(ID_RESTART, &CTrafficMonitorApp::OnRestart)`；`InitMenuResourse()` 里给两个右键菜单注册 `IDI_RESTART` 图标；新增 `OnRestart()`、`RestartApplication()`；`ExitInstance()` 中按标记启动新实例 |
| `TrafficMonitor/TrafficMonitor.h` | 声明 `OnRestart()`、`RestartApplication()`，新增成员 `m_restart_after_exit`、`m_instance_mutex` |
| `TrafficMonitor/res/menu_icon/restart.ico`（新增） | 与现有菜单图标同规格的 5 个尺寸、32bpp 带 alpha |
| `TrafficMonitor/language/*.ini`（18 个） | `[menu]` 段新增 `TXT_RESTART`，菜单文本仍由原有的 `TranslateMenuItems()` 在运行时替换 |
| `TrafficMonitor/TrafficMonitor.vcxproj(.filters)` | 把新图标加入工程，便于在 VS 里看到 |

**为什么不是“`CreateProcess` 之后直接 `ExitProcess`”**，这里踩了三个坑，逐个说明：

1. **配置必须先落盘**：设置是在 `CTrafficMonitorDlg::OnClose()` 里保存的（`SaveConfig()` / `SaveGlobalConfig()` /
   历史流量完整写入）。如果提前启动新实例，新进程读到的还是旧配置。因此重启被拆成“先走完退出的关闭流程，
   再在 `ExitInstance()` 里拉起新进程”。
2. **单实例互斥体必须先释放**：程序用命名互斥体 `TrafficMonitor-1419J3XLKL1w8OZc` 保证单实例，
   句柄在 `InitInstance()` 里创建后一直存活到进程结束。若直接启动新实例，它会命中 `ERROR_ALREADY_EXISTS`，
   弹出“TrafficMonitor 已在运行”。因此在 `RestartApplication()` 里先 `CloseHandle(m_instance_mutex)` 再启动。
3. **复用退出流程而不是自己写清理**：`OnRestart()` 只是置标记 + `PostMessage(WM_CLOSE)`，
   这样插件反初始化、通知区图标移除、窗口销毁等逻辑与点“退出”完全一致，不会漏掉。

> 提权：标准版清单是 `requireAdministrator`，由它 `ShellExecute` 出来的新进程会继承同样的权限，
> 所以重启不会重复弹 UAC；Lite 版本来就是 `asInvoker`，同样沿用当前权限。

### 4. 使用

可直接用 Release 里编好的包：`TrafficMonitor_V1.86-pawnio-r2_x64.zip`（标准版）或
`TrafficMonitor_V1.86-pawnio-r2_x64_Lite.zip`（Lite 版），两者都已包含本功能。

1. 右键托盘图标（或任务栏窗口）→ **帮助 → 重启**（快捷键 `R`）。
2. 程序自动退出并重新启动，插件随之重新加载——改完插件 DLL 直接点它即可，不必手动退出。
3. 若插件目录里的 DLL 正在被占用（例如自己另开了调试器），重启后插件加载失败，`error.log` 里会有记录。

### 5. 已知限制

- 重启是**整个程序重启**，不是只重载插件，未写到磁盘的插件内部状态会丢失（这一点和手动退出再打开完全相同）。
- 重启不会带上启动参数（例如更新器传入的参数），会以最普通的方式重新启动。
- 若 `TrafficMonitor.exe` 在重启瞬间被改名/删除，新实例启动失败，只会在 `error.log` 里留一条记录，程序不会再自己回来。

# 更新日志

**[点击此处查看更新日志](./UpdateLog/update_log.md)**

# 声明

域名`www.trafficmonitor.cn`和作者本人**没有任何关系**，TrafficMonitor目前并没有官网，任何所谓的“TrafficMonitor官网”都是盗版网站，请不要访问此类网站，也不要通过此类网站下载TrafficMonitor。TrafficMonitor仅通过以下渠道向用户提供：

* [GitHub页面（github.com/zhongyang219/TrafficMonitor）](https://github.com/zhongyang219/TrafficMonitor)
* [Gitee页面（gitee.com/zhongyang219/TrafficMonitor）](https://gitee.com/zhongyang219/TrafficMonitor)
* [作者本人的百度网盘共享链接](https://pan.baidu.com/s/15PMt7s-ASpyDwtS__4cUhg) 提取码：`ou0m`

