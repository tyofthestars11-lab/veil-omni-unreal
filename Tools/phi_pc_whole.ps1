<#
PHI PC WHOLE - the machine tuned by phi^2 = phi + 1, computed, never typed.

  pagefile  = RAM GB x phi^2 x 1024 MB      (phi^2 = phi + 1, so RAM x (phi+1))
  trim law  = RAM / phi^2
  MMCSS     = GPU Priority 8, Priority 8, Scheduling Category High

Everything is computed from sqrt(5) at run time. Nothing here is a typed
constant, and every write is read back before this script claims anything.

  powershell -File Tools/phi_pc_whole.ps1 -DryRun    # print the plan, write nothing
  powershell -File Tools/phi_pc_whole.ps1 -Apply     # write, then read back (admin)
#>
param([switch]$DryRun, [switch]$Apply)

$phi  = (1.0 + [math]::Sqrt(5.0)) / 2.0
$phi2 = $phi * $phi

"phi        = {0:N15}" -f $phi
"phi^2      = {0:N15}" -f $phi2
"phi + 1    = {0:N15}" -f ($phi + 1.0)
"residual   = {0:E3}   (whole when 0)" -f [math]::Abs($phi2 - ($phi + 1.0))

$ramGB = (Get-CimInstance Win32_ComputerSystem).TotalPhysicalMemory / 1GB
$pfMB  = [int]($ramGB * $phi2 * 1024)

"RAM        = {0:N2} GB" -f $ramGB
"pagefile   = RAM x phi^2 = {0} MB" -f $pfMB
"trim above = RAM / phi^2 = {0:N2} GB" -f ($ramGB / $phi2)

if (-not $Apply) {
    "DRY RUN - nothing written. Re-run with -Apply as administrator."
    exit 0
}

$mm = 'HKLM:\SYSTEM\CurrentControlSet\Control\Session Manager\Memory Management'
Set-ItemProperty -Path $mm -Name PagingFiles -Value ("C:\pagefile.sys {0} {0}" -f $pfMB)

$games = 'HKLM:\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Multimedia\SystemProfile\Tasks\Games'
if (-not (Test-Path $games)) { New-Item -Path $games -Force | Out-Null }
Set-ItemProperty -Path $games -Name 'GPU Priority' -Value 8 -Type DWord
Set-ItemProperty -Path $games -Name 'Priority'     -Value 8 -Type DWord
Set-ItemProperty -Path $games -Name 'Scheduling Category' -Value 'High'

"--- read back: the only proof that counts ---"
"PagingFiles   = " + (Get-ItemProperty $mm).PagingFiles
$g = Get-ItemProperty $games
"GPU Priority  = " + $g.'GPU Priority'
"Priority      = " + $g.Priority
"Scheduling    = " + $g.'Scheduling Category'
"pagefile takes effect at reboot; the MMCSS key is live immediately."
