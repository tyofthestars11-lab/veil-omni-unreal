# Photograph the blocking modal itself, so its exact words are read rather than
# inferred from an empty static control.
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
param([string]$Out = 'D:\VEIL-OMNI\proof\modal.png')
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class MS {
  public delegate bool EP(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EP cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("shcore.dll")] public static extern int SetProcessDpiAwareness(int value);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
}
'@
try { [MS]::SetProcessDpiAwareness(2) | Out-Null } catch { }

$script:found = [IntPtr]::Zero
$cb = {
  param($h, $l)
  if (-not [MS]::IsWindowVisible($h)) { return $true }
  $cls = New-Object System.Text.StringBuilder 256
  [void][MS]::GetClassName($h, $cls, 256)
  if ($cls.ToString() -eq '#32770') { $script:found = $h; return $false }
  return $true
}
[void][MS]::EnumWindows($cb, [IntPtr]::Zero)
if ($script:found -eq [IntPtr]::Zero) { Write-Output 'NO MODAL'; exit 2 }

$r = New-Object MS+RECT
[void][MS]::GetWindowRect($script:found, [ref]$r)
$w = $r.R - $r.L; $hh = $r.B - $r.T
Write-Output ("modal {0}x{1}" -f $w, $hh)
$bmp = New-Object System.Drawing.Bitmap($w, $hh)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
[void][MS]::PrintWindow($script:found, $hdc, 2)
$g.ReleaseHdc($hdc); $g.Dispose()
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Output ("saved " + $Out)
