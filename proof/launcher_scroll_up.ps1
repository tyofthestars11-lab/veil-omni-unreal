# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class Mu {
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, int d, IntPtr e);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public const uint WHEEL = 0x0800;
}
'@
$p = Get-Process EpicGamesLauncher | Select-Object -First 1
[Mu]::SetForegroundWindow($p.MainWindowHandle) | Out-Null
Start-Sleep -Milliseconds 600
$r = New-Object Mu+RECT
[Mu]::GetWindowRect($p.MainWindowHandle, [ref]$r) | Out-Null
$x = $r.L + 950
$y = $r.T + 400
[Mu]::SetCursorPos($x, $y) | Out-Null
for ($i = 0; $i -lt 8; $i++) { [Mu]::mouse_event([Mu]::WHEEL, 0, 0, 120, [IntPtr]::Zero); Start-Sleep -Milliseconds 150 }
Write-Output 'scrolled up'
