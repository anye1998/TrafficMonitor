#Requires -Version 5.1
<#
.SYNOPSIS
    Deploy the PawnIO-based LibreHardwareMonitor runtime into a TrafficMonitor installation.

.DESCRIPTION
    Replaces the legacy LibreHardwareMonitorLib.dll (which embedded the vulnerable WinRing0 1.2.0.5
    kernel driver, CVE-2020-14979) with LibreHardwareMonitor 0.9.6 (net472, PawnIO backend),
    and copies its runtime dependencies next to TrafficMonitor.exe.

.PARAMETER Target
    TrafficMonitor installation directory (the folder containing TrafficMonitor.exe).

.PARAMETER Rollback
    Restore the backup created by a previous deployment.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\deploy.ps1 -Target "D:\TrafficMonitor"

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\deploy.ps1 -Target "D:\TrafficMonitor" -Rollback
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)]
    [string]$Target,
    [switch]$Rollback
)

$ErrorActionPreference = 'Stop'
$here = Split-Path -Parent $MyInvocation.MyCommand.Path
$filesDir = Join-Path $here 'files'
$backupDir = Join-Path $Target 'backup-winring0'

function Write-Step($msg) { Write-Host "==> $msg" -ForegroundColor Cyan }
function Write-Ok($msg) { Write-Host "    OK   $msg" -ForegroundColor Green }
function Write-Warn($msg) { Write-Host "    WARN $msg" -ForegroundColor Yellow }

# --- sanity checks -----------------------------------------------------------
if (-not (Test-Path (Join-Path $Target 'TrafficMonitor.exe'))) {
    throw "TrafficMonitor.exe not found in '$Target'. Pass the installation directory via -Target."
}
if (-not $Rollback -and -not (Test-Path $filesDir)) {
    throw "Payload folder '$filesDir' is missing."
}

# --- make sure the app is not running (files would be locked) ----------------
$running = Get-Process -Name 'TrafficMonitor' -ErrorAction SilentlyContinue
if ($running) {
    $ids = ($running | Select-Object -ExpandProperty Id) -join ', '
    throw "TrafficMonitor is running (PID: $ids). Exit it from the tray icon menu first, then re-run this script."
}

# --- rollback ----------------------------------------------------------------
if ($Rollback) {
    Write-Step 'Rolling back to the legacy LibreHardwareMonitorLib.dll'
    $bak = Join-Path $backupDir 'LibreHardwareMonitorLib.dll'
    if (-not (Test-Path $bak)) { throw "No backup found at '$bak'." }
    Copy-Item $bak (Join-Path $Target 'LibreHardwareMonitorLib.dll') -Force
    Write-Ok 'legacy DLL restored'
    Write-Warn 'WinRing0 is back - security software will flag it again. Remove the extra dependency DLLs manually if needed.'
    return
}

# --- backup ------------------------------------------------------------------
Write-Step "Backing up to $backupDir"
New-Item -ItemType Directory -Path $backupDir -Force | Out-Null
$oldDll = Join-Path $Target 'LibreHardwareMonitorLib.dll'
if (Test-Path $oldDll) {
    Copy-Item $oldDll (Join-Path $backupDir 'LibreHardwareMonitorLib.dll') -Force
    $h = (Get-FileHash $oldDll -Algorithm SHA1).Hash
    Write-Ok "LibreHardwareMonitorLib.dll (SHA1 $h)"
    if ($h -eq 'A989AD6F4D9EACDC30B31344059A57735E3C5377') {
        Write-Warn 'this is the legacy build that embeds WinRing0 1.2.0.5'
    }
} else {
    Write-Warn 'no existing LibreHardwareMonitorLib.dll found, nothing to back up'
}

# --- deploy ------------------------------------------------------------------
# 期望的程序集版本（强名称程序集必须逐位相等，否则 0x80131040 绑定失败）
$expected = @{
    'LibreHardwareMonitorLib'                 = '0.9.6.0'
    'DiskInfoToolkit'                         = '1.1.2.0'
    'BlackSharp.Core'                         = '1.0.7.0'
    'HidSharp'                                = '2.6.4.0'
    'RAMSPDToolkit-NDD'                       = '1.4.2.0'
    'System.Memory'                           = '4.0.5.0'
    'System.Buffers'                          = '4.0.5.0'
    'System.Numerics.Vectors'                 = '4.1.6.0'
    'System.Runtime.CompilerServices.Unsafe'  = '6.0.3.0'
    'System.Threading.AccessControl'          = '10.0.0.3'
    'System.Security.AccessControl'           = '6.0.0.0'
    'System.Security.Principal.Windows'       = '5.0.0.0'
}

