# Capture the Play-In-Editor window itself - the world the runner is standing in.
# The editor viewport renders the editor world, so a viewport capture during Play
# shows an empty plaza. This shoots the window the game is actually drawn in.
param(
  [string]$Out = "proof/pie_window.png"
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'

Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class PW {
  public delegate bool EP(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EP cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint p);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool ClientToScreen(IntPtr h, ref POINT p);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
  [StructLayout(LayoutKind.Sequential)] public struct POINT { public int X, Y; }
}
'@

Add-Type -AssemblyName System.Drawing

$proc = Get-Process UnrealEditor -ErrorAction Stop | Select-Object -First 1
$best = [IntPtr]::Zero
$bestArea = 0
$bestTitle = ""
$cb = {
  param($h, $l)
  $winPid = 0
  [void][PW]::GetWindowThreadProcessId($h, [ref]$winPid)
  if ($winPid -eq $proc.Id -and [PW]::IsWindowVisible($h)) {
    $len = [PW]::GetWindowTextLength($h)
    $sb = New-Object System.Text.StringBuilder ($len + 1)
    [void][PW]::GetWindowText($h, $sb, $sb.Capacity)
    $title = $sb.ToString()
    $r = New-Object PW+RECT
    [void][PW]::GetClientRect($h, [ref]$r)
    $area = ($r.Right - $r.Left) * ($r.Bottom - $r.Top)
    # the Play window is a real client-size window that is not the tiny docked tabs
    if ($area -gt 200000 -and $area -gt $script:bestArea) {
      $script:bestArea = $area
      $script:best = $h
      $script:bestTitle = $title
    }
  }
  return $true
}
$script:bestArea = 0
[void][PW]::EnumWindows($cb, [IntPtr]::Zero)

if ($script:best -eq [IntPtr]::Zero) { Write-Output "no PIE window found"; exit 2 }

$rect = New-Object PW+RECT
[void][PW]::GetClientRect($script:best, [ref]$rect)
$origin = New-Object PW+POINT
[void][PW]::ClientToScreen($script:best, [ref]$origin)
$w = $rect.Right - $rect.Left
$hgt = $rect.Bottom - $rect.Top

[void][PW]::SetForegroundWindow($script:best)
Start-Sleep -Milliseconds 700

$bmp = New-Object System.Drawing.Bitmap $w, $hgt
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($origin.X, $origin.Y, 0, 0, $bmp.Size)
$bmp.Save((Join-Path (Get-Location) $Out), [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
Write-Output ("captured '" + $script:bestTitle + "' ${w}x${hgt} -> " + $Out)
