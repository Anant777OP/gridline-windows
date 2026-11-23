<#
.SYNOPSIS
  Turns the Gridline setup off (back to the normal Windows look) or on again.
.DESCRIPTION
  -Off: disables the Gridline-related Windhawk mods and restores your previous cursors,
        sounds and screensaver. Fonts, icons, accent and wallpaper stay (see uninstall.ps1).
  -On:  re-enables the mods and reinstalls the Gridline cursors, sounds and screensaver.
#>
param([switch]$Off, [switch]$On, [switch]$Elevated)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\common.ps1"
if ($Off -eq $On) { throw 'Use -Off or -On' }

$wh = Get-Windhawk
if ($wh) {
    if (-not $wh.Portable -and -not (Test-Admin) -and -not $Elevated) {
        Invoke-Elevated $PSCommandPath @($(if ($Off) { '-Off' } else { '-On' }), '-Elevated')
    } else {
        foreach ($m in $GridlineMods) { [void](Set-ModDisabled $wh $m ([bool]$Off)) }
    }
    if ($Elevated) { return }
} else { Write-Host 'Windhawk not found: only cursors, sounds and screensaver are switched.' }

foreach ($s in 'install-cursors.ps1', 'install-sounds.ps1', 'install-screensaver.ps1') {
    # one failing part must not stop the others
    try { if ($Off) { & "$PSScriptRoot\$s" -Restore } else { & "$PSScriptRoot\$s" } }
    catch { Write-Warning "$s failed: $($_.Exception.Message)" }
}
if ($wh) { Restart-Windhawk $wh }
Restart-Shell
if ($On) {
    # Restarting Explorer can make Windows reapply its theme, which resets the sound scheme.
    Start-Sleep -Seconds 5
    Repair-SoundScheme
}
Write-Host ("Gridline OS for Windows is now " + $(if ($Off) { 'OFF' } else { 'ON' }) + '.')
