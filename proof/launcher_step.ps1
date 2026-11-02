# VEIL launcher step: move the Launcher window, force it to the foreground,
# screenshot the desktop, and optionally click a window-relative point.
# One process does focus + shot + click so another app cannot steal focus between steps.
param(
  [int]$WX = 0,
  [int]$WY = -100,
  [int]$WW = 1600,
  [int]$WH = 1000,
  [int]$ClickX = -1,
  [int]$ClickY = -1,
  [switch]$ClickOnly,
  [string]$Out = 'D:\VEIL-OMNI\proof\step.png',
  [int]$WaitMs = 2500
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class LS {
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int cx, int cy, uint f);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId();
  [DllImport("user32.dll")] public static extern bool AttachThreadInput(uint a, uint b, bool f);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, uint d, IntPtr e);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public const uint SWP_SHOWWINDOW = 0x0040;
  public const uint SWP_NOSIZE = 0x0001;
}
'@
$p = Get-Process EpicGamesLauncher | Select-Object -First 1
if (-not $p) { Write-Output 'NO LAUNCHER'; exit 2 }
$h = $p.MainWindowHandle
[LS]::ShowWindow($h, 9) | Out-Null
if (-not $ClickOnly) {
  [LS]::SetWindowPos($h, [IntPtr]::Zero, $WX, $WY, $WW, $WH, [LS]::SWP_SHOWWINDOW) | Out-Null
  Start-Sleep -Milliseconds 1800
}
# focus
[uint32]$fgPid = 0
$fg = [LS]::GetForegroundWindow()
$fgThread = [LS]::GetWindowThreadProcessId($fg, [ref]$fgPid)
$cur = [LS]::GetCurrentThreadId()
[LS]::AttachThreadInput($cur, $fgThread, $true) | Out-Null
[LS]::BringWindowToTop($h) | Out-Null
[LS]::SetForegroundWindow($h) | Out-Null
[LS]::AttachThreadInput($cur, $fgThread, $false) | Out-Null
Start-Sleep -Milliseconds 1200
$r = New-Object LS+RECT
[LS]::GetWindowRect($h, [ref]$r) | Out-Null
Write-Output ("RECT $($r.L),$($r.T) " + ($r.R - $r.L) + 'x' + ($r.B - $r.T))

if (-not $ClickOnly) {
  $b = New-Object System.Drawing.Bitmap 1280, 720
  $g = [System.Drawing.Graphics]::FromImage($b)
  $g.CopyFromScreen(0, 0, 0, 0, $b.Size)
  $g.Dispose()
  $b.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
  $b.Dispose()
  Write-Output "saved $Out"
}

if ($ClickX -ge 0 -and $ClickY -ge 0) {
  $sx = $r.L + $ClickX
  $sy = $r.T + $ClickY
  [LS]::SetCursorPos($sx, $sy) | Out-Null
  Start-Sleep -Milliseconds 350
  [LS]::mouse_event(0x0002, 0, 0, 0, [IntPtr]::Zero)
  Start-Sleep -Milliseconds 100
  [LS]::mouse_event(0x0004, 0, 0, 0, [IntPtr]::Zero)
  Start-Sleep -Milliseconds 1500
  Write-Output "clicked screen $sx,$sy (win-rel $ClickX,$ClickY)"
  $b = New-Object System.Drawing.Bitmap 1280, 720
  $g = [System.Drawing.Graphics]::FromImage($b)
  $g.CopyFromScreen(0, 0, 0, 0, $b.Size)
  $g.Dispose()
  $b.Save($Out.Replace('.png', '-after.png'), [System.Drawing.Imaging.ImageFormat]::Png)
  $b.Dispose()
  Write-Output "saved $($Out.Replace('.png','-after.png'))"
}
