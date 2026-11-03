$ErrorActionPreference = 'SilentlyContinue'
Remove-Item 'D:\VEIL-OMNI\Saved\Screenshots\*.png' -Force
Remove-Item 'D:\VEIL-OMNI\talk.log' -Force
$exe = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$p = Start-Process -FilePath $exe -ArgumentList 'D:\VEIL-OMNI\VEIL.uproject','/Game/Maps/VEILDistrict','-game','-windowed','-resx=1280','-resy=720','-log','-abslog=D:\VEIL-OMNI\talk.log','-phisnap','-phitalk' -PassThru
Start-Sleep -Seconds 45
Stop-Process -Id $p.Id -Force
Add-Type -AssemblyName System.Drawing
foreach ($f in Get-ChildItem 'D:\VEIL-OMNI\Saved\Screenshots\*.png') {
  $src = [System.Drawing.Image]::FromFile($f.FullName)
  $dst = New-Object System.Drawing.Bitmap 900,506
  $g = [System.Drawing.Graphics]::FromImage($dst)
  $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
  $g.DrawImage($src,0,0,900,506)
  $g.Dispose()
  $dst.Save(('D:\VEIL-OMNI\talk_' + $f.BaseName + '.jpg'),[System.Drawing.Imaging.ImageFormat]::Jpeg)
  $src.Dispose(); $dst.Dispose()
}
