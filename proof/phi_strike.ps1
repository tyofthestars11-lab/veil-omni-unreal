# Tap the strike key (F) into the running VEIL game window, so the live game
# crosses a temporal field the same way a player does - no forced state, no flag.
# The window is found by the UnrealEditor process and its "UnrealWindow" class:
# matching by title alone can land on this shell's console instead.
param(
  [int]$Presses = 16,
  [int]$GapMs = 400
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class ST {
  public delegate bool EP(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EP cb, IntPtr l);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll")] public static extern void keybd_event(byte k, byte scan, uint f, IntPtr e);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, uint d, IntPtr e);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public static IntPtr Game(int pid) {
    IntPtr found = IntPtr.Zero;
    EnumWindows((h, l) => {
      uint p; GetWindowThreadProcessId(h, out p);
      if (p != (uint)pid) return true;
      var c = new StringBuilder(256); GetClassName(h, c, 256);
      if (c.ToString() == "UnrealWindow") { found = h; return false; }
      return true;
    }, IntPtr.Zero);
    return found;
  }
}
'@
$proc = Get-Process UnrealEditor -ErrorAction SilentlyContinue |
  Where-Object { $_.MainWindowTitle -like '*VEIL*' } | Select-Object -First 1
if (-not $proc) { Write-Output 'NO VEIL UnrealEditor process'; exit 2 }
$h = [ST]::Game($proc.Id)
if ($h -eq [IntPtr]::Zero) { Write-Output 'NO UnrealWindow for that process'; exit 3 }
[void][ST]::ShowWindow($h, 9)
[void][ST]::BringWindowToTop($h)
[void][ST]::SetForegroundWindow($h)
# A click inside the viewport first: Windows will not hand keyboard focus to a
# game window that has never been activated, and the presses would vanish.
$r = New-Object ST+RECT
[void][ST]::GetWindowRect($h, [ref]$r)
[void][ST]::SetCursorPos([int](($r.L + $r.R) / 2), [int](($r.T + $r.B) / 2))
Start-Sleep -Milliseconds 300
[ST]::mouse_event(0x0002, 0, 0, 0, [IntPtr]::Zero)
Start-Sleep -Milliseconds 90
[ST]::mouse_event(0x0004, 0, 0, 0, [IntPtr]::Zero)
Start-Sleep -Milliseconds 1200
for ($i = 1; $i -le $Presses; $i++) {
  [ST]::keybd_event(0x46, 0, 0, [IntPtr]::Zero)   # F down
  Start-Sleep -Milliseconds 60
  [ST]::keybd_event(0x46, 0, 2, [IntPtr]::Zero)   # F up
  Start-Sleep -Milliseconds $GapMs
}
Write-Output ("pid {0} window {1}: sent {2} strike presses" -f $proc.Id, $h, $Presses)
