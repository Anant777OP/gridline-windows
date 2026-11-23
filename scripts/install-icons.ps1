<#
.SYNOPSIS
  Gridline icons for File Explorer: all folders, Desktop/Documents/Downloads/Music/Pictures/Videos,
  This PC, Home, Gallery, Network, Recycle Bin and drives. One admin prompt (folders + drives).
  -Restore undoes everything from the saved backups.
#>
param([switch]$Restore, [switch]$Elevated)
$ErrorActionPreference = 'Stop'
. "$PSScriptRoot\common.ps1"
$bk = Join-Path $BackupDir 'icons'
New-Item -ItemType Directory -Force $bk | Out-Null
$userIcons = Join-Path $DataDir 'icons'
$sysIcons = Join-Path $env:ProgramData 'Gridline-Windows\icons'
$shellIcons = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Shell Icons'
$driveKey = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\DriveIcons'

# ------------------------------------------------ admin part: folders + drives (system-wide)
if ($Elevated) {
    if ($Restore) {
        $list = Join-Path $bk 'drives-added.txt'
        if (Test-Path $list) { foreach ($l in Get-Content $list) { if ($l) { Remove-Item "$driveKey\$l" -Recurse -Force -ErrorAction SilentlyContinue } } }
        if (Test-Path $shellIcons) { Remove-ItemProperty $shellIcons -Name '3', '4', '7', '8' -ErrorAction SilentlyContinue }
        return
    }
    New-Item -ItemType Directory -Force $sysIcons | Out-Null
    Copy-Item (Join-Path $RepoRoot 'icons\*.ico') $sysIcons -Force
    if (-not (Test-Path $shellIcons)) { New-Item $shellIcons -Force | Out-Null }
    Set-ItemProperty $shellIcons -Name '3' -Value "$sysIcons\gridline-folder.ico,0"
    Set-ItemProperty $shellIcons -Name '4' -Value "$sysIcons\gridline-folder-open.ico,0"
    Set-ItemProperty $shellIcons -Name '8' -Value "$sysIcons\drive.ico,0"
    Set-ItemProperty $shellIcons -Name '7' -Value "$sysIcons\drive-usb.ico,0"
    $added = @(); $sysLetter = $env:SystemDrive.TrimEnd(':')
    foreach ($d in Get-CimInstance Win32_LogicalDisk -Filter 'DriveType=3') {
        $l = $d.DeviceID.TrimEnd(':')
        if (Test-Path "$driveKey\$l") { continue }   # keep drives that already have a custom icon
        New-Item "$driveKey\$l\DefaultIcon" -Force | Out-Null
        Set-ItemProperty "$driveKey\$l\DefaultIcon" -Name '(default)' -Value "$sysIcons\$(if ($l -eq $sysLetter) { 'drive-system' } else { 'drive' }).ico,0"
        $added += $l
    }
    $added | Set-Content (Join-Path $bk 'drives-added.txt')
    return
}

# ------------------------------------------------ per-user part
$explorerClsid = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\CLSID'
$classesClsid = 'HKCU:\Software\Classes\CLSID'
$desktopIcons = @{ '{20D04FE0-3AEA-1069-A2D8-08002B30309D}' = 'this-pc'; '{F02C1A0D-BE21-4350-88B0-7367FC96EF3C}' = 'network'
                   '{645FF040-5081-101B-9F08-00AA002F954E}' = 'recycle' }
