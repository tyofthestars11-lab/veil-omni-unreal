param(
  [int]$Steps = 3,
  [string]$Out = 'D:\VEIL-OMNI\proof\epic-zoom-out.png'
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class EZ2 {
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
  [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, IntPtr extra);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public const byte CTRL = 0x11;
  public const uint KEYUP = 0x0002;
  public static void Tap(byte vk) {
    keybd_event(CTRL, 0, 0, IntPtr.Zero);
    System.Threading.Thread.Sleep(60);
    keybd_event(vk, 0, 0, IntPtr.Zero);
    System.Threading.Thread.Sleep(60);
    keybd_event(vk, 0, KEYUP, IntPtr.Zero);
    System.Threading.Thread.Sleep(60);
    keybd_event(CTRL, 0, KEYUP, IntPtr.Zero);
    System.Threading.Thread.Sleep(320);
  }
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
[void][EZ2]::SetProcessDpiAwarenessContext([IntPtr](-4))
$p = Get-Process EpicGamesLauncher | Select-Object -First 1
$h = [EZ2]::Biggest([uint32]$p.Id)
if ($h -eq [IntPtr]::Zero) { Write-Output 'NO WINDOW'; exit 2 }

# Windows only lets the foreground app take focus; attach to it first.
$fg = [EZ2]::GetForegroundWindow()
$fgPid = 0
$fgThread = [EZ2]::GetWindowThreadProcessId($fg, [ref]$fgPid)
$myThread = [EZ2]::GetCurrentThreadId()
[void][EZ2]::AttachThreadInput($myThread, $fgThread, $true)
[EZ2]::BringWindowToTop($h) | Out-Null
[EZ2]::SetForegroundWindow($h) | Out-Null
[void][EZ2]::AttachThreadInput($myThread, $fgThread, $false)
Start-Sleep -Milliseconds 900
Write-Output ("foreground now: " + [EZ2]::GetForegroundWindow() + " target: " + $h)

[EZ2]::Tap(0x30)          # Ctrl+0 -> reset to 100%
for ($i = 0; $i -lt $Steps; $i++) { [EZ2]::Tap(0xBD) }   # Ctrl+- -> step down
Start-Sleep -Seconds 2
Write-Output "zoom reset then stepped down $Steps"

$r = New-Object EZ2+RECT
[EZ2]::GetWindowRect($h, [ref]$r) | Out-Null
$cw = $r.R - $r.L; $ch = $r.B - $r.T
$bmp = New-Object System.Drawing.Bitmap($cw, $ch)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
[EZ2]::PrintWindow($h, $hdc, 2) | Out-Null
$g.ReleaseHdc($hdc); $g.Dispose()
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Output "saved $Out (${cw}x${ch})"
