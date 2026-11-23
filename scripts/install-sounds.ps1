<#
.SYNOPSIS
  Installs the "Gridline" UI sound scheme for the current user.
  -Restore re-imports the sound settings saved before the first install.
.NOTES
  Windows resets the sound scheme to "Windows Default" whenever it reapplies its theme
  (sign-in, unlock, light/dark switch). The "Keep Gridline cursors and sounds" option of the
  Gridline Desktop mod puts it back while the scheme exists; -Restore deletes the scheme.
#>
param([switch]$Restore)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\common.ps1"
$root = 'HKCU:\AppEvents\Schemes'
$scheme = '.GridlineSound'
$backup = Join-Path $BackupDir 'sounds-before.reg'

if ($Restore) {
    if (Test-Path $backup) { [void](Import-RegFile $backup) }
    # Delete the scheme (also if the backup brought it back) so the mod stops reapplying it.
    Remove-Item "$root\Names\$scheme" -Recurse -Force -ErrorAction SilentlyContinue
    foreach ($ev in Get-ChildItem "$root\Apps\.Default" -ErrorAction SilentlyContinue) {
        Remove-Item (Join-Path $ev.PSPath $scheme) -Recurse -Force -ErrorAction SilentlyContinue
    }
    $active = (Get-ItemProperty $root).'(default)'
    if (-not (Test-Path $backup) -or $active -eq $scheme) {
        # No usable backup: fall back to the Windows Default scheme.
        foreach ($ev in Get-ChildItem "$root\Apps\.Default" -ErrorAction SilentlyContinue) {
            $def = Join-Path $ev.PSPath '.Default'
            $cur = Join-Path $ev.PSPath '.Current'
            if ((Test-Path $def) -and (Test-Path $cur)) {
                Set-ItemProperty $cur -Name '(default)' -Value ((Get-ItemProperty $def).'(default)')
            }
        }
        Set-ItemProperty $root -Name '(default)' -Value '.Default'
        Write-Host 'Switched to the Windows Default sound scheme.'
    } else {
        Write-Host 'Previous sound settings restored.'
    }
    return
}

$dst = Join-Path $DataDir 'sounds'
New-Item -ItemType Directory -Force $dst | Out-Null
Copy-Item (Join-Path $RepoRoot 'sounds\*.wav') $dst -Force
if (-not (Test-Path $backup)) { & reg.exe export 'HKCU\AppEvents\Schemes' $backup /y | Out-Null }

New-Item "$root\Names\$scheme" -Force | Out-Null
Set-ItemProperty "$root\Names\$scheme" -Name '(default)' -Value 'Gridline'
$count = 0
foreach ($line in Get-Content (Join-Path $RepoRoot 'sounds\events.txt')) {
    if (-not $line.Trim() -or $line.StartsWith('#')) { continue }
    $ev, $file = $line -split '=', 2
    $wav = Join-Path $dst $file.Trim()
    if (-not (Test-Path $wav)) { Write-Warning "Missing sound file $file"; continue }
    foreach ($sub in '.Current', $scheme) {
        $k = "$root\Apps\.Default\$($ev.Trim())\$sub"
        if (-not (Test-Path $k)) { New-Item $k -Force | Out-Null }
        Set-ItemProperty $k -Name '(default)' -Value $wav
    }
    $count++
}
Set-ItemProperty $root -Name '(default)' -Value $scheme

# Play the new "information" sound as a confirmation.
Add-Type -Namespace DM -Name Snd -MemberDefinition '[DllImport("winmm.dll", CharSet = CharSet.Unicode)] public static extern bool PlaySound(string s, System.IntPtr h, uint f);' -ErrorAction SilentlyContinue
[void][DM.Snd]::PlaySound('SystemAsterisk', [IntPtr]::Zero, 0x10000 -bor 0x0002)
Write-Host "Gridline sounds installed for $count events. Undo: install-sounds.ps1 -Restore"
