# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms, System.Drawing
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class EZ {
  public delegate bool EnumProc(IntPtr h, IntPtr lp);
  [DllImport("user32.dll")] public static extern bool SetProcessDpiAwarenessContext(IntPtr c);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EnumProc cb, IntPtr lp);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [DllImport("user32.dll")] public static extern void mouse_event(uint f, uint dx, uint dy, int d, IntPtr e);
  [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, uint flags, IntPtr extra);
  [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr h, IntPtr hdc, uint flags);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public const uint WHEEL = 0x0800;
  public const byte CTRL = 0x11;
  public const uint KEYUP = 0x0002;

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
  public static string Title(IntPtr h) {
    int n = GetWindowTextLength(h);
    var sb = new StringBuilder(n + 1);
    GetWindowText(h, sb, sb.Capacity);
    return sb.ToString();
  }
}
'@
[void][EZ]::SetProcessDpiAwarenessContext([IntPtr](-4))

$p = Get-Process EpicGamesLauncher -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $p) { Write-Output 'LAUNCHER NOT RUNNING'; exit 2 }
$h = [EZ]::Biggest([uint32]$p.Id)
if ($h -eq [IntPtr]::Zero) { Write-Output 'NO VISIBLE WINDOW'; exit 3 }
Write-Output ("window='" + [EZ]::Title($h) + "'")

[EZ]::ShowWindow($h, 9) | Out-Null          # restore if minimised / cloaked
[EZ]::BringWindowToTop($h) | Out-Null
[EZ]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 1200

$r = New-Object EZ+RECT
[EZ]::GetWindowRect($h, [ref]$r) | Out-Null
$w = $r.R - $r.L; $ht = $r.B - $r.T
Write-Output ("rect $($r.L),$($r.T) ${w}x${ht}")

# Zoom out over the content area: Ctrl + wheel down.
[EZ]::SetCursorPos($r.L + [int]($w * 0.55), $r.T + [int]($ht * 0.45)) | Out-Null
Start-Sleep -Milliseconds 400
[EZ]::keybd_event([EZ]::CTRL, 0, 0, [IntPtr]::Zero)
for ($i = 0; $i -lt 6; $i++) {
  [EZ]::mouse_event([EZ]::WHEEL, 0, 0, -120, [IntPtr]::Zero)
  Start-Sleep -Milliseconds 220
}
[EZ]::keybd_event([EZ]::CTRL, 0, [EZ]::KEYUP, [IntPtr]::Zero)
Start-Sleep -Seconds 2

$r2 = New-Object EZ+RECT
[EZ]::GetWindowRect($h, [ref]$r2) | Out-Null
$cw = $r2.R - $r2.L; $ch = $r2.B - $r2.T
$bmp = New-Object System.Drawing.Bitmap($cw, $ch)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$hdc = $g.GetHdc()
[EZ]::PrintWindow($h, $hdc, 2) | Out-Null
$g.ReleaseHdc($hdc); $g.Dispose()
$bmp.Save('D:\VEIL-OMNI\proof\epic-zoom-out.png', [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Output 'saved D:\VEIL-OMNI\proof\epic-zoom-out.png'
