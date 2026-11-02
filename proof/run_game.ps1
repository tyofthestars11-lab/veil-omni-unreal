# Launch VEIL in Play with a fixed window, from the command line - no Launcher.
# A frame is captured by proof/game_shot.ps1 afterwards.
# Switches arrive as ONE space-separated string: passing a string[] from bash merged
# them into a single "-phishow,-phisurface" token, which Unreal does not recognise.
param(
  [string]$ExtraText = '',
  [string]$Map = '/Game/Maps/VEILDistrict'
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
$exe = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
if (-not (Test-Path $exe)) { throw "UnrealEditor.exe not found at $exe" }

$list = @('D:\VEIL-OMNI\VEIL.uproject', $Map, '-game', '-windowed', '-ResX=1280', '-ResY=720', '-nosplash')
if ($ExtraText.Trim()) { $list += ($ExtraText.Trim() -split '\s+') }
Write-Output ("launch: " + ($list -join ' '))
Start-Process -FilePath $exe -ArgumentList $list
