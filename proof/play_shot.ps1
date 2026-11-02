# Play photo: bring the running game window on screen and capture exactly it.
# Physical pixels (this desktop runs at 300% scaling).
param(
  [string]$Title = 'VEIL',
  [int]$W = 1280,
  [int]$H = 720,
  [string]$Out = 'D:\VEIL-OMNI\proof\play-photo.png'
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class PS {
  [DllImport("user32.dll")] public static extern bool SetProcessDpiAwarenessContext(IntPtr c);
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int cx, int cy, uint flags);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
}
'@
[void][PS]::SetProcessDpiAwarenessContext([IntPtr](-4))

$p = Get-Process | Where-Object { $_.MainWindowTitle -like "*$Title*" } | Select-Object -First 1
if (-not $p) { Write-Output "NO WINDOW for '$Title'"; exit 2 }
$h = $p.MainWindowHandle
Write-Output ("pid=" + $p.Id + " title='" + $p.MainWindowTitle + "'")
# Leave the game's own window size alone: resizing it under 300% DPI scaling
# squashed the render into a portrait strip and the photo lied about the game.
[PS]::ShowWindow($h, 9) | Out-Null
[PS]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 1800

$r = New-Object PS+RECT
[PS]::GetWindowRect($h, [ref]$r) | Out-Null
if ($r.L -lt 0 -or $r.T -lt 0) { $r.L = 0; $r.T = 0 }
$cw = $r.R - $r.L; $ch = $r.B - $r.T
Write-Output ("window=$cw x $ch at $($r.L),$($r.T)")
$b = New-Object System.Drawing.Bitmap($cw, $ch)
$g = [System.Drawing.Graphics]::FromImage($b)
$g.CopyFromScreen($r.L, $r.T, 0, 0, $b.Size)
$g.Dispose()
$b.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$b.Dispose()
Write-Output "saved $Out"
