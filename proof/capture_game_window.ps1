# Capture the running packaged game's window to a PNG, without keystrokes.
#
# The console route needs the window focused and the packaged build does not cooperate
# with SendKeys, so this reads the window rectangle and blits those pixels instead.
# Read-only: it inspects a window and writes one image file.
param(
  [string]$Out = 'D:\VEIL-OMNI\proof\packaged_v5_raw.png',
  [string]$ProcessName = 'VEIL'
)

Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class GameWin {
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, IntPtr rect);
  [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
}
'@

$p = Get-Process $ProcessName -ErrorAction Stop | Select-Object -First 1
$h = $p.MainWindowHandle
if ($h -eq [IntPtr]::Zero) { throw "no main window for $ProcessName" }

if ([GameWin]::IsIconic($h)) { [GameWin]::ShowWindow($h, 9) | Out-Null }
[GameWin]::SetForegroundWindow($h) | Out-Null
Start-Sleep -Milliseconds 1500

$mem = [System.Runtime.InteropServices.Marshal]::AllocHGlobal(16)
try {
  if (-not [GameWin]::GetWindowRect($h, $mem)) { throw "GetWindowRect failed" }
  $left   = [System.Runtime.InteropServices.Marshal]::ReadInt32($mem, 0)
  $top    = [System.Runtime.InteropServices.Marshal]::ReadInt32($mem, 4)
  $right  = [System.Runtime.InteropServices.Marshal]::ReadInt32($mem, 8)
  $bottom = [System.Runtime.InteropServices.Marshal]::ReadInt32($mem, 12)
} finally {
  [System.Runtime.InteropServices.Marshal]::FreeHGlobal($mem)
}

$w = $right - $left
$hh = $bottom - $top
if ($w -lt 100 -or $hh -lt 100) { throw "window rect looks wrong: ${w}x${hh}" }

$bmp = New-Object System.Drawing.Bitmap $w, $hh
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($left, $top, 0, 0, $bmp.Size)
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$g.Dispose(); $bmp.Dispose()
Write-Output "$Out ${w}x${hh}"
