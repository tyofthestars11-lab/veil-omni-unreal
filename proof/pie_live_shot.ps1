# A floating Play window that never had focus keeps showing the frame it drew
# when it opened. This activates it with a real click, lets it render, then
# captures the live game.
param(
  [string]$Out = "proof/pie_live.png",
  [int]$Width = 1280,
  [int]$Height = 720
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class PL {
  public delegate bool EP(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EP cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll")] public static extern bool MoveWindow(IntPtr h, int x, int y, int w, int hh, bool repaint);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr SetActiveWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr SetFocus(IntPtr h);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref POINT p);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, uint d, IntPtr e);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
}
'@
$found = [IntPtr]::Zero
$foundTitle = ""
$cb = {
  param($h, $l)
  if ([PL]::IsWindowVisible($h)) {
    $len = [PL]::GetWindowTextLength($h)
    $sb = New-Object System.Text.StringBuilder ($len + 1)
    [void][PL]::GetWindowText($h, $sb, $sb.Capacity)
    $t = $sb.ToString()
    if ($t -like "*VEIL Preview*") { $script:found = $h; $script:foundTitle = $t; return $false }
  }
  return $true
}
[void][PL]::EnumWindows($cb, [IntPtr]::Zero)
if ($script:found -eq [IntPtr]::Zero) { Write-Output "no Play window found"; exit 2 }

[void][PL]::MoveWindow($script:found, 0, 0, $Width, $Height, $true)
[void][PL]::BringWindowToTop($script:found)
[void][PL]::SetActiveWindow($script:found)
[void][PL]::SetForegroundWindow($script:found)
[void][PL]::SetFocus($script:found)
Start-Sleep -Milliseconds 600

# the click that gives the window real focus (and makes it draw)
$o = New-Object PL+POINT
[void][PL]::ClientToScreen($script:found, [ref]$o)
[void][PL]::SetCursorPos($o.X + [int]($Width / 2), $o.Y + [int]($Height / 2))
Start-Sleep -Milliseconds 200
[PL]::mouse_event(0x0002, 0, 0, 0, [IntPtr]::Zero)   # left down
Start-Sleep -Milliseconds 80
[PL]::mouse_event(0x0004, 0, 0, 0, [IntPtr]::Zero)   # left up
Start-Sleep -Seconds 4

$r = New-Object PL+RECT
[void][PL]::GetClientRect($script:found, [ref]$r)
$o2 = New-Object PL+POINT
[void][PL]::ClientToScreen($script:found, [ref]$o2)
$w = $r.Right - $r.Left
$hh = $r.Bottom - $r.Top
$bmp = New-Object System.Drawing.Bitmap $w, $hh
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($o2.X, $o2.Y, 0, 0, $bmp.Size)
$bmp.Save((Join-Path (Get-Location) $Out), [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
Write-Output ("captured live '" + $script:foundTitle + "' ${w}x${hh} -> " + $Out)
