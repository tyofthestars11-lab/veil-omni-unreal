"""Round 3: proof camera without a pawn, toolsets off the way the engine does it."""
import os
import re

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
INI = "C:/Users/Admin/HarkGame/Config/DefaultEngine.ini"
BASE = "C:/Program Files/Epic Games/UE_5.8/Engine/Config/BaseGame.ini"

gm = open(GM, encoding="utf-8", errors="replace").read()

old = ('\t\tif (Idx == 4)\n'
       '\t\t{\n'
       '\t\t\tif (const APawn* ProofPawn = PC->GetPawn())\n'
       '\t\t\t{\n'
       '\t\t\t\tconst FVector Fw = ProofPawn->GetActorForwardVector();\n'
       '\t\t\t\tconst FVector Rt = FVector::CrossProduct(FVector::UpVector, Fw).GetSafeNormal();\n'
       '\t\t\t\tCamAt = ProofPawn->GetActorLocation() - Fw * 420.0 + FVector(0.0, 0.0, 175.0) + Rt * 95.0;\n'
       '\t\t\t\tAimPoint = ProofPawn->GetActorLocation() + Fw * 1400.0 + FVector(0.0, 0.0, 140.0);\n'
       '\t\t\t\tUE_LOG(LogVEILWorld, Display,\n'
       '\t\t\t\t\tTEXT("VEIL PROOF CAMERA: over the shoulder, 420 behind and 175 above the Runner"));\n'
       '\t\t\t}\n'
       '\t\t}')
new = ('\t\tif (Idx == 4)\n'
       '\t\t{\n'
       '\t\t\t// The controller has no pawn during a record run, so stand the proof camera\n'
       '\t\t\t// on the Runner\'s standing point and use the eye rotation the Runner is given.\n'
       '\t\t\tconst FRotator Eye = PC->GetControlRotation();\n'
       '\t\t\tconst FVector Fw = Eye.Vector();\n'
       '\t\t\tconst FVector Rt = FRotationMatrix(Eye).GetScaledAxis(EAxis::Y);\n'
       '\t\t\tconst FVector Body = PC->GetPawn() ? PC->GetPawn()->GetActorLocation() : Stand;\n'
       '\t\t\tCamAt = Body - Fw * 420.0 + FVector(0.0, 0.0, 175.0) + Rt * 95.0;\n'
       '\t\t\tAimPoint = Body + Fw * 1400.0 + FVector(0.0, 0.0, 140.0);\n'
       '\t\t\tUE_LOG(LogVEILWorld, Display,\n'
       '\t\t\t\tTEXT("VEIL PROOF CAMERA: over the shoulder at (%.0f, %.0f, %.0f), 420 behind and 175 above the Runner"),\n'
       '\t\t\t\tBody.X, Body.Y, Body.Z);\n'
       '\t\t}')

if "over the shoulder at (%.0f" in gm:
    print("proof camera: already pawn-independent")
elif old in gm:
    gm = gm.replace(old, new, 1)
    open(GM, "w", encoding="utf-8").write(gm)
    print("proof camera: now keyed to the stand point + eye rotation")
else:
    print("proof camera: MISS")

# Which section does the engine use for DisabledPlugins?
base = open(BASE, encoding="utf-8", errors="replace").read().splitlines()
section = None
for line in base:
    if line.strip().startswith("["):
        section = line.strip()
    if "DisabledPlugins=USDStageEditor" in line:
        break
print("engine section:", section)

ini = open(INI, encoding="utf-8", errors="replace").read()
guard = "; VEIL: the experimental toolsets mount with Python init scripts that throw"
if guard in ini:
    print("ini: already patched")
elif section:
    block = ("\n" + guard + " on this engine build (unreal has no AgentSkill/PythonTestRunner).\n"
             + section + "\n"
             "+DisabledPlugins=EditorToolset\n"
             "+DisabledPlugins=ToolsetRegistry\n"
             "+DisabledPlugins=Toolsets\n")
    ini = ini.rstrip("\n") + "\n" + block
    open(INI, "w", encoding="utf-8").write(ini)
    print("ini: disabled EditorToolset, ToolsetRegistry, Toolsets in", section)
else:
    print("ini: MISS no engine section found")
