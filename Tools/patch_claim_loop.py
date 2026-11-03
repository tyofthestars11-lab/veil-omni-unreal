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

# ------------------------------------------------------------------ 1. MINE
H = "Source/VEIL/Public/World/VEILWilds.h"
sub(H,
'''	const FString& MineName() const { return Name; }''',
'''	/** Claim this seam. False if it is already someone's. */
	bool Claim(AActor* Claimant);
	/** Restore a claim from the save: marked and lit, no gold, no line. */
	void MarkClaimedSilently();
	bool IsClaimed() const { return bClaimed; }
	int32 ClaimValue() const { return Gold; }

	const FString& MineName() const { return Name; }''',
"mine.h claim api")

sub(H,
'''	float RespawnAt = 0.f;
	bool bSpent = false;''',
'''	bool bClaimed = false;
	bool bPrompted = false;''',
"mine.h state")

sub(H,
'''	/** Radius of the seam's prompt. */
''', '', "noop") if False else None

sub(H,
'''	int32 EnemiesSpawned() const { return SpawnedEnemies; }
	int32 MinesPlaced() const { return SpawnedMines; }''',
'''	int32 EnemiesSpawned() const { return SpawnedEnemies; }
	int32 MinesPlaced() const { return SpawnedMines; }
	int32 MinesClaimed() const;

	/** The nearest seam still worth walking to, inside MaxCm. */
	AVEILMineNode* NearestUnclaimedMine(const FVector& From, float MaxCm) const;

	UPROPERTY() TArray<TWeakObjectPtr<AVEILMineNode>> MineNodes;''',
"wilds.h mine list")

sub(H,
'''	float EcoClock = 0.f;
	float DecideClock = 0.f;''',
'''	float EcoClock = 0.f;
	float DecideClock = 0.f;
	/** One a second is enough to point at the next seam. */
	float TrackClock = 0.f;''',
"wilds.h track clock")

# ------------------------------------------------------------------ 2. MINE CPP
C = "Source/VEIL/Private/World/VEILWilds.cpp"
sub(C,
'''void AVEILMineNode::OnEnter(UPrimitiveComponent* OverlappedComp, AActor* Other,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (bSpent) { return; }
	bSpent = true;
	RespawnAt = GetWorld() ? GetWorld()->GetTimeSeconds() + 120.f : 0.f;

	if (UOMNIQuestSystem* Q = GetWorld() ? GetWorld()->GetSubsystem<UOMNIQuestSystem>() : nullptr)
	{
		Q->Gold += Gold;
		Q->AwardXP(45, TEXT("an ore seam stripped"));
	}
	SetActorHiddenInGame(false);
	// The seam does not disappear: it is someone's mine. It goes dark and comes
	// back, and the line names whose ground was worked.
	UE_LOG(LogVEIL, Warning, TEXT("PHI MINE: %02d %s stripped - %s %s on %s ground, +%d gold."),
		Index, *Name, *Ore, *Ore, *Owner, Gold);
}

void AVEILMineNode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bSpent || !GetWorld()) { return; }
	if (GetWorld()->GetTimeSeconds() < RespawnAt) { return; }
	bSpent = false;
	UE_LOG(LogVEIL, Log, TEXT("PHI MINE: %02d %s re-formed."), Index, *Name);
}''',
'''void AVEILMineNode::OnEnter(UPrimitiveComponent* OverlappedComp, AActor* Other,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	// Walking past a seam used to pay out by itself, which meant the runner never
	// had to do anything to own the world. Now standing here only tells you what it
	// is and that it can be taken; the taking is the key press.
	if (bClaimed || bPrompted) { return; }
	bPrompted = true;
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI MINE: %02d %s is within reach - %s seam, %.0f m down, %s ground. Press E to claim it."),
		Index, *Name, *Ore, DepthM, *Owner);
}

bool AVEILMineNode::Claim(AActor* Claimant)
{
	if (bClaimed) { return false; }
	bClaimed = true;
	if (Lamp)
	{
		Lamp->SetLightColor(FLinearColor(1.f, 0.78f, 0.30f));
		Lamp->SetIntensity(5200.f);
	}
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI MINE CLAIMED: %02d %s - %s seam on %s ground is the runner's, +%d gold."),
		Index, *Name, *Ore, *Owner, Gold);
	return true;
}

void AVEILMineNode::MarkClaimedSilently()
{
	bClaimed = true;
	bPrompted = true;
	if (Lamp)
	{
		Lamp->SetLightColor(FLinearColor(1.f, 0.78f, 0.30f));
		Lamp->SetIntensity(5200.f);
	}
}

void AVEILMineNode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}''',
"wilds.cpp claim")

