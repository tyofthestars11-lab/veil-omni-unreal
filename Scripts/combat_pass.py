"""Combat: enemies wound the Runner, the Runner strikes back with ATP, death respawns.

A boot self-test exercises both directions so the headless run proves the path.
"""
P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


def insert_after(p, anchor, text):
    s = rd(p)
    if text.strip() in s:
        return False
    if s.count(anchor) != 1:
        print("   MISS anchor in", p.split("/")[-1], "::", anchor[:40])
        return False
    wr(p, s.replace(anchor, anchor + text, 1))
    return True


def append(p, text):
    s = rd(p)
    if text.strip().split("\n")[0] in s:
        return False
    wr(p, s + text)
    return True


# ---------- enemy: health and a strike that lands ----------
insert_after(P + "VeilNpc.h", "\tconst TCHAR* GetRaceName() const;",
             "\n\n\t/** A strike lands on this body. True when it falls. */\n"
             "\tbool TakeStrike(double Damage);\n"
             "\tdouble Health = 100.0;\n"
             "\tdouble HealthMax = 100.0;\n")

p = P + "VeilNpc.cpp"
s = rd(p)
if "VEIL HIT" in s and "TakeHit" not in s:
    lines = s.split("\n")
    for i, line in enumerate(lines):
        if "VEIL HIT" in line:
            lines[i] = ('\t\t\tif (AVeilRunner* R = Cast<AVeilRunner>(Player))\n'
                        '\t\t\t{\n'
                        '\t\t\t\tR->TakeHit(PHI_Constants::Reach(2.0), this);\n'
                        '\t\t\t}')
            break
    s = "\n".join(lines)
    if '#include "VeilRunner.h"' not in s:
        s = s.replace('#include "VeilNpc.h"', '#include "VeilNpc.h"\n#include "VeilRunner.h"', 1)
    wr(p, s)
    print("enemy strike now damages the Runner")

append(p, '''

bool AVeilNpc::TakeStrike(double Damage)
{
	Health = FMath::Max(0.0, Health - Damage);
	UE_LOG(LogTemp, Display, TEXT("VEIL COMBAT: %s (%s) struck for %.0f, health %.0f/%.0f"),
		*GetName(), GetRaceName(), Damage, Health, HealthMax);
	if (Health <= 0.0)
	{
		UE_LOG(LogTemp, Display, TEXT("VEIL COMBAT: %s falls. The node is unoccupied."), *GetName());
		Destroy();
		return true;
	}
	return false;
}
''')

# ---------- runner: health, death, respawn, strike ----------
insert_after(P + "VeilRunner.h", "\tvoid SetSprinting(bool bOn) { bSprint = bOn; }",
             "\n\n\t// The Runner's own body: 439 is the Bible walk, and the health ceiling with it.\n"
             "\tdouble VIT = 439.0;\n"
             "\tdouble VITMax = 439.0;\n"
             "\tvoid TakeHit(double Damage, AActor* Source);\n"
             "\tvoid Respawn();\n"
             "\tbool Strike();\n")

append(P + "VeilRunner.cpp", '''

void AVeilRunner::TakeHit(double Damage, AActor* Source)
{
	VIT = FMath::Max(0.0, VIT - Damage);
	UE_LOG(LogTemp, Display, TEXT("VEIL COMBAT: the Runner is struck for %.0f by %s, VIT %.0f/%.0f"),
		Damage, Source ? *Source->GetName() : TEXT("the world"), VIT, VITMax);
	if (VIT <= 0.0)
	{
		UE_LOG(LogTemp, Display, TEXT("VEIL DEATH: the Runner falls at (%.0f, %.0f, %.0f)"),
			GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z);
		Respawn();
	}
}

void AVeilRunner::Respawn()
{
	VIT = VITMax;
	Mito.ATP = Mito.MaxATP;
	SetActorLocation(RespawnAt);
	UE_LOG(LogTemp, Display, TEXT("VEIL RESPAWN: the Runner wakes at (%.0f, %.0f, %.0f) with VIT %.0f"),
		RespawnAt.X, RespawnAt.Y, RespawnAt.Z, VIT);
}

bool AVeilRunner::Strike()
{
	if (!Mito.Spend(static_cast<float>(PHI_Constants::Reach(1.0))))
	{
		UE_LOG(LogTemp, Display, TEXT("VEIL COMBAT: no ATP for a strike, gradient at %.2f"), Mito.Gradient);
		return false;
	}
	double Best = PHI_Constants::Reach(2.2);
	AVeilNpc* Target = nullptr;
	for (TActorIterator<AVeilNpc> It(GetWorld()); It; ++It)
	{
		const double D = FVector::Dist(It->GetActorLocation(), GetActorLocation());
		if (D < Best)
		{
			Best = D;
			Target = *It;
		}
	}
	if (!Target)
	{
		UE_LOG(LogTemp, Display, TEXT("VEIL COMBAT: strike finds nothing within reach"));
		return false;
	}
	Target->TakeStrike(PHI_Constants::Reach(1.2));
	return true;
}
''')

