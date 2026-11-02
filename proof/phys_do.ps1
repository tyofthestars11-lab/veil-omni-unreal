# Physical-pixel launcher driver.
# This PC is 3840x2160 at 300% scaling, so default (DPI-unaware) processes only see a
# 1280x720 virtualised desktop and the launcher's UI is cropped. Making THIS process
# per-monitor DPI aware gives the real 3840x2160 desktop, the window's real rect, and
# full-resolution screenshots and clicks.
param(
  [ValidateSet('shot','click','scroll')] [string]$Action = 'shot',
  [int]$X = -1,
  [int]$Y = -1,
  [int]$Scroll = 0,
  [string]$Out = 'D:\VEIL-OMNI\proof\phys.png',
  [switch]$Isolate,
  [switch]$NoMove,
  [int]$WW = 2600,
  [int]$WH = 2000
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'

Add-Type @'
using System;
using System.Runtime.InteropServices;
public class PA {
  [DllImport("user32.dll")] public static extern bool SetProcessDpiAwarenessContext(IntPtr ctx);
  [DllImport("shcore.dll")] public static extern int SetProcessDpiAwareness(int v);
  [DllImport("user32.dll")] public static extern bool SetProcessDPIAware();
  [DllImport("user32.dll")] public static extern int GetSystemMetrics(int i);
}
'@
# -4 = PER_MONITOR_AWARE_V2 ; fall back to older APIs on failure
if (-not [PA]::SetProcessDpiAwarenessContext([IntPtr](-4))) {
  try { [void][PA]::SetProcessDpiAwareness(2) } catch { [void][PA]::SetProcessDPIAware() }
}

Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class PD {
  public delegate bool EnumProc(IntPtr h, IntPtr lp);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr lp);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int cx, int cy, uint f);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern bool AttachThreadInput(uint a, uint b, bool f);
  [DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId();
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, uint d, IntPtr e);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public const uint SWP_SHOWWINDOW = 0x0040;
  public const uint SWP_NOSIZE = 0x0001;
  public const uint SWP_NOMOVE = 0x0002;
  public static IntPtr BiggestVisible(uint pid) {
    IntPtr best = IntPtr.Zero; long bestArea = 0;
    IntPtr any = IntPtr.Zero; long anyArea = 0;
    EnumWindows((h, lp) => {
      uint p; GetWindowThreadProcessId(h, out p);
      if (p != pid) return true;
      RECT r; GetWindowRect(h, out r);
      long a = (long)(r.R - r.L) * (r.B - r.T);
      if (a > anyArea) { anyArea = a; any = h; }
      if (IsWindowVisible(h) && a > bestArea) { bestArea = a; best = h; }
      return true;
    }, IntPtr.Zero);
    // the launcher drops to the tray (hidden) when it loses focus; wake it back up
    if (best == IntPtr.Zero && any != IntPtr.Zero) {
      ShowWindow(any, 5);   // SW_SHOW
      ShowWindow(any, 9);   // SW_RESTORE
      return any;
    }
    return best;
  }
  public static void MinimizeOthers(IntPtr keep, uint keepPid) {
    EnumWindows((h, lp) => {
      if (h == keep || !IsWindowVisible(h)) return true;
      uint p; GetWindowThreadProcessId(h, out p);
      if (p == keepPid) return true;
      int len = GetWindowTextLength(h);
      if (len == 0) return true;
      var sb = new StringBuilder(len + 1);
      GetWindowText(h, sb, sb.Capacity);
      string t = sb.ToString();
      if (t == "Program Manager" || t == "Taskbar" || t == "SystemTray") return true;
      ShowWindow(h, 6);
      return true;
    }, IntPtr.Zero);
  }
}
'@
$ScrW = [int][PA]::GetSystemMetrics(0); $ScrH = [int][PA]::GetSystemMetrics(1)
Write-Output "physical desktop = ${ScrW}x${ScrH}"

