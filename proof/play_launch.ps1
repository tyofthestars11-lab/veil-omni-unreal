# Launch the game the way a player's desktop does: no shell in between, so
# /Game/Maps/VEILDistrict reaches Unreal as a CONTENT path and is not rewritten
# into a Windows path by Git Bash's MSYS argument conversion (that rewrite is
# what produced "The map specified on the commandline ... could not be found",
# pointing at the freebuff-desktop folder instead of D:\VEIL-OMNI).
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
param(
  [string[]]$Extra = @(),
  [switch]$Editor,
  # -Plain omits every window flag and lets Unreal use its own saved window size.
  # Forcing -ResX/-ResY on this 300%-scaled desktop produced a 1288x728 window with
  # a 427x240 render in the corner of it (the numbers were divided by the scale),
  # which is a lying photo, not a game bug.
  [switch]$Plain,
  [int]$ResW = 1280,
  [int]$ResH = 720
)
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$Engine = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$Project = 'D:\VEIL-OMNI\VEIL.uproject'
if (-not (Test-Path $Project)) { Write-Output "MISSING PROJECT $Project"; exit 2 }

$args = @($Project)
if (-not $Editor) {
  $args += '/Game/Maps/VEILDistrict'
  $args += '-game'
  $args += '-log'
  if (-not $Plain) {
    $args += '-Windowed'
    $args += ('-ResX=' + $ResW)
    $args += ('-ResY=' + $ResH)
  }
  $args += '-NoSplash'
}
$args += $Extra

Write-Output ("LAUNCH " + $Engine)
Write-Output ("ARGS   " + ($args -join ' '))
$p = Start-Process -FilePath $Engine -ArgumentList $args -WorkingDirectory 'D:\VEIL-OMNI' -PassThru
Write-Output ("pid " + $p.Id)
