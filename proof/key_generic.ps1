# Focus the VEIL game window and send one key by name (arrows, letters, space).
param(
  [string]$Title = 'VEIL',
  [Parameter(Mandatory=$true)][string]$Key = 'g',
  [int]$SettleMs = 600,
  [switch]$Hold,
  [string]$Map = ''
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
# The root is D:\VEIL-OMNI, read from ONE place. Nothing here is ever built from
# the Freebuff app folder - that is what produced
#   .../@codebufffreebuff-desktop/resources/git/Game/Maps/VEILDistrict
#   could not be found
#
# The map is the CONTENT path /Game/Maps/VEILDistrict, never a Windows path.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_paths.ps1')
# The guard, called HERE and not only inside veil_paths: every script states it.
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)
if (-not [string]::IsNullOrWhiteSpace($Map)) { [void](Assert-VeilMapArg -Map $Map) }
Write-Output ("root  {0}" -f $VeilRoot)
Write-Output ("map   {0}  (content path)" -f $VeilMap)

$ErrorActionPreference = 'Stop'

. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_paths.ps1')

# A map argument on this script is a CONTENT path - checked when passed.
if ($PSBoundParameters.ContainsKey('Map') -and $Map) { [void](Assert-VeilMapArg -Map $Map) }

$codes = @{
  space = 0x20; enter = 0x0D; escape = 0x1B; tab = 0x09
  up = 0x26; down = 0x28; left = 0x25; right = 0x27
  w = 0x57; a = 0x41; s = 0x53; d = 0x44; e = 0x45; g = 0x47
  r = 0x52; c = 0x43; m = 0x4D; p = 0x50; o = 0x4F
  one = 0x31; two = 0x32; three = 0x33
}
if (-not $codes.ContainsKey($Key.ToLower())) { Write-Output "unknown key $Key"; exit 3 }
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class KG {
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
# A map argument on this script is a CONTENT path - checked before anything runs.
if ($PSBoundParameters.ContainsKey('Map')) { [void](Assert-VeilMapArg -Map $Map) }

$h = [KG]::Find($Title)
if ($h -eq [IntPtr]::Zero) { Write-Output "NO WINDOW matching '$Title'"; exit 2 }
[void][KG]::ShowWindow($h, 9)
[void][KG]::BringWindowToTop($h)
[void][KG]::SetForegroundWindow($h)
Start-Sleep -Milliseconds 400
$code = [byte]$codes[$Key.ToLower()]
[KG]::keybd_event($code, 0, 0, [IntPtr]::Zero)
Start-Sleep -Milliseconds 120
[KG]::keybd_event($code, 0, 2, [IntPtr]::Zero)
if ($Hold) { Start-Sleep -Milliseconds $SettleMs } else { Start-Sleep -Milliseconds $SettleMs }
Write-Output "sent $Key"
