# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms,System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class Cz2 {
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, int d, IntPtr e);
  [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, IntPtr extra);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public const uint WHEEL = 0x0800;
  public const byte CTRL = 0x11;
  public const uint KEYUP = 0x0002;
}
'@
$p = Get-Process EpicGamesLauncher | Select-Object -First 1
[Cz2]::SetForegroundWindow($p.MainWindowHandle) | Out-Null
Start-Sleep -Milliseconds 800
$r = New-Object Cz2+RECT
[Cz2]::GetWindowRect($p.MainWindowHandle, [ref]$r) | Out-Null
[Cz2]::SetCursorPos($r.L + 950, $r.T + 450) | Out-Null
[Cz2]::keybd_event([Cz2]::CTRL, 0, 0, [IntPtr]::Zero)
for ($i = 0; $i -lt 5; $i++) {
    [Cz2]::mouse_event([Cz2]::WHEEL, 0, 0, -120, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 200
}
[Cz2]::keybd_event([Cz2]::CTRL, 0, [Cz2]::KEYUP, [IntPtr]::Zero)
Start-Sleep -Seconds 2

$r2 = New-Object Cz2+RECT
[Cz2]::GetWindowRect($p.MainWindowHandle, [ref]$r2) | Out-Null
$bmp = New-Object System.Drawing.Bitmap(($r2.R - $r2.L), ($r2.B - $r2.T))
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
[Cz2]::PrintWindow($p.MainWindowHandle, $hdc, 2) | Out-Null
$g.ReleaseHdc($hdc); $g.Dispose()
$bmp.Save('D:\VEIL-OMNI\proof\launcher-window.png', [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Output 'zoomed and captured'
