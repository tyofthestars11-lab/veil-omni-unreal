# Stop the game/editor by PID only - never by name pattern (an agent's own
# command line can contain the same words). Prints what it found and stopped.
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
param([int[]]$Ids = @(), [switch]$All)
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$procs = @(Get-Process UnrealEditor -ErrorAction SilentlyContinue)
if ($procs.Count -eq 0) { Write-Output 'none running'; exit 0 }
foreach ($p in $procs) {
  $go = $All -or ($Ids -contains $p.Id)
  Write-Output ("found pid={0} title='{1}' {2}" -f $p.Id, $p.MainWindowTitle, $(if ($go) { 'STOP' } else { 'keep' }))
  if ($go) { Stop-Process -Id $p.Id -Force }
}
Start-Sleep -Seconds 5
$left = @(Get-Process UnrealEditor -ErrorAction SilentlyContinue)
Write-Output ("remaining " + $left.Count)
