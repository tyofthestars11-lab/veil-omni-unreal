param([string]$In,[string]$Out,[int]$W=900)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

Add-Type -AssemblyName System.Drawing
$s=[System.Drawing.Image]::FromFile($In)
$h=[int]($s.Height*($W/$s.Width))
$b=New-Object System.Drawing.Bitmap($W,$h)
$g=[System.Drawing.Graphics]::FromImage($b)
$g.InterpolationMode='HighQualityBicubic'
$g.DrawImage($s,0,0,$W,$h)
$c=[System.Drawing.Imaging.ImageCodecInfo]::GetImageEncoders() | Where-Object { $_.MimeType -eq 'image/jpeg' }
$pr=New-Object System.Drawing.Imaging.EncoderParameters(1)
$pr.Param[0]=New-Object System.Drawing.Imaging.EncoderParameter([System.Drawing.Imaging.Encoder]::Quality,92)
$b.Save($Out,$c,$pr)
$s.Dispose(); $b.Dispose()
Write-Output "ok $Out"
