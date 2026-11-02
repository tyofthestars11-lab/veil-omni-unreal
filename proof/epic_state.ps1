# Final state check: any Epic/Unreal process, any visible Epic window, and who owns the one zombie.

# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

Add-Type @'
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
public class W {
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  public static List<string> Visible() {
    var outp = new List<string>();
    EnumWindows((h, l) => {
      if (!IsWindowVisible(h)) return true;
      var t = new StringBuilder(512); GetWindowText(h, t, 512);
      var c = new StringBuilder(256); GetClassName(h, c, 256);
      uint pid; GetWindowThreadProcessId(h, out pid);
      if (t.Length > 0) outp.Add(pid + "|" + c + "|" + t);
      return true;
    }, IntPtr.Zero);
    return outp;
  }
}
'@

Write-Output '=== Epic / Unreal / EOS processes ==='
$procs = Get-Process -ErrorAction SilentlyContinue |
    Where-Object { $_.ProcessName -like 'Epic*' -or $_.ProcessName -like 'Unreal*' -or $_.ProcessName -like 'EOS*' }
if ($procs) {
    foreach ($p in $procs) {
        Write-Output ("  pid={0} name={1} exited={2} window={3}" -f $p.Id, $p.ProcessName, $p.HasExited, $p.MainWindowHandle)
    }
} else {
    Write-Output '  none'
}

Write-Output ''
Write-Output '=== Visible windows mentioning Epic or Unreal ==='
$hits = [W]::Visible() | Where-Object { $_ -match '(?i)epic|unreal' }
if ($hits) { $hits | ForEach-Object { Write-Output ("  " + $_) } }
else { Write-Output '  none on screen' }

Write-Output ''
Write-Output '=== Parent of lingering pid 56632 ==='
$cim = Get-CimInstance Win32_Process -Filter "ProcessId=56632" -ErrorAction SilentlyContinue
if ($cim) {
    $par = Get-CimInstance Win32_Process -Filter ("ProcessId=" + $cim.ParentProcessId) -ErrorAction SilentlyContinue
    Write-Output ("  parent pid={0} name={1}" -f $cim.ParentProcessId, $(if ($par) { $par.Name } else { '(gone)' }))
} else {
    Write-Output '  pid 56632 no longer in the process table'
}
