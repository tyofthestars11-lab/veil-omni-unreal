# Whole-desktop photo at PHYSICAL pixels: when a window-rect capture comes back
# black, this says whether the screen itself is black or the window was missed.
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
param([string]$Out = 'D:\VEIL-OMNI\proof\screen.png', [int]$Scale = 2)
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class SS {
  [DllImport("user32.dll")] public static extern bool SetProcessDpiAwarenessContext(IntPtr c);
  [DllImport("user32.dll")] public static extern int GetSystemMetrics(int i);
}
'@
[void][SS]::SetProcessDpiAwarenessContext([IntPtr](-4))
$w = [SS]::GetSystemMetrics(78)   # SM_CXVIRTUALSCREEN
$h = [SS]::GetSystemMetrics(79)   # SM_CYVIRTUALSCREEN
$x = [SS]::GetSystemMetrics(76)
$y = [SS]::GetSystemMetrics(77)
Write-Output "virtual screen ${w}x${h} at $x,$y"

$bmp = New-Object System.Drawing.Bitmap($w, $h)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($x, $y, 0, 0, $bmp.Size)
$g.Dispose()
if ($Scale -gt 1) {
  $sw = [int]($w / $Scale); $sh = [int]($h / $Scale)
  $small = New-Object System.Drawing.Bitmap($sw, $sh)
  $g2 = [System.Drawing.Graphics]::FromImage($small)
  $g2.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
  $g2.DrawImage($bmp, 0, 0, $sw, $sh)
  $g2.Dispose()
  $small.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
  $small.Dispose()
} else {
  $bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
}
$bmp.Dispose()
Write-Output "saved $Out"
