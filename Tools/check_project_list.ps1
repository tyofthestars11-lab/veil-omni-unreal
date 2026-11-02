# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$paths = @(
  (Join-Path $env:LOCALAPPDATA 'EpicGamesLauncher\Saved\Config\Windows\GameUserSettings.ini'),
  (Join-Path $env:LOCALAPPDATA 'EpicGamesLauncher\Saved\Config\WindowsEditor\GameUserSettings.ini'),
  (Join-Path $env:LOCALAPPDATA 'UnrealEngine\5.8\Saved\Config\WindowsEditor\EditorSettings.ini')
)
foreach ($p in $paths) {
  Write-Output "--- $p"
  if (Test-Path $p) {
    $hits = Select-String -Path $p -Pattern 'CreatedProjectPaths' -ErrorAction SilentlyContinue
    if ($hits) { $hits | ForEach-Object { Write-Output ("  " + $_.Line.Trim()) } }
    else { Write-Output '  (no CreatedProjectPaths line)' }
  }
  else { Write-Output '  (file missing)' }
}
Write-Output '--- uproject'
if (Test-Path 'D:\VEIL-OMNI\VEIL.uproject') { Write-Output '  D:\VEIL-OMNI\VEIL.uproject FOUND' } else { Write-Output '  MISSING' }
