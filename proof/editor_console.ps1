# Open the Unreal console in the running editor and run one command line.
# Used to start the MCP server without restarting the editor.
param(
  [Parameter(Mandatory=$true)][string]$Command,
  [int]$SettleMs = 1200
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class UC {
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
$p = Get-Process UnrealEditor -ErrorAction Stop | Select-Object -First 1
$h = $p.MainWindowHandle
if ($h -eq [IntPtr]::Zero) { Write-Output 'NO editor main window'; exit 2 }
if ([UC]::IsIconic($h)) { [UC]::ShowWindow($h, 9) | Out-Null; Start-Sleep -Seconds 1 }
$fg = [UC]::GetForegroundWindow()
[uint32]$fgPid = 0
$fgThread = [UC]::GetWindowThreadProcessId($fg, [ref]$fgPid)
$cur = [UC]::GetCurrentThreadId()
[UC]::AttachThreadInput($cur, $fgThread, $true) | Out-Null
[UC]::BringWindowToTop($h) | Out-Null
[UC]::SetForegroundWindow($h) | Out-Null
[UC]::SetFocus($h) | Out-Null
[UC]::AttachThreadInput($cur, $fgThread, $false) | Out-Null
Start-Sleep -Milliseconds 900

# tilde opens the console
[System.Windows.Forms.SendKeys]::SendWait('`')
Start-Sleep -Milliseconds 500
# type the command literally, then run it
[System.Windows.Forms.SendKeys]::SendWait($Command.Replace('+','{+}').Replace('^','{^}').Replace('%','{%}').Replace('~','{~}').Replace('(','{(}').Replace(')','{)}').Replace('[','{[').Replace(']','{]}').Replace('{','{{}').Replace('}','{}}'))
Start-Sleep -Milliseconds 250
[System.Windows.Forms.SendKeys]::SendWait('{ENTER}')
Start-Sleep -Milliseconds $SettleMs
Write-Output ("ran in editor console: " + $Command)
