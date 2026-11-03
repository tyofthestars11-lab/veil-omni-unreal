$ErrorActionPreference = 'SilentlyContinue'
Remove-Item 'D:\VEIL-OMNI\mount.log' -Force
$exe = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$p = Start-Process -FilePath $exe -ArgumentList 'D:\VEIL-OMNI\VEIL.uproject','/Game/Maps/VEILDistrict','-game','-windowed','-resx=1280','-resy=720','-log','-abslog=D:\VEIL-OMNI\mount.log','-phimount' -PassThru
Start-Sleep -Seconds 45
Stop-Process -Id $p.Id -Force
