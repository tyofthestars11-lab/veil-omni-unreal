# VEIL launcher driver: find the Launcher's real window by enumeration, place it,
# force it topmost + foreground, optionally scroll/click, and screenshot the desktop.
param(
  [int]$WX = 0,
  [int]$WY = 0,
  [int]$WW = 1280,
  [int]$WH = 980,
  [int]$Scroll = 0,
  [int]$ScrollX = 1000,
  [int]$ScrollY = 380,
  [int]$ClickX = -1,
  [int]$ClickY = -1,
  [string]$Out = 'D:\VEIL-OMNI\proof\do.png',
  [int]$SettleMs = 1200,
  [switch]$Isolate
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class LD {
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
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public const uint SWP_SHOWWINDOW = 0x0040;
  public const uint SWP_NOSIZE = 0x0001;
  public const uint SWP_NOMOVE = 0x0002;
  // the launcher's real UI is the biggest window it owns; small helper windows are 156x24 etc.
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  // minimise every other visible top-level window so nothing can sit above the launcher
  public static void MinimizeOthers(IntPtr keep, uint keepPid) {
    EnumWindows((h, lp) => {
      if (h == keep || !IsWindowVisible(h)) return true;
      uint op; GetWindowThreadProcessId(h, out op);
      if (op == keepPid) return true;
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
  // prefer the biggest VISIBLE window; only fall back to hidden ones if none is visible
  public static IntPtr Biggest(uint pid) {
    IntPtr best = IntPtr.Zero; long bestArea = 0;
    IntPtr bestAny = IntPtr.Zero; long bestAnyArea = 0;
    EnumWindows((h, lp) => {
      uint p; GetWindowThreadProcessId(h, out p);
      if (p != pid) return true;
      RECT r; GetWindowRect(h, out r);
      long area = (long)(r.R - r.L) * (r.B - r.T);
      if (area > bestAnyArea) { bestAnyArea = area; bestAny = h; }
      if (IsWindowVisible(h) && area > bestArea) { bestArea = area; best = h; }
      return true;
    }, IntPtr.Zero);
    return best != IntPtr.Zero ? best : bestAny;
  }
}
'@
$p = Get-Process EpicGamesLauncher | Select-Object -First 1
if (-not $p) { Write-Output 'NO LAUNCHER'; exit 2 }
[LD]::EnumWindows({ param($h, $lp) $true }, [IntPtr]::Zero) | Out-Null
$h = [LD]::Biggest([uint32]$p.Id)
if ($h -eq [IntPtr]::Zero) { Write-Output 'NO WINDOW'; exit 2 }
[LD]::ShowWindow($h, 9) | Out-Null          # SW_RESTORE
Start-Sleep -Milliseconds 1200
Write-Output ("handle=" + $h.ToInt64())

for ($try = 0; $try -lt 4; $try++) {
  [LD]::SetWindowPos($h, [IntPtr]::Zero, $WX, $WY, $WW, $WH, [LD]::SWP_SHOWWINDOW) | Out-Null
  Start-Sleep -Milliseconds 900
  $chk = New-Object LD+RECT
  [LD]::GetWindowRect($h, [ref]$chk) | Out-Null
  if ($chk.L -eq $WX -and $chk.T -eq $WY -and ($chk.R - $chk.L) -eq $WW) { break }
}

[LD]::SetWindowPos($h, [IntPtr](-1), 0, 0, 0, 0, [LD]::SWP_NOMOVE -bor [LD]::SWP_NOSIZE) | Out-Null
[uint32]$fgPid = 0
$fg = [LD]::GetForegroundWindow()
$fgThread = [LD]::GetWindowThreadProcessId($fg, [ref]$fgPid)
$cur = [LD]::GetCurrentThreadId()
[LD]::AttachThreadInput($cur, $fgThread, $true) | Out-Null
[LD]::BringWindowToTop($h) | Out-Null
[LD]::SetForegroundWindow($h) | Out-Null
[LD]::AttachThreadInput($cur, $fgThread, $false) | Out-Null
Start-Sleep -Milliseconds 1000

$r = New-Object LD+RECT
[LD]::GetWindowRect($h, [ref]$r) | Out-Null
Write-Output ("RECT $($r.L),$($r.T) " + ($r.R - $r.L) + 'x' + ($r.B - $r.T))

if ($Scroll -ne 0) {
  [LD]::SetCursorPos(($r.L + $ScrollX), ($r.T + $ScrollY)) | Out-Null
  Start-Sleep -Milliseconds 400
  $dir = if ($Scroll -gt 0) { [uint32]120 } else { [uint32]4294967176 }
  $n = [Math]::Abs($Scroll)
  for ($i = 0; $i -lt $n; $i++) {
    [LD]::mouse_event(0x0800, 0, 0, $dir, [IntPtr]::Zero)
    Start-Sleep -Milliseconds 220
  }
  Start-Sleep -Milliseconds 1200
  Write-Output "scrolled $Scroll"
}

if ($ClickX -ge 0 -and $ClickY -ge 0) {
  $sx = $r.L + $ClickX; $sy = $r.T + $ClickY
  [LD]::SetCursorPos($sx, $sy) | Out-Null
  Start-Sleep -Milliseconds 400
  [LD]::mouse_event(0x0002, 0, 0, 0, [IntPtr]::Zero)
  Start-Sleep -Milliseconds 110
  [LD]::mouse_event(0x0004, 0, 0, 0, [IntPtr]::Zero)
  Start-Sleep -Milliseconds 2000
  Write-Output "clicked screen $sx,$sy (win-rel $ClickX,$ClickY)"
}

if ($Isolate) {
  [LD]::MinimizeOthers($h, [uint32]$p.Id) | Out-Null
  [LD]::SetWindowPos($h, [IntPtr](-1), 0, 0, 0, 0, [LD]::SWP_NOMOVE -bor [LD]::SWP_NOSIZE) | Out-Null
  [LD]::SetForegroundWindow($h) | Out-Null
  Start-Sleep -Milliseconds 1500
}
Start-Sleep -Milliseconds $SettleMs
$b = New-Object System.Drawing.Bitmap 1280, 720
$g = [System.Drawing.Graphics]::FromImage($b)
$g.CopyFromScreen(0, 0, 0, 0, $b.Size)
$g.Dispose()
$b.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$b.Dispose()
[LD]::SetWindowPos($h, [IntPtr](-2), 0, 0, 0, 0, [LD]::SWP_NOMOVE -bor [LD]::SWP_NOSIZE) | Out-Null
Write-Output "saved $Out"
