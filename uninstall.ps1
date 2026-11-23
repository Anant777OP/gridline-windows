<#
.SYNOPSIS
  Removes Gridline OS for Windows and restores what was there before.
.DESCRIPTION
  Disables the Gridline-related Windhawk mods, then restores cursors, sounds, screensaver,
  icons and accent colour from the backups in %LOCALAPPDATA%\Gridline-Windows\backup.
  Fonts are kept unless -RemoveFonts is given. Your wallpaper and dark/light mode stay.
#>
param([switch]$RemoveFonts)
$ErrorActionPreference = 'Continue'
$s = Join-Path $PSScriptRoot 'scripts'
. "$s\common.ps1"

& "$s\gridline-power.ps1" -Off
& "$s\install-icons.ps1" -Restore
& "$s\set-accent.ps1" -Restore -NoRestart
if ($RemoveFonts) { & "$s\install-fonts.ps1" -Remove }
foreach ($n in 'Gridline Light-Dark', 'Gridline - Off', 'Gridline - On') {
    Remove-Item (Join-Path ([Environment]::GetFolderPath('Desktop')) "$n.lnk") -ErrorAction SilentlyContinue
}
Restart-Shell @('explorer')
Write-Host ''
Write-Host 'Gridline OS for Windows removed. To remove the mods completely, uninstall them in Windhawk.'
