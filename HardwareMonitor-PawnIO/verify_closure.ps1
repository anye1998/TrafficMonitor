param(
    [string]$Dir = 'D:\工作\WorkBuddy\2026-10-25-20-08-14\stage2'
)
# 只做元数据级校验：不执行任何被检查程序集的代码。
$ErrorActionPreference = 'Stop'

$gacRoots = @(
    'C:\Windows\Microsoft.NET\assembly\GAC_MSIL',
    'C:\Windows\Microsoft.NET\assembly\GAC_32',
    'C:\Windows\Microsoft.NET\assembly\GAC_64',
    'C:\Windows\assembly\GAC_MSIL',
    'C:\Windows\assembly\GAC_32',
    'C:\Windows\assembly\GAC_64'
)

function Find-InGac([string]$name, [version]$ver, [string]$token) {
    $folder = '{0}\v4.0_{1}__{2}' -f $name, $ver, $token
    foreach ($r in $gacRoots) {
        $p = Join-Path $r $folder
        if (Test-Path $p) {
            $f = Get-ChildItem $p -Filter "$name.dll" -ErrorAction SilentlyContinue | Select-Object -First 1
            if ($f) { return $f.FullName }
        }
    }
    # 再按名称模糊找（版本可能不同）
    foreach ($r in $gacRoots) {
        $p = Join-Path $r $name
        if (Test-Path $p) {
            $cand = Get-ChildItem $p -Recurse -Filter "$name.dll" -ErrorAction SilentlyContinue
            foreach ($c in $cand) { if ($c.Directory.Name -like "*$ver*") { return $c.FullName } }
            if ($cand) { return $cand[0].FullName + ' (版本不符)' }
        }
    }
    return $null
}

$assemblies = @{}
foreach ($f in Get-ChildItem $Dir -Filter *.dll) {
    try {
        $a = [System.Reflection.Assembly]::ReflectionOnlyLoadFrom($f.FullName)
        $n = $a.GetName()
        $tok = ''
        if ($n.GetPublicKeyToken()) { $tok = ($n.GetPublicKeyToken() | ForEach-Object { $_.ToString('x2') }) -join '' }
        $assemblies[$n.Name] = [pscustomobject]@{ Version = $n.Version; Token = $tok; Path = $f.FullName }
    } catch {
        Write-Output ("! 无法读取元数据: " + $f.Name)
    }
}

Write-Output "=== 目录内程序集 ($($assemblies.Count)) ==="
$assemblies.GetEnumerator() | Sort-Object Name | ForEach-Object {
    Write-Output ("  {0,-42} {1,-10} token={2}" -f $_.Key, $_.Value.Version, ($(if ($_.Value.Token) { $_.Value.Token } else { '(无)' })))
}

$frameworkNames = @('mscorlib', 'System', 'System.Core', 'System.Numerics', 'System.Xml',
                    'System.Drawing', 'System.Runtime', 'System.IO', 'System.Net.Http')

Write-Output ""
Write-Output "=== 引用解析校验 ==="
$bad = 0
foreach ($kv in ($assemblies.GetEnumerator() | Sort-Object Name)) {
    $self = $kv.Value
    $a = [System.Reflection.Assembly]::ReflectionOnlyLoadFrom($self.Path)
    foreach ($r in $a.GetReferencedAssemblies()) {
        $tok = ''
        if ($r.GetPublicKeyToken()) { $tok = ($r.GetPublicKeyToken() | ForEach-Object { $_.ToString('x2') }) -join '' }
        $ver = $r.Version

        if ($assemblies.ContainsKey($r.Name)) {
            $have = $assemblies[$r.Name]
            $strongSelf = $have.Token -ne ''
            if ($have.Version -eq $ver) {
                Write-Output ("  OK    {0,-30} -> {1,-40} {2}" -f $kv.Key, $r.Name, $ver)
            } elseif (-not $strongSelf) {
                Write-Output ("  OK*   {0,-30} -> {1,-40} 需要 {2} 实际 {3}（双方均无强名称，按简单名绑定）" -f $kv.Key, $r.Name, $ver, $have.Version)
            } else {
                Write-Output ("  BAD   {0,-30} -> {1,-40} 需要 {2} 实际 {3}" -f $kv.Key, $r.Name, $ver, $have.Version)
                $bad++
            }
            continue
        }

        if ($frameworkNames -contains $r.Name -or $r.Name -eq 'netstandard' -or $r.Name -like 'System.*') {
            $gac = Find-InGac $r.Name $ver $tok
            if ($gac) {
                Write-Output ("  GAC   {0,-30} -> {1,-40} {2}" -f $kv.Key, $r.Name, $ver)
            } else {
                Write-Output ("  MISS  {0,-30} -> {1,-40} {2}  (GAC 未找到)" -f $kv.Key, $r.Name, $ver)
                $bad++
            }
            continue
        }

        Write-Output ("  MISS  {0,-30} -> {1,-40} {2}" -f $kv.Key, $r.Name, $ver)
        $bad++
    }
}
Write-Output ""
if ($bad -eq 0) { Write-Output "结论: 闭包完整，无版本冲突。" } else { Write-Output "结论: 存在 $bad 个问题。" }
