# One-shot: bring the launcher forward, click Unreal Engine, scroll to My Projects, capture
# at the native 3840x2160 resolution (this desktop runs at 300% scaling).
param([string]$Dir = 'D:\VEIL-OMNI\proof')
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class FP {
  public delegate bool EnumProc(IntPtr h, IntPtr lp);
  [DllImport("user32.dll")] public static extern bool SetProcessDpiAwarenessContext(IntPtr c);
  [DllImport("user32.dll")] public static extern int GetSystemMetrics(int i);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr lp);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool SetWindowPos(IntPtr h, IntPtr after, int x, int y, int cx, int cy, uint f);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern bool AttachThreadInput(uint a, uint b, bool f);
  [DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId();
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [StructLayout(LayoutKind.Sequential)] public struct MOUSEINPUT { public int dx; public int dy; public uint mouseData; public uint dwFlags; public uint time; public IntPtr dwExtraInfo; }
  [StructLayout(LayoutKind.Sequential)] public struct INPUT { public uint type; public MOUSEINPUT mi; }
  [DllImport("user32.dll", SetLastError=true)] public static extern uint SendInput(uint n, INPUT[] inputs, int size);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }

  public static void MoveAbs(int x, int y) {
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    var i = new INPUT[1]; i[0].type = 0;
    i[0].mi.dx = (int)((long)x * 65535 / (sw - 1));
    i[0].mi.dy = (int)((long)y * 65535 / (sh - 1));
    i[0].mi.dwFlags = 0x0001 | 0x8000;
    SendInput(1, i, Marshal.SizeOf(typeof(INPUT)));
  }
  public static void Click(int x, int y) {
    MoveAbs(x, y);
    System.Threading.Thread.Sleep(300);
    var d = new INPUT[1]; d[0].type = 0; d[0].mi.dwFlags = 0x0002;
    SendInput(1, d, Marshal.SizeOf(typeof(INPUT)));
    System.Threading.Thread.Sleep(120);
    var u = new INPUT[1]; u[0].type = 0; u[0].mi.dwFlags = 0x0004;
    SendInput(1, u, Marshal.SizeOf(typeof(INPUT)));
  }
  public static void Wheel(int x, int y, int notches) {
    MoveAbs(x, y);
    System.Threading.Thread.Sleep(300);
    int step = notches > 0 ? 120 : -120;
    for (int n = 0; n < Math.Abs(notches); n++) {
      var w = new INPUT[1]; w[0].type = 0; w[0].mi.dwFlags = 0x0800; w[0].mi.mouseData = (uint)step;
      SendInput(1, w, Marshal.SizeOf(typeof(INPUT)));
      System.Threading.Thread.Sleep(200);
    }
  }
  public static IntPtr BiggestVisible(uint pid) {
    IntPtr best = IntPtr.Zero; long bestArea = 0;
    EnumWindows((h, lp) => {
      uint p; GetWindowThreadProcessId(h, out p);
      if (p != pid || !IsWindowVisible(h)) return true;
      RECT r; GetWindowRect(h, out r);
      long a = (long)(r.R - r.L) * (r.B - r.T);
      if (a > bestArea) { bestArea = a; best = h; }
      return true;
    }, IntPtr.Zero);
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
[void][FP]::SetProcessDpiAwarenessContext([IntPtr](-4))
Add-Type -AssemblyName System.Drawing
$SW = [int][FP]::GetSystemMetrics(0); $SH = [int][FP]::GetSystemMetrics(1)
Write-Output "screen ${SW}x${SH}"

$p = Get-Process EpicGamesLauncher | Select-Object -First 1
$h = [FP]::BiggestVisible([uint32]$p.Id)
if ($h -eq [IntPtr]::Zero) { Write-Output 'NO WINDOW'; exit 2 }
[FP]::ShowWindow($h, 9) | Out-Null
[FP]::SetWindowPos($h, [IntPtr](-1), 0, 0, 0, 0, 0x0001 -bor 0x0002) | Out-Null   # topmost
[FP]::MinimizeOthers($h, [uint32]$p.Id) | Out-Null
[uint32]$fgPid = 0
$fg = [FP]::GetForegroundWindow()
$ft = [FP]::GetWindowThreadProcessId($fg, [ref]$fgPid)
$ct = [FP]::GetCurrentThreadId()
[FP]::AttachThreadInput($ct, $ft, $true) | Out-Null
[FP]::BringWindowToTop($h) | Out-Null
[FP]::SetForegroundWindow($h) | Out-Null
[FP]::AttachThreadInput($ct, $ft, $false) | Out-Null
Start-Sleep -Milliseconds 1500

$r = New-Object FP+RECT
[FP]::GetWindowRect($h, [ref]$r) | Out-Null
Write-Output ("rect $($r.L),$($r.T) " + ($r.R - $r.L) + 'x' + ($r.B - $r.T))

# rail "Unreal Engine" sits at x=342, y=775 once maximised on this screen
[FP]::Click(342, 775)
Start-Sleep -Milliseconds 2500
[FP]::Click(342, 775)
Start-Sleep -Milliseconds 3000

function Shot($path) {
  $b = New-Object System.Drawing.Bitmap([int]$SW, [int]$SH)
  $g = [System.Drawing.Graphics]::FromImage($b)
  $g.CopyFromScreen(0, 0, 0, 0, $b.Size)
  $g.Dispose()
  $b.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
  $b.Dispose()
  Write-Output "saved $path"
}
Shot (Join-Path $Dir 'proof-top.png')

# scroll the content column down toward My Projects
[FP]::Wheel(1900, 1100, 5)
Start-Sleep -Milliseconds 1500
Shot (Join-Path $Dir 'proof-myprojects.png')

[FP]::SetWindowPos($h, [IntPtr](-2), 0, 0, 0, 0, 0x0001 -bor 0x0002) | Out-Null
Write-Output 'done'
