import io

ROOT = r"D:\VEIL-OMNI" + "\\"
applied = []

def rd(p):
    return io.open(ROOT + p, encoding="utf-8", newline=None).read()

def wr(p, s):
    io.open(ROOT + p, "w", encoding="utf-8", newline="\r\n").write(s)

def sub(path, old, new, tag):
    t = rd(path)
    if new in t:
        applied.append(tag + " (already)")
        return
    if old not in t:
        applied.append(tag + " >>> MISSED")
        return
    wr(path, t.replace(old, new, 1))
    applied.append(tag)

QH = "Source/VEIL/Public/Systems/OMNIQuestSystem.h"
QC = "Source/VEIL/Private/Systems/OMNIQuestSystem.cpp"
PH = "Source/VEIL/Public/Systems/VEILProgression.h"
PC = "Source/VEIL/Private/Systems/VEILProgression.cpp"
CH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
HD = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"

# ------------------------------------------------------------ quest header
sub(QH,
'''	static int32 XPNeedForLevel(int32 InLevel);''',
'''	/** The nearest seam still worth walking to: name, distance, whose ground. */
	void TrackMine(const FString& Name, float Centimetres, const FString& Owner);

	/** Claim a named seam. False if it is already the runner's - and never pays twice. */
	bool ClaimMine(const FName& MineName, const FString& Owner, int32 MineGold, int32 Total);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Quest")
	TArray<FName> ClaimedMines;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Quest")
	FString TrackedMine;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Quest")
	FString TrackedOwner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Quest")
	float TrackedCm = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Quest")
	FString LastClaimName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Quest")
	FString LastClaimOwner;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Quest")
	float LastClaimAt = -100.f;

	static int32 XPNeedForLevel(int32 InLevel);''',
"quest.h claim api")

# ------------------------------------------------------------ quest cpp
sub(QC,
'''int32 UOMNIQuestSystem::XPNeedForLevel(int32 InLevel)''',
'''void UOMNIQuestSystem::TrackMine(const FString& Name, float Centimetres, const FString& Owner)
{
	// Logged on change only: a per-second line would bury the run in itself.
	if (TrackedMine == Name && FMath::IsNearlyEqual(TrackedCm, Centimetres, 1.f)) { return; }
	TrackedMine = Name;
	TrackedCm = Centimetres;
	TrackedOwner = Owner;
	if (Name.IsEmpty())
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI TRACK: every named seam in the world is the runner's."));
		return;
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI TRACK: nearest unclaimed seam %s on %s ground, %.0f m away."),
		*Name, *Owner, Centimetres / 100.f);
}

bool UOMNIQuestSystem::ClaimMine(const FName& MineName, const FString& Owner, int32 MineGold, int32 Total)
{
	if (MineName.IsNone() || ClaimedMines.Contains(MineName)) { return false; }
	ClaimedMines.Add(MineName);
	Gold += MineGold;
	LastClaimName = MineName.ToString();
	LastClaimOwner = Owner;
	LastClaimAt = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	AwardXP(60, TEXT("a named seam claimed"));
	PersistToBook();
	UE_LOG(LogVEIL, Warning, TEXT("PHI CLAIM: %s on %s ground is the runner's - %d of %d seams, +%d gold."),
		*LastClaimName, *Owner, ClaimedMines.Num(), Total, MineGold);
	return true;
}

int32 UOMNIQuestSystem::XPNeedForLevel(int32 InLevel)''',
"quest.cpp claim + track")

sub(QC,
'''		Fragments = Loaded->Fragments;
		if (const UVEILOmniSave* Omni = Cast<UVEILOmniSave>(Loaded))''',
'''		Fragments = Loaded->Fragments;
		ClaimedMines = Loaded->ClaimedMines;
		if (const UVEILOmniSave* Omni = Cast<UVEILOmniSave>(Loaded))''',
"quest.cpp load seams")

# ------------------------------------------------------------ save struct
sub(PH,
'''	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	int32 Fragments = 0;''',
'''	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	int32 Fragments = 0;

	/** Which named seams this run actually took. A claim is progress, so it saves. */
	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	TArray<FName> ClaimedMines;''',
"save.h claimed seams")

sub(PH,
'''	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Progression")
	int32 Fragments = 0;''',
'''	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Progression")
	int32 Fragments = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Progression")
	TArray<FName> ClaimedMines;''',
"progression.h claimed seams")

