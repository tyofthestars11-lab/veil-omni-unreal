# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class LW {
  public delegate bool EP(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EP cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint p);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int Left, Top, Right, Bottom; }
}
'@
$proc = Get-Process UnrealEditor -ErrorAction Stop | Select-Object -First 1
$rows = New-Object System.Collections.ArrayList
$cb = {
  param($h, $l)
  $winPid = 0
  [void][LW]::GetWindowThreadProcessId($h, [ref]$winPid)
  if ($winPid -eq $proc.Id -and [LW]::IsWindowVisible($h)) {
    $len = [LW]::GetWindowTextLength($h)
    $sb = New-Object System.Text.StringBuilder ($len + 1)
    [void][LW]::GetWindowText($h, $sb, $sb.Capacity)
    $r = New-Object LW+RECT
    [void][LW]::GetClientRect($h, [ref]$r)
    $w = $r.Right - $r.Left
    $h2 = $r.Bottom - $r.Top
    if ($w -gt 50 -and $h2 -gt 50) {
      [void]$rows.Add(("[" + $sb.ToString() + "] ${w}x${h2}"))
    }
  }
  return $true
}
$script:proc = $proc
[void][LW]::EnumWindows($cb, [IntPtr]::Zero)
$rows | ForEach-Object { Write-Output $_ }
