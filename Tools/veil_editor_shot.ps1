# Capture the EDITOR window by enumerating the process's real top-level windows.
#
# MainWindowTitle goes blank on the UE editor (many top-level windows, and the
# "main" one is not the biggest), so the proof looks for the biggest visible
# window the process owns, brings it forward and photographs exactly it.
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
param(
  [string]$Out = 'D:\VEIL-OMNI\proof\editor_mapload.png',
  [string]$MustContain = 'VEIL'
)
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Drawing
Add-Type -AssemblyName System.Windows.Forms
Add-Type @'
using System;
using System.Text;
using System.Runtime.InteropServices;
public class ES {
  public delegate bool EP(IntPtr h, IntPtr l);
  [DllImport("user32.dll")] public static extern bool EnumWindows(EP cb, IntPtr l);
  [DllImport("user32.dll")] public static extern bool IsWindowVisible(IntPtr h);
  [DllImport("user32.dll")] public static extern bool IsIconic(IntPtr h);
  [DllImport("user32.dll")] public static extern int GetWindowText(IntPtr h, StringBuilder s, int n);
  [DllImport("user32.dll")] public static extern int GetWindowTextLength(IntPtr h);
  [DllImport("user32.dll")] public static extern uint GetWindowThreadProcessId(IntPtr h, out uint p);
  [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr h, out RECT r);
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern bool ShowWindow(IntPtr h, int c);
  [DllImport("user32.dll")] public static extern bool SetProcessDpiAwarenessContext(IntPtr c);
  [DllImport("user32.dll")] public static extern int GetSystemMetrics(int i);
  [StructLayout(LayoutKind.Sequential)] public struct RECT { public int L, T, R, B; }
}
'@
[void][ES]::SetProcessDpiAwarenessContext([IntPtr](-4))

$procs = @(Get-Process UnrealEditor -ErrorAction SilentlyContinue)
if ($procs.Count -eq 0) { Write-Output 'NO EDITOR PROCESS'; exit 2 }
$pids = $procs | ForEach-Object { $_.Id }

$best = [IntPtr]::Zero
$bestArea = 0
$bestTitle = ''
$rows = New-Object System.Collections.ArrayList
$cb = {
  param($h, $l)
  $winPid = 0
  [void][ES]::GetWindowThreadProcessId($h, [ref]$winPid)
  if ($script:pids -notcontains [int]$winPid) { return $true }
  if (-not [ES]::IsWindowVisible($h)) { return $true }
  $len = [ES]::GetWindowTextLength($h)
  $sb = New-Object System.Text.StringBuilder ($len + 1)
  [void][ES]::GetWindowText($h, $sb, $sb.Capacity)
  $title = $sb.ToString()
  $r = New-Object ES+RECT
  [void][ES]::GetWindowRect($h, [ref]$r)
  $w = $r.R - $r.L; $ht = $r.B - $r.T
  if ($w -lt 200 -or $ht -lt 200) { return $true }
  [void]$rows.Add(("  [{0}] {1}x{2} at {3},{4}" -f $title, $w, $ht, $r.L, $r.T))
  if ($title -notmatch $script:MustContain) { return $true }
  $area = $w * $ht
  if ($area -gt $script:bestArea) {
    $script:bestArea = $area; $script:best = $h; $script:bestTitle = $title
  }
  return $true
}
$script:pids = $pids
$script:MustContain = $MustContain
$script:best = [IntPtr]::Zero
$script:bestArea = 0
$script:bestTitle = ''
[void][ES]::EnumWindows($cb, [IntPtr]::Zero)

Write-Output 'editor windows:'
$rows | ForEach-Object { Write-Output $_ }
if ($best -eq [IntPtr]::Zero) { Write-Output "NO WINDOW matching '$MustContain'"; exit 3 }
Write-Output ("capturing: '{0}'  {1} px2" -f $bestTitle, $bestArea)

[void][ES]::ShowWindow($best, 9)
[void][ES]::SetForegroundWindow($best)
Start-Sleep -Milliseconds 2500

$r = New-Object ES+RECT
[void][ES]::GetWindowRect($best, [ref]$r)
# Clip to the desktop: at 300% scaling the editor window can be larger than the
# screen, and CopyFromScreen cannot read pixels that are not there. Screen bounds
# come from GetSystemMetrics, which is physical once the process is DPI aware.
$SW = [ES]::GetSystemMetrics(0)
$SH = [ES]::GetSystemMetrics(1)
Write-Output ("desktop {0}x{1}" -f $SW, $SH)
# NOTE: PowerShell variables are case-insensitive, so the clip edges are NOT
# called $L/$R/$T/$B - that would overwrite the $r RECT itself.
$boxL = [Math]::Max($r.L, 0)
$boxT = [Math]::Max($r.T, 0)
$boxR = [Math]::Min($r.R, $SW)
$boxB = [Math]::Min($r.B, $SH)
$cw = $boxR - $boxL; $ch = $boxB - $boxT
if ($cw -le 0 -or $ch -le 0) { Write-Output 'WINDOW IS OFF SCREEN'; exit 4 }
Write-Output ("window={0}x{1}  captured region={2}x{3} at {4},{5}" -f ($r.R - $r.L), ($r.B - $r.T), $cw, $ch, $boxL, $boxT)

$bmp = New-Object System.Drawing.Bitmap($cw, $ch)
$g = [System.Drawing.Graphics]::FromImage($bmp)
$g.CopyFromScreen($boxL, $boxT, 0, 0, $bmp.Size)
$g.Dispose()
$bmp.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
$bmp.Dispose()
Write-Output "saved $Out"
