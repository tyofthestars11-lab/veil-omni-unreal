param(
  [string]$In  = 'D:\VEIL-OMNI\proof\tetra_final1.png',
  [string]$Out = 'D:\VEIL-OMNI\proof\tetra_final1_s.jpg',
  [int]$Width = 1000
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

$src = [System.Drawing.Image]::FromFile($In)
$h = [int]($src.Height * $Width / $src.Width)
$bmp = New-Object System.Drawing.Bitmap($Width, $h)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.InterpolationMode = 'HighQualityBicubic'
$g.DrawImage($src, 0, 0, $Width, $h)
$g.Dispose()
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Jpeg)
$src.Dispose()
$bmp.Dispose()
Write-Output ("saved " + $Out)