Write-Step 'Copying PawnIO-based runtime'
Get-ChildItem (Join-Path $filesDir '*.dll') | ForEach-Object {
    Copy-Item $_.FullName (Join-Path $Target $_.Name) -Force
    Write-Ok $_.Name
}

# System.Management / System.CodeDom 交给 .NET Framework 的 GAC 提供，残留的 NuGet 版本会引发版本冲突
Write-Step 'Removing conflicting leftovers'
foreach ($stale in @('System.Management.dll', 'System.CodeDom.dll')) {
    $p = Join-Path $Target $stale
    if (Test-Path $p) {
        Remove-Item $p -Force
        Write-Ok "removed $stale (use the GAC copy instead)"
    }
}

# --- assembly version validation ---------------------------------------------
Write-Step 'Validating assembly versions'
$bad = 0
foreach ($kv in $expected.GetEnumerator()) {
    $name = "$($kv.Key).dll"
    $p = Join-Path $Target $name
    if (-not (Test-Path $p)) { Write-Warn "$name missing"; $bad++; continue }
    try {
        $actual = [System.Reflection.AssemblyName]::GetAssemblyName($p).Version.ToString()
        if ($actual -eq $kv.Value) {
            Write-Ok ("{0,-42} {1}" -f $name, $actual)
        } else {
            Write-Warn ("{0,-42} expected {1}, found {2}" -f $name, $kv.Value, $actual)
            $bad++
        }
    } catch {
        Write-Warn "$name unreadable: $($_.Exception.Message)"
        $bad++
    }
}
if ($bad -eq 0) { Write-Ok 'all assembly versions match' }
else { Write-Warn "$bad assembly version problem(s) - the plugin will fail to load (0x80131040)" }

# --- verify payload ----------------------------------------------------------
Write-Step 'Verifying payload does not contain WinRing0'
$lhm = Join-Path $Target 'LibreHardwareMonitorLib.dll'
$bytes = [System.IO.File]::ReadAllBytes($lhm)
$needle = [System.Text.Encoding]::ASCII.GetBytes('WinRing0')
$found = $false
for ($i = 0; $i -le $bytes.Length - $needle.Length; $i++) {
    if ($bytes[$i] -eq $needle[0]) {
        $match = $true
        for ($j = 1; $j -lt $needle.Length; $j++) {
            if ($bytes[$i + $j] -ne $needle[$j]) { $match = $false; break }
        }
        if ($match) { $found = $true; break }
    }
}
if ($found) {
    Write-Warn 'WinRing0 still referenced in LibreHardwareMonitorLib.dll - wrong payload?'
} else {
    Write-Ok 'no WinRing0 reference (0 hits)'
}

# --- PawnIO check ------------------------------------------------------------
Write-Step 'Checking PawnIO driver'
$pawnKey = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\PawnIO'
$svc = Get-Service -Name 'PawnIO' -ErrorAction SilentlyContinue
if ($svc) {
    Write-Ok "service PawnIO: $($svc.Status)"
} else {
    Write-Warn 'service PawnIO not found'
}
if (Test-Path $pawnKey) {
    $ver = (Get-ItemProperty $pawnKey).DisplayVersion
    Write-Ok "installed version: $ver"
} else {
    Write-Warn 'PawnIO is NOT installed - CPU/mainboard temperature will be unavailable (no crash, no block).'
    Write-Warn 'Install it with: winget install --id namazso.PawnIO --exact'
}

Write-Host ''
Write-Host 'Done. Start TrafficMonitor (as administrator) and check:' -ForegroundColor Cyan
Write-Host '  1) no *.sys file appears in the installation folder'
Write-Host '  2) security software no longer blocks a driver'
Write-Host '  3) CPU temperature shows a real value'
