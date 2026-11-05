"""Hostile posts are held outside the Hub sanctuary, and the proof hunter goes home."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


h = P + "VeilGameMode.h"
s = rd(h)
if "SanctuaryPass" not in s:
    a = "\tvoid AiTestReport();"
    if s.count(a) != 1:
        raise SystemExit("MISS header ai anchor")
    s = s.replace(a, a + "\n\tTWeakObjectPtr<class AVeilNpc> AiTestBody;\n"
                       "\tFVector AiTestHome = FVector::ZeroVector;\n"
                       "\tvoid SanctuaryPass();", 1)
    wr(h, s)
    print("header: sanctuary + proof home")

c = P + "VeilGameMode.cpp"
s = rd(c)

# remember where the proof hunter came from
a = "\t\t\tIt->SetActorLocation(R->GetActorLocation() + FVector(PHI_Constants::Reach(6.0), 0.0, 0.0));"
if a in s and "AiTestHome = It->GetActorLocation();" not in s:
    s = s.replace(a, "\t\t\tAiTestBody = *It;\n\t\t\tAiTestHome = It->GetActorLocation();\n" + a, 1)
    print("proof: hunter home remembered")

# send it back after the report
a = "\t\tbStruck ? TEXT(\"STRIKE FIRED FROM ITS POST\") : TEXT(\"NO STRIKE\"));"
if a in s and "AiTestBody->SetActorLocation(AiTestHome)" not in s:
    s = s.replace(a, a + "\n\tif (AiTestBody.IsValid())\n\t{\n\t\tAiTestBody->SetActorLocation(AiTestHome);\n\t\tUE_LOG(LogVEILWorld, Display, TEXT(\"VEIL AI PROOF: hunter returned to its own post\"));\n\t}", 1)
    print("proof: hunter returned home")

# the sanctuary law runs before the timers
a = "\t\t\tAiTestVitBefore = SelfTestRunner->VIT;"
if a in s and "SanctuaryPass();" not in s:
    s = s.replace(a, "\t\t\tSanctuaryPass();\n" + a, 1)
    print("sanctuary: called at boot")

if "AVeilGameMode::SanctuaryPass" not in s:
    s += '''

void AVeilGameMode::SanctuaryPass()
{
	APawn* P = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
	if (!P)
	{
		return;
	}
	const FVector Spawn = P->GetActorLocation();
	const double Hold = PHI_Constants::Reach(11.0);
	int32 Moved = 0;
	int32 Idx = 0;
	double MinD = 1.0e9;
	for (TActorIterator<AVeilNpc> It(GetWorld()); It; ++It, ++Idx)
	{
		if (!It->IsHunter())
		{
			continue;
		}
		FVector Loc = It->GetActorLocation();
		const double D = FVector::Dist2D(Loc, Spawn);
		if (D < Hold)
		{
			FVector Dir = (Loc - Spawn).GetSafeNormal2D();
			if (Dir.IsNearlyZero())
			{
				const double A = FMath::DegreesToRadians(PHI_Constants::GoldenAngleDeg * static_cast<double>(Idx));
				Dir = FVector(FMath::Cos(A), FMath::Sin(A), 0.0);
			}
			const FVector Next = Spawn + Dir * Hold;
			const FVector From(Next.X, Next.Y, Spawn.Z + PHI_Constants::Reach(13.0));
			const FVector To(Next.X, Next.Y, Spawn.Z - PHI_Constants::Reach(12.0));
			FHitResult Hit;
			It->SetActorLocation(FVector(Next.X, Next.Y,
				GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_WorldStatic) ? Hit.ImpactPoint.Z : Spawn.Z));
			++Moved;
			Loc = It->GetActorLocation();
		}
		MinD = FMath::Min(MinD, FVector::Dist2D(Loc, Spawn));
	}
	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL SANCTUARY: %d hostile post(s) held out at >= %.0f from the Hub spawn, nearest now %.0f"),
		Moved, Hold, MinD);
}
'''
    wr(c, s)
    print("sanctuary: function appended")
else:
    wr(c, s)
print("sanctuary pass done")
