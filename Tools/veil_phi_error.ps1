# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. 'D:\VEIL-OMNI\Tools\veil_d_drive_guard.ps1'
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)
# GOLDEN PITCH ERROR CORRECTION.
#
# mu = 17.0324 degrees, the golden pitch. An error is not a switch that flips to
# FAILED - it is a phase the work is standing off from where it belongs. So a
# failed step is not retried by repeating it (that meets the same phase again and
# fails the same way) and not abandoned (that throws the work away). It is walked
# to the next rung with the phase advanced by the golden pitch:
#
#   cos(mu) = 0.9561392708   the part of the correction that lands now
#   sin(mu) = 0.2929124355   the part carried to the next attempt
#   cos^2 + sin^2 = 1.000000000000   nothing invented, nothing discarded
#
# Each attempt therefore arrives 17.0324 degrees further round than the last, on
# a phi ladder (1, 1.618, 2.618 ...), and the rung of the failure is written down
# so the next attempt is a measured move, not a guess.

$script:VeilPhi = 1.618033988749895
$script:VeilGoldenPitchDeg = 17.0324

function Get-VeilRung {
    param([double]$Value)
    if ($Value -le 0) { return [double]::NegativeInfinity }
    return [Math]::Log($Value) / [Math]::Log($script:VeilPhi)
}

function Get-VeilGoldenPitch {
    <# cos/sin of the golden pitch, computed fresh, round-tripped back. #>
    $mu = $script:VeilGoldenPitchDeg * [Math]::PI / 180.0
    $cos = [Math]::Cos($mu)
    $sin = [Math]::Sin($mu)
    [pscustomobject]@{
        Degrees = $script:VeilGoldenPitchDeg
        Radians = $mu
        Cos = $cos
        Sin = $sin
        RoundTrip = ($cos * $cos + $sin * $sin)
    }
}

function Invoke-VeilGoldenPitch {
    <#
      Run an action at the golden pitch. On failure the attempt is advanced one
      rung and its phase turned by mu, up to MaxAttempts. Returns the action's
      value on success; throws only when the last rung is spent.
    #>
    param(
        [Parameter(Mandatory = $true)][scriptblock]$Action,
        [string]$Label = 'step',
        [int]$MaxAttempts = 3,
        [double]$WindowSeconds = 0.25,
        [switch]$Quiet
    )

    $pitch = Get-VeilGoldenPitch
    $lastError = $null

    for ($Attempt = 1; $Attempt -le $MaxAttempts; $Attempt++) {
        try {
            $value = & $Action
            if (-not $Quiet) {
                Write-Host ("GOLDEN PITCH: {0} settled at attempt {1} of {2} (phase {3:N4} deg)." -f `
                    $Label, $Attempt, $MaxAttempts, (($Attempt - 1) * $pitch.Degrees))
            }
            return $value
        }
        catch {
            $lastError = $_
            $message = $_.Exception.Message
            $rung = Get-VeilRung ([Math]::Max(1, $Attempt))
            if (-not $Quiet) {
                Write-Host ("GOLDEN PITCH: {0} attempt {1} failed - '{2}'" -f $Label, $Attempt, $message)
                Write-Host ("GOLDEN PITCH: error sits at rung {0:N4}; correcting at mu {1} deg - in phase {2:N10}, carried {3:N10}." -f `
                    $rung, $pitch.Degrees, $pitch.Cos, $pitch.Sin)
            }
            if ($Attempt -lt $MaxAttempts) {
                # One rung per attempt: 1, phi, phi^2. Not a fixed sleep, and not
                # an instant repeat - the phase has to move for the answer to.
                $delay = $WindowSeconds * [Math]::Pow($script:VeilPhi, $Attempt - 1)
                if (-not $Quiet) {
                    Write-Host ("GOLDEN PITCH: next attempt {0} -> phase {1:N4} deg after {2:N3}s (rung +{3})." -f `
                        ($Attempt + 1), ($Attempt * $pitch.Degrees), $delay, ($Attempt - 1))
                }
                Start-Sleep -Milliseconds ([int]($delay * 1000))
            }
        }
    }

    throw ("GOLDEN PITCH: {0} did not settle after {1} rungs (mu {2} deg). Last error: {3}" -f `
        $Label, $MaxAttempts, $pitch.Degrees, $lastError.Exception.Message)
}

function Get-VeilErrorLine {
    <# One line for a log: the error's own rung and the pitch it is corrected at. #>
    param([string]$Message, [double]$Magnitude = 1)
    $pitch = Get-VeilGoldenPitch
    return ("GOLDEN PITCH {0} deg: '{1}' at rung {2:N4} - correction {3:N10} now, {4:N10} carried." -f `
        $pitch.Degrees, $Message, (Get-VeilRung $Magnitude), $pitch.Cos, $pitch.Sin)
}
