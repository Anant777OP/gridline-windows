<#
.SYNOPSIS
  Rebuilds GridlineClock.scr from gridline-clock.cpp using the clang compiler that ships
  with Windhawk (or any clang++ on PATH).
#>
$ErrorActionPreference = 'Stop'
. (Join-Path (Split-Path -Parent $PSScriptRoot) 'scripts\common.ps1')
$clang = $null
$wh = Get-Windhawk
if ($wh -and (Test-Path (Join-Path $wh.Dir 'Compiler\bin\clang++.exe'))) { $clang = Join-Path $wh.Dir 'Compiler\bin\clang++.exe' }
if (-not $clang) { $clang = (Get-Command clang++ -ErrorAction SilentlyContinue).Source }
if (-not $clang) { throw 'No clang++ found. Install Windhawk (it includes one) or LLVM-MinGW.' }
& $clang -O2 -std=c++20 -municode -target x86_64-w64-mingw32 -static '-Wl,--subsystem,windows' `
    "$PSScriptRoot\gridline-clock.cpp" -o "$PSScriptRoot\GridlineClock.scr" -lgdiplus -lgdi32 -luser32
Write-Host "Built $PSScriptRoot\GridlineClock.scr"
