$ErrorActionPreference = 'SilentlyContinue'
Remove-Item 'D:\VEIL-OMNI\Saved\Screenshots\lineup.png' -Force
Remove-Item 'D:\VEIL-OMNI\lineup.log' -Force
$exe = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$p = Start-Process -FilePath $exe -ArgumentList 'D:\VEIL-OMNI\VEIL.uproject','/Game/Maps/VEILDistrict','-game','-windowed','-resx=1280','-resy=720','-log','-abslog=D:\VEIL-OMNI\lineup.log','-phisnap','-philineup' -PassThru
Start-Sleep -Seconds 28
Stop-Process -Id $p.Id -Force
Add-Type -AssemblyName System.Drawing
if (Test-Path 'D:\VEIL-OMNI\Saved\Screenshots\lineup.png') {
  $src = [System.Drawing.Image]::FromFile('D:\VEIL-OMNI\Saved\Screenshots\lineup.png')
  $dst = New-Object System.Drawing.Bitmap 900,506
  $g = [System.Drawing.Graphics]::FromImage($dst)
  $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
  $g.DrawImage($src,0,0,900,506); $g.Dispose()
  $dst.Save('D:\VEIL-OMNI\lineup_frame.jpg',[System.Drawing.Imaging.ImageFormat]::Jpeg)
  $src.Dispose(); $dst.Dispose()
}
