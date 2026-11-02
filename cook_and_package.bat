@echo off
REM COOK AND PACKAGE - run this when you want to LOOK at the game.
REM It is heavy: it holds the machine for a long time and writes gigabytes.
"C:\Program Files\Epic Games\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun ^
 -project="D:\VEIL-OMNI\VEIL.uproject" -noP4 -platform=Win64 -clientconfig=Development ^
 -cook -allmaps -build -stage -pak -archive -archivedirectory="D:\VEIL-OMNI\Saved\StagedBuilds"
