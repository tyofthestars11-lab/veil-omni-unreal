# Focus a window by title and send one key (space by default) - used to leave the title screen.
param(
  [Parameter(Mandatory=$true)][string]$Title,
  [ValidateSet('space','enter','escape','one','two','three','w','e')] [string]$Key = 'space',
  [int]$SettleMs = 2500,
  [int]$HoldMs = 0
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
$codes = @{ space = 0x20; enter = 0x0D; escape = 0x1B; one = 0x31; two = 0x32; three = 0x33; w = 0x57; e = 0x45 }
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class KS {
  public delegate bool EP(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EP cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool BringWindowToTop(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll")] public static extern void keybd_event(byte k, byte scan, uint f, IntPtr e);
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
$h = [KS]::Find($Title)
if ($h -eq [IntPtr]::Zero) { Write-Output "NO WINDOW matching '$Title'"; exit 2 }
[void][KS]::ShowWindow($h, 9)
[void][KS]::BringWindowToTop($h)
[void][KS]::SetForegroundWindow($h)
Start-Sleep -Milliseconds 800
$code = [byte]$codes[$Key]
[KS]::keybd_event($code, 0, 0, [IntPtr]::Zero)
if ($HoldMs -gt 0) { Start-Sleep -Milliseconds $HoldMs } else { Start-Sleep -Milliseconds 120 }
[KS]::keybd_event($code, 0, 2, [IntPtr]::Zero)
Start-Sleep -Milliseconds $SettleMs
Write-Output "sent $Key to '$Title'"
