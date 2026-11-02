# Read what is ON SCREEN: every visible window of the running UnrealEditor, plus
# the text of its child controls, so a modal dialog is quoted instead of guessed.
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class WT {
  public delegate bool EP(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EP cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumChildWindows(IntPtr h, EP cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetClassName(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint p);
}
'@

$procs = @(Get-Process UnrealEditor -ErrorAction SilentlyContinue)
if ($procs.Count -eq 0) { Write-Output 'NO UnrealEditor PROCESS'; exit 2 }
$ids = @{}
foreach ($p in $procs) { $ids[$p.Id] = $true }

function TextOf([IntPtr]$h) {
  $len = [WT]::GetWindowTextLength($h)
  $sb = New-Object System.Text.StringBuilder ($len + 2)
  [void][WT]::GetWindowText($h, $sb, $sb.Capacity)
  return $sb.ToString()
}

$script:top = New-Object System.Collections.ArrayList
$cb = {
  param($h, $l)
  $winPid = 0
  [void][WT]::GetWindowThreadProcessId($h, [ref]$winPid)
  if (-not $ids.ContainsKey([int]$winPid)) { return $true }
  if (-not [WT]::IsWindowVisible($h)) { return $true }
  $cls = New-Object System.Text.StringBuilder 256
  [void][WT]::GetClassName($h, $cls, 256)
  $title = TextOf $h
  if ($title.Length -eq 0 -and $cls.ToString() -ne 'UnrealWindow') { return $true }
  [void]$script:top.Add(("WINDOW pid={0} class=[{1}] title=[{2}]" -f $winPid, $cls.ToString(), $title))
  $childCb = {
    param($ch, $cl)
    $t = TextOf $ch
    if ($t.Length -gt 0) { [void]$script:top.Add("    CHILD  " + $t) }
    return $true
  }
  [void][WT]::EnumChildWindows($h, $childCb, [IntPtr]::Zero)
  return $true
}
[void][WT]::EnumWindows($cb, [IntPtr]::Zero)
$script:top | ForEach-Object { Write-Output $_ }
