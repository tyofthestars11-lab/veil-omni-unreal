$ErrorActionPreference = 'SilentlyContinue'
Remove-Item 'D:\VEIL-OMNI\Saved\Screenshots\WindowsEditor\HighresScreenshot*.png' -Force
Remove-Item 'D:\VEIL-OMNI\lineup.log' -Force
$exe = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$p = Start-Process -FilePath $exe -ArgumentList 'D:\VEIL-OMNI\VEIL.uproject','/Game/Maps/VEILDistrict','-game','-windowed','-resx=1280','-resy=720','-log','-abslog=D:\VEIL-OMNI\lineup.log','-philineup' -PassThru
Start-Sleep -Seconds 32
Stop-Process -Id $p.Id -Force
Add-Type -AssemblyName System.Drawing
$shot = Get-ChildItem 'D:\VEIL-OMNI\Saved\Screenshots\WindowsEditor\HighresScreenshot*.png' | Sort-Object LastWriteTime | Select-Object -Last 1
if ($shot) {
  $src = [System.Drawing.Image]::FromFile($shot.FullName)
  $dst = New-Object System.Drawing.Bitmap 900,506
  $g = [System.Drawing.Graphics]::FromImage($dst)
  $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
  $g.DrawImage($src,0,0,900,506); $g.Dispose()
  $dst.Save('D:\VEIL-OMNI\lineup_frame.jpg',[System.Drawing.Imaging.ImageFormat]::Jpeg)
  $src.Dispose(); $dst.Dispose()
  Write-Output ('converted ' + $shot.Name)
} else { Write-Output 'no HighResShot written' }