# ------------------------------------------------------------ save io
sub(PC,
'''	Save->Fragments = Fragments;''',
'''	Save->Fragments = Fragments;
	Save->ClaimedMines = ClaimedMines;''',
"save.cpp write seams")

sub(PC,
'''	Fragments = Save->Fragments;''',
'''	Fragments = Save->Fragments;
	ClaimedMines = Save->ClaimedMines;''',
"save.cpp read seams")

sub(PC,
'''	Quest->Fragments = Fragments;
}''',
'''	Quest->Fragments = Fragments;
	Quest->ClaimedMines = ClaimedMines;
}''',
"save.cpp push seams")

sub(PC,
'''	Fragments = Quest->Fragments;
}''',
'''	Fragments = Quest->Fragments;
	ClaimedMines = Quest->ClaimedMines;
}''',
"save.cpp pull seams")

# ------------------------------------------------------------ character
sub(CH,
'''#include "Systems/OMNIQuestSystem.h"''',
'''#include "EngineUtils.h"
#include "Systems/OMNIQuestSystem.h"
#include "World/VEILWilds.h"''',
"character includes")

sub(CH,
'''	// A BEACON IS FORGED BY STANDING AT IT AND PRESSING THE INTERACT KEY, which is''',
'''	// A SEAM IS CLAIMED BY PRESSING THE KEY, not by walking near it. The nearest
	// unclaimed mine within arm's length is what this press is about; the quest book
	// decides whether it was already taken, so the gold can never be paid twice.
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AVEILWildsPopulation> It(World); It; ++It)
		{
			AVEILMineNode* Mine = It->NearestUnclaimedMine(GetActorLocation(), 700.f);
			if (!Mine) { break; }
			UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();
			if (Quest && Quest->ClaimMine(FName(*Mine->MineName()), Mine->MineOwner(), Mine->ClaimValue(), 24))
			{
				Mine->Claim(this);
			}
			break;
		}
	}

	// A BEACON IS FORGED BY STANDING AT IT AND PRESSING THE INTERACT KEY, which is''',
"character claim on E")

# ------------------------------------------------------------ hud
sub(HD,
'''		Shadow(FString::Printf(TEXT("FORGE %d   BEACONS %d/3   %s"),
			Q->ForgeLevel, Q->ClaimedBeacons.Num(), *RegionName),''',
'''		Shadow(FString::Printf(TEXT("FORGE %d   BEACONS %d/3   SEAMS %d/24   %s"),
			Q->ForgeLevel, Q->ClaimedBeacons.Num(), Q->ClaimedMines.Num(), *RegionName),''',
"hud seams counter")

sub(HD,
'''	if (LoopSys && LoopSys->Stage != EVEILStage::FindSignal && LoopSys->Stage != EVEILStage::Won)''',
'''	// THE NEXT SEAM, NAMED. A name, a distance and whose ground - so the runner can
	// walk to somewhere the world actually told him about.
	if (Q && !Q->TrackedMine.IsEmpty())
	{
		Shadow(FString::Printf(TEXT("%s   %.0f m   %s ground"), *Q->TrackedMine.ToUpper(),
			Q->TrackedCm / 100.f, *Q->TrackedOwner),
			FLinearColor(0.95f, 0.80f, 0.44f), PX, PY + PH + 8.f, Med, 0.60f);
	}

	// A CLAIM ANNOUNCES ITSELF: six seconds of the name, in the middle of the screen.
	if (Q && !Q->LastClaimName.IsEmpty() && W->GetTimeSeconds() - Q->LastClaimAt < 6.f)
	{
		const FString Line = FString::Printf(TEXT("%s CLAIMED  -  %s GROUND"),
			*Q->LastClaimName.ToUpper(), *Q->LastClaimOwner.ToUpper());
		float LW = 0.f, LH = 0.f;
		Canvas->StrLen(Big, Line, LW, LH);
		Shadow(Line, FLinearColor(1.f, 0.88f, 0.45f),
			(Canvas->ClipX - LW * 0.80f) * 0.5f, 132.f, Big, 0.80f);
	}

	if (LoopSys && LoopSys->Stage != EVEILStage::FindSignal && LoopSys->Stage != EVEILStage::Won)''',
"hud track + claim banner")

print("\n".join(applied))
