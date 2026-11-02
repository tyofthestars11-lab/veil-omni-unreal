# One bounded packet capture around an editor start, so the host behind the
# ssl_client_socket_impl handshake failures is named from the wire instead of
# inferred. TLS ClientHello carries the SNI in clear text.
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Continue'
$etl = 'D:\VEIL-OMNI\proof\cap.etl'
$txt = 'D:\VEIL-OMNI\proof\cap.txt'

Get-Process UnrealEditor -ErrorAction SilentlyContinue | Stop-Process -Force
Start-Sleep -Seconds 3
Remove-Item $etl, $txt -ErrorAction SilentlyContinue
pktmon stop | Out-Null
pktmon start --capture --pkt-size 512 --file-name $etl --comp nics | Out-Null
if (-not (Test-Path $etl)) { 'CAPTURE DID NOT START'; exit 3 }

Start-Process -FilePath 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe' `
  -ArgumentList '"D:\VEIL-OMNI\VEIL.uproject"', '-windowed', '-ResX=1280', '-ResY=720'
Start-Sleep -Seconds 55
pktmon stop | Out-Null

pktmon etl2txt $etl -o $txt | Out-Null
if (Test-Path $txt) { 'TEXT OK ' + (Get-Item $txt).Length } else { 'NO TEXT OUTPUT' }
