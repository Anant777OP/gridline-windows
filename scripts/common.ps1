# Shared helpers for the Gridline OS for Windows scripts. Dot-source it:  . "$PSScriptRoot\common.ps1"

$RepoRoot = Split-Path -Parent $PSScriptRoot
$DataDir = Join-Path $env:LOCALAPPDATA 'Gridline-Windows'
$BackupDir = Join-Path $DataDir 'backup'
New-Item -ItemType Directory -Force $DataDir, $BackupDir | Out-Null

function Test-Admin {
    ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
}

# Runs a script elevated (one UAC prompt) and waits for it.
function Invoke-Elevated([string]$Script, [string[]]$Arguments = @()) {
    # an empty argument list can arrive as $null; drop it so Start-Process gets no null element
    $a = @('-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', "`"$Script`"") + @($Arguments | Where-Object { $_ })
    if (Test-Admin) { & powershell.exe @a; return }
    Start-Process powershell.exe -Verb RunAs -Wait -WindowStyle Hidden -ArgumentList $a
}

# Restarts shell processes so they reload themes, fonts and icons. Never call this elevated.
function Restart-Shell([string[]]$Processes = @('explorer', 'StartMenuExperienceHost', 'ShellExperienceHost', 'ShellHost', 'SystemSettings')) {
    foreach ($p in $Processes) {
        Get-Process $p -ErrorAction SilentlyContinue | Where-Object SessionId -eq (Get-Process -Id $PID).SessionId | ForEach-Object { try { Stop-Process -Id $_.Id -Force -ErrorAction Stop } catch { } }
    }
    Start-Sleep -Seconds 2
    # only this session counts: another signed-in user has their own explorer.exe
    $sid = (Get-Process -Id $PID).SessionId
    if (-not (Get-Process explorer -ErrorAction SilentlyContinue | Where-Object SessionId -eq $sid)) { Start-Process explorer.exe }
}

# Windows resets the sound scheme to "Windows Default" when it reapplies its theme. If the
# Gridline sounds are installed, copy them back into .Current (the Gridline Desktop mod does
# the same after unlock and sign-in).
function Repair-SoundScheme([string]$scheme = '.GridlineSound') {
    if (-not (Test-Path "HKCU:\AppEvents\Schemes\Names\$scheme")) { return }
    foreach ($app in Get-ChildItem 'HKCU:\AppEvents\Schemes\Apps' -ErrorAction SilentlyContinue) {
        foreach ($ev in Get-ChildItem $app.PSPath -ErrorAction SilentlyContinue) {
            $src = Join-Path $ev.PSPath $scheme
            if (-not (Test-Path $src)) { continue }
            $val = (Get-ItemProperty $src -ErrorAction SilentlyContinue).'(default)'
            if (-not $val) { continue }
            $cur = Join-Path $ev.PSPath '.Current'
            if (-not (Test-Path $cur)) { New-Item $cur -Force | Out-Null }
            Set-ItemProperty $cur -Name '(default)' -Value $val
        }
    }
    Set-ItemProperty 'HKCU:\AppEvents\Schemes' -Name '(default)' -Value $scheme
}

# Locates Windhawk and where it stores mod settings (portable ini files or the registry).
function Get-Windhawk {
    $dir = $null
    $proc = Get-Process windhawk -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($proc -and $proc.Path) { $dir = Split-Path $proc.Path }
    if (-not $dir) {
        foreach ($k in 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Windhawk',
                       'HKLM:\SOFTWARE\WOW6432Node\Microsoft\Windows\CurrentVersion\Uninstall\Windhawk') {
            $loc = (Get-ItemProperty $k -ErrorAction SilentlyContinue).InstallLocation
            if ($loc -and (Test-Path $loc)) { $dir = $loc; break }
        }
    }
    if (-not $dir -and (Test-Path "$env:ProgramFiles\Windhawk\windhawk.exe")) { $dir = "$env:ProgramFiles\Windhawk" }
    if (-not $dir) { return $null }

    $portable = $false; $appData = Join-Path $env:ProgramData 'Windhawk'
    $ini = Join-Path $dir 'windhawk.ini'
    if (Test-Path $ini) {
        $cfg = Get-Content $ini
        if ($cfg -match '^\s*Portable\s*=\s*1') {
            $portable = $true
            $rel = ($cfg | Where-Object { $_ -match '^\s*AppDataPath\s*=' } | Select-Object -First 1) -replace '^\s*AppDataPath\s*=\s*', ''
            $appData = if ([IO.Path]::IsPathRooted($rel)) { $rel } else { Join-Path $dir $rel }
        }
    }
    [pscustomobject]@{
        Dir         = $dir
        Exe         = Join-Path $dir 'windhawk.exe'
        Portable    = $portable
        ModsIniDir  = Join-Path $appData 'Engine\Mods'
        ProfilePath = Join-Path $appData 'userprofile.json'
        RegMods     = 'HKLM:\SOFTWARE\Windhawk\Engine\Mods'
    }
}

function Test-ModInstalled($wh, [string]$ModId) {
    if ($wh.Portable) { return Test-Path (Join-Path $wh.ModsIniDir "$ModId.ini") }
    return Test-Path (Join-Path $wh.RegMods $ModId)
}

# Turns a JSON settings object into Windhawk's flat keys: a.b, list[0], list[0].field[1] ...
function ConvertTo-FlatSettings($obj, [string]$prefix = '') {
    $out = [ordered]@{}
    foreach ($p in $obj.PSObject.Properties) {
        $key = if ($prefix) { "$prefix.$($p.Name)" } else { $p.Name }
        $v = $p.Value
        if ($v -is [System.Array]) {
            for ($i = 0; $i -lt $v.Count; $i++) {
                $item = $v[$i]
                if ($item -is [pscustomobject]) {
                    foreach ($q in $item.PSObject.Properties) {
                        if ($q.Value -is [System.Array]) {
                            for ($j = 0; $j -lt $q.Value.Count; $j++) { $out["$key[$i].$($q.Name)[$j]"] = [string]$q.Value[$j] }
                        } else { $out["$key[$i].$($q.Name)"] = $q.Value }
                    }
                } else { $out["$key[$i]"] = if ($null -eq $item) { '' } else { [string]$item } }
            }
        } elseif ($v -is [pscustomobject]) {
            $sub = ConvertTo-FlatSettings $v $key
            foreach ($k in $sub.Keys) { $out[$k] = $sub[$k] }
        } elseif ($v -is [bool]) { $out[$key] = [int]$v
        } elseif ($null -eq $v) { $out[$key] = ''
        } else { $out[$key] = $v }
    }
    return $out
}

# Writes settings for one mod. List keys present in $settings replace all of their old entries.
function Write-ModSettings($wh, [string]$ModId, $settings) {
    $listPrefixes = @($settings.Keys | ForEach-Object { if ($_ -match '^([^\[.]+)\[') { $Matches[1] + '[' } } | Select-Object -Unique)
    if ($wh.Portable) {
        $path = Join-Path $wh.ModsIniDir "$ModId.ini"
        $text = [IO.File]::ReadAllText($path, [Text.Encoding]::Unicode)
        $split = $text.IndexOf('[Settings]')
        $head = if ($split -ge 0) { $text.Substring(0, $split) } else { $text.TrimEnd() + "`r`n`r`n" }
        $keep = [ordered]@{}
        if ($split -ge 0) {
            foreach ($l in ($text.Substring($split) -split "`r?`n" | Select-Object -Skip 1)) {
                if ($l -notmatch '=') { continue }
                $k, $v = $l -split '=', 2
                if ($settings.Contains($k)) { continue }
                if ($listPrefixes | Where-Object { $k.StartsWith($_) }) { continue }
                $keep[$k] = $v
            }
        }
        $lines = @('[Settings]') + ($settings.Keys | ForEach-Object { "$_=$($settings[$_])" }) + ($keep.Keys | ForEach-Object { "$_=$($keep[$_])" })
        $new = $head + ($lines -join "`r`n") + "`r`n"
        for ($i = 0; $i -lt 20; $i++) {
            try { [IO.File]::WriteAllText($path, $new, [Text.Encoding]::Unicode); return } catch { Start-Sleep -Milliseconds 250 }
        }
        throw "Could not write $path"
    } else {
        $key = Join-Path (Join-Path $wh.RegMods $ModId) 'Settings'
        if (-not (Test-Path $key)) { New-Item $key -Force | Out-Null }
        $existing = (Get-Item $key).GetValueNames()
        foreach ($n in $existing) { if ($listPrefixes | Where-Object { $n.StartsWith($_) }) { Remove-ItemProperty $key -Name $n } }
        foreach ($k in $settings.Keys) {
            $v = $settings[$k]
            if ($v -is [int] -or $v -is [long]) { Set-ItemProperty $key -Name $k -Value ([int]$v) -Type DWord }
            else { Set-ItemProperty $key -Name $k -Value ([string]$v) -Type String }
        }
    }
}

function Set-ModDisabled($wh, [string]$ModId, [bool]$Disabled) {
    if ($wh.Portable) {
        $path = Join-Path $wh.ModsIniDir "$ModId.ini"
        if (-not (Test-Path $path)) { return $false }
        $text = [IO.File]::ReadAllText($path, [Text.Encoding]::Unicode)
        $text = [regex]::Replace($text, '(?m)^Disabled=\d', "Disabled=$([int]$Disabled)", 1)
        [IO.File]::WriteAllText($path, $text, [Text.Encoding]::Unicode)
        if (Test-Path $wh.ProfilePath) {
            $prof = Get-Content $wh.ProfilePath -Raw -Encoding UTF8 | ConvertFrom-Json
            $entry = $prof.mods.$ModId
            if ($entry) {
                if ($Disabled) { $entry | Add-Member -NotePropertyName disabled -NotePropertyValue $true -Force }
                elseif ($entry.PSObject.Properties['disabled']) { $entry.PSObject.Properties.Remove('disabled') }
                [IO.File]::WriteAllText($wh.ProfilePath, ($prof | ConvertTo-Json -Depth 10), (New-Object Text.UTF8Encoding($false)))
            }
        }
    } else {
        $key = Join-Path $wh.RegMods $ModId
        if (-not (Test-Path $key)) { return $false }
        Set-ItemProperty $key -Name 'Disabled' -Value ([int]$Disabled) -Type DWord
    }
    return $true
}

function Restart-Windhawk($wh) {
    if ($wh -and (Test-Path $wh.Exe)) { & $wh.Exe -restart -tray-only; Start-Sleep -Seconds 6 }
}

# Theme mods and the extra mod settings this project ships.
$ThemeMods = [ordered]@{
    'windows-11-taskbar-styler'             = 'taskbar-styler'
    'windows-11-start-menu-styler'          = 'start-menu-styler'
    'windows-11-notification-center-styler' = 'notification-center-styler'
    'windows-11-file-explorer-styler'       = 'file-explorer-styler'
    'windows-11-settings-styler'            = 'settings-styler'
}
$ExtraModSettings = [ordered]@{
    'windows-animations'     = 'windows-animations'
    'taskbar-dock-animation' = 'taskbar-dock-animation'
}
$GridlineMods = @($ThemeMods.Keys) + @($ExtraModSettings.Keys) + @('local@gridline-desktop', 'local@gridline-window-frames', 'dark-menus')

# Dark -> light colour swap used by the light theme (each pair is unique so it reverses cleanly).
$LightPairs = [ordered]@{
    'bg=#020608' = 'bg=#F3FBFC'; 'surface=#061419' = 'surface=#E3F3F6'; 'surface=#07161C' = 'surface=#E5F4F7'
    'hover=#0B2730' = 'hover=#D2EBF0'; 'line=#0F5664' = 'line=#8FCFDA'
    'fg=#D8FBFF' = 'fg=#02262E'; 'muted=#5E9FAB' = 'muted=#3F7782'
    'accent=#00E5FF' = 'accent=#0098AC'; 'accent2=#FF8A1F' = 'accent2=#D9650A'
    'Fill=#0E3440' = 'Fill=#C5E3E9'; '#F2020608' = '#F2F3FBFC'; '\"#E8020608\"' = '\"#E8F3FBFC\"'
}