$classIcons = @{
    '{20D04FE0-3AEA-1069-A2D8-08002B30309D}' = 'this-pc'; '{F02C1A0D-BE21-4350-88B0-7367FC96EF3C}' = 'network'
    '{f874310e-b6b7-47dc-bc84-b9e6b38f5903}' = 'home'; '{e88865ea-0e1c-4e20-9aa6-edcd0212c87c}' = 'gallery'
    '{B4BFCC3A-DB2C-424C-B029-7FE99A87C641}' = 'folder-desktop'; '{d3162b92-9365-467a-956b-92703aca08af}' = 'folder-documents'
    '{088e3905-0323-4b02-9826-5d99428e115f}' = 'folder-downloads'; '{3dfdf296-dbec-4fb4-81d1-6a3438bcf4de}' = 'folder-music'
    '{24ad3ad4-a569-4530-98e1-ab02f9417aa8}' = 'folder-pictures'; '{f86fa3ab-70d2-4fc7-9c99-fcbf05467f3a}' = 'folder-videos'
}
$usf = Get-ItemProperty 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\User Shell Folders'
$knownFolders = [ordered]@{
    'folder-desktop' = [Environment]::GetFolderPath('Desktop'); 'folder-documents' = [Environment]::GetFolderPath('MyDocuments')
    'folder-downloads' = [Environment]::ExpandEnvironmentVariables($usf.'{374DE290-123F-4565-9164-39C4925E467B}')
    'folder-music' = [Environment]::GetFolderPath('MyMusic'); 'folder-pictures' = [Environment]::GetFolderPath('MyPictures')
    'folder-videos' = [Environment]::GetFolderPath('MyVideos')
}

function Set-DesktopIni([string]$folder, [string]$iconPath) {
    $ini = Join-Path $folder 'desktop.ini'
    $enc = [Text.Encoding]::Unicode; $lines = @()
    if (Test-Path $ini) {
        $bytes = [IO.File]::ReadAllBytes($ini)
        if (-not ($bytes.Length -ge 2 -and $bytes[0] -eq 0xFF -and $bytes[1] -eq 0xFE)) { $enc = [Text.Encoding]::Default }
        $lines = [IO.File]::ReadAllText($ini, $enc) -split "`r?`n"
        (Get-Item $ini -Force).Attributes = 'Normal'
    }
    $out = New-Object Collections.Generic.List[string]; $inSection = $false; $done = $false
    foreach ($l in $lines) {
        if ($l -match '^\s*\[') {
            if ($inSection -and -not $done) { $out.Add("IconResource=$iconPath,0"); $done = $true }
            $inSection = $l -match '^\s*\[\.ShellClassInfo\]'
        } elseif ($inSection -and $l -match '^\s*(IconResource|IconFile|IconIndex)\s*=') { continue }
        $out.Add($l)
    }
    if ($inSection -and -not $done) { $out.Add("IconResource=$iconPath,0"); $done = $true }
    if (-not $done) { $out.Insert(0, "IconResource=$iconPath,0"); $out.Insert(0, '[.ShellClassInfo]') }
    while ($out.Count -and $out[$out.Count - 1] -eq '') { $out.RemoveAt($out.Count - 1) }
    [IO.File]::WriteAllText($ini, ($out -join "`r`n") + "`r`n", $enc)
    (Get-Item $ini -Force).Attributes = 'Hidden, System, Archive'
    $f = Get-Item $folder -Force
    if (-not ($f.Attributes -band [IO.FileAttributes]::ReadOnly)) { $f.Attributes = $f.Attributes -bor [IO.FileAttributes]::ReadOnly }
}

