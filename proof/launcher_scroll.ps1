# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class Ms {
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, int d, IntPtr e);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public const uint WHEEL = 0x0800;
  public const uint LEFTDOWN = 0x0002;
  public const uint LEFTUP = 0x0004;
  public static void ClickAt(int x, int y) {
    SetCursorPos(x, y);
    mouse_event(LEFTDOWN, 0, 0, 0, IntPtr.Zero);
    System.Threading.Thread.Sleep(80);
    mouse_event(LEFTUP, 0, 0, 0, IntPtr.Zero);
  }
}
'@
$p = Get-Process EpicGamesLauncher | Select-Object -First 1
[Ms]::SetForegroundWindow($p.MainWindowHandle) | Out-Null
Start-Sleep -Milliseconds 800
$r = New-Object Ms+RECT
[Ms]::GetWindowRect($p.MainWindowHandle, [ref]$r) | Out-Null
Write-Output ("window " + $r.L + "," + $r.T + " " + ($r.R - $r.L) + "x" + ($r.B - $r.T))
# hover over the rail and scroll down
$x = $r.L + 340
$y = $r.T + 560
[Ms]::SetCursorPos($x, $y) | Out-Null
for ($i = 0; $i -lt 5; $i++) { [Ms]::mouse_event([Ms]::WHEEL, 0, 0, -120, [IntPtr]::Zero); Start-Sleep -Milliseconds 250 }
Write-Output ("scrolled at " + $x + "," + $y)
