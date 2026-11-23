<#
.SYNOPSIS
  Switches the whole setup between Gridline dark and Gridline light: Windows app/system
  mode, wallpaper, and the five styler themes. The desktop widgets follow Windows.
.EXAMPLE
  .\gridline-mode.ps1              # toggle
  .\gridline-mode.ps1 -Mode light
#>
param([ValidateSet('toggle', 'dark', 'light')][string]$Mode = 'toggle', [switch]$KeepWallpaper)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\common.ps1"

$personalize = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize'
$isLight = (Get-ItemProperty $personalize -ErrorAction SilentlyContinue).AppsUseLightTheme -eq 1
$target = if ($Mode -eq 'toggle') { if ($isLight) { 'dark' } else { 'light' } } else { $Mode }

$v = [int]($target -eq 'light')
Set-ItemProperty $personalize -Name AppsUseLightTheme -Value $v -Type DWord
Set-ItemProperty $personalize -Name SystemUsesLightTheme -Value $v -Type DWord

if (-not $KeepWallpaper) {
    $file = if ($target -eq 'light') { '13-light-grid.png' } else { '01-horizon-grid.png' }
    $wall = Join-Path $DataDir "wallpaper-$target.png"
    Copy-Item (Join-Path $RepoRoot "wallpapers\$file") $wall -Force
    Add-Type @'
using System.Runtime.InteropServices;
public static class NdWall { [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern bool SystemParametersInfoW(uint a, uint p, string v, uint f); }
'@
    Set-ItemProperty 'HKCU:\Control Panel\Desktop' -Name WallpaperStyle -Value '10'
    Set-ItemProperty 'HKCU:\Control Panel\Desktop' -Name TileWallpaper -Value '0'
    [void][NdWall]::SystemParametersInfoW(0x0014, 0, $wall, 3)
}

& "$PSScriptRoot\apply-themes.ps1" -Mode $target
# The light/dark switch makes Windows reapply its theme, which resets the sound scheme.
Start-Sleep -Seconds 3
Repair-SoundScheme
Write-Host "Gridline $target."
