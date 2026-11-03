"""Add the all-in-one movement proof to AVEILCharacter::Tick, or abort without writing.

The proof: eight seconds after boot it closes the Field Manual and chart, pushes an
ANALOG axis (0 -> 1 over 0.6 s, held, ramped down) and then a diagonal stick-shaped
axis, and measures the limb between frames. Binary input reads as a zero step and
then a jump; a grid step reads as equal-sized jumps; smooth motion is a continuous
ramp. It finishes by writing the save through the cheats subsystem.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

ANCHOR = "\t// STUCK DOCTOR: ACCELERATION WITHOUT VELOCITY MEANS THE CAPSULE IS INSIDE"

BLOCK = (
    "\t// PHI SMOOTH PROOF - ALL IN ONE. Eight seconds after boot: close the Field\n"
    "\t// Manual and the chart, push an ANALOG axis (0 -> 1 over 0.6 s, held, ramped\n"
    "\t// down), then a diagonal stick-shaped axis, measuring the limb between frames.\n"
    "\t// Binary input reads as a zero step and then a jump; a grid step reads as\n"
    "\t// equal-sized jumps; smooth motion reads as a continuous ramp. Then it saves.\n"
    "\t{\n"
    "\t\tstatic int32 Phase = 0;\n"
    "\t\tstatic float Clock = 0.f;\n"
    "\t\tstatic FVector Prev = FVector::ZeroVector;\n"
    "\t\tstatic float MinStep = 1.0e9f;\n"
    "\t\tstatic float MaxStep = 0.f;\n"
    "\t\tstatic float SumStep = 0.f;\n"
    "\t\tstatic int32 Frames = 0;\n"
    "\t\tstatic int32 ZeroFrames = 0;\n"
    "\t\tstatic float FwdMoved = 0.f;\n"
    "\t\tstatic float DiagMoved = 0.f;\n"
    "\n"
    "\t\tClock += DeltaSeconds;\n"
    "\t\tif (Phase == 0 && Clock > 8.f)\n"
    "\t\t{\n"
    "\t\t\tPhase = 1;\n"
    "\t\t\tClock = 0.f;\n"
    "\t\t\tPrev = GetActorLocation();\n"
    "\t\t\tif (AVEILPlayerController* ProofPC = Cast<AVEILPlayerController>(GetController()))\n"
    "\t\t\t{\n"
    "\t\t\t\tProofPC->bShowFieldManual = false;\n"
    "\t\t\t\tProofPC->bShowChart = false;\n"
    "\t\t\t\tProofPC->SetInputMode(FInputModeGameOnly());\n"
    "\t\t\t\tProofPC->bShowMouseCursor = false;\n"
    "\t\t\t}\n"
    "\t\t\tUE_LOG(LogVEIL, Warning,\n"
    "\t\t\t\tTEXT(\"PHI SMOOTH PROOF BEGIN: manual=0 chart=0, runner at %s.\"),\n"
    "\t\t\t\t*GetActorLocation().ToCompactString());\n"
    "\t\t}\n"
    "\t\telse if (Phase >= 1 && Phase <= 4)\n"
    "\t\t{\n"
    "\t\t\tfloat Axis = 0.f;\n"
    "\t\t\tFVector2D Dir(0.f, 1.f);\n"
    "\t\t\tif (Phase == 1)\n"
    "\t\t\t{\n"
    "\t\t\t\tAxis = FMath::Clamp(Clock / 0.6f, 0.f, 1.f);\n"
    "\t\t\t\tif (Clock >= 1.6f) { Phase = 2; Clock = 0.f; }\n"
    "\t\t\t}\n"
    "\t\t\telse if (Phase == 2)\n"
    "\t\t\t{\n"
    "\t\t\t\tAxis = FMath::Clamp(1.f - Clock / 0.3f, 0.f, 1.f);\n"
    "\t\t\t\tif (Clock >= 0.3f) { Phase = 3; Clock = 0.f; Prev = GetActorLocation(); }\n"
    "\t\t\t}\n"
    "\t\t\telse if (Phase == 3)\n"
    "\t\t\t{\n"
    "\t\t\t\tAxis = FMath::Clamp(Clock / 0.4f, 0.f, 1.f);\n"
    "\t\t\t\tDir = FVector2D(0.7071f, 0.7071f);\n"
    "\t\t\t\tif (Clock >= 1.0f) { Phase = 4; Clock = 0.f; }\n"
    "\t\t\t}\n"
    "\t\t\telse\n"
    "\t\t\t{\n"
    "\t\t\t\tAxis = FMath::Clamp(1.f - Clock / 0.2f, 0.f, 1.f);\n"
    "\t\t\t\tDir = FVector2D(0.7071f, 0.7071f);\n"
    "\t\t\t\tif (Clock >= 0.2f) { Phase = 5; }\n"
    "\t\t\t}\n"
    "\n"
    "\t\t\tif (Axis > 0.f && Phase <= 4)\n"
    "\t\t\t{\n"
    "\t\t\t\tMove(FInputActionValue(EInputActionValueType::Axis2D,\n"
    "\t\t\t\t\tFVector(Dir.X * Axis, Dir.Y * Axis, 0.f)));\n"
    "\t\t\t\tconst FVector Now = GetActorLocation();\n"
    "\t\t\t\tconst float Step = FVector::Dist2D(Now, Prev);\n"
    "\t\t\t\tPrev = Now;\n"
    "\t\t\t\t++Frames;\n"
    "\t\t\t\tSumStep += Step;\n"
    "\t\t\t\tMinStep = FMath::Min(MinStep, Step);\n"
    "\t\t\t\tMaxStep = FMath::Max(MaxStep, Step);\n"
    "\t\t\t\tif (Step < 0.01f) { ++ZeroFrames; }\n"
    "\t\t\t\tif (Phase <= 2) { FwdMoved += Step; } else { DiagMoved += Step; }\n"
    "\t\t\t}\n"
    "\t\t}\n"
    "\t\telse if (Phase == 5)\n"
    "\t\t{\n"
    "\t\t\tPhase = 6;\n"
    "\t\t\tconst float Mean = (Frames > 0) ? (SumStep / (float)Frames) : 0.f;\n"
    "\t\t\tconst bool bSmooth = Frames > 30 && Mean > 0.5f && MaxStep <= Mean * 3.f + 1.f && ZeroFrames == 0;\n"
    "\t\t\tUE_LOG(LogVEIL, Warning,\n"
    "\t\t\t\tTEXT(\"PHI SMOOTH PROOF: frames=%d forwarded=%.0f cm diagonal=%.0f cm mean=%.2f min=%.2f max=%.2f zero=%d verdict=%s\"),\n"
    "\t\t\t\tFrames, FwdMoved, DiagMoved, Mean, (MinStep > 1.0e8f ? 0.f : MinStep), MaxStep, ZeroFrames,\n"
    "\t\t\t\tbSmooth ? TEXT(\"SMOOTH - analog ramp, no binary step, no grid jump\") : TEXT(\"NOT SMOOTH - see min/max/zero\"));\n"
    "\t\t\tif (UWorld* ProofWorld = GetWorld())\n"
    "\t\t\t{\n"
    "\t\t\t\tif (UPHICheats* Cheats = ProofWorld->GetSubsystem<UPHICheats>())\n"
    "\t\t\t\t{\n"
    "\t\t\t\t\tCheats->SaveNow();\n"
    "\t\t\t\t\tUE_LOG(LogVEIL, Warning, TEXT(\"PHI SAVE PROOF: SaveNow called after the move test.\"));\n"
    "\t\t\t\t}\n"
    "\t\t\t}\n"
    "\t\t}\n"
    "\t}\n"
    "\n"
)

old = ANCHOR.replace("\n", NL)
if src.count(old) != 1:
    print("ABORTED - anchor found %d times (want 1)" % src.count(old))
    raise SystemExit(1)

if '#include "Systems/PHICheats.h"' not in src:
    inc = '#include "Systems/VEILTime.h"'
    if src.count(inc.replace("\n", NL)) != 1:
        print("ABORTED - include anchor not unique")
        raise SystemExit(1)
    src = src.replace(inc.replace("\n", NL), inc.replace("\n", NL) + NL + '#include "Systems/PHICheats.h"', 1)

out = src.replace(old, BLOCK.replace("\n", NL) + old, 1)
io.open(PATH, "w", encoding="utf-8", newline="").write(out)
print("WROTE %s: %d -> %d chars" % (PATH, len(src), len(out)))
