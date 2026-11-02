#include "Systems/VEILCampaign.h"

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Systems/OMNIQuestSystem.h"
#include "VEIL.h"

namespace
{
	// The five places the world already names, in the order the builder lays them.
	const TCHAR* GRegionNames[] =
	{
		TEXT("THE HUB"),
		TEXT("THE NEST"),
		TEXT("THE SPIRE"),
		TEXT("OMNI DISTRICT"),
		TEXT("THE DEEP")
	};
	constexpr int32 GRegionCount = UE_ARRAY_COUNT(GRegionNames);

	// What each counter is asked for, and how the ask is worded. The thresholds
	// climb along the golden ladder so the last activities are not chores.
	struct FKindDef
	{
		EVEILCounter Kind;
		const TCHAR* Verb;
		int32 Steps[10];
		int32 Gold;
		int32 XP;
	};

	const FKindDef GKinds[] =
	{
		{ EVEILCounter::Kills,     TEXT("Hunt %d echoes in %s"),        { 3, 6, 10, 15, 21, 28, 36, 45, 55, 66 }, 40, 60 },
		{ EVEILCounter::Fragments, TEXT("Recover %d fragments near %s"), { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },       70, 90 },
		{ EVEILCounter::Voices,    TEXT("Hear %d voices in %s"),        { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },      30, 45 },
		{ EVEILCounter::Caches,    TEXT("Read %d survey caches in %s"), { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },      45, 70 },
		{ EVEILCounter::Waves,     TEXT("Hold %d waves at %s"),         { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 },     100, 140 },
		{ EVEILCounter::Beacons,   TEXT("Forge %d beacons for %s"),     { 1, 2, 3, 3, 3, 3, 3, 3, 3, 3 },        90, 120 },
	};
	constexpr int32 GKindCount = UE_ARRAY_COUNT(GKinds);
}

void UVEILCampaignSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	BuildActivities();

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI CAMPAIGN: %d activities across %d regions, six acts. Objective: %s"),
		Activities.Num(), GRegionCount, *ObjectiveLine());

	const int32 First = Activities.Num() > 0 ? 0 : INDEX_NONE;
	if (First != INDEX_NONE)
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI CAMPAIGN: first activity - %s (%d gold, %d XP)."),
			*Activities[0].Title, Activities[0].Gold, Activities[0].XP);
	}
}

TStatId UVEILCampaignSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UVEILCampaignSubsystem, STATGROUP_Tickables);
}

bool UVEILCampaignSubsystem::IsTickable() const
{
	return !IsTemplate();
}

UWorld* UVEILCampaignSubsystem::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

void UVEILCampaignSubsystem::BuildActivities()
{
	if (Activities.Num() > 0)
	{
		return;
	}
	Activities.Reserve(50);

	// FIFTY, AND NOT ONE OF THEM A COPY. Five regions times ten asks, and the ask
	// rotates through the six counter kinds so a player is never doing the same
	// verb five times in a row for five different names.
	for (int32 Slot = 0; Slot < 10; ++Slot)
	{
		for (int32 Region = 0; Region < GRegionCount; ++Region)
		{
			const FKindDef& Kind = GKinds[(Slot + Region) % GKindCount];
			const int32 Step = FMath::Clamp(Slot, 0, 9);

			FVEILActivity Act;
			Act.Kind = Kind.Kind;
			Act.Region = Region;
			Act.Required = Kind.Steps[Step];
			// Reward breathes with the ask, never a flat number.
			Act.Gold = Kind.Gold + Step * 12;
			Act.XP = Kind.XP + Step * 18;
			// FormatStringSan refuses a format string that is not a literal, and it is
			// right to: the verb is data. The two tokens are substituted instead.
			FString Title = Kind.Verb;
			Title = Title.Replace(TEXT("%d"), *FString::FromInt(Act.Required));
			Title = Title.Replace(TEXT("%s"), GRegionNames[Region]);
			Act.Title = Title;
			Activities.Add(Act);
		}
	}
}

int32 UVEILCampaignSubsystem::CountOf(EVEILCounter Kind) const
{
	switch (Kind)
	{
	case EVEILCounter::Kills:     return Kills;
	case EVEILCounter::Fragments: return Fragments;
	case EVEILCounter::Beacons:   return Beacons;
	case EVEILCounter::Caches:    return Caches;
	case EVEILCounter::Waves:     return Waves;
	case EVEILCounter::Voices:    return Voices;
	case EVEILCounter::Boss:      return Bosses;
	}
	return 0;
}

void UVEILCampaignSubsystem::NoteKill(bool bBoss)
{
	++Kills;
	if (bBoss) { ++Bosses; }
	Scan();
}

void UVEILCampaignSubsystem::NoteFragment() { ++Fragments; Scan(); }
void UVEILCampaignSubsystem::NoteBeacon()   { ++Beacons;   Scan(); }
void UVEILCampaignSubsystem::NoteCache()    { ++Caches;    Scan(); }
void UVEILCampaignSubsystem::NoteWave()     { ++Waves;     Scan(); }
void UVEILCampaignSubsystem::NoteVoice()    { ++Voices;    Scan(); }

void UVEILCampaignSubsystem::Tick(float DeltaSeconds)
{
	// A scan a second is enough: every counter is event-driven, so this is only
	// the safety net that catches an activity the hooks could not see.
	ScanClock += DeltaSeconds;
	if (ScanClock < 1.f) { return; }
	ScanClock = 0.f;
	Scan();
}