sub(C,
'''		Node->Init(m);
		UE_LOG(LogVEIL, Log, TEXT("PHI MINE %02d %s stands %.0f cm inside %s ground at %s."),''',
'''		Node->Init(m);

		// A SEAM ALREADY WON STAYS WON. The save knows which names were claimed, so a
		// reload lights them gold instead of asking the runner to take the same mine
		// twice - and never pays the gold a second time.
		if (UOMNIQuestSystem* Quest = W->GetSubsystem<UOMNIQuestSystem>())
		{
			if (Quest->ClaimedMines.Contains(FName(*Node->MineName()))) { Node->MarkClaimedSilently(); }
		}
		MineNodes.Add(Node);

		UE_LOG(LogVEIL, Log, TEXT("PHI MINE %02d %s stands %.0f cm inside %s ground at %s."),''',
"wilds.cpp populate register")

sub(C,
'''float AVEILWildsPopulation::IntentOf(int32 Camp) const
{
	return Camps.IsValidIndex(Camp) ? Camps[Camp].Intent : 0.f;
}''',
'''float AVEILWildsPopulation::IntentOf(int32 Camp) const
{
	return Camps.IsValidIndex(Camp) ? Camps[Camp].Intent : 0.f;
}

int32 AVEILWildsPopulation::MinesClaimed() const
{
	int32 N = 0;
	for (const TWeakObjectPtr<AVEILMineNode>& M : MineNodes)
	{
		if (M.IsValid() && M->IsClaimed()) { ++N; }
	}
	return N;
}

AVEILMineNode* AVEILWildsPopulation::NearestUnclaimedMine(const FVector& From, float MaxCm) const
{
	AVEILMineNode* Best = nullptr;
	float BestD = MaxCm;
	for (const TWeakObjectPtr<AVEILMineNode>& M : MineNodes)
	{
		if (!M.IsValid() || M->IsClaimed()) { continue; }
		const float D = FVector::Dist2D(M->GetActorLocation(), From);
		if (D <= BestD) { BestD = D; Best = M.Get(); }
	}
	return Best;
}''',
"wilds.cpp nearest")

sub(C,
'''void AVEILWildsPopulation::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Camps.Num() == 0) { return; }''',
'''void AVEILWildsPopulation::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// POINT AT THE NEXT SEAM. Once a second the nearest unclaimed mine is handed to
	// the quest book, so the HUD can say how far and which way - the difference
	// between a world with mines in it and a game with something to do.
	if (UWorld* World = GetWorld())
	{
		TrackClock += DeltaSeconds;
		if (TrackClock >= 1.f)
		{
			TrackClock = 0.f;
			if (UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>())
			{
				if (const APawn* Runner = UGameplayStatics::GetPlayerPawn(this, 0))
				{
					if (const AVEILMineNode* Closest = NearestUnclaimedMine(Runner->GetActorLocation(), 400000.f))
					{
						Quest->TrackMine(Closest->MineName(),
							FVector::Dist2D(Closest->GetActorLocation(), Runner->GetActorLocation()),
							Closest->MineOwner());
					}
					else
					{
						Quest->TrackMine(FString(), 0.f, FString());
					}
				}
			}
		}
	}

	if (Camps.Num() == 0) { return; }''',
"wilds.cpp track")

print("\n".join(applied))
