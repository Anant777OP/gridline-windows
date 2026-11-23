<#
.SYNOPSIS
  Installs the fonts for all users (one admin prompt). The taskbar, Start and other shell
  surfaces only load fonts installed this way.
  -Remove uninstalls them again.
#>
param([switch]$Remove)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\common.ps1"
if (-not (Test-Admin)) {
    Write-Host 'Asking for administrator permission to install fonts...'
    Invoke-Elevated $PSCommandPath $(if ($Remove) { @('-Remove') } else { @() })
    return
}

$winFonts = Join-Path $env:WINDIR 'Fonts'
$reg = 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts'
$bundled = @(
    @('Orbitron-Variable.ttf', 'Orbitron (TrueType)'),
    @('ShareTechMono-Regular.ttf', 'Share Tech Mono Regular (TrueType)'),
    @('GridlineIcons.ttf', 'Gridline Icons (TrueType)')
)
# Windows' own icon fonts that ship inside system components but aren't installed as fonts.
# Local copies let the Gridline font rules fall back to them so Settings icons keep drawing.
$settingsFonts = 'SystemResources\Windows.UI.SettingsAppThreshold\SystemSettings\Assets\Fonts'
$system = @(
    @('SystemApps\MicrosoftWindows.Client.Core_cw5n1h2txyewy\SystemTray\Assets\Sysbatt.ttf', 'GridlineCopy-SysBatt.ttf', 'SysBatt Fluent Icons (TrueType)'),
    @("$settingsFonts\SettingsIcons.ttf", 'GridlineCopy-SettingsIcons.ttf', 'Settings Fluent Icons (TrueType)'),
    @("$settingsFonts\SetMDL2.ttf", 'GridlineCopy-SetMDL2.ttf', 'Settings MDL2 Assets (TrueType)')
)

if ($Remove) {
    foreach ($f in $bundled + ($system | ForEach-Object { , @($_[1], $_[2]) })) {
        Remove-ItemProperty $reg -Name $f[1] -ErrorAction SilentlyContinue
        $file = Join-Path $winFonts $f[0]
        try { Remove-Item $file -Force -ErrorAction Stop } catch { if (Test-Path $file) { Write-Host "  $($f[0]) is in use; it is unregistered and will be removed after a restart." } }
    }
    Write-Host 'Fonts removed.'
    return
}

foreach ($f in $bundled) {
    $to = Join-Path $winFonts $f[0]
    if (-not (Test-Path $to)) { Copy-Item (Join-Path $RepoRoot "fonts\$($f[0])") $to }
    Set-ItemProperty $reg -Name $f[1] -Value $f[0]
    Write-Host "  installed $($f[0])"
}
foreach ($f in $system) {
    $from = Join-Path $env:WINDIR $f[0]
    if (-not (Test-Path $from)) { Write-Host "  skipped $($f[2]) (not on this Windows version)"; continue }
    $to = Join-Path $winFonts $f[1]
    if (-not (Test-Path $to)) { Copy-Item $from $to }
    Set-ItemProperty $reg -Name $f[2] -Value $f[1]
    Write-Host "  installed local copy of $($f[2])"
}
Write-Host 'Fonts installed.'
