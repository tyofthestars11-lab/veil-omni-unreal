# Aim the game camera: send relative mouse motion to the focused VEIL window.
param(
  [int]$Dx = 0,
  [int]$Dy = 0,
  [int]$Steps = 10,
  [int]$SettleMs = 400
)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
$sig = '[DllImport("user32.dll")] public static extern void mouse_event(uint f, int dx, int dy, uint d, System.IntPtr e);'
$api = Add-Type -MemberDefinition $sig -Name Mouse -Namespace Win32 -PassThru
$sx = [int]($Dx / [Math]::Max(1, $Steps))
$sy = [int]($Dy / [Math]::Max(1, $Steps))
for ($i = 0; $i -lt $Steps; $i++) {
  $api::mouse_event(0x0001, $sx, $sy, 0, [IntPtr]::Zero)
  Start-Sleep -Milliseconds 25
}
Start-Sleep -Milliseconds $SettleMs
Write-Output "look dx=$Dx dy=$Dy"
