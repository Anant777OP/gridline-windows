<#
.SYNOPSIS
  One-step installer for Gridline OS for Windows.

.DESCRIPTION
  Runs every part in order. Expect up to three administrator prompts (fonts, icons, and
  theme settings when Windhawk is installed rather than portable). Every change is backed
  up under %LOCALAPPDATA%\Gridline-Windows\backup and can be undone with uninstall.ps1.

.PARAMETER Skip
  Parts to leave out: Fonts, Accent, Cursors, Sounds, Screensaver, Icons, Wallpaper, Themes, Shortcuts
.PARAMETER Light
  Install the light version instead of dark.
.EXAMPLE
  powershell -ExecutionPolicy Bypass -File .\install.ps1
  powershell -ExecutionPolicy Bypass -File .\install.ps1 -Skip Sounds,Screensaver
#>
param(
    [ValidateSet('Fonts', 'Accent', 'Cursors', 'Sounds', 'Screensaver', 'Icons', 'Wallpaper', 'Themes', 'Shortcuts')]
    [string[]]$Skip = @(),
    [switch]$Light
)
$ErrorActionPreference = 'Stop'
$s = Join-Path $PSScriptRoot 'scripts'
. "$s\common.ps1"
$mode = if ($Light) { 'light' } else { 'dark' }

function Step([string]$name, [scriptblock]$action) {
    if ($Skip -contains $name) { Write-Host "-- $name (skipped)"; return }
    Write-Host "== $name"
    try { & $action } catch { Write-Warning "$name failed: $($_.Exception.Message)" }
}

if ([Environment]::OSVersion.Version.Build -lt 22000) { Write-Warning 'This theme is designed for Windows 11.' }

Step 'Fonts' { & "$s\install-fonts.ps1" }
Step 'Accent' { & "$s\set-accent.ps1" -NoRestart }
Step 'Cursors' { & "$s\install-cursors.ps1" -Style $(if ($Light) { 'clear' } else { 'carbon' }) }
Step 'Sounds' { & "$s\install-sounds.ps1" }
Step 'Screensaver' { & "$s\install-screensaver.ps1" }
Step 'Icons' { & "$s\install-icons.ps1" }
if ($Skip -contains 'Wallpaper') {
    Step 'Themes' { & "$s\apply-themes.ps1" -Mode $mode }
} else {
    Step 'Wallpaper' { & "$s\gridline-mode.ps1" -Mode $mode }   # also applies the themes
}
Step 'Shortcuts' {
    $desk = [Environment]::GetFolderPath('Desktop'); $sh = New-Object -ComObject WScript.Shell
    $ps = "$env:WINDIR\System32\WindowsPowerShell\v1.0\powershell.exe"
    foreach ($x in @(@('Gridline Light-Dark', 'gridline-mode.ps1', '', 197), @('Gridline - Off', 'gridline-power.ps1', '-Off', 131),
                     @('Gridline - On', 'gridline-power.ps1', '-On', 101))) {
        $l = $sh.CreateShortcut((Join-Path $desk "$($x[0]).lnk"))
        $l.TargetPath = $ps
        $l.Arguments = "-NoProfile -WindowStyle Hidden -ExecutionPolicy Bypass -File `"$s\$($x[1])`" $($x[2])"
        $l.WorkingDirectory = $s; $l.IconLocation = "$env:WINDIR\System32\imageres.dll,$($x[3])"; $l.Save()
    }
}

# ---------------------------------------------------------------- what's left in Windhawk
$wh = Get-Windhawk
Write-Host ''
Write-Host '== Windhawk mods'
if (-not $wh) {
    Write-Host 'Windhawk was not found. Install it from https://windhawk.net, add the mods below, then run scripts\apply-themes.ps1.'
}
$catalog = [ordered]@{
    'windows-11-taskbar-styler' = 'Windows 11 Taskbar Styler'; 'windows-11-start-menu-styler' = 'Windows 11 Start Menu Styler'
    'windows-11-notification-center-styler' = 'Windows 11 Notification Center Styler'
    'windows-11-file-explorer-styler' = 'Windows 11 File Explorer Styler'; 'windows-11-settings-styler' = 'Windows 11 Settings Styler'
    'windows-animations' = 'Windows Animations (optional)'; 'taskbar-dock-animation' = 'Taskbar Dock Animation (optional)'
    'dark-menus' = 'Dark mode context menus (optional)'
}
foreach ($id in $catalog.Keys) {
    $ok = $wh -and (Test-ModInstalled $wh $id)
    Write-Host ("  [{0}] {1}" -f $(if ($ok) { 'x' } else { ' ' }), $catalog[$id])
}
foreach ($m in @(@('local@gridline-desktop', 'mods\gridline-desktop.wh.cpp'), @('local@gridline-window-frames', 'mods\gridline-window-frames.wh.cpp'))) {
    $ok = $wh -and (Test-ModInstalled $wh $m[0])
    Write-Host ("  [{0}] {1}  (Create a New Mod, paste {1}, Compile)" -f $(if ($ok) { 'x' } else { ' ' }), $m[1])
}
Write-Host ''
Write-Host 'Install any unchecked mods from Windhawk > Explore, then run:  scripts\apply-themes.ps1'
Write-Host 'Undo everything any time with:  uninstall.ps1'
