$P = "D:\VEIL-OMNI"; $U = "$P\VEIL.uproject"; $D = "$P\Docs"; $R = "$D\report.txt"
New-Item -ItemType Directory -Force $D | Out-Null
Copy-Item $U "$U.bak" -Force
$j = Get-Content $U -Raw | ConvertFrom-Json
if (-not ($j.PSObject.Properties.Name -contains 'Plugins')) { $j | Add-Member -NotePropertyName Plugins -NotePropertyValue @() }
$list = @($j.Plugins)
foreach ($n in 'PythonScriptPlugin','PCG','EditorScriptingUtilities') {
  $hit = $list | Where-Object { $_.Name -eq $n }
  if ($hit) { $hit.Enabled = $true } else { $list += [pscustomobject]@{ Name = $n; Enabled = $true } }
}
$j.Plugins = $list
[IO.File]::WriteAllText($U, ($j | ConvertTo-Json -Depth 20))

$o = @()
$o += "=== VEIL REPORT $(Get-Date)"
$o += "--- PLUGINS ENABLED IN UPROJECT"
$o += ((Get-Content $U -Raw | ConvertFrom-Json).Plugins | Where-Object Enabled | ForEach-Object Name)
$o += "--- UE INSTALLS"
$o += (Get-ChildItem 'C:\Program Files\Epic Games' -Directory -Filter 'UE_*' -ErrorAction SilentlyContinue | ForEach-Object FullName)
$o += "--- CONTENT FOLDERS (uasset count)"
Get-ChildItem "$P\Content" -Directory -ErrorAction SilentlyContinue | ForEach-Object {
  $c = (Get-ChildItem $_.FullName -Recurse -Filter *.uasset -ErrorAction SilentlyContinue).Count
  $o += "$($_.Name)  $c"
}
$o += "--- FAB / MEGASCANS FOLDERS"
$o += (Get-ChildItem "$P\Content" -Directory -Recurse -ErrorAction SilentlyContinue | Where-Object { $_.Name -match 'Fab|Megascans|Quixel' } | ForEach-Object FullName)
$o += "--- MISSING.TXT"
if (Test-Path "$D\missing.txt") { $o += Get-Content "$D\missing.txt" } else { $o += 'none' }
$o += "--- UBT LOG (result + errors)"
$ubt = "$env:LOCALAPPDATA\UnrealBuildTool\Log.txt"
if (Test-Path $ubt) { $o += (Select-String $ubt -Pattern 'Result:|error C|error LNK|Succeeded|Failed' | Select-Object -Last 25 | ForEach-Object Line) } else { $o += 'no UBT log' }
$o += "--- EDITOR LOG (errors)"
$el = Get-ChildItem "$P\Saved\Logs\*.log" -ErrorAction SilentlyContinue | Sort-Object LastWriteTime | Select-Object -Last 1
if ($el) { $o += (Select-String $el.FullName -Pattern 'Error|Fatal|failed' | Select-Object -Last 20 | ForEach-Object Line) } else { $o += 'no editor log' }
$o | Set-Content $R -Encoding UTF8
Write-Output ("report: " + $R)
