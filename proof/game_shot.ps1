# Capture the running VEIL game window exactly as it is - no move, no resize.
param(
  [string]$Title = 'VEIL',
  [string]$Out = 'D:\VEIL-OMNI\proof\game_morphfix_play.png'
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class WS {
  // Without this the process is DPI-virtualized: GetWindowRect reports the window
  // divided by the display scale (250% here), so the capture came out 432x253
  // instead of the real 1066x600. Declaring DPI awareness fixes the measurement.
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("shcore.dll")] public static extern int SetProcessDpiAwareness(int value);
  // The game process can own BOTH a console window and the UnrealWindow, and
  // MainWindowTitle then reports the console. Find the UnrealWindow by class.
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr l);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, System.Text.StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  public static IntPtr FindUnrealWindow(int pid) {
    IntPtr found = IntPtr.Zero;
    EnumWindows((h, l) => {
      uint p; GetWindowThreadProcessId(h, out p);
      if (p != (uint)pid) return true;
      var c = new System.Text.StringBuilder(256); GetClassName(h, c, 256);
      if (c.ToString() == "UnrealWindow") { found = h; return false; }
      return true;
    }, IntPtr.Zero);
    return found;
  }
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
}
'@

try { [WS]::SetProcessDpiAwareness(2) | Out-Null } catch { [WS]::SetProcessDPIAware() | Out-Null }

$p = Get-Process | Where-Object { $_.MainWindowTitle -like "*$Title*" -and $_.MainWindowHandle -ne 0 } | Select-Object -First 1
$h = [IntPtr]::Zero
if ($p) { $h = $p.MainWindowHandle }
if ($h -eq [IntPtr]::Zero) {
    foreach ($proc in (Get-Process UnrealEditor -ErrorAction SilentlyContinue)) {
        $cand = [WS]::FindUnrealWindow($proc.Id)
        if ($cand -ne [IntPtr]::Zero) { $h = $cand; $p = $proc; break }
    }
}
if ($h -eq [IntPtr]::Zero) { Write-Output "NO WINDOW for '$Title'"; exit 2 }
Write-Output ("window pid={0} handle={1} title='{2}'" -f $p.Id, $h, $p.MainWindowTitle)
$r = New-Object WS+RECT
[WS]::GetWindowRect($h, [ref]$r) | Out-Null
$w = $r.R - $r.L
$hh = $r.B - $r.T
Write-Output ("size {0}x{1}" -f $w, $hh)
if ($w -le 0 -or $hh -le 0) { Write-Output 'BAD RECT'; exit 3 }

$bmp = New-Object System.Drawing.Bitmap($w, $hh)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
[WS]::PrintWindow($h, $hdc, 2) | Out-Null   # PW_RENDERFULLCONTENT -> captures D3D content
$g.ReleaseHdc($hdc)
$g.Dispose()
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Output ("saved " + $Out)
