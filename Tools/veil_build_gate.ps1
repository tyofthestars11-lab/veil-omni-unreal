# VEIL BUILD GATE - no yellow, no red.
#
# UBT refuses bWarningsAsErrors on any target whose build products are shared with
# the installed engine ("VEIL has build products in common with UnrealGame"), so the
# rule cannot live in the .Target.cs files. It lives here instead, at the same gate:
# run the build, parse every compiler line, and FAIL on a single warning or error.
# The exit code is the gate, so a wrapper cannot "succeed with warnings".
#
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
param(
  [string]$Target = 'VEILEditor',
  [string]$Platform = 'Win64',
  [string]$Configuration = 'Development',
  [string]$Project = 'D:\VEIL-OMNI\VEIL.uproject'
)
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
$Build = 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat'
if (-not (Test-Path $Build)) { Write-Output "BUILD GATE: Build.bat not found at $Build"; exit 3 }

$log = Join-Path ([System.IO.Path]::GetTempPath()) ("veil_gate_{0}.txt" -f $Target)
$args = @($Target, $Platform, $Configuration, "-Project=$Project", '-WaitMutex', '-FromMsBuild', '-MaxParallelActions=12')
& $Build @args 2>&1 | Tee-Object -FilePath $log | Out-Null
$code = $LASTEXITCODE

$lines = Get-Content $log
$warn = @($lines | Where-Object { $_ -match 'warning C|warning LNK' })
$err = @($lines | Where-Object { $_ -match 'error C|error LNK|error :' })
$result = ($lines | Where-Object { $_ -match '^Result:' } | Select-Object -Last 1)

Write-Output ("BUILD GATE: {0} {1} {2}" -f $Target, $Platform, $Configuration)
# PowerShell 5.1 has no ternary operator - it parses '? :' as a syntax error, and
# a gate that crashes instead of reporting is worse than no gate.
if ($result) { $line = $result.Trim() } else { $line = 'no Result line' }
Write-Output ("BUILD GATE: {0}" -f $line)
Write-Output ("BUILD GATE: Errors = {0}, Warnings = {1}" -f $err.Count, $warn.Count)
foreach ($e in ($err | Select-Object -Unique -First 10)) { Write-Output ("  ERROR   " + $e.Trim()) }
foreach ($w in ($warn | Select-Object -Unique -First 10)) { Write-Output ("  WARNING " + $w.Trim()) }

if ($code -ne 0 -or $err.Count -gt 0 -or $warn.Count -gt 0) {
  Write-Output "BUILD GATE: FAILED - no yellow, no red."
  exit 1
}
Write-Output "BUILD GATE: PASSED - 0 errors, 0 warnings."
exit 0
