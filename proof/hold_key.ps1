# Drive the game by hand: focus the editor window, click into the viewport so it
# owns keyboard focus, then hold one key down for a while (a tap is not a step).
param(
  [string]$Title = 'VEIL - Unreal Editor',
  [ValidateSet('w','a','s','d','v','space','left','right')] [string]$Key = 'w',
  [int]$HoldMs = 900,
  [int]$ClickX = 640,
  [int]$ClickY = 300
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class KB {
  [DllImport("user32.dll")] public static extern void keybd_event(byte k, byte scan, uint f, IntPtr e);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, uint d, IntPtr e);
}
'@
$codes = @{ v = 0x56; w = 0x57; a = 0x41; s = 0x53; d = 0x44; space = 0x20; left = 0x25; right = 0x27 }
$p = Get-Process | Where-Object { $_.MainWindowTitle -like "*$Title*" } | Select-Object -First 1
if (-not $p) { Write-Output "NO WINDOW for '$Title'"; exit 2 }
$h = $p.MainWindowHandle
[KB]::ShowWindow($h, 9) | Out-Null
[KB]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 700
[KB]::SetCursorPos($ClickX, $ClickY) | Out-Null
Start-Sleep -Milliseconds 200
[KB]::mouse_event(0x0002, 0, 0, 0, [IntPtr]::Zero)
Start-Sleep -Milliseconds 80
[KB]::mouse_event(0x0004, 0, 0, 0, [IntPtr]::Zero)
Start-Sleep -Milliseconds 700
$code = [byte]$codes[$Key]
[KB]::keybd_event($code, 0, 0, [IntPtr]::Zero)          # key down
Start-Sleep -Milliseconds $HoldMs
[KB]::keybd_event($code, 0, 2, [IntPtr]::Zero)          # key up
Start-Sleep -Milliseconds 600
Write-Output "held $Key for ${HoldMs}ms after clicking $ClickX,$ClickY"
