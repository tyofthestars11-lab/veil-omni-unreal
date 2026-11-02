# CANONICAL VEIL PATHS - one place, absolute, no app folder anywhere.
#
# The editor resolves a map given as a FILESYSTEM path against its working
# directory. So "/Game/Maps/VEILDistrict" handed over while the shell sat in the
# Freebuff app folder became
#     .../@codebufffreebuff-desktop/resources/git/Game/Maps/VEILDistrict
# and the editor answered "could not be found" - a path into the app, not the
# game. The game is D:\VEIL-OMNI, and /Game/Maps/VEILDistrict is an Unreal
# CONTENT path. Everything that opens, builds or plays the project reads it here.
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path $PSScriptRoot 'veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$VeilRoot    = 'D:\VEIL-OMNI'
$VeilProject = Join-Path $VeilRoot 'VEIL.uproject'
$VeilMap     = '/Game/Maps/VEILDistrict'          # content path, never a file path
$VeilEditor  = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$VeilEditorCmd = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
$VeilBuildBat = 'C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\Build.bat'

function Assert-VeilMapArg {
    <# A map argument must be the CONTENT path - never a Windows path. #>
    param([string]$Map)
    if ([string]::IsNullOrWhiteSpace($Map)) { throw 'VEIL MAP: empty map argument' }
    if ($Map -match '^[A-Za-z]:' -or $Map.Contains('\') -or $Map.Contains(':') -or $Map.Contains('resources')) {
        throw "VEIL MAP: '$Map' is a file path, not a content path - use $VeilMap"
    }
    if ($Map -ne $VeilMap) { throw "VEIL MAP: expected $VeilMap, got $Map" }
    return $true
}

function Start-VeilEditor {
    <#
      Open the project with the map. Absolute .uproject, content-path map, and
      WorkingDirectory pinned to the game root, so a mistaken relative argument
      can only ever resolve inside D:\VEIL-OMNI.
    #>
    param(
        [string]$Map = $VeilMap,
        [switch]$Game,
        [int]$ResX = 1600,
        [int]$ResY = 900
    )
    [void](Assert-VeilMapArg -Map $Map)
    if (-not (Test-Path -LiteralPath $VeilProject)) { throw "VEIL PROJECT: missing $VeilProject" }

    $launchArgs = @('"' + $VeilProject + '"', $Map)
    if ($Game) { $launchArgs += @('-game', '-windowed', "-ResX=$ResX", "-ResY=$ResY", '-log') }

    Start-Process -FilePath $VeilEditor -ArgumentList $launchArgs -WorkingDirectory $VeilRoot
    Write-Output ("launched: {0} {1} {2} (cwd {3})" -f $VeilEditor, $VeilProject, ($launchArgs -join ' '), $VeilRoot)
}
