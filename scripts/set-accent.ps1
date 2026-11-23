<#
.SYNOPSIS
  Sets the Windows accent colour (default Gridline cyan #00E5FF). The first run saves your
  previous accent; -Restore puts it back.
#>
param([string]$Color = '#00E5FF', [switch]$Restore, [switch]$NoRestart)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\common.ps1"
$accentKey = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\Accent'
$dwmKey = 'HKCU:\Software\Microsoft\Windows\DWM'
$backup = Join-Path $BackupDir 'accent-before.json'
$names = @{ $accentKey = 'AccentPalette', 'AccentColorMenu', 'StartColorMenu'; $dwmKey = 'AccentColor', 'ColorizationColor', 'ColorizationAfterglow' }

if ($Restore) {
    if (-not (Test-Path $backup)) { Write-Host 'No accent backup found; nothing to restore.'; return }
    $old = Get-Content $backup -Raw | ConvertFrom-Json
    foreach ($e in $old) {
        if ($e.Kind -eq 'Binary') { Set-ItemProperty $e.Key -Name $e.Name -Value ([byte[]]$e.Value) -Type Binary }
        else { Set-ItemProperty $e.Key -Name $e.Name -Value ([int]$e.Value) -Type DWord }
    }
} else {
    if (-not (Test-Path $backup)) {
        $save = foreach ($k in $names.Keys) { foreach ($n in $names[$k]) {
            $v = (Get-ItemProperty $k -ErrorAction SilentlyContinue).$n
            if ($null -ne $v) { [pscustomobject]@{ Key = $k; Name = $n; Kind = $(if ($v -is [byte[]]) { 'Binary' } else { 'DWord' }); Value = $v } } } }
        ConvertTo-Json @($save) -Depth 4 | Set-Content $backup
    }
    $hex = $Color.TrimStart('#')
    if ($hex -notmatch '^[0-9A-Fa-f]{6}$') { throw 'Color must look like #00E5FF' }
    $r = [Convert]::ToInt32($hex.Substring(0, 2), 16); $g = [Convert]::ToInt32($hex.Substring(2, 2), 16); $b = [Convert]::ToInt32($hex.Substring(4, 2), 16)
    function Mix([int]$c, [double]$t) { if ($t -ge 0) { [byte][Math]::Round($c + (255 - $c) * $t) } else { [byte][Math]::Round($c * (1 + $t)) } }
    $steps = 0.75, 0.5, 0.25, 0, -0.25, -0.5, -0.75, -0.85
    $palette = New-Object byte[] 32
    for ($i = 0; $i -lt 8; $i++) {
        $palette[$i * 4] = Mix $r $steps[$i]; $palette[$i * 4 + 1] = Mix $g $steps[$i]
        $palette[$i * 4 + 2] = Mix $b $steps[$i]; $palette[$i * 4 + 3] = 0xFF
    }
    function Dword([int[]]$bytes) { [BitConverter]::ToInt32([byte[]]$bytes, 0) }
    if (-not (Test-Path $accentKey)) { New-Item $accentKey -Force | Out-Null }
    Set-ItemProperty $accentKey -Name AccentPalette -Value $palette -Type Binary
    Set-ItemProperty $accentKey -Name AccentColorMenu -Value (Dword @($r, $g, $b, 0xFF)) -Type DWord
    Set-ItemProperty $accentKey -Name StartColorMenu -Value (Dword @($palette[16], $palette[17], $palette[18], 0xFF)) -Type DWord
    Set-ItemProperty $dwmKey -Name AccentColor -Value (Dword @($r, $g, $b, 0xFF)) -Type DWord
    Set-ItemProperty $dwmKey -Name ColorizationColor -Value (Dword @($b, $g, $r, 0xC4)) -Type DWord
    Set-ItemProperty $dwmKey -Name ColorizationAfterglow -Value (Dword @($b, $g, $r, 0xC4)) -Type DWord
    Set-ItemProperty 'HKCU:\Control Panel\Desktop' -Name AutoColorization -Value 0 -Type DWord
}
if (-not $NoRestart) { Restart-Shell @('explorer') }
Write-Host $(if ($Restore) { 'Previous accent colour restored.' } else { "Accent set to #$($hex.ToUpper())." })
