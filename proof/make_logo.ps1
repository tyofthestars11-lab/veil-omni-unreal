# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing

$W = 1024
$bmp = New-Object System.Drawing.Bitmap($W, $W)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
$g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit
$g.Clear([System.Drawing.Color]::FromArgb(11, 13, 18))

# soft gold vignette
$ellipse = New-Object System.Drawing.Drawing2D.GraphicsPath
$ellipse.AddEllipse(40, 30, $W - 80, $W - 60)
$vbrush = New-Object System.Drawing.Drawing2D.PathGradientBrush($ellipse)
$vbrush.CenterColor = [System.Drawing.Color]::FromArgb(26, 214, 178, 90)
$vbrush.SurroundColors = @([System.Drawing.Color]::FromArgb(0, 11, 13, 18))
$g.FillPath($vbrush, $ellipse)

$phi = (1 + [math]::Sqrt(5)) / 2
$b = [math]::Log($phi) / ([math]::PI / 2)
$cx = 512.0
$cy = 424.0
$rMax = 296.0
$thetaMax = 4 * [math]::PI
$a = $rMax / [math]::Exp($b * $thetaMax)

$pts = New-Object 'System.Collections.Generic.List[System.Drawing.PointF]'
for ($i = 0; $i -le 1600; $i++) {
    $th = $thetaMax * $i / 1600.0
    $r = $a * [math]::Exp($b * $th)
    $pts.Add((New-Object System.Drawing.PointF([float]($cx + $r * [math]::Cos($th)), [float]($cy + $r * [math]::Sin($th)))))
}

# glow then core stroke
$glow = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(48, 224, 184, 90), 22)
$glow.LineJoin = 'Round'
$g.DrawLines($glow, $pts.ToArray())
$core = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(255, 232, 199, 122), 5)
$core.LineJoin = 'Round'
$g.DrawLines($core, $pts.ToArray())

# four tetra anchors on the spiral's outer turn
$dotBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(255, 255, 226, 160))
foreach ($k in 0..3) {
    $th = $thetaMax - ($k * [math]::PI / 2.0)
    $r = $a * [math]::Exp($b * $th)
    $x = $cx + $r * [math]::Cos($th)
    $y = $cy + $r * [math]::Sin($th)
    $g.FillEllipse($dotBrush, [float]($x - 7), [float]($y - 7), 14, 14)
}

function Draw-Tracked {
    param($g, $text, $font, $brush, $centerX, $topY, $tracking)
    $widths = @()
    $total = 0.0
    foreach ($ch in $text.ToCharArray()) {
        $s = [string]$ch
        $w = $g.MeasureString($s, $font).Width
        $widths += $w
        $total += $w + $tracking
    }
    $total -= $tracking
    $x = $centerX - $total / 2.0
    for ($i = 0; $i -lt $text.Length; $i++) {
        $g.DrawString([string]$text[$i], $font, $brush, [float]$x, [float]$topY)
        $x += $widths[$i] + $tracking
    }
}

$white = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(255, 240, 240, 245))
$gold = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(255, 232, 199, 122))

$titleFont = New-Object System.Drawing.Font('Segoe UI', 128, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
$subFont = New-Object System.Drawing.Font('Segoe UI', 27, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)

Draw-Tracked -g $g -text 'VEIL' -font $titleFont -brush $white -centerX 512 -topY 726 -tracking 26
Draw-Tracked -g $g -text 'THE LAST SIGNAL' -font $subFont -brush $gold -centerX 512 -topY 892 -tracking 9

# thin rules above and below the subtitle, sized by phi
$rulePen = New-Object System.Drawing.Pen([System.Drawing.Color]::FromArgb(140, 232, 199, 122), 2)
$g.DrawLine($rulePen, 512 - 250, 872, 512 + 250, 872)

$bmp.Save('D:\VEIL-OMNI\Content\Branding\VEIL_Logo.png', [System.Drawing.Imaging.ImageFormat]::Png)

# multi-size ICO (PNG-compressed entries)
$sizes = @(256, 128, 64, 48, 32, 16)
$pngBytes = @()
foreach ($s in $sizes) {
    $small = New-Object System.Drawing.Bitmap($s, $s)
    $sg = [System.Drawing.Graphics]::FromImage($small)
    $sg.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $sg.DrawImage($bmp, 0, 0, $s, $s)
    $ms = New-Object System.IO.MemoryStream
    $small.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
    $pngBytes += , $ms.ToArray()
    $ms.Dispose(); $sg.Dispose(); $small.Dispose()
}

$icoPath = 'D:\VEIL-OMNI\Content\Branding\VEIL_Logo.ico'
$fs = [System.IO.File]::Create($icoPath)
$bw = New-Object System.IO.BinaryWriter($fs)
$bw.Write([UInt16]0); $bw.Write([UInt16]1); $bw.Write([UInt16]$sizes.Count)
$offset = 6 + 16 * $sizes.Count
for ($i = 0; $i -lt $sizes.Count; $i++) {
    $s = $sizes[$i]
    $bw.Write([byte]($(if ($s -ge 256) { 0 } else { $s })))
    $bw.Write([byte]($(if ($s -ge 256) { 0 } else { $s })))
    $bw.Write([byte]0); $bw.Write([byte]0)
    $bw.Write([UInt16]1); $bw.Write([UInt16]32)
    $bw.Write([UInt32]$pngBytes[$i].Length)
    $bw.Write([UInt32]$offset)
    $offset += $pngBytes[$i].Length
}
foreach ($pb in $pngBytes) { $bw.Write($pb) }
$bw.Flush(); $bw.Close(); $fs.Close()

$g.Dispose(); $bmp.Dispose()
Write-Output 'logo written'
