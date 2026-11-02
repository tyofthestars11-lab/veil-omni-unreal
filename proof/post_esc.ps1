# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type @'
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
public class Msg {
  public delegate bool EnumProc(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr p, EnumProc cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
  [DllImport("user32.dll")] public static extern bool GetClientRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern StringBuilder GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
  public const uint WM_KEYDOWN = 0x0100;
  public const uint WM_KEYUP = 0x0101;
  public static int Count = 0;
  public static IntPtr Main;
  public static bool Cb(IntPtr h, IntPtr l) {
    RECT r; GetClientRect(h, out r);
    if (r.R - r.L > 200 && r.B - r.T > 200) {
      PostMessage(h, WM_KEYDOWN, (IntPtr)0x1B, IntPtr.Zero);
      PostMessage(h, WM_KEYUP, (IntPtr)0x1B, IntPtr.Zero);
      Count++;
    }
    return true;
  }
  public static void SendMain(IntPtr h) {
    PostMessage(h, WM_KEYDOWN, (IntPtr)0x1B, IntPtr.Zero);
    PostMessage(h, WM_KEYUP, (IntPtr)0x1B, IntPtr.Zero);
  }
}
'@
$p = Get-Process UnrealEditor | Select-Object -First 1
$h = $p.MainWindowHandle
[Msg]::SendMain($h) | Out-Null
[Msg]::EnumChildWindows($h, [Msg+EnumProc]{ param($c, $l) [Msg]::Cb($c, $l) }, [IntPtr]::Zero) | Out-Null
Write-Output ("posted ESC to main + " + [Msg]::Count + " child windows")
