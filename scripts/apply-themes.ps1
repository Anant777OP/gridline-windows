<#
.SYNOPSIS
  Writes the Gridline themes into your Windhawk styler mods (taskbar, Start, notifications,
  File Explorer, Settings) plus the animation and dock settings, then reloads the shell.

.PARAMETER Mode
  dark (default) or light.
.PARAMETER WifiName
  Your Wi-Fi network name, so its Quick Settings tile gets the dot icon. Detected
  automatically when omitted.
.EXAMPLE
  .\apply-themes.ps1
  .\apply-themes.ps1 -Mode light
#>
param(
    [ValidateSet('dark', 'light')][string]$Mode = 'dark',
    [string]$WifiName,
    [switch]$NoRestart,
    [switch]$Elevated
)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\common.ps1"

$wh = Get-Windhawk
if (-not $wh) { throw 'Windhawk was not found. Install it from https://windhawk.net and run this again.' }

# Installed (non-portable) Windhawk keeps settings in HKLM: that part needs admin.
if (-not $wh.Portable -and -not (Test-Admin) -and -not $Elevated) {
    $a = @('-Mode', $Mode, '-NoRestart', '-Elevated')
    if ($WifiName) { $a += @('-WifiName', "`"$WifiName`"") }
    Write-Host 'Windhawk stores mod settings in the registry: asking for administrator permission...'
    Invoke-Elevated $PSCommandPath $a
    if (-not $NoRestart) { Restart-Shell }
    return
}

if (-not $WifiName) {
    $line = (netsh wlan show interfaces 2>$null) | Where-Object { $_ -match '^\s*SSID\s*:' } | Select-Object -First 1
    if ($line) { $WifiName = ($line -replace '^\s*SSID\s*:\s*', '').Trim() }
}

$applied = @()
foreach ($modId in $ThemeMods.Keys) {
    if (-not (Test-ModInstalled $wh $modId)) { Write-Host "  skipped $modId (not installed in Windhawk)"; continue }
    $json = Get-Content (Join-Path $RepoRoot "themes\$($ThemeMods[$modId]).json") -Raw -Encoding UTF8
    if ($Mode -eq 'light') { foreach ($k in $LightPairs.Keys) { $json = $json.Replace($k, $LightPairs[$k]) } }
    $theme = $json | ConvertFrom-Json
    if ($WifiName -and $modId -eq 'windows-11-notification-center-styler') {
        foreach ($rule in $theme.controlStyles) {
            if ($rule.target -like '*AutomationProperties.Name=Wi-Fi]*') {
                $extra = ($rule.target -split ', ' | Where-Object { $_ -like '*Name=Wi-Fi]*' }) | ForEach-Object { $_.Replace('Name=Wi-Fi]', "Name=$WifiName]") }
                $rule.target = (@($rule.target) + $extra) -join ', '
            }
        }
    }
    Write-ModSettings $wh $modId (ConvertTo-FlatSettings $theme)
    $applied += $modId
}
foreach ($modId in $ExtraModSettings.Keys) {
    if (-not (Test-ModInstalled $wh $modId)) { continue }
    $obj = Get-Content (Join-Path $RepoRoot "settings\$($ExtraModSettings[$modId]).json") -Raw -Encoding UTF8 | ConvertFrom-Json
    Write-ModSettings $wh $modId (ConvertTo-FlatSettings $obj)
    $applied += $modId
}
Write-Host ("Applied ($Mode): " + ($applied -join ', '))
if ($Elevated -or $NoRestart) { return }
Restart-Shell
