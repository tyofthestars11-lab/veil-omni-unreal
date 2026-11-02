# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. 'D:\VEIL-OMNI\Tools\veil_d_drive_guard.ps1'
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)
# PROOF: the map opens from D:\VEIL-OMNI, and a file path is refused.
#
# The bug this closes:
#   .../@codebufffreebuff-desktop/resources/git/Game/Maps/VEILDistrict
#   could not be found
# That path is the Freebuff app folder, not the game. The map is a CONTENT path
# (/Game/Maps/VEILDistrict) and the project is absolute (D:\VEIL-OMNI\VEIL.uproject).
#
# Step 1 refuses a filesystem path outright.
# Step 2 opens the project at the content path with the working directory pinned
#        to the game root, then reads the editor's own log for the map load.

$ErrorActionPreference = 'Stop'
. 'D:\VEIL-OMNI\Tools\veil_paths.ps1'

Write-Output '=== 1. THE GUARD REFUSES A FILE PATH ==='
$refused = $false
$reason = ''
try {
    [void](Assert-VeilMapArg -Map 'C:\Users\Admin\AppData\Local\Programs\@codebufffreebuff-desktop\resources\git\Game\Maps\VEILDistrict')
}
catch {
    $refused = $true
    $reason = $_.Exception.Message
}
Write-Output ("file path refused: {0}" -f $refused)
Write-Output ("  reason: {0}" -f $reason)

$accepted = $false
try { [void](Assert-VeilMapArg -Map $VeilMap); $accepted = $true } catch { $accepted = $false }
Write-Output ("content path accepted: {0} ({1})" -f $accepted, $VeilMap)

Write-Output ''
Write-Output '=== 2. OPEN THE PROJECT AT THE MAP ==='
Write-Output ("root     {0}" -f $VeilRoot)
Write-Output ("project  {0}  exists={1}" -f $VeilProject, (Test-Path -LiteralPath $VeilProject))
Write-Output ("map      {0}  (content path)" -f $VeilMap)

Get-Process UnrealEditor -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Seconds 3

# The editor numbers its logs (VEIL.log, VEIL_2.log, ...), so the proof watches
# the DIRECTORY and reads whichever file the launch actually wrote.
$logDir = Join-Path $VeilRoot 'Saved\Logs'
$launchTime = Get-Date
$logPath = $null

Start-VeilEditor -Map $VeilMap

Write-Output ''
Write-Output '=== 3. WAIT FOR THE MAP TO REALLY LOAD ==='
$loaded = $false
$line = ''
$deadline = (Get-Date).AddSeconds(300)
while ((Get-Date) -lt $deadline) {
    Start-Sleep -Seconds 5
    $newest = Get-ChildItem -LiteralPath $logDir -Filter *.log -ErrorAction SilentlyContinue |
        Where-Object { $_.LastWriteTime -gt $launchTime } | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if (-not $newest) { continue }
    $logPath = $newest.FullName
    # The editor's own words. In the EDITOR the proof is the absolute .umap it
    # resolved (D:/VEIL-OMNI/Content/Maps/VEILDistrict.umap) plus the world it
    # registered; in -game it is the world brought up for play. Either one is
    # the map loaded, not merely requested.
    $hit = Select-String -Path $logPath -Pattern 'Cmd: MAP LOAD FILE=.*VEILDistrict|registered with world ''VEILDistrict''|LogWorld: Bringing World .*VEILDistrict|world=VEILDistrict' -ErrorAction SilentlyContinue |
        Select-Object -Last 1
    if ($hit) { $loaded = $true; $line = $hit.Line.Trim(); break }
    $bad = Select-String -Path $logPath -Pattern 'could not be found' -ErrorAction SilentlyContinue | Select-Object -Last 1
    if ($bad) { $line = $bad.Line.Trim(); break }
}

Write-Output ("log read: {0}" -f $logPath)
Write-Output ("editor window: {0}" -f ((Get-Process UnrealEditor -ErrorAction SilentlyContinue | Select-Object -First 1).MainWindowTitle))
Write-Output ("map loaded: {0}" -f $loaded)
if ($line) { Write-Output ("  log: {0}" -f $line) }

Write-Output ''
if ($refused -and $accepted -and $loaded) {
    Write-Output 'MAP PATH RESULT: PASS - content path opens from D:\VEIL-OMNI, file paths refused.'
} else {
    Write-Output 'MAP PATH RESULT: FAIL - see the lines above.'
}
