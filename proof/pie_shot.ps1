# Find the floating Play window, give it a real size, and shoot the game itself.
param(
  [string]$Out = "proof/pie_game.png",
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
public class PS {
  public delegate bool EP(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EP cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll")] public static extern bool MoveWindow(IntPtr h, int x, int y, int w, int hh, bool repaint);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref POINT p);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
}
'@
$found = [IntPtr]::Zero
$foundTitle = ""
$cb = {
  param($h, $l)
  if ([PS]::IsWindowVisible($h)) {
    $len = [PS]::GetWindowTextLength($h)
    $sb = New-Object System.Text.StringBuilder ($len + 1)
    [void][PS]::GetWindowText($h, $sb, $sb.Capacity)
    $t = $sb.ToString()
    if ($t -like "*VEIL Preview*") { $script:found = $h; $script:foundTitle = $t; return $false }
  }
  return $true
}
[void][PS]::EnumWindows($cb, [IntPtr]::Zero)
if ($script:found -eq [IntPtr]::Zero) { Write-Output "no Play window found"; exit 2 }

[void][PS]::MoveWindow($script:found, 40, 40, $Width, $Height, $true)
[void][PS]::BringWindowToTop($script:found)
[void][PS]::SetForegroundWindow($script:found)
Start-Sleep -Seconds 3

$r = New-Object PS+RECT
[void][PS]::GetClientRect($script:found, [ref]$r)
$o = New-Object PS+POINT
[void][PS]::ClientToScreen($script:found, [ref]$o)
$w = $r.Right - $r.Left
$hh = $r.Bottom - $r.Top
$bmp = New-Object System.Drawing.Bitmap $w, $hh
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($o.X, $o.Y, 0, 0, $bmp.Size)
$bmp.Save((Join-Path (Get-Location) $Out), [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
Write-Output ("captured '" + $script:foundTitle + "' ${w}x${hh} -> " + $Out)
