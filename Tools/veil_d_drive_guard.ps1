# NON-DELETE GUARD - THE ENTIRE D: DRIVE.
#
# Tyree's word, whole drive, not just the game folder:
#   - no delete, no format, no clean, no initialize, no move on D: for any
#     reason, unless Tyree named that exact file for that exact action in his
#     own words.
#   - no Format-Volume, no Clear-Disk, no Initialize-Disk, no Remove-Item D:\*,
#     no Disk Management format / initialize on D: ever.
#   - back up to a NAMED folder before any build, install or cleanup that
#     touches D:, and never wipe a whole drive "to make space".
#   - if D: is unplugged or missing: stop every build, do not reassign the drive
#     letter, do not format the "new" drive Windows shows, wait for Tyree to
#     re-plug D: and verify D:\VEIL-OMNI is back, then continue.
#
# Code cannot hear Tyree's words, so the guard treats them as ABSENT: it stops.
# Lifting it is Tyree's hand only - he says the words, a human runs it, or he
# edits this file himself. Nothing in a build, install or cleanup can lift it.
#
# The guard file itself lives at D:\DO_NOT_DELETE.txt (whole drive) and
# D:\VEIL-OMNI\DO_NOT_DELETE.txt (the project inside it).

$script:VeilGuardStopLine = 'NON-DELETE GUARD: D drive protected, stopped'
$script:VeilGuardDriveFile = 'D:\DO_NOT_DELETE.txt'
$script:VeilGuardProjectFile = 'D:\VEIL-OMNI\DO_NOT_DELETE.txt'

function Get-VeilGuardStopLine { return $script:VeilGuardStopLine }

function Test-VeilDrivePresent {
    # A missing D: is not an empty D:. One is unplugged, the other is erased -
    # and writing to the "new" drive Windows offers is how erased becomes gone.
    $present = Test-Path -LiteralPath 'D:\'
    $guarded = Test-Path -LiteralPath $script:VeilGuardDriveFile
    return [pscustomobject]@{
        Present = $present
        GuardFile = $guarded
        Ok = ($present -and $guarded)
    }
}

function Test-VeilDriveRootTarget {
    param([string]$Path)
    if ([string]::IsNullOrWhiteSpace($Path)) { return $true }   # empty target = whole drive
    $p = $Path.Trim()
    if ($p -match '^[Dd]:[\\/]?$') { return $true }
    if ($p -match '^[Dd]:[\\/]\*$') { return $true }
    if ($p -match '^[Dd]:[\\/][^\\/]+[\\/]\*$' -and $p -match '^[Dd]:[\\/]\*$') { return $true }
    return $false
}

function Test-VeilPathUnderD {
    param([string]$Path)
    if ([string]::IsNullOrWhiteSpace($Path)) { return $false }
    return ($Path.Trim() -match '^[Dd]:([\\/]|$)')
}

function Test-VeilNamedFile {
    param([string]$Path)
    # A file Tyree could have named: full path, a real name, an extension.
    if ([string]::IsNullOrWhiteSpace($Path)) { return $false }
    $name = Split-Path -Leaf $Path
    if ([string]::IsNullOrWhiteSpace($name)) { return $false }
    if ($name -match '[\*\?]') { return $false }
    return ($name -match '\.[A-Za-z0-9]{1,8}$')
}

function Stop-VeilGuard {
    param([string]$Reason, [string]$Script = '')
    Write-Output $script:VeilGuardStopLine
    if ($Reason) { Write-Output ("  why: " + $Reason) }
    if ($Script) { Write-Output ("  script: " + $Script) }
    Write-Output ("  guard: " + $script:VeilGuardDriveFile)
    Write-Output "  Tyree's word only lifts this guard. Nothing here deletes, formats, cleans or moves D:."
    exit 9
}

function Assert-VeilDDriveSafe {
    <#
      The one gate every script passes before it touches D:.
      Actions: Write, Copy, Move, Delete, Format, ClearDisk, InitializeDisk, Clean.
    #>
    param(
        [Parameter(Mandatory = $true)]
        [ValidateSet('Write', 'Copy', 'Move', 'Delete', 'Format', 'ClearDisk', 'InitializeDisk', 'Clean')]
        [string]$Action,

        [string]$Path = '',

        [string]$Script = '',

        # Set ONLY when Tyree has named this exact file for this exact action.
        [switch]$TyreeNamed
    )

    # 0. The drive itself must be there, guard file and all.
    $state = Test-VeilDrivePresent
    if (-not $state.Present) {
        Stop-VeilGuard -Reason "D: is not present - builds stopped, do not reassign the letter, do not format what Windows shows" -Script $Script
    }
    if (-not $state.GuardFile) {
        Stop-VeilGuard -Reason "the whole-drive guard file is missing - refusing to act until it is back" -Script $Script
    }

    # 1. Format / clean / initialize: never. Not by a flag, not by a script.
    if ($Action -in @('Format', 'ClearDisk', 'InitializeDisk', 'Clean')) {
        Stop-VeilGuard -Reason ("$Action on D: is never allowed - Tyree's word only, and code cannot hear it") -Script $Script
    }

    # 2. Only D: targets go past this point.
    if (-not (Test-VeilPathUnderD -Path $Path)) { return $true }

    # 3. The drive root itself is never a target for any action.
    if (Test-VeilDriveRootTarget -Path $Path) {
        Stop-VeilGuard -Reason ("$Action targets the drive root itself (" + $Path + ")") -Script $Script
    }

    # 4. Delete / move: only the exact FILE Tyree named, never a folder, never a wildcard.
    if ($Action -in @('Delete', 'Move')) {
        if (-not $TyreeNamed) {
            Stop-VeilGuard -Reason ("$Action on " + $Path + " was not named by Tyree for this exact file") -Script $Script
        }
        if (-not (Test-VeilNamedFile -Path $Path)) {
            Stop-VeilGuard -Reason ("$Action on " + $Path + " is a folder or a wildcard, not one named file") -Script $Script
        }
    }

    # 5. Write / copy to a named path under D: is what a build does. Allowed.
    return $true
}

function New-VeilBackup {
    <#
      Back up a named path BEFORE anything touches it. The backup goes OUTSIDE D:
      by default, because a backup on the same drive dies with the drive.
    #>
    param(
        [Parameter(Mandatory = $true)][string]$Source,
        [string]$Label = 'backup',
        [string]$To = ''
    )
    $state = Test-VeilDrivePresent
    if (-not $state.Present) {
        Stop-VeilGuard -Reason "D: is not present - nothing to back up, and no build should run"
    }
    if (-not (Test-Path -LiteralPath $Source)) {
        Write-Host ("NON-DELETE GUARD: nothing at " + $Source + " to back up")
        return $null
    }
    if ([string]::IsNullOrWhiteSpace($To)) {
        # Prefer a place that survives D: going missing.
        $To = Join-Path $env:USERPROFILE 'VEIL_BACKUPS'
    }
    $stamp = Get-Date -Format 'yyyyMMdd-HHmmss'
    $dest = Join-Path $To ($stamp + '_' + $Label)
    New-Item -ItemType Directory -Path $dest -Force | Out-Null
    Copy-Item -LiteralPath $Source -Destination $dest -Recurse -Force -ErrorAction Stop
    Write-Host ("NON-DELETE GUARD: backed up " + $Source + " -> " + $dest)
    return $dest
}
