# Focus the VEIL window and roll the mouse wheel N notches, so the wheel's owner
# is proven by the log line it produces instead of by assumption.
param(
  [string]$Title = 'VEIL',
  [int]$Notches = 3,
  [int]$SettleMs = 700
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_paths.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type @'
using System;
using System.Runtime.InteropServices;
public class Wheel {
  [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr h);
  [DllImport("user32.dll")] public static extern IntPtr FindWindow(string cls, string name);
  [DllImport("user32.dll")] public static extern void mouse_event(uint flags, uint dx, uint dy, int data, UIntPtr extra);
  public const uint WHEEL = 0x0800;
  public static void Roll(int notches) {
    for (int i = 0; i < Math.Abs(notches); i++) {
      mouse_event(WHEEL, 0, 0, notches > 0 ? 120 : -120, UIntPtr.Zero);
      System.Threading.Thread.Sleep(120);
    }
  }
}
'@

$p = Get-Process | Where-Object { $_.MainWindowTitle -like "*$Title*" -and $_.MainWindowHandle -ne 0 } | Select-Object -First 1
if (-not $p) { Write-Output "NO WINDOW for '$Title'"; exit 2 }
[void][Wheel]::SetForegroundWindow($p.MainWindowHandle)
Start-Sleep -Milliseconds 300
[Wheel]::Roll($Notches)
Start-Sleep -Milliseconds $SettleMs
Write-Output ("wheel {0} notch(es) into pid {1}" -f $Notches, $p.Id)