$p = Get-Process EpicGamesLauncher | Select-Object -First 1
if (-not $p) { Write-Output 'NO LAUNCHER'; exit 2 }
$h = [PD]::BiggestVisible([uint32]$p.Id)
if ($h -eq [IntPtr]::Zero) { Write-Output 'NO VISIBLE WINDOW'; exit 2 }
if ([PD]::IsIconic($h)) { [PD]::ShowWindow($h, 9) | Out-Null; Start-Sleep -Milliseconds 1200 }

if (-not $NoMove) {
  [PD]::SetWindowPos($h, [IntPtr]::Zero, 0, 0, [Math]::Min($WW, $ScrW), [Math]::Min($WH, $ScrH), [PD]::SWP_SHOWWINDOW) | Out-Null
  Start-Sleep -Milliseconds 1500
}

if ($Isolate) {
  [PD]::MinimizeOthers($h, [uint32]$p.Id) | Out-Null
  Start-Sleep -Milliseconds 1000
}

[uint32]$fgPid = 0
$fg = [PD]::GetForegroundWindow()
$fgThread = [PD]::GetWindowThreadProcessId($fg, [ref]$fgPid)
$cur = [PD]::GetCurrentThreadId()
[PD]::AttachThreadInput($cur, $fgThread, $true) | Out-Null
[PD]::BringWindowToTop($h) | Out-Null
[PD]::SetForegroundWindow($h) | Out-Null
[PD]::AttachThreadInput($cur, $fgThread, $false) | Out-Null
Start-Sleep -Milliseconds 900

$r = New-Object PD+RECT
[PD]::GetWindowRect($h, [ref]$r) | Out-Null
$cloak = 0
Add-Type @'
using System;using System.Runtime.InteropServices;
public class DW2 { [DllImport("dwmapi.dll")] public static extern int DwmGetWindowAttribute(IntPtr h, int a, out int v, int s); }
'@ -ErrorAction SilentlyContinue
[void][DW2]::DwmGetWindowAttribute($h, 14, [ref]$cloak, 4)
Write-Output ("cloaked=" + $cloak)
Write-Output ("window rect = $($r.L),$($r.T) .. $($r.R),$($r.B)  size " + ($r.R - $r.L) + 'x' + ($r.B - $r.T))

if ($Action -eq 'scroll' -and $Scroll -ne 0) {
  [PD]::SetCursorPos($X, $Y) | Out-Null
  Start-Sleep -Milliseconds 400
  $dir = if ($Scroll -gt 0) { [uint32]120 } else { [uint32]4294967176 }
  for ($i = 0; $i -lt [Math]::Abs($Scroll); $i++) {
    [PD]::mouse_event(0x0800, 0, 0, $dir, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 200
  }
  Start-Sleep -Milliseconds 1200
  Write-Output "scrolled $Scroll at $X,$Y"
}

if ($Action -eq 'click' -and $X -ge 0 -and $Y -ge 0) {
  [PD]::SetCursorPos($X, $Y) | Out-Null
  Start-Sleep -Milliseconds 350
  [PD]::mouse_event(0x0002, 0, 0, 0, [IntPtr]::Zero)
  Start-Sleep -Milliseconds 110
  [PD]::mouse_event(0x0004, 0, 0, 0, [IntPtr]::Zero)
  Start-Sleep -Milliseconds 2000
  Write-Output "clicked $X,$Y"
}

Start-Sleep -Milliseconds 800
Write-Output "capture ${ScrW}x${ScrH}"
$b = New-Object System.Drawing.Bitmap([int]$ScrW, [int]$ScrH)
$g = [System.Drawing.Graphics]::FromImage($b)
$g.CopyFromScreen(0, 0, 0, 0, $b.Size)
$g.Dispose()
$b.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$b.Dispose()
Write-Output "saved $Out (${ScrW}x${ScrH})"
