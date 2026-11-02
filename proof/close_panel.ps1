param([Parameter(Mandatory=$true)][string]$Title)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class CP2 {
  public delegate bool EP(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EP cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr h, uint m, IntPtr w, IntPtr l);
}
'@
$found = [IntPtr]::Zero
$cb = {
  param($h, $l)
  if ([CP2]::IsWindowVisible($h)) {
    $len = [CP2]::GetWindowTextLength($h)
    $sb = New-Object System.Text.StringBuilder ($len + 1)
    [void][CP2]::GetWindowText($h, $sb, $sb.Capacity)
    if ($sb.ToString() -eq $Title) { $script:found = $h; return $false }
  }
  return $true
}
[void][CP2]::EnumWindows($cb, [IntPtr]::Zero)
if ($script:found -eq [IntPtr]::Zero) { Write-Output "no window titled '$Title'"; exit 2 }
[void][CP2]::PostMessage($script:found, 0x0010, [IntPtr]::Zero, [IntPtr]::Zero)
Start-Sleep -Milliseconds 800
Write-Output "closed '$Title'"
