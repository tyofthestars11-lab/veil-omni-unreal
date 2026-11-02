# List every top-level window belonging to the Epic Launcher: handle, visibility, size, position, title.
param([string]$Exe = 'EpicGamesLauncher')
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type @'
using System;
using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;
public class TW {
  public delegate bool EnumProc(IntPtr h, IntPtr lp);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr lp);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public static List<string> List(uint want) {
    var res = new List<string>();
    EnumWindows((h, lp) => {
      uint pid; GetWindowThreadProcessId(h, out pid);
      if (pid != want) return true;
      int len = GetWindowTextLength(h);
      var sb = new StringBuilder(len + 1);
      GetWindowText(h, sb, sb.Capacity);
      RECT r; GetWindowRect(h, out r);
      int w = r.R - r.L, ht = r.B - r.T;
      res.Add(string.Format("{0}|{1}|{2}x{3}|{4},{5}|{6}",
        h.ToInt64(), IsWindowVisible(h) ? "V" : "H", w, ht, r.L, r.T, sb.ToString()));
      return true;
    }, IntPtr.Zero);
    return res;
  }
}
'@
Get-Process $Exe -ErrorAction SilentlyContinue | ForEach-Object {
  Write-Output ("pid=" + $_.Id)
  [TW]::List([uint32]$_.Id) | ForEach-Object { Write-Output $_ }
}
