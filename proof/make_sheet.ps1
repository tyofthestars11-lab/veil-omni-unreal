param([string]$Out = 'D:\VEIL-OMNI\proof\tp5_sheet.jpg')
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

$names = @('tp5_1', 'tp5_2', 'tp5_3', 'tp5_4')
$imgs = @()
foreach ($n in $names) { $imgs += [System.Drawing.Image]::FromFile("D:\VEIL-OMNI\proof\$n.png") }

$w = 760
$h = 446
$sheet = New-Object System.Drawing.Bitmap(($w * 2), ($h * 2))
$g = [System.Drawing.Graphics]::FromImage($sheet)
$g.InterpolationMode = 'HighQualityBicubic'
for ($i = 0; $i -lt 4; $i++) {
    $x = ($i % 2) * $w
    $y = [int]([Math]::Floor($i / 2)) * $h
    $g.DrawImage($imgs[$i], $x, $y, $w, $h)
}
$g.Dispose()
$sheet.Save($Out, [System.Drawing.Imaging.ImageFormat]::Jpeg)
foreach ($im in $imgs) { $im.Dispose() }
$sheet.Dispose()
Write-Output ("saved " + $Out)
