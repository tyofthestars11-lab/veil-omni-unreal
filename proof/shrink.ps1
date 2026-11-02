# Downscale a captured frame so it can be shown in the chat.
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
param(
  [string]$In = 'D:\VEIL-OMNI\Saved\Screenshots\WindowsEditor\HighresScreenshot00001.png',
  [string]$Out = 'D:\VEIL-OMNI\proof\shot_s.png',
  [int]$W = 860
)
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

Add-Type -AssemblyName System.Drawing
$src = [System.Drawing.Image]::FromFile($In)
$h = [int]($src.Height * $W / $src.Width)
$b = New-Object System.Drawing.Bitmap($W, $h)
$g = [System.Drawing.Graphics]::FromImage($b)
$g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
$g.DrawImage($src, 0, 0, $W, $h)
$g.Dispose(); $src.Dispose()
$b.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$b.Dispose()
Write-Output ("saved {0} ({1}x{2})" -f $Out, $W, $h)
