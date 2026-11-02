# Click with SendInput (absolute, physical pixels) instead of legacy mouse_event,
# then screenshot. Used to check whether injected input reaches the launcher at all.
param([int]$X = 1107, [int]$Y = 182, [string]$Out = 'D:\VEIL-OMNI\proof\clicktest.png')
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class SI {
  [DllImport("user32.dll")] public static extern bool SetProcessDpiAwarenessContext(IntPtr c);
  [DllImport("user32.dll")] public static extern int GetSystemMetrics(int i);
  [DllImport("user32.dll")] public static extern bool SetCursorPos(int x, int y);
  [StructLayout(LayoutKind.Sequential)] public struct MOUSEINPUT {
    public int dx; public int dy; public uint mouseData; public uint dwFlags; public uint time; public IntPtr dwExtraInfo;
  }
  [StructLayout(LayoutKind.Sequential)] public struct INPUT {
    public uint type; public MOUSEINPUT mi;
  }
  [DllImport("user32.dll", SetLastError=true)] public static extern uint SendInput(uint n, INPUT[] inputs, int size);
  public static uint Move(int x, int y) {
    int sw = GetSystemMetrics(0), sh = GetSystemMetrics(1);
    var inp = new INPUT[1];
    inp[0].type = 0; // mouse
    inp[0].mi.dx = (int)((long)x * 65535 / (sw - 1));
    inp[0].mi.dy = (int)((long)y * 65535 / (sh - 1));
    inp[0].mi.dwFlags = 0x0001 | 0x8000; // MOVE | ABSOLUTE
    return SendInput(1, inp, Marshal.SizeOf(typeof(INPUT)));
  }
  public static uint Down() {
    var inp = new INPUT[1];
    inp[0].type = 0; inp[0].mi.dwFlags = 0x0002; // LEFTDOWN
    return SendInput(1, inp, Marshal.SizeOf(typeof(INPUT)));
  }
  public static uint Up() {
    var inp = new INPUT[1];
    inp[0].type = 0; inp[0].mi.dwFlags = 0x0004; // LEFTUP
    return SendInput(1, inp, Marshal.SizeOf(typeof(INPUT)));
  }
}
'@
[void][SI]::SetProcessDpiAwarenessContext([IntPtr](-4))
Write-Output ("screen " + [SI]::GetSystemMetrics(0) + 'x' + [SI]::GetSystemMetrics(1))
Write-Output ("move sent=" + [SI]::Move($X, $Y))
Start-Sleep -Milliseconds 500
[void][SI]::SetCursorPos($X, $Y)
Start-Sleep -Milliseconds 200
Write-Output ("down=" + [SI]::Down())
Start-Sleep -Milliseconds 120
Write-Output ("up=" + [SI]::Up())
Start-Sleep -Milliseconds 1800
Add-Type -AssemblyName System.Drawing
$w = [SI]::GetSystemMetrics(0); $h = [SI]::GetSystemMetrics(1)
$b = New-Object System.Drawing.Bitmap([int]$w, [int]$h)
$g = [System.Drawing.Graphics]::FromImage($b)
$g.CopyFromScreen(0, 0, 0, 0, $b.Size)
$g.Dispose()
$b.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$b.Dispose()
Write-Output "saved $Out"