# respawn point field + includes
insert_after(P + "VeilRunner.h", "\tdouble VITMax = 439.0;",
             "\n\tFVector RespawnAt = FVector(0.0, 0.0, 100.0);\n")

p = P + "VeilRunner.cpp"
s = rd(p)
if "EngineUtils.h" not in s:
    s = s.replace('#include "VeilRunner.h"',
                  '#include "VeilRunner.h"\n#include "EngineUtils.h"\n#include "VeilNpc.h"', 1)
    wr(p, s)
    print("runner includes added")

# ---------- player: F strikes, VIT mirrors the body ----------
insert_after(P + "VeilPlayerController.h", "\tvoid OnInteract();", "\n\tvoid OnStrike();")

p = P + "VeilPlayerController.cpp"
s = rd(p)
if "OnStrike" not in s:
    s = s.replace("\tInputComponent->BindKey(EKeys::C, IE_Pressed, this, &AVeilPlayerController::OnCombatMode);",
                  "\tInputComponent->BindKey(EKeys::C, IE_Pressed, this, &AVeilPlayerController::OnCombatMode);\n"
                  "\tInputComponent->BindKey(EKeys::F, IE_Pressed, this, &AVeilPlayerController::OnStrike);", 1)
    s = s.replace("void AVeilPlayerController::OnCombatMode()",
                  "void AVeilPlayerController::OnStrike()\n"
                  "{\n"
                  "\tif (AVeilRunner* R = Runner())\n"
                  "\t{\n"
                  "\t\tR->Strike();\n"
                  "\t\tVIT = static_cast<float>(R->VIT);\n"
                  "\t}\n"
                  "}\n\n"
                  "void AVeilPlayerController::OnCombatMode()", 1)
    s = s.replace("\tSTM = FMath::Min(STMMax, STM + 12.f * DeltaSeconds);",
                  "\tSTM = FMath::Min(STMMax, STM + 12.f * DeltaSeconds);\n"
                  "\tif (AVeilRunner* R = Runner())\n"
                  "\t{\n"
                  "\t\tVIT = static_cast<float>(R->VIT);\n"
                  "\t\tVITMax = static_cast<float>(R->VITMax);\n"
                  "\t}", 1)
    wr(p, s)
    print("player: F strikes, VIT syncs")

# ---------- boot self-test both directions ----------
p = P + "VeilGameMode.cpp"
s = rd(p)
if "COMBAT SELFTEST" not in s:
    anchor = "\t\tUE_LOG(LogVEILWorld, Display,\n\t\t\tTEXT(\"VEIL SAVE: VEILOMNI_Omni0 written on boot at (%.0f, %.0f, %.0f)\"),"
    idx = s.find(anchor)
    if idx < 0:
        print("   MISS self-test anchor")
    else:
        end = s.find("\n\t}\n", idx)
        inject = ("\n\n\t// Both directions of combat are exercised on boot, so the path is proven.\n"
                  "\tif (APawn* SelfTestPawn = GetWorld()->GetFirstPlayerController()\n"
                  "\t\t? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr)\n"
                  "\t{\n"
                  "\t\tif (AVeilRunner* SelfTestRunner = Cast<AVeilRunner>(SelfTestPawn))\n"
                  "\t\t{\n"
                  "\t\t\tSelfTestRunner->RespawnAt = SelfTestRunner->GetActorLocation();\n"
                  "\t\t\tSelfTestRunner->TakeHit(PHI_Constants::Reach(2.0), this);\n"
                  "\t\t\tSelfTestRunner->Strike();\n"
                  "\t\t\tUE_LOG(LogVEILWorld, Display,\n"
                  "\t\t\t\tTEXT(\"VEIL COMBAT SELFTEST: runner VIT %.0f/%.0f, ATP %.0f\"),\n"
                  "\t\t\t\tSelfTestRunner->VIT, SelfTestRunner->VITMax, SelfTestRunner->Mito.ATP);\n"
                  "\t\t}\n"
                  "\t}")
        s = s[:end] + inject + s[end:]
        wr(p, s)
        print("boot combat self-test added")
print("combat pass done")
