<#
.SYNOPSIS
  Sets the Gridline Clock screensaver (light-line clock drifting above a moving grid).
  -Minutes sets the idle time (default 10). -Restore puts back the previous settings.
#>
param([int]$Minutes = 10, [switch]$Restore)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\common.ps1"
$key = 'HKCU:\Control Panel\Desktop'
$backup = Join-Path $BackupDir 'screensaver-before.json'
Add-Type @'
using System.Runtime.InteropServices;
public static class NdScr { [DllImport("user32.dll")] public static extern bool SystemParametersInfoW(uint a, uint p, System.IntPtr v, uint f); }
'@

if ($Restore) {
    if (-not (Test-Path $backup)) { Write-Host 'No screensaver backup found; nothing to restore.'; return }
    $old = Get-Content $backup -Raw | ConvertFrom-Json
    foreach ($n in 'SCRNSAVE.EXE', 'ScreenSaveActive', 'ScreenSaveTimeOut') {
        if ($old.$n) { Set-ItemProperty $key -Name $n -Value $old.$n } else { Remove-ItemProperty $key -Name $n -ErrorAction SilentlyContinue }
    }
    [void][NdScr]::SystemParametersInfoW(0x0011, [uint32]($old.ScreenSaveActive -eq '1'), [System.IntPtr]::Zero, 3)
    Write-Host 'Previous screensaver settings restored.'
    return
}

$dst = Join-Path $DataDir 'GridlineClock.scr'
Copy-Item (Join-Path $RepoRoot 'screensaver\GridlineClock.scr') $dst -Force
if (-not (Test-Path $backup)) {
    $cur = Get-ItemProperty $key
    [ordered]@{ 'SCRNSAVE.EXE' = $cur.'SCRNSAVE.EXE'; ScreenSaveActive = $cur.ScreenSaveActive; ScreenSaveTimeOut = $cur.ScreenSaveTimeOut } |
        ConvertTo-Json | Set-Content $backup
}
Set-ItemProperty $key -Name 'SCRNSAVE.EXE' -Value $dst
Set-ItemProperty $key -Name 'ScreenSaveActive' -Value '1'
Set-ItemProperty $key -Name 'ScreenSaveTimeOut' -Value ([string]($Minutes * 60))
[void][NdScr]::SystemParametersInfoW(0x0011, 1, [System.IntPtr]::Zero, 3)
[void][NdScr]::SystemParametersInfoW(0x000F, [uint32]($Minutes * 60), [System.IntPtr]::Zero, 3)
Write-Host "Gridline Clock screensaver set ($Minutes min)."
