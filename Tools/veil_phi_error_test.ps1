# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. 'D:\VEIL-OMNI\Tools\veil_d_drive_guard.ps1'
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)
# PROOF: the golden pitch applied to an error.
#
# Two cases, both real:
#   1. A step that fails twice and answers on the third attempt - an error that
#      the pitch walks in rather than retrying blind or giving up.
#   2. A step that never answers - the last rung is spent and the corrector says
#      so, with its numbers, instead of hanging or lying.
#
# Nothing here touches D: beyond reading; the counter lives in %TEMP%.

$ErrorActionPreference = 'Stop'
. 'D:\VEIL-OMNI\Tools\veil_phi_error.ps1'

$pitch = Get-VeilGoldenPitch
Write-Output '=== THE ANGLE, COMPUTED FRESH ==='
Write-Output ("mu          {0} deg = {1:N12} rad" -f $pitch.Degrees, $pitch.Radians)
Write-Output ("cos(mu)     {0:N10}   sin(mu)     {1:N10}" -f $pitch.Cos, $pitch.Sin)
Write-Output ("cos^2+sin^2 {0:N12}   (round-trip: the correction invents nothing)" -f $pitch.RoundTrip)
Write-Output ("rung(cos)   {0:N4}     rung(sin)   {1:N4}" -f (Get-VeilRung $pitch.Cos), (Get-VeilRung $pitch.Sin))

Write-Output ''
Write-Output '=== CASE 1: an error the pitch walks in (fails twice, answers on the third) ==='
$counter = Join-Path $env:TEMP 'veil_phi_err_counter.txt'
if (Test-Path $counter) { Remove-Item -LiteralPath $counter -Force }
Set-Content -LiteralPath $counter -Value '0'

$value = Invoke-VeilGoldenPitch -Label 'opening the door' -MaxAttempts 3 -WindowSeconds 0.1 -Action {
    $n = [int](Get-Content -LiteralPath $counter)
    $n++
    Set-Content -LiteralPath $counter -Value $n
    if ($n -lt 3) {
        throw ("door stuck at attempt {0}" -f $n)
    }
    return ("open on attempt {0}" -f $n)
}
Write-Output ("result: {0}  (attempts used: {1})" -f $value, (Get-Content -LiteralPath $counter))

Write-Output ''
Write-Output '=== CASE 2: an error that never settles (the last rung is spent and said out loud) ==='
$counter2 = Join-Path $env:TEMP 'veil_phi_err_counter2.txt'
if (Test-Path $counter2) { Remove-Item -LiteralPath $counter2 -Force }
Set-Content -LiteralPath $counter2 -Value '0'

$failed = $false
$final = ''
try {
    [void](Invoke-VeilGoldenPitch -Label 'a door with no key' -MaxAttempts 3 -WindowSeconds 0.1 -Quiet -Action {
        $n = [int](Get-Content -LiteralPath $counter2)
        $n++
        Set-Content -LiteralPath $counter2 -Value $n
        throw 'no key, ever'
    })
}
catch {
    $failed = $true
    $final = $_.Exception.Message
}
Write-Output ("stopped after {0} attempts: {1}" -f (Get-Content -LiteralPath $counter2), $final)

Write-Output ''
Write-Output '=== CASE 3: an error line, so any app failure reads in rungs ==='
Write-Output (Get-VeilErrorLine -Message 'provider request interrupted' -Magnitude 2)

Write-Output ''
if ($value -match 'open on attempt 3' -and $failed -and $final -match 'did not settle after 3 rungs') {
    Write-Output 'GOLDEN PITCH RESULT: PASS - errors are walked in at 17.0324 deg, and a spent ladder says so.'
} else {
    Write-Output 'GOLDEN PITCH RESULT: FAIL - see the lines above.'
}
