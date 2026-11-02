# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class W4 { [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h); }
'@
$p = Get-Process UnrealEditor | Select-Object -First 1
[W4]::SetForegroundWindow($p.MainWindowHandle) | Out-Null
Start-Sleep -Seconds 1
[System.Windows.Forms.SendKeys]::SendWait([char]96)
Start-Sleep -Seconds 2
[System.Windows.Forms.SendKeys]::SendWait('HighResShot 1920x1080')
Start-Sleep -Milliseconds 600
[System.Windows.Forms.SendKeys]::SendWait('{ENTER}')
Write-Output 'shot command sent'
