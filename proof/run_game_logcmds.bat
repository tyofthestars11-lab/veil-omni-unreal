@echo off
cd /d D:\VEIL-OMNI
"C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\VEIL-OMNI\VEIL.uproject" /Engine/Maps/Entry -game -windowed -log -stdout -LogCmds="Log LogVEIL Log" > "D:\VEIL-OMNI\proof\phi_logcmds.log" 2>&1
