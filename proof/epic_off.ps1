# Kill anything Epic: launcher, EOS overlay/service, Epic web helpers, and the editor if it's up.
# Reports what was running before and after. Does not touch project files.

# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$names = @(
    'EpicGamesLauncher', 'EpicWebHelper', 'EpicOnlineServicesUserHelper',
    'EpicOnlineServicesHost', 'EpicOnlineServicesHostProxy',
    'UnrealEditor', 'UnrealEditor-Cmd', 'UnrealVersionSelector',
    'EOSSDK-Win64-Shipping', 'EasyAntiCheat', 'EasyAntiCheat_EOS'
)

Write-Output '=== BEFORE ==='
$before = Get-Process -ErrorAction SilentlyContinue |
    Where-Object { $names -contains $_.ProcessName -or $_.ProcessName -like 'Epic*' } |
    Select-Object Id, ProcessName
if ($before) { $before | ForEach-Object { Write-Output ("  {0,-6} {1}" -f $_.Id, $_.ProcessName) } }
else { Write-Output '  (nothing Epic running)' }

foreach ($p in (Get-Process -ErrorAction SilentlyContinue |
        Where-Object { $names -contains $_.ProcessName -or $_.ProcessName -like 'Epic*' })) {
    try { $p.CloseMainWindow() | Out-Null } catch { }
}
Start-Sleep -Seconds 2
foreach ($p in (Get-Process -ErrorAction SilentlyContinue |
        Where-Object { $names -contains $_.ProcessName -or $_.ProcessName -like 'Epic*' })) {
    try { Stop-Process -Id $p.Id -Force -ErrorAction Stop } catch { }
}
Start-Sleep -Seconds 2

Write-Output ''
Write-Output '=== AFTER ==='
$after = Get-Process -ErrorAction SilentlyContinue |
    Where-Object { $names -contains $_.ProcessName -or $_.ProcessName -like 'Epic*' } |
    Select-Object Id, ProcessName
if ($after) { $after | ForEach-Object { Write-Output ("  STILL UP  {0,-6} {1}" -f $_.Id, $_.ProcessName) } }
else { Write-Output '  clean - no Epic or Unreal process running' }
