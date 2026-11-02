@echo off
REM COOK AND PACKAGE - run this when you want to LOOK at the game.
REM It is heavy: it holds the machine for a while and writes gigabytes.
REM No -archive: archiving to the same folder it stages into is what made UAT
REM copy each manifest onto itself until it gave up. The staged build plays.
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
 -project="D:\VEIL-OMNI\VEIL.uproject" -noP4 -platform=Win64 -clientconfig=Development ^
 -cook -allmaps -build -stage -pak