void UVEILCampaignSubsystem::Scan()
{
	int32 Done = 0;
	for (FVEILActivity& A : Activities)
	{
		if (A.bDone) { ++Done; continue; }
		if (CountOf(A.Kind) < A.Required) { continue; }

		A.bDone = true;
		++Done;
		if (UOMNIQuestSystem* Q = GetWorld() ? GetWorld()->GetSubsystem<UOMNIQuestSystem>() : nullptr)
		{
			Q->Gold += A.Gold;
			Q->AwardXP(A.XP, TEXT("an activity closed"));
		}
		Announce(FString::Printf(TEXT("ACTIVITY %d/50 CLOSED: %s  +%dG +%dXP"),
			Done, *A.Title, A.Gold, A.XP));
		UE_LOG(LogVEIL, Warning, TEXT("PHI ACTIVITY: %d/50 closed - %s (region %d, required %d)."),
			Done, *A.Title, A.Region, A.Required);
	}

	if (Done > LastDone)
	{
		LastDone = Done;
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow,
				FString::Printf(TEXT("ACTIVITIES  %d / %d"), Done, Activities.Num()));
		}
	}

	// THE ENDING. All three beacons and the Warden down: the story closes once.
	if (!bComplete && Beacons >= 3 && Bosses >= 1)
	{
		CloseTheStory();
	}
}

void UVEILCampaignSubsystem::CloseTheStory()
{
	bComplete = true;
	Announce(TEXT("THE SIGNAL IS WHOLE. The lamps burn, the Warden is down, and the Vault has nothing left to ask."));
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI CAMPAIGN: COMPLETE - %d of %d activities, %d kills, %d fragments, %d beacons, %d voices."),
		DoneCount(), Activities.Num(), Kills, Fragments, Beacons, Voices);
	if (UOMNIQuestSystem* Q = GetWorld() ? GetWorld()->GetSubsystem<UOMNIQuestSystem>() : nullptr)
	{
		Q->AwardXP(500, TEXT("THE SIGNAL IS WHOLE"));
	}
}

FString UVEILCampaignSubsystem::ObjectiveLine() const
{
	if (bComplete)
	{
		return TEXT("The signal is whole - walk the district, runner");
	}
	if (const UOMNIQuestSystem* Q = GetWorld() ? GetWorld()->GetSubsystem<UOMNIQuestSystem>() : nullptr)
	{
		switch (Q->Stage)
		{
		case EOMNIQuestStage::MeetKeeper:      return TEXT("Let the Keeper count you at her beacon");
		case EOMNIQuestStage::GatherFragments: return FString::Printf(TEXT("Recover %d of 3 fragments of a broken voice"), FMath::Min(Q->Fragments, 3));
		case EOMNIQuestStage::MeetSeer:        return TEXT("Find the Seer above the falls");
		case EOMNIQuestStage::ReachCache:      return TEXT("Claim the survey cache at the far edge");
		case EOMNIQuestStage::WardenAwake:     return TEXT("Face WARDEN PRIME in the arena");
		case EOMNIQuestStage::Clear:           return TEXT("The chain is clear - hold the district");
		}
	}
	return TEXT("Follow the signal");
}

FString UVEILCampaignSubsystem::ActTitle() const
{
	if (bComplete) { return TEXT("THE LAMP THAT STAYED LIT"); }
	if (const UOMNIQuestSystem* Q = GetWorld() ? GetWorld()->GetSubsystem<UOMNIQuestSystem>() : nullptr)
	{
		switch (Q->Stage)
		{
		case EOMNIQuestStage::MeetKeeper:      return TEXT("ACT I - THE LAMP THAT WAITED");
		case EOMNIQuestStage::GatherFragments: return TEXT("ACT II - TEN PIECES OF A VOICE");
		case EOMNIQuestStage::MeetSeer:        return TEXT("ACT III - THE SEER AND THE ASH");
		case EOMNIQuestStage::ReachCache:      return TEXT("ACT IV - THE CACHE AT THE EDGE");
		case EOMNIQuestStage::WardenAwake:     return TEXT("ACT V - WHAT THE DEEP SENDS");
		case EOMNIQuestStage::Clear:           return TEXT("ACT VI - THE LAMP THAT STAYED LIT");
		}
	}
	return TEXT("ACT I - THE LAMP THAT WAITED");
}

FString UVEILCampaignSubsystem::ActivityLine() const
{
	return FString::Printf(TEXT("ACTIVITIES  %d / %d"), DoneCount(), Activities.Num());
}

int32 UVEILCampaignSubsystem::DoneCount() const
{
	int32 Done = 0;
	for (const FVEILActivity& A : Activities)
	{
		if (A.bDone) { ++Done; }
	}
	return Done;
}

void UVEILCampaignSubsystem::Announce(const FString& Line) const
{
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 8.f, FColor(255, 205, 70), Line); }
	UE_LOG(LogVEIL, Warning, TEXT("PHI CAMPAIGN: %s"), *Line);
}

// phi.acts - the whole board, so the fifty can be read without a menu.
static FAutoConsoleCommandWithWorld GPhiActs(
	TEXT("phi.acts"),
	TEXT("List the fifty activities and their state."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		UVEILCampaignSubsystem* C = World ? World->GetSubsystem<UVEILCampaignSubsystem>() : nullptr;
		if (!C)
		{
			UE_LOG(LogVEIL, Warning, TEXT("PHI CAMPAIGN: no campaign in this world."));
			return;
		}
		UE_LOG(LogVEIL, Warning, TEXT("PHI CAMPAIGN: %d of %d closed. %s"),
			C->DoneCount(), C->TotalCount(), *C->ObjectiveLine());
		const TArray<FVEILActivity>& All = C->All();
		for (int32 i = 0; i < All.Num(); ++i)
		{
			UE_LOG(LogVEIL, Log, TEXT("  %s %02d  %s"), All[i].bDone ? TEXT("[X]") : TEXT("[ ]"), i + 1, *All[i].Title);
		}
	}));
