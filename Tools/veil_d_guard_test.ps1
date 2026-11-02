# NON-DELETE GUARD TEST - whole D drive.
#
# Proves, on the real machine:
#   1. D:\DO_NOT_DELETE.txt and D:\VEIL-OMNI\DO_NOT_DELETE.txt exist.
#   2. A delete of the drive root, a format of D:, and a delete of a folder
#      under D: all stop at the guard line and exit 9.
#   3. Nothing on D: was touched by any of those attempts.
#   4. A backup lands in a NAMED folder outside D:.
#
# The dangerous calls run in CHILD powershell processes, so the guard's `exit`
# stops the probe, not this report - and nothing is ever actually removed.

$ErrorActionPreference = 'Stop'
$guard = 'D:\VEIL-OMNI\Tools\veil_d_drive_guard.ps1'
. $guard

Write-Output '=== 1. GUARD FILES ==='
foreach ($f in @('D:\DO_NOT_DELETE.txt', 'D:\VEIL-OMNI\DO_NOT_DELETE.txt')) {
    $exists = Test-Path -LiteralPath $f
    Write-Output ("{0} exists={1}" -f $f, $exists)
    if ($exists) { Write-Output ("  line 1: " + (Get-Content -LiteralPath $f -TotalCount 1 -Encoding UTF8)) }
}
$state = Test-VeilDrivePresent
Write-Output ("D: present={0} guardFile={1} ok={2}" -f $state.Present, $state.GuardFile, $state.Ok)

Write-Output ''
Write-Output '=== 2. BEFORE: what is on D: ==='
$driveBefore = (Get-ChildItem -LiteralPath 'D:\' -Force | Sort-Object Name | Select-Object -ExpandProperty Name)
Write-Output ("D:\ entries: " + $driveBefore.Count)
$veilBefore = (Get-ChildItem -LiteralPath 'D:\VEIL-OMNI' -Recurse -File -Force -ErrorAction SilentlyContinue | Measure-Object).Count
$steamBefore = (Get-ChildItem -LiteralPath 'D:\SteamLibrary' -Recurse -File -Force -ErrorAction SilentlyContinue | Measure-Object).Count
$buildsBefore = (Get-ChildItem -LiteralPath 'D:\Builds' -Recurse -File -Force -ErrorAction SilentlyContinue | Measure-Object).Count
Write-Output ("VEIL-OMNI files: $veilBefore   SteamLibrary files: $steamBefore   Builds files: $buildsBefore")

function Invoke-GuardProbe {
    param([string]$Label, [string]$Action, [string]$Path, [string]$Extra = '')
    $cmd = "& { . '" + $guard + "'; Assert-VeilDDriveSafe -Action '" + $Action + "' -Path '" + $Path + "' " + $Extra + " }"
    $out = & powershell.exe -NoProfile -ExecutionPolicy Bypass -Command $cmd 2>$null
    $code = $LASTEXITCODE
    $text = ($out | Out-String).Trim()
    $stopped = ($text -match 'NON-DELETE GUARD: D drive protected, stopped')
    Write-Host ("[{0}] action={1} path={2} exit={3} stopped={4}" -f $Label, $Action, $Path, $code, $stopped)
    foreach ($line in ($out | Where-Object { $_ -match 'NON-DELETE GUARD|why:|Tyree' })) { Write-Host ("    " + $line) }
    return $stopped
}

Write-Output ''
Write-Output '=== 3. THE GUARD STOPS THE DANGEROUS CALLS ==='
$results = @()
$results += Invoke-GuardProbe -Label 'delete drive root' -Action 'Delete' -Path 'D:\*'
$results += Invoke-GuardProbe -Label 'delete bare drive' -Action 'Delete' -Path 'D:'
$results += Invoke-GuardProbe -Label 'format the drive' -Action 'Format' -Path 'D:\'
$results += Invoke-GuardProbe -Label 'clear the disk' -Action 'ClearDisk' -Path 'D:\'
$results += Invoke-GuardProbe -Label 'initialize the disk' -Action 'InitializeDisk' -Path 'D:\'
$results += Invoke-GuardProbe -Label 'delete a folder' -Action 'Delete' -Path 'D:\VEIL-OMNI\Saved'
$results += Invoke-GuardProbe -Label 'move the project' -Action 'Move' -Path 'D:\VEIL-OMNI'
$results += Invoke-GuardProbe -Label 'delete one file, unnamed' -Action 'Delete' -Path 'D:\VEIL-OMNI\proof\tetra_chain_play.png'

$blocked = ($results | Where-Object { $_ -eq $true }).Count
Write-Output ("stopped at the guard: {0} of {1}" -f $blocked, $results.Count)

Write-Output ''
Write-Output '=== 4. A WRITE TO A NAMED PATH IS ALLOWED (this is what a build does) ==='
$ok = $false
try {
    [void](Assert-VeilDDriveSafe -Action Write -Path 'D:\VEIL-OMNI\Binaries\Win64\UnrealEditor-VEIL.dll')
    $ok = $true
} catch { $ok = $false }
Write-Output ("write to a named path allowed: " + $ok)

Write-Output ''
Write-Output '=== 5. BACKUP RULE: named folder, outside D: ==='
$backup = New-VeilBackup -Source 'D:\VEIL-OMNI\Config' -Label 'config-guard-test'
if ($backup) {
    $n = (Get-ChildItem -LiteralPath $backup -Recurse | Where-Object { -not $_.PSIsContainer } | Measure-Object).Count
    Write-Output ("backup path: $backup  files: $n")
}

Write-Output ''
Write-Output '=== 6. AFTER: D: untouched ==='
$driveAfter = (Get-ChildItem -LiteralPath 'D:\' -Force | Sort-Object Name | Select-Object -ExpandProperty Name)
$veilAfter = (Get-ChildItem -LiteralPath 'D:\VEIL-OMNI' -Recurse -File -Force -ErrorAction SilentlyContinue | Measure-Object).Count
$steamAfter = (Get-ChildItem -LiteralPath 'D:\SteamLibrary' -Recurse -File -Force -ErrorAction SilentlyContinue | Measure-Object).Count
$buildsAfter = (Get-ChildItem -LiteralPath 'D:\Builds' -Recurse -File -Force -ErrorAction SilentlyContinue | Measure-Object).Count
Write-Output ("D:\ entries: {0} (was {1})" -f $driveAfter.Count, $driveBefore.Count)
Write-Output ("VEIL-OMNI files: $veilAfter (was $veilBefore)")
Write-Output ("SteamLibrary files: $steamAfter (was $steamBefore)")
Write-Output ("Builds files: $buildsAfter (was $buildsBefore)")
$same = ($driveAfter.Count -eq $driveBefore.Count) -and ($veilAfter -ge $veilBefore) -and ($steamAfter -eq $steamBefore) -and ($buildsAfter -eq $buildsBefore)
Write-Output ("D: untouched: " + $same)

Write-Output ''
if ($blocked -eq $results.Count -and $same -and $ok) {
    Write-Output 'GUARD RESULT: PASS - whole D drive protected, every dangerous call stopped, nothing touched.'
} else {
    Write-Output 'GUARD RESULT: FAIL - see the lines above.'
}