if ($Restore) {
    foreach ($g in $desktopIcons.Keys) { Remove-Item "$explorerClsid\$g\DefaultIcon" -Recurse -Force -ErrorAction SilentlyContinue }
    foreach ($g in $classIcons.Keys) { Remove-Item "$classesClsid\$g\DefaultIcon" -Recurse -Force -ErrorAction SilentlyContinue }
    foreach ($reg in Get-ChildItem $bk -Filter 'reg-*.reg' -ErrorAction SilentlyContinue) { & reg.exe import $reg.FullName 2>$null }
    foreach ($k in $knownFolders.Keys) {
        $ini = Join-Path $knownFolders[$k] 'desktop.ini'; $b = Join-Path $bk "$k.desktop.ini"; $none = Join-Path $bk "$k.none"
        if (Test-Path $b) {
            if (Test-Path $ini) { (Get-Item $ini -Force).Attributes = 'Normal' }
            Copy-Item $b $ini -Force; (Get-Item $ini -Force).Attributes = 'Hidden, System, Archive'
        } elseif ((Test-Path $none) -and (Test-Path $ini)) { (Get-Item $ini -Force).Attributes = 'Normal'; Remove-Item $ini -Force }
    }
} else {
    New-Item -ItemType Directory -Force $userIcons | Out-Null
    Copy-Item (Join-Path $RepoRoot 'icons\*.ico') $userIcons -Force
    $i = 0
    foreach ($pair in @(@($explorerClsid, $desktopIcons.Keys), @($classesClsid, $classIcons.Keys))) {
        foreach ($g in $pair[1]) {
            $k = "$($pair[0])\$g\DefaultIcon"; $i++; $b = Join-Path $bk "reg-$i.reg"
            if ((Test-Path $k) -and -not (Test-Path $b)) { & reg.exe export ($k -replace '^HKCU:', 'HKCU') $b /y | Out-Null }
        }
    }
    foreach ($g in $desktopIcons.Keys) {
        $k = "$explorerClsid\$g\DefaultIcon"; New-Item $k -Force | Out-Null
        if ($desktopIcons[$g] -eq 'recycle') {
            Set-ItemProperty $k -Name '(default)' -Value "$userIcons\recycle-full.ico,0"
            Set-ItemProperty $k -Name 'Full' -Value "$userIcons\recycle-full.ico,0"
            Set-ItemProperty $k -Name 'Empty' -Value "$userIcons\recycle-empty.ico,0"
        } else { Set-ItemProperty $k -Name '(default)' -Value "$userIcons\$($desktopIcons[$g]).ico,0" }
    }
    foreach ($g in $classIcons.Keys) {
        $k = "$classesClsid\$g\DefaultIcon"; New-Item $k -Force | Out-Null
        Set-ItemProperty $k -Name '(default)' -Value "$userIcons\$($classIcons[$g]).ico,0"
    }
    foreach ($k in $knownFolders.Keys) {
        $folder = $knownFolders[$k]
        if (-not $folder -or -not (Test-Path $folder)) { continue }
        $ini = Join-Path $folder 'desktop.ini'; $b = Join-Path $bk "$k.desktop.ini"; $none = Join-Path $bk "$k.none"
        if (-not (Test-Path $b) -and -not (Test-Path $none)) {
            if (Test-Path $ini) { Copy-Item $ini $b -Force; (Get-Item $b -Force).Attributes = 'Normal' } else { New-Item $none -ItemType File | Out-Null }
        }
        Set-DesktopIni $folder "$userIcons\$k.ico"
    }
}

Write-Host 'Asking for administrator permission for folder and drive icons...'
try { Invoke-Elevated $PSCommandPath $(if ($Restore) { @('-Elevated', '-Restore') } else { @('-Elevated') }) } catch { Write-Host 'System-wide icons skipped (permission declined).' }

# refresh the icon cache
Get-Process explorer -ErrorAction SilentlyContinue | ForEach-Object { try { Stop-Process -Id $_.Id -Force -ErrorAction Stop } catch { } }
Start-Sleep -Seconds 1
Get-ChildItem "$env:LOCALAPPDATA\Microsoft\Windows\Explorer" -Filter 'iconcache_*.db' -Force -ErrorAction SilentlyContinue | ForEach-Object { try { Remove-Item $_.FullName -Force -ErrorAction Stop } catch { } }
& "$env:WINDIR\System32\ie4uinit.exe" -show
Start-Sleep -Seconds 1
if (-not (Get-Process explorer -ErrorAction SilentlyContinue)) { Start-Process explorer.exe }
Write-Host $(if ($Restore) { 'Explorer icons restored.' } else { 'Gridline icons set.' })
