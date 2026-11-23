<#
.SYNOPSIS
  Plays every Gridline sound with the Windows event it is used for.
  -Name notify (etc.) plays just one.
#>
param([string]$Name)
$dir = Join-Path (Split-Path -Parent $PSScriptRoot) 'sounds'
$sounds = [ordered]@{
    'notify' = 'Notifications'; 'message' = 'New message / mail'; 'reminder' = 'Calendar reminder'
    'tick' = 'Default beep'; 'info' = 'Information'; 'warning' = 'Warning'; 'error' = 'Critical stop'
    'connect' = 'Device connected'; 'disconnect' = 'Device disconnected'; 'battery-low' = 'Low battery'
    'unlock' = 'Unlock / sign in'
}
foreach ($k in $sounds.Keys) {
    if ($Name -and $k -ne $Name) { continue }
    Write-Host ("{0,-12} {1}" -f $k, $sounds[$k])
    (New-Object System.Media.SoundPlayer (Join-Path $dir "$k.wav")).PlaySync()
    Start-Sleep -Milliseconds 600
}
