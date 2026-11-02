# Crop a rectangle out of a captured frame, at native pixels, for close review.
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
param(
  [string]$In = 'D:\VEIL-OMNI\Saved\Screenshots\WindowsEditor\HighresScreenshot00015.png',
  [string]$Out = 'D:\VEIL-OMNI\proof\crop.png',
  [int]$X = 0, [int]$Y = 0, [int]$W = 700, [int]$H = 320, [int]$Scale = 2
)
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

Add-Type -AssemblyName System.Drawing
$src = [System.Drawing.Image]::FromFile($In)
$W = [int][Math]::Min([int]$W, ([int]$src.Width - [int]$X))
$H = [int][Math]::Min([int]$H, ([int]$src.Height - [int]$Y))
$b = New-Object System.Drawing.Bitmap([int]($W * $Scale), [int]($H * $Scale))
$g = [System.Drawing.Graphics]::FromImage($b)
$g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
$g.DrawImage($src, (New-Object System.Drawing.Rectangle(0, 0, ($W * $Scale), ($H * $Scale))),
  (New-Object System.Drawing.Rectangle($X, $Y, $W, $H)), [System.Drawing.GraphicsUnit]::Pixel)
$g.Dispose(); $src.Dispose()
$b.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$b.Dispose()
Write-Output ("saved {0} {1}x{2} from {3},{4}" -f $Out, $W, $H, $X, $Y)
