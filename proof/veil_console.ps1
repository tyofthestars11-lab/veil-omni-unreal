# Type one console command into the running game and get out of the way.
#
# The game is a -game session, so this is the player's own console: ` opens it,
# the command runs on Enter, and ` closes it again before the photo is taken.
# Keystrokes go through SendKeys to the FOREGROUND window, so the window is
# brought forward and given a moment first.
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
param(
  [string]$Title = 'VEIL (',
  [Parameter(Mandatory=$true)][string]$Command,
  [int]$SettleMs = 900
)
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class CV {
  public delegate bool EP(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EP cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll")] public static extern bool SetProcessDpiAwarenessContext(IntPtr c);
  public static IntPtr Find(string needle) {
    IntPtr found = IntPtr.Zero;
    EnumWindows((h, lp) => {
      if (!IsWindowVisible(h)) return true;
      int len = GetWindowTextLength(h);
      if (len == 0) return true;
      var sb = new StringBuilder(len + 1);
      GetWindowText(h, sb, sb.Capacity);
      if (sb.ToString().IndexOf(needle, StringComparison.OrdinalIgnoreCase) >= 0) { found = h; return false; }
      return true;
    }, IntPtr.Zero);
    return found;
  }
}
'@
[void][CV]::SetProcessDpiAwarenessContext([IntPtr](-4))

$h = [CV]::Find($Title)
if ($h -eq [IntPtr]::Zero) { Write-Output "NO WINDOW matching '$Title'"; exit 2 }
[void][CV]::ShowWindow($h, 9)
[void][CV]::BringWindowToTop($h)
[void][CV]::SetForegroundWindow($h)
Start-Sleep -Milliseconds 700

# Open, command, Enter, close.
[System.Windows.Forms.SendKeys]::SendWait('`')
Start-Sleep -Milliseconds 450
[System.Windows.Forms.SendKeys]::SendWait($Command)
Start-Sleep -Milliseconds 250
[System.Windows.Forms.SendKeys]::SendWait('{ENTER}')
Start-Sleep -Milliseconds 700
[System.Windows.Forms.SendKeys]::SendWait('`')
Start-Sleep -Milliseconds $SettleMs
Write-Output "sent: $Command"
