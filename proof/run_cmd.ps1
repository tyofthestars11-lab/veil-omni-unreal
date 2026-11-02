param([Parameter(Mandatory=$true)][string]$PyFile)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class Fy {
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
if ([Fy]::IsIconic($h)) { [Fy]::ShowWindow($h, 9) | Out-Null; Start-Sleep -Seconds 1 }
$fg = [Fy]::GetForegroundWindow()
[uint32]$fgPid = 0
$fgThread = [Fy]::GetWindowThreadProcessId($fg, [ref]$fgPid)
$cur = [Fy]::GetCurrentThreadId()
[Fy]::AttachThreadInput($cur, $fgThread, $true) | Out-Null
[Fy]::BringWindowToTop($h) | Out-Null
[Fy]::SetForegroundWindow($h) | Out-Null
[Fy]::SetFocus($h) | Out-Null
[Fy]::AttachThreadInput($cur, $fgThread, $false) | Out-Null
Start-Sleep -Seconds 2
[System.Windows.Forms.SendKeys]::SendWait('{ESC}')
Start-Sleep -Milliseconds 700
[System.Windows.Forms.SendKeys]::SendWait([char]96)
Start-Sleep -Seconds 2
[System.Windows.Forms.SendKeys]::SendWait('py "' + $PyFile + '"')
Start-Sleep -Milliseconds 700
[System.Windows.Forms.SendKeys]::SendWait('{ENTER}')
Write-Output ('sent py ' + $PyFile)
