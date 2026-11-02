# Dump the Epic Launcher's UI Automation tree so its real text (project names) can be read,
# instead of inferring them from pixels.
param([string]$Exe = 'EpicGamesLauncher', [int]$Max = 400)
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
. (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')
[void](Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath)

$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName UIAutomationClient
Add-Type -AssemblyName UIAutomationTypes
$p = Get-Process $Exe -ErrorAction SilentlyContinue | Select-Object -First 1
if (-not $p) { Write-Output 'NO PROCESS'; exit 2 }
Write-Output ("pid=" + $p.Id)

$cond = New-Object System.Windows.Automation.PropertyCondition(
  [System.Windows.Automation.AutomationElement]::ProcessIdProperty, $p.Id)
$wins = [System.Windows.Automation.AutomationElement]::RootElement.FindAll(
  [System.Windows.Automation.TreeScope]::Children, $cond)
Write-Output ("windows=" + $wins.Count)

$walker = [System.Windows.Automation.TreeWalker]::RawViewWalker
$script:count = 0
function Walk($el, $depth) {
  if ($script:count -ge $Max) { return }
  if ($null -eq $el) { return }
  try {
    $name = $el.Current.Name
    $type = $el.Current.ControlType.ProgrammaticName
    $r = $el.Current.BoundingRectangle
  } catch { return }
  if ($name -and $name.Trim().Length -gt 0) {
    $script:count++
    Write-Output ("{0}{1} | {2} | {3} | {4},{5} {6}x{7}" -f ('  ' * $depth), $type, $name,
      $el.Current.AutomationId, [int]$r.X, [int]$r.Y, [int]$r.Width, [int]$r.Height)
  }
  if ($depth -ge 12) { return }
  $child = $walker.GetFirstChild($el)
  while ($null -ne $child) {
    Walk $child ($depth + 1)
    if ($script:count -ge $Max) { return }
    $child = $walker.GetNextSibling($child)
  }
}
for ($i = 0; $i -lt $wins.Count; $i++) {
  Write-Output ("=== window " + $i + " ===")
  Walk $wins.Item($i) 0
}
Write-Output ("elements=" + $script:count)
