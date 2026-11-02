param(
  [int]$X = 0,
  [int]$Y = 0,
  [int]$WaitMs = 3000,
  [string]$Out = 'D:\VEIL-OMNI\proof\epic-nav.png'
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class EN {
  public delegate bool EnumProc(IntPtr h, IntPtr lp);
  [DllImport("user32.dll")] public static extern bool SetProcessDpiAwarenessContext(IntPtr c);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr lp);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern bool AttachThreadInput(uint a, uint b, bool f);
  [DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId();
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, uint d, IntPtr e);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public static IntPtr Biggest(uint pid) {
    IntPtr best = IntPtr.Zero; long area = 0;
    EnumWindows((h, lp) => {
      uint p; GetWindowThreadProcessId(h, out p);
      if (p != pid || !IsWindowVisible(h)) return true;
      RECT r; GetWindowRect(h, out r);
      long a = (long)(r.R - r.L) * (r.B - r.T);
      if (a > area) { area = a; best = h; }
      return true;
    }, IntPtr.Zero);
    return best;
  }
}
'@
[void][EN]::SetProcessDpiAwarenessContext([IntPtr](-4))
$p = Get-Process EpicGamesLauncher | Select-Object -First 1
$h = [EN]::Biggest([uint32]$p.Id)
if ($h -eq [IntPtr]::Zero) { Write-Output 'NO WINDOW'; exit 2 }

$fg = [EN]::GetForegroundWindow()
$fgPid = 0
$fgThread = [EN]::GetWindowThreadProcessId($fg, [ref]$fgPid)
$myThread = [EN]::GetCurrentThreadId()
[void][EN]::AttachThreadInput($myThread, $fgThread, $true)
[EN]::BringWindowToTop($h) | Out-Null
[EN]::SetForegroundWindow($h) | Out-Null
[void][EN]::AttachThreadInput($myThread, $fgThread, $false)
Start-Sleep -Milliseconds 700

$r = New-Object EN+RECT
[EN]::GetWindowRect($h, [ref]$r) | Out-Null
$sx = $r.L + $X; $sy = $r.T + $Y
[EN]::SetCursorPos($sx, $sy) | Out-Null
Start-Sleep -Milliseconds 350
[EN]::mouse_event(0x0002, 0, 0, 0, [IntPtr]::Zero)
Start-Sleep -Milliseconds 120
[EN]::mouse_event(0x0004, 0, 0, 0, [IntPtr]::Zero)
Write-Output "clicked $X,$Y (screen $sx,$sy)"
Start-Sleep -Milliseconds $WaitMs

$r2 = New-Object EN+RECT
[EN]::GetWindowRect($h, [ref]$r2) | Out-Null
$cw = $r2.R - $r2.L; $ch = $r2.B - $r2.T
$bmp = New-Object System.Drawing.Bitmap($cw, $ch)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
[EN]::PrintWindow($h, $hdc, 2) | Out-Null
$g.ReleaseHdc($hdc); $g.Dispose()
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Output "saved $Out (${cw}x${ch})"
