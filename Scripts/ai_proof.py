"""Prove the hostile proximity trigger fires: post a hunter next to the Runner."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


# 1. hunter getter
p = P + "VeilNpc.h"
s = rd(p)
if "IsHunter" not in s:
    a = "\tconst TCHAR* GetRaceName() const;"
    if s.count(a) != 1:
        raise SystemExit("MISS npc getter anchor")
    s = s.replace(a, a + "\n\tbool IsHunter() const { return bHunter; }", 1)
    wr(p, s)
    print("npc: IsHunter exposed")

# 2. header members
p = P + "VeilGameMode.h"
s = rd(p)
if "AiTestMove" not in s:
    i = s.rfind("};")
    if i < 0:
        raise SystemExit("MISS header tail")
    s = s[:i] + ("\tFTimerHandle AiTestMoveTimer;\n"
                 "\tFTimerHandle AiTestReportTimer;\n"
                 "\tdouble AiTestVitBefore = 0.0;\n"
                 "\tFString AiTestSubject;\n"
                 "\tvoid AiTestMove();\n"
                 "\tvoid AiTestReport();\n") + s[i:]
    wr(p, s)
    print("header: ai proof members")

# 3. schedule the proof
p = P + "VeilGameMode.cpp"
s = rd(p)
if "AiTestMoveTimer" not in s:
    a = "\t\t\t\tSelfTestRunner->VIT, SelfTestRunner->VITMax, SelfTestRunner->Mito.ATP);"
    if s.count(a) != 1:
        raise SystemExit("MISS schedule anchor")
    s = s.replace(a, a + ("\n\t\t\tAiTestVitBefore = SelfTestRunner->VIT;\n"
                          "\t\t\tGetWorldTimerManager().SetTimer(AiTestMoveTimer, this, &AVeilGameMode::AiTestMove, 3.0f, false);\n"
                          "\t\t\tGetWorldTimerManager().SetTimer(AiTestReportTimer, this, &AVeilGameMode::AiTestReport, 7.0f, false);"), 1)
    if '#include "EngineUtils.h"' not in s:
        s = s.replace('#include "VeilGameMode.h"', '#include "VeilGameMode.h"\n#include "EngineUtils.h"', 1)
    wr(p, s)
    print("game mode: ai proof scheduled at 3s, report at 7s")

if "AVeilGameMode::AiTestMove" not in s:
    s = rd(p)
    s += '''

void AVeilGameMode::AiTestMove()
{
	APawn* P = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
	AVeilRunner* R = Cast<AVeilRunner>(P);
	if (!R)
	{
		UE_LOG(LogVEILWorld, Display, TEXT("VEIL AI PROOF: no Runner to test against"));
		return;
	}
	for (TActorIterator<AVeilNpc> It(GetWorld()); It; ++It)
	{
		if (It->IsHunter())
		{
			It->SetActorLocation(R->GetActorLocation() + FVector(PHI_Constants::Reach(1.8), 0.0, 0.0));
			AiTestSubject = FString(It->GetName()) + TEXT(" (") + It->GetRaceName() + TEXT(")");
			UE_LOG(LogVEILWorld, Display,
				TEXT("VEIL AI PROOF: %s posted inside its own reach, Runner VIT %.0f"),
				*AiTestSubject, R->VIT);
			return;
		}
	}
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL AI PROOF: no hunter found in the world"));
}

void AVeilGameMode::AiTestReport()
{
	APawn* P = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
	AVeilRunner* R = Cast<AVeilRunner>(P);
	if (!R)
	{
		return;
	}
	const bool bStruck = R->VIT < AiTestVitBefore - 1.0;
	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL AI PROOF: %s | VIT %.0f -> %.0f | %s"),
		AiTestSubject.IsEmpty() ? TEXT("unknown body") : *AiTestSubject,
		AiTestVitBefore, R->VIT,
		bStruck ? TEXT("STRIKE FIRED FROM ITS POST") : TEXT("NO STRIKE"));
}
'''
    wr(p, s)
    print("game mode: ai proof functions appended")
print("ai proof pass done")
