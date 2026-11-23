<#
.SYNOPSIS
  Builds widget_preview.exe with the clang that ships with Windhawk (or clang++ on PATH)
  and renders the README widget previews into docs\images.
#>
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
. (Join-Path $root 'scripts\common.ps1')
$clang = $null
$wh = Get-Windhawk
if ($wh -and (Test-Path (Join-Path $wh.Dir 'Compiler\bin\clang++.exe'))) { $clang = Join-Path $wh.Dir 'Compiler\bin\clang++.exe' }
if (-not $clang) { $clang = (Get-Command clang++ -ErrorAction SilentlyContinue).Source }
if (-not $clang) { throw 'No clang++ found. Install Windhawk (it includes one) or LLVM-MinGW.' }

$exe = Join-Path $env:TEMP 'gridline-widget-preview.exe'
& $clang -std=c++23 -O1 -municode -target x86_64-w64-mingw32 -static -DUNICODE -D_UNICODE `
    -DWINVER=0x0A00 -D_WIN32_WINNT=0x0A00 -DNTDDI_VERSION=0x0A000008 `
    (Join-Path $PSScriptRoot 'widget_preview.cpp') -o $exe `
    -lgdiplus -lgdi32 -luser32 -lwinhttp -lole32 -loleaut32 -ladvapi32 -lruntimeobject -lwtsapi32
if ($LASTEXITCODE) { throw 'Build failed.' }

$img = Join-Path $root 'docs\images'
$wall = Join-Path $root 'wallpapers\01-horizon-grid.png'
$bg = if (Test-Path $wall) { $wall } else { $null }
& $exe (Join-Path $img 'widgets-dark.png') dark solid $bg
& $exe (Join-Path $img 'widgets-light.png') light solid
& $exe (Join-Path $img 'widgets-nocard.png') dark none $bg
Write-Host "Previews written to $img"
