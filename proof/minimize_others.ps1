# Minimize every visible top-level window except the Epic Launcher and the shell,
# so the launcher's own UI is unobstructed for a screenshot.
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type @'
using System;
using System.Text;
using System.Collections.Generic;
using System.Runtime.InteropServices;
public class MinW {
  public delegate bool EnumProc(IntPtr h, IntPtr lp);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr lp);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int cmd);
  [DllImport("user32.dll")] public static extern IntPtr GetWindow(IntPtr h, uint cmd);
  public static List<string> Run() {
    var done = new List<string>();
    EnumWindows((h, lp) => {
      if (!IsWindowVisible(h)) return true;
      int len = GetWindowTextLength(h);
      if (len == 0) return true;
      var sb = new StringBuilder(len + 1);
      GetWindowText(h, sb, sb.Capacity);
      string t = sb.ToString();
      uint pid; GetWindowThreadProcessId(h, out pid);
      if (t.Contains("Epic Games Launcher")) return true;
      if (t == "Program Manager" || t == "Taskbar" || t.Contains("Task Manager")) return true;
      ShowWindow(h, 6);   // SW_MINIMIZE
      done.Add(t);
      return true;
    }, IntPtr.Zero);
    return done;
  }
}
'@
$done = [MinW]::Run()
$done | ForEach-Object { Write-Output ("minimized: " + $_) }
Write-Output ("count=" + $done.Count)
