# -*- coding: utf-8 -*-
"""THE CHOICE, AND THE MARKET.

1. AVEILHubMarket is spawned once at the Hub's authority point and builds the
   reference's market: 12 lantern-capped phi pillars with blue banners, 8 stalls
   under blue canvas, 5 faceted crystal peaks. It stands down if one is present.

2. The runner is a choice: Garret (male) or Elara (female), both real Polyart
   bodies with their own locomotion graphs. F5 swaps in game and the log names
   which body is standing.
"""
import io
import sys

ROOT = "D:/VEIL-OMNI/"
fails = []


def read(p):
    with io.open(ROOT + p, "r", encoding="utf-8", newline="") as f:
        raw = f.read()
    return raw.replace("\r\n", "\n"), ("\r\n" in raw)


def write(p, text, crlf):
    with io.open(ROOT + p, "w", encoding="utf-8", newline="") as f:
        f.write(text.replace("\n", "\r\n") if crlf else text)


def sub(t, old, new, label):
    n = t.count(old)
    if n != 1:
        fails.append("%s count=%d" % (label, n))
        return t
    return t.replace(old, new, 1)


# ---------------------------------------------------------------------------
# GameMode: include, spawn, key, method.
# ---------------------------------------------------------------------------
GM = "Source/VEIL/Private/Game/VEILGameMode.cpp"
gm, gm_crlf = read(GM)
if "VEILHubMarket.h" not in gm:
    gm = sub(gm, '#include "Story/VEILCast.h"\n',
             '#include "Story/VEILCast.h"\n#include "World/VEILHubMarket.h"\n', "hub include")

if "PHI HUB MARKET SPAWN" not in gm:
    spawn_anchor = '\t\t\tUE_LOG(LogTemp, Display, TEXT("PHI CONTENT OWNER: concept composition holds %d piece(s)."), Composition);\n\t\t}, 3.2f, false);'
    spawn_new = ('\t\t\tUE_LOG(LogTemp, Display, TEXT("PHI CONTENT OWNER: concept composition holds %d piece(s)."), Composition);\n'
                 '\t\t\t// PHI HUB MARKET SPAWN. The reference Hub is a cliff market under the Spire;\n'
                 '\t\t\t// the families pass stands down when the composition owns the district, so\n'
                 '\t\t\t// this is the pass that lays the twelve pillars, eight blue-canvas stalls and\n'
                 '\t\t\t// five crystal peaks at the Hub\'s own authority point. Once, and guarded.\n'
                 '\t\t\t{\n'
                 '\t\t\t\tint32 Market = 0;\n'
                 '\t\t\t\tfor (TActorIterator<AVEILHubMarket> It(GetWorld()); It; ++It) { ++Market; }\n'
                 '\t\t\t\tif (Market == 0)\n'
                 '\t\t\t\t{\n'
                 '\t\t\t\t\tFActorSpawnParameters HubParams;\n'
                 '\t\t\t\t\tHubParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;\n'
                 '\t\t\t\t\tif (AVEILHubMarket* Built = GetWorld()->SpawnActor<AVEILHubMarket>(\n'
                 '\t\t\t\t\t\tAVEILHubMarket::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, HubParams))\n'
                 '\t\t\t\t\t{\n'
                 '\t\t\t\t\t\tBuilt->Build();\n'
                 '\t\t\t\t\t}\n'
                 '\t\t\t\t}\n'
                 '\t\t\t}\n'
                 '\t\t}, 3.2f, false);')
    gm = sub(gm, spawn_anchor, spawn_new, "hub spawn")

if "&AVEILPlayerController::ToggleRunnerBody" not in gm:
    gm = sub(gm, '\t\tInputComponent->BindKey(EKeys::H, IE_Pressed, this, &AVEILPlayerController::ToggleFieldManual);\n',
             '\t\tInputComponent->BindKey(EKeys::H, IE_Pressed, this, &AVEILPlayerController::ToggleFieldManual);\n'
             '\t\t// F5: THE RUNNER IS A CHOICE. Garret (male) and Elara (female) are both real\n'
             '\t\t// Polyart bodies with their own locomotion graphs; the swap is live.\n'
             '\t\tInputComponent->BindKey(EKeys::F5, IE_Pressed, this, &AVEILPlayerController::ToggleRunnerBody);\n',
             "f5 binding")

if "void AVEILPlayerController::ToggleRunnerBody" not in gm:
    anchor = "void AVEILPlayerController::ToggleLog()\n"
    impl = ('void AVEILPlayerController::ToggleRunnerBody()\n'
            '{\n'
            '\tif (AVEILCharacter* Runner = Cast<AVEILCharacter>(GetPawn()))\n'
            '\t{\n'
            '\t\tRunner->SetRunnerFemale(!Runner->IsFemaleRunner());\n'
            '\t}\n'
            '}\n\n')
    gm = sub(gm, anchor, impl + anchor, "toggle impl")

