# VEIL window helper: capture a window even when it is larger than the screen,
# and click a point inside it by temporarily moving the window onto the screen.
param(
  [ValidateSet('capture','click')] [string]$Action = 'capture',
  [string]$Title = 'Epic Games Launcher',
  [int]$W = 2600,
  [int]$H = 1600,
  [int]$X = 0,
  [int]$Y = 0,
  [string]$Out = 'D:\VEIL-OMNI\proof\launcher-window.png'
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class VW {
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int cx, int cy, uint flags);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, uint d, IntPtr e);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public const uint SWP_SHOWWINDOW = 0x0040;
  public const uint SWP_NOSIZE = 0x0001;
}
'@
$SW = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Width
$SH = [System.Windows.Forms.Screen]::PrimaryScreen.Bounds.Height
$p = Get-Process | Where-Object { $_.MainWindowTitle -like "*$Title*" } | Select-Object -First 1
if (-not $p) { Write-Output "NO WINDOW for '$Title'"; exit 2 }
$h = $p.MainWindowHandle
[VW]::ShowWindow($h, 9) | Out-Null          # SW_RESTORE
[VW]::SetWindowPos($h, [IntPtr]::Zero, -30000, -30000, $W, $H, [VW]::SWP_SHOWWINDOW) | Out-Null
Start-Sleep -Milliseconds 1200
[VW]::SetWindowPos($h, [IntPtr]::Zero, 0, 0, $W, $H, [VW]::SWP_SHOWWINDOW) | Out-Null
Start-Sleep -Milliseconds 1500
$r = New-Object VW+RECT
[VW]::GetWindowRect($h, [ref]$r) | Out-Null
$cw = $r.R - $r.L; $ch = $r.B - $r.T
Write-Output ("window=$cw x $ch at $($r.L),$($r.T)  screen=${SW}x${SH}")

if ($Action -eq 'capture') {
  $bmp = New-Object System.Drawing.Bitmap($cw, $ch)
  $g = [System.Drawing.Graphics]::FromImage($bmp)
  $hdc = $g.GetHdc()
  [VW]::PrintWindow($h, $hdc, 2) | Out-Null
  $g.ReleaseHdc($hdc); $g.Dispose()
  $bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
  $bmp.Dispose()
  Write-Output "saved $Out"
  exit 0
}

# click: window-relative (X,Y) -> screen point; move the window if it is off-screen
$sx = $r.L + $X
$sy = $r.T + $Y
$newL = $r.L; $newT = $r.T
if ($sx -lt 20 -or $sx -gt $SW - 20) { $newL = [int]($SW / 3) - $X }
if ($sy -lt 20 -or $sy -gt $SH - 20) { $newT = [int]($SH / 2) - $Y }
if ($newL -ne $r.L -or $newT -ne $r.T) {
  [VW]::SetWindowPos($h, [IntPtr]::Zero, $newL, $newT, 0, 0, [VW]::SWP_NOSIZE -bor [VW]::SWP_SHOWWINDOW) | Out-Null
  Start-Sleep -Milliseconds 800
  [VW]::GetWindowRect($h, [ref]$r) | Out-Null
  $sx = $r.L + $X; $sy = $r.T + $Y
  Write-Output "moved window to $($r.L),$($r.T)"
}
[VW]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 400
[VW]::SetCursorPos($sx, $sy) | Out-Null
Start-Sleep -Milliseconds 250
[VW]::mouse_event(0x0002, 0, 0, 0, [IntPtr]::Zero)   # left down
Start-Sleep -Milliseconds 90
[VW]::mouse_event(0x0004, 0, 0, 0, [IntPtr]::Zero)   # left up
Start-Sleep -Milliseconds 600
Write-Output "clicked screen $sx,$sy  (window-rel $X,$Y)"
