# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. 'D:\VEIL-OMNI\Tools\veil_d_drive_guard.ps1'
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)
$p = 'D:\VEIL-OMNI\proof\key_generic.ps1'
$t = [IO.File]::ReadAllText($p)
$vt = [string][char]11
$bad = "Tools" + $vt + "eil_paths.ps1"
$good = "Tools" + [char]92 + "veil_paths.ps1"
if ($t.Contains($bad)) {
  $t = $t.Replace($bad, $good)
  [IO.File]::WriteAllText($p, $t)
  Write-Output 'fixed veil_paths dot-source'
} else {
  Write-Output 'nothing to fix'
}
Select-String -Path $p -Pattern 'veil_paths' | ForEach-Object { $_.Line }
