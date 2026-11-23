<#
.SYNOPSIS
  Installs the Gridline Pointer cursors for the current user.
  -Style carbon (default): near-black body, cyan outline, orange accent.
  -Style clear: pale body, dark cyan outline.
  -Restore switches back to the standard Windows cursors.
.NOTES
  Windows' theme service may put its own cursors back after sign-in or unlock. The
  "Keep Gridline cursors and sounds" option of the Gridline Desktop mod reloads them automatically.
#>
param([ValidateSet('carbon', 'clear')][string]$Style = 'carbon', [switch]$Restore)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\common.ps1"

Add-Type @'
using System; using System.Runtime.InteropServices;
public static class NdCur {
    [DllImport("user32.dll", SetLastError=true, CharSet=CharSet.Unicode)] public static extern IntPtr LoadCursorFromFileW(string f);
    [DllImport("user32.dll")] public static extern bool DestroyCursor(IntPtr h);
    [DllImport("user32.dll")] public static extern bool SystemParametersInfoW(uint a, uint p, IntPtr v, uint f);
}
'@

$key = 'HKCU:\Control Panel\Cursors'
$names = 'Arrow','Help','AppStarting','Wait','Crosshair','IBeam','NWPen','No','SizeNS','SizeWE','SizeNWSE','SizeNESW','SizeAll','UpArrow','Hand','Pin','Person'

# Windows re-applies the cursors listed in the active .theme file. Keep it in step with the registry.
function Set-ThemeCursors([hashtable]$values, [string]$schemeName) {
    $theme = (Get-ItemProperty 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Themes' -ErrorAction SilentlyContinue).CurrentTheme
    if (-not $theme -or -not (Test-Path $theme)) { return }
    $bak = Join-Path $BackupDir ([IO.Path]::GetFileName($theme) + '.bak')
    if (-not (Test-Path $bak)) { Copy-Item $theme $bak }
    $lines = [Collections.Generic.List[string]](Get-Content $theme)
    $start = $lines.FindIndex({ param($l) $l -match '^\[Control Panel\\Cursors\]' })
    $section = @('[Control Panel\Cursors]') + ($names | ForEach-Object { "$_=$($values[$_])" }) + @("DefaultValue=$schemeName", '')
    if ($start -lt 0) { $lines.AddRange([string[]]$section) } else {
        $end = $start + 1
        while ($end -lt $lines.Count -and $lines[$end] -notmatch '^\[') { $end++ }
        $lines.RemoveRange($start, $end - $start)
        $lines.InsertRange($start, [string[]]$section)
    }
    Set-Content $theme $lines -Encoding Unicode
}

if ($Restore) {
    foreach ($n in $names) { Set-ItemProperty $key -Name $n -Value '' }
    Set-ItemProperty $key -Name '(default)' -Value 'Windows Default'
    Set-ItemProperty $key -Name 'Scheme Source' -Value 2 -Type DWord
    $aero = @{ Arrow = 'aero_arrow.cur'; Help = 'aero_helpsel.cur'; AppStarting = 'aero_working.ani'; Wait = 'aero_busy.ani'
               NWPen = 'aero_pen.cur'; No = 'aero_unavail.cur'; SizeNS = 'aero_ns.cur'; SizeWE = 'aero_ew.cur'
               SizeNWSE = 'aero_nwse.cur'; SizeNESW = 'aero_nesw.cur'; SizeAll = 'aero_move.cur'; UpArrow = 'aero_up.cur'
               Hand = 'aero_link.cur'; Pin = 'aero_pin.cur'; Person = 'aero_person.cur' }
    $vals = @{}; foreach ($n in $names) { $vals[$n] = $(if ($aero[$n]) { "%SystemRoot%\cursors\$($aero[$n])" } else { '' }) }
    Set-ThemeCursors $vals 'Windows Aero'
    [void][NdCur]::SystemParametersInfoW(0x57, 0, [IntPtr]::Zero, 3)
    Write-Host 'Standard Windows cursors restored.'
    return
}

$dst = Join-Path $DataDir "cursors-$Style"
$schemeName = if ($Style -eq 'carbon') { 'Gridline Pointer Carbon' } else { 'Gridline Pointer Clear' }
New-Item -ItemType Directory -Force $dst | Out-Null
Copy-Item (Join-Path $RepoRoot "cursors\$Style\*") $dst -Include *.cur, *.ani -Force

$map = @{
    Arrow = 'arrow.cur'; Help = 'help.cur'; AppStarting = 'working.ani'; Wait = 'busy.ani'; Crosshair = 'cross.cur'
    IBeam = 'text.cur'; No = 'no.cur'; SizeNS = 'ns.cur'; SizeWE = 'we.cur'; SizeNWSE = 'nwse.cur'; SizeNESW = 'nesw.cur'
    SizeAll = 'move.cur'; UpArrow = 'up.cur'; Hand = 'link.cur'; NWPen = 'pen.cur'; Pin = 'pin.cur'; Person = 'person.cur'
}
foreach ($f in $map.Values) {   # make sure Windows can load every file before switching
    $h = [NdCur]::LoadCursorFromFileW((Join-Path $dst $f))
    if ($h -eq [IntPtr]::Zero) { throw "Windows could not load $f" }
    [void][NdCur]::DestroyCursor($h)
}

$backup = Join-Path $BackupDir 'cursors-before.reg'
if (-not (Test-Path $backup)) { & reg.exe export 'HKCU\Control Panel\Cursors' $backup /y | Out-Null }

foreach ($n in $map.Keys) { Set-ItemProperty $key -Name $n -Value (Join-Path $dst $map[$n]) }
Set-ItemProperty $key -Name '(default)' -Value $schemeName
Set-ItemProperty $key -Name 'Scheme Source' -Value 1 -Type DWord
$vals = @{}; foreach ($n in $names) { $vals[$n] = Join-Path $dst $map[$n] }
Set-ThemeCursors $vals $schemeName
if (-not (Test-Path "$key\Schemes")) { New-Item "$key\Schemes" -Force | Out-Null }
Set-ItemProperty "$key\Schemes" -Name $schemeName -Value (($names | ForEach-Object { Join-Path $dst $map[$_] }) -join ',')
[void][NdCur]::SystemParametersInfoW(0x57, 0, [IntPtr]::Zero, 3)
Write-Host "$schemeName cursors installed."
