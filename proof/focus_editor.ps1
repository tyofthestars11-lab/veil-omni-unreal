# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms,System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class Fx {
  [DllImport("user32.dll")] public static extern IntPtr GetForegroundWindow();
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint pid);
  [DllImport("kernel32.dll")] public static extern uint GetCurrentThreadId();
  [DllImport("user32.dll")] public static extern bool AttachThreadInput(uint a, uint b, bool f);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr SetFocus(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
}
'@
$p = Get-Process UnrealEditor | Select-Object -First 1
$h = $p.MainWindowHandle
if ([Fx]::IsIconic($h)) { [Fx]::ShowWindow($h, 9) | Out-Null; Start-Sleep -Seconds 1 }
$fg = [Fx]::GetForegroundWindow()
[uint32]$fgPid = 0
$fgThread = [Fx]::GetWindowThreadProcessId($fg, [ref]$fgPid)
$cur = [Fx]::GetCurrentThreadId()
[Fx]::AttachThreadInput($cur, $fgThread, $true) | Out-Null
[Fx]::BringWindowToTop($h) | Out-Null
[Fx]::SetForegroundWindow($h) | Out-Null
[Fx]::SetFocus($h) | Out-Null
[Fx]::AttachThreadInput($cur, $fgThread, $false) | Out-Null
Start-Sleep -Seconds 2
$vs = [System.Windows.Forms.SystemInformation]::VirtualScreen
$bmp = New-Object System.Drawing.Bitmap $vs.Width, $vs.Height
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($vs.Left, $vs.Top, 0, 0, $bmp.Size)
$bmp.Save('D:\VEIL-OMNI\proof\focused.png', [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
$now = [Fx]::GetForegroundWindow()
Write-Output ("editorHandle=" + $h + " foregroundNow=" + $now + " match=" + ($now -eq $h))