# ---------------------------------------------------------------------------
# Header + character implementation.
# ---------------------------------------------------------------------------
GH = "Source/VEIL/Public/Game/VEILGameMode.h"
gh, gh_crlf = read(GH)
if "ToggleRunnerBody" not in gh:
    gh = sub(gh, "\tvoid ToggleFieldManual();\n",
             "\tvoid ToggleFieldManual();\n\n"
             "\t/** F5: swap the runner's body between Garret (male) and Elara (female). */\n"
             "\tvoid ToggleRunnerBody();\n", "toggle decl")

CH = "Source/VEIL/Public/Character/VEILCharacter.h"
ch, ch_crlf = read(CH)
if "SetRunnerFemale" not in ch:
    ch = sub(ch, "\tbool IsSprintingNow() const { return bSprinting; }\n",
             "\t/** THE RUNNER IS A CHOICE: a man or a woman, both real Polyart bodies with their\n"
             "\t *  own locomotion graphs. F5 swaps in game; the log names the body. */\n"
             "\tUFUNCTION(BlueprintCallable, Category = \"VEIL|Body\")\n"
             "\tvoid SetRunnerFemale(bool bFemale);\n"
             "\tUFUNCTION(BlueprintPure, Category = \"VEIL|Body\")\n"
             "\tbool IsFemaleRunner() const { return bFemaleRunner; }\n\n"
             "\tbool IsSprintingNow() const { return bSprinting; }\n", "runner decl")
    ch = sub(ch, "\tbool bSprinting = false;\n",
             "\tbool bSprinting = false;\n"
             "\t/** Which body the player chose. */\n"
             "\tbool bFemaleRunner = false;\n", "runner member")

CC = "Source/VEIL/Private/Character/VEILCharacter.cpp"
cc, cc_crlf = read(CC)
if "void AVEILCharacter::SetRunnerFemale" not in cc:
    if '#include "Animation/AnimInstance.h"' not in cc:
        cc = sub(cc, '#include "Character/VEILCharacter.h"\n',
                 '#include "Character/VEILCharacter.h"\n#include "Animation/AnimInstance.h"\n#include "Engine/SkeletalMesh.h"\n',
                 "runner includes")
    impl = (r'''void AVEILCharacter::SetRunnerFemale(bool bFemale)
{
	// TWO REAL BODIES. Garret and Elara ship in the Polyart pack with their own
	// locomotion graphs, so the choice is a body swap, not a tint. The mesh, the
	// graph and the flag change together, and the log names which one is standing
	// so a still frame can be checked against it.
	const TCHAR* MeshPath = bFemale
		? TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Elara_FullBody.SK_Elara_FullBody")
		: TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody");
	const TCHAR* GraphPath = bFemale
		? TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/AnimationBlueprints/ABP_FemaleCharacter.ABP_FemaleCharacter_C")
		: TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/AnimationBlueprints/ABP_MaleCharacter.ABP_MaleCharacter_C");

	USkeletalMesh* Body = LoadObject<USkeletalMesh>(nullptr, MeshPath);
	if (!Body || !GetMesh())
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI RUNNER: %s body did not load; keeping the one that is standing."),
			bFemale ? TEXT("Elara") : TEXT("Garret"));
		return;
	}
	GetMesh()->SetSkeletalMeshAsset(Body);
	GetMesh()->bDisableMorphTarget = true;
	if (UClass* Graph = LoadClass<UAnimInstance>(nullptr, GraphPath))
	{
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(Graph);
	}
	bFemaleRunner = bFemale;
	UE_LOG(LogVEIL, Warning, TEXT("PHI RUNNER: body is now %s (%s) - the player's choice, not a placeholder."),
		bFemale ? TEXT("ELARA") : TEXT("GARRET"), bFemale ? TEXT("female") : TEXT("male"));
}

''')
    cc = sub(cc, "void AVEILCharacter::BuildFallbackInput()\n", impl + "void AVEILCharacter::BuildFallbackInput()\n", "runner impl")

if fails:
    print("FAILS:", fails)
    sys.exit(1)

write(GM, gm, gm_crlf)
write(GH, gh, gh_crlf)
write(CH, ch, ch_crlf)
write(CC, cc, cc_crlf)
print("hub spawn =", "PHI HUB MARKET SPAWN" in gm)
print("f5 =", "ToggleRunnerBody" in gm and "ToggleRunnerBody" in gh)
print("runner =", "SetRunnerFemale" in ch and "SetRunnerFemale" in cc)
print("PATCH: PASS")
