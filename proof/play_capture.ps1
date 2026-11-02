# Capture a game window at a sane size: resize it to fit the screen first,
# then PrintWindow (works even when the window is partly offscreen).
param(
  [string]$Title = 'VEIL Preview',
  [int]$W = 1264,
  [int]$H = 681,
  [string]$Out = 'D:\VEIL-OMNI\proof\play_shot.png'
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class PW {
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int cx, int cy, uint flags);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
}
'@
$p = Get-Process | Where-Object { $_.MainWindowTitle -like "*$Title*" } | Select-Object -First 1
if (-not $p) { Write-Output "NO WINDOW for '$Title'"; exit 2 }
$h = $p.MainWindowHandle
[PW]::ShowWindow($h, 9) | Out-Null
[PW]::SetWindowPos($h, [IntPtr]::Zero, 0, 0, $W, $H, 0x0040) | Out-Null
Start-Sleep -Milliseconds 1800
$r = New-Object PW+RECT
[PW]::GetClientRect($h, [ref]$r) | Out-Null
$cw = $r.R - $r.L; $ch = $r.B - $r.T
$bmp = New-Object System.Drawing.Bitmap($cw, $ch)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
[PW]::PrintWindow($h, $hdc, 2) | Out-Null
$g.ReleaseHdc($hdc); $g.Dispose()
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Output "captured $cw x $ch -> $Out"
