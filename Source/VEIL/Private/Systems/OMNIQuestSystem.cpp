// VEIL OMNI quest system, VEIL module version. Clean replacement for the
// PC file that failed to build. Fixes, by photo line:
//  - Line 233 col 84, C2065 'C': the PC code wrote "C///FM->AdjustRep"
//    where the variable is CFM. "C" is undeclared and "//" comments out
//    the rest of the line. Fixed here: "CFM->AdjustRep(...)" with CFM
//    declared in the same statement, no stray slashes.
//  - Line 234, C2143 missing ';' before 'if': the CacheEpic
//    SpawnActorDeferred call was left unclosed. Fixed here: the call
//    is fully parenthesised, finished with FinishSpawning, and closed
//    with a semicolon (see SpawnCacheGear).
//  - Line 224 note, '{' not yet matched, and lines 243 / 248, C2601
//    local function definitions illegal, and line 276, fatal C1075
//    '{' no matching token: one function body above ChoiceOpen was
//    left open, so ChoiceOpen / AnswerChoice parsed as nested locals
//    and the file never closed. Fixed here: every function body is
//    opened and closed in this file, in order, with no nesting.
// Quest chain: MeetKeeper -> GatherFragments (3) -> MeetSeer ->
// ReachCache (Cache at 0,-6200) -> WardenAwake -> Clear.
// Anchors: Keeper (900,0), Seer (-450,780), Cache (0,-6200),
// CacheEpic gear (300,-6200,80).
#include "Systems/OMNIQuestSystem.h"

#include "AI/VEILFactionAI.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/VEILProgression.h"
#include "Systems/VEILOmniAll.h"
#include "Systems/VEILCampaign.h"
#include "Systems/VEILLoopSubsystem.h"
#include "VEIL.h"

// THE THREE SITES, in one place. They are the world's own landmarks - the same
// coordinates the builder dresses and the Keeper/Seer/Cartographer chain names -
// so the counter on the HUD and the stones on the ground cannot disagree.
namespace
{
	struct FPhiBeaconSite
	{
		const TCHAR* Name;
		float X;
		float Y;
	};

	const FPhiBeaconSite GPhiBeaconSites[] =
	{
		{ TEXT("KEEPER"),       900.f,    0.f },
		{ TEXT("SEER"),        -450.f,  780.f },
		{ TEXT("CARTOGRAPHER"), -4150.f, -780.f },
	};
	constexpr int32 GPhiBeaconCount = UE_ARRAY_COUNT(GPhiBeaconSites);
}

bool UOMNIQuestSystem::ClaimBeacon(int32 Index)
{
	if (Index < 0 || Index >= GPhiBeaconCount) { return false; }
	const FPhiBeaconSite& Site = GPhiBeaconSites[Index];
	const FName Id(Site.Name);
	if (ClaimedBeacons.Contains(Id))
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI BEACON: %s is already forged (%d of %d)."),
			Site.Name, ClaimedBeacons.Num(), GPhiBeaconCount);
		return false;
	}
	ClaimedBeacons.Add(Id);
	if (UVEILCampaignSubsystem* Camp = GetWorld() ? GetWorld()->GetSubsystem<UVEILCampaignSubsystem>() : nullptr) { Camp->NoteBeacon(); }
	AwardXP(25, TEXT("beacon forged"));

	// THE KEEPER'S BEACON IS THE KEEPER BEAT. The objective line on the HUD is the
	// loop's current act, and forging the first beacon is exactly what "let her count
	// you" means - so the two systems are joined here instead of the counter ticking
	// up while the objective still tells the runner to walk to a stone they already
	// lit. Forging KEEPER opens Act II; the other two are the chain she asked for.
	if (UVEILLoopSubsystem* Loop = GetWorld() ? GetWorld()->GetSubsystem<UVEILLoopSubsystem>() : nullptr)
	{
		if (Id == FName(TEXT("KEEPER")) && Loop->Stage == EVEILStage::FindSignal)
		{
			Loop->SignalFound();
			UE_LOG(LogVEIL, Warning, TEXT("PHI BEACON: KEEPER beacon forged - the Keeper's beat is met, Act II opens."));
		}
	}
	UE_LOG(LogVEIL, Warning, TEXT("PHI BEACON FORGED: %s at (%.0f, %.0f) - %d of %d."),
		Site.Name, Site.X, Site.Y, ClaimedBeacons.Num(), GPhiBeaconCount);
	if (ClaimedBeacons.Num() >= GPhiBeaconCount)
	{
		AdvanceTo(EOMNIQuestStage::MeetSeer);
		UE_LOG(LogVEIL, Warning, TEXT("PHI BEACON: all %d forged - the chain opens toward the Seer."), GPhiBeaconCount);
	}
	return true;
}

bool UOMNIQuestSystem::ClaimBeaconNear(const FVector& From, float ReachCm)
{
	// The reach is a step, not a district: phi of the site's own debris ring (620 cm),
	// so "standing at the beacon" means within about ten metres of its stones.
	const float Reach = ReachCm > 0.f ? ReachCm : 620.f * 1.6180339887f;
	for (int32 i = 0; i < GPhiBeaconCount; ++i)
	{
		const FPhiBeaconSite& Site = GPhiBeaconSites[i];
		if (FVector2D::Distance(FVector2D(From.X, From.Y), FVector2D(Site.X, Site.Y)) <= Reach)
		{
			return ClaimBeacon(i);
		}
	}
	return false;
}

void UOMNIQuestSystem::RestoreBeacons(int32 Count)
{
	ClaimedBeacons.Reset();
	const int32 N = FMath::Clamp(Count, 0, GPhiBeaconCount);
	for (int32 i = 0; i < N; ++i)
	{
		ClaimedBeacons.Add(FName(GPhiBeaconSites[i].Name));
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI BEACON RESTORE: %d of %d from the save."), N, GPhiBeaconCount);
}

void UOMNIQuestSystem::PersistToBook()
{
	UWorld* World = GetWorld();
	if (!World) { return; }
	if (UVEILProgression* Book = World->GetSubsystem<UVEILProgression>())
	{
		Book->Level = Level;
		Book->XP = XP;
		Book->Gold = Gold;
		Book->Fragments = Fragments;
	}
}

void UOMNIQuestSystem::TrackMine(const FString& Name, float Centimetres, const FString& Owner)
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

int32 UOMNIQuestSystem::XPNeedForLevel(int32 InLevel)
{
	const double Phi = 1.618033988749895;
	return FMath::RoundToInt(20.0 * FMath::Pow(Phi, (double)FMath::Max(0, InLevel - 1)));
}

void UOMNIQuestSystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	LoadFromSave();
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI QUEST ONLINE: stage=MeetKeeper, level 1, first rung needs %d XP."), XPNeedForLevel(1));
}

void UOMNIQuestSystem::LoadFromSave()
{
	UWorld* World = GetWorld();
	if (!World) { return; }

	// The omni slot is the one save; the base slot is its mirror and the fallback
	// for a run written before the omni slot existed.
	static const TCHAR* Slots[] = { TEXT("VEILOMNI_Omni0"), TEXT("VEILOMNI_Save0") };
	for (const TCHAR* Slot : Slots)
	{
		if (!UGameplayStatics::DoesSaveGameExist(Slot, 0)) { continue; }
		UVEILSaveGame* Loaded = Cast<UVEILSaveGame>(UGameplayStatics::LoadGameFromSlot(Slot, 0));
		if (!Loaded) { continue; }

		Level = Loaded->Level;
		XP = Loaded->XP;
		Gold = Loaded->Gold;
		Fragments = Loaded->Fragments;
		ClaimedMines = Loaded->ClaimedMines;
		if (const UVEILOmniSave* Omni = Cast<UVEILOmniSave>(Loaded))
		{
			Stage = (EOMNIQuestStage)Omni->QuestStage;
			RestoreBeacons(Omni->BeaconsClaimed);
		}
		bLoadedFromSave = true;
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI HUD LOAD: loaded=1 slot=%s level=%d xp=%d gold=%d fragments=%d beacons=%d stage=%d - the panel and the save are one number."),
			Slot, Level, XP, Gold, Fragments, ClaimedBeacons.Num(), (int32)Stage);
		return;
	}

	UE_LOG(LogVEIL, Log, TEXT("PHI HUD LOAD: loaded=0 - no save yet, the panel opens on a fresh run."));
}

void UOMNIQuestSystem::AwardXP(int32 Amount, const TCHAR* Reason)
{
	if (Amount <= 0)
	{
		return;
	}
	XP += Amount;
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI XP +%d (%s) -> %d XP at level %d (rung needs %d)."), Amount, Reason, XP, Level, XPNeedForLevel(Level));
	while (XP >= XPNeedForLevel(Level))
	{
		XP -= XPNeedForLevel(Level);
		++Level;
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI LEVEL UP: level %d. Next rung needs %d XP."), Level, XPNeedForLevel(Level));
	}
	PersistToBook();
}

void UOMNIQuestSystem::AdvanceTo(EOMNIQuestStage NewStage)
{
	if (NewStage <= Stage && NewStage != EOMNIQuestStage::Clear)
	{
		return;
	}
	Stage = NewStage;
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI QUEST STAGE: %d"), (int32)Stage);
}

void UOMNIQuestSystem::TalkTo(FName Giver)
{
	if (Giver == FName(TEXT("Keeper")))
	{
		if (Stage == EOMNIQuestStage::MeetKeeper)
		{
			AdvanceTo(EOMNIQuestStage::GatherFragments);
			UE_LOG(LogVEIL, Log, TEXT("PHI OMNI QUEST [Keeper 900,0]: gather 3 fragments. Stage -> GatherFragments."));
		}
	}
	else if (Giver == FName(TEXT("Seer")))
	{
		if (Stage == EOMNIQuestStage::MeetSeer)
		{
			AdvanceTo(EOMNIQuestStage::ReachCache);
			UE_LOG(LogVEIL, Log, TEXT("PHI OMNI QUEST [Seer -450,780]: cache at 0,-6200. Stage -> ReachCache."));
		}
	}
	else if (Giver == FName(TEXT("Cartographer")))
	{
		Gold += 10;
		AwardXP(5, TEXT("the Cartographer's word"));
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI QUEST [Cartographer]: +10 gold -> %d."), Gold);
	}
}

FString UOMNIQuestSystem::GetRegionName() const
{
	if (RegionMap.IsEmpty())
	{
		return TEXT("the signal");
	}
	return RegionMap;
}

void UOMNIQuestSystem::AddShard()
{
	// A SHARD IS NOT A FRAGMENT.
	//
	// This used to call CollectFragment(), so every machine the runner put down
	// advanced the story: eight kills and the Keeper's ten-piece sentence was
	// "recovered" without the runner ever picking one up. Shards pay experience
	// and nothing else now; the fragments come from the fragments.
	++Shards;
	AwardXP(2, TEXT("a shard from a fallen machine"));
	if (UVEILCampaignSubsystem* KillCamp = GetWorld() ? GetWorld()->GetSubsystem<UVEILCampaignSubsystem>() : nullptr)
	{
		KillCamp->NoteKill(false);
	}
}

void UOMNIQuestSystem::CollectFragment()
{
	++Fragments;
	AwardXP(15, TEXT("a fragment of the signal"));
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI FRAGMENT %d/3 gathered."), Fragments);
	if (Stage == EOMNIQuestStage::GatherFragments && Fragments >= 3)
	{
		AdvanceTo(EOMNIQuestStage::MeetSeer);
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI QUEST: signal whole. The Seer waits. Stage -> MeetSeer."));
	}
}

AActor* UOMNIQuestSystem::SpawnCacheGear()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return nullptr;
	}
	const FTransform CacheTransform(FRotator::ZeroRotator, FVector(300.f, -6200.f, 80.f));
	AActor* CacheEpic = World->SpawnActorDeferred<AActor>(AActor::StaticClass(), CacheTransform);
	if (!CacheEpic)
	{
		return nullptr;
	}
	CacheEpic->FinishSpawning(CacheTransform);
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI CACHE GEAR: CacheEpic spawned at 300,-6200,80."));
	return CacheEpic;
}

void UOMNIQuestSystem::ReachCache()
{
	if (Stage != EOMNIQuestStage::ReachCache)
	{
		return;
	}
	Gold += 80;
	if (UVEILCampaignSubsystem* CampC = GetWorld() ? GetWorld()->GetSubsystem<UVEILCampaignSubsystem>() : nullptr) { CampC->NoteCache(); }
	AwardXP(25, TEXT("the survey cache"));
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI QUEST: cache claimed (+80 gold -> %d)."), Gold);
	// This is the PC line 233 block, corrected. CFM is the variable,
	// not "C", and the statement is closed before the next statement.
	if (UVEILFactionManager* CFM = GetWorld()->GetSubsystem<UVEILFactionManager>())
	{
		CFM->AdjustRep(EVEILFaction::Remnant, 5.f);
	}
	SpawnCacheGear();
	bChoiceOpen = true;
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI QUEST: choice open at the cache. AnswerChoice closes it."));
	WakeWarden();
}

void UOMNIQuestSystem::WakeWarden()
{
	AdvanceTo(EOMNIQuestStage::WardenAwake);
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI QUEST: WARDEN PRIME arena gate open. Stage -> WardenAwake."));
}

bool UOMNIQuestSystem::ChoiceOpen() const
{
	return bChoiceOpen;
}

void UOMNIQuestSystem::AnswerChoice(int32 N)
{
	if (!bChoiceOpen)
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI CHOICE: no choice open, AnswerChoice(%d) ignored."), N);
		return;
	}
	bChoiceOpen = false;
	if (N == 0)
	{
		if (UVEILFactionManager* CFM = GetWorld()->GetSubsystem<UVEILFactionManager>())
		{
			CFM->AdjustRep(EVEILFaction::Remnant, 5.f);
		}
		AwardXP(25, TEXT("the Remnant answer"));
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI CHOICE: answer 0 accepted, Remnant rep +5."));
	}
	else
	{
		AwardXP(10, TEXT("an answer at the cache"));
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI CHOICE: answer %d accepted."), N);
	}
}

void UOMNIQuestSystem::DefeatWarden()
{
	if (Stage != EOMNIQuestStage::WardenAwake)
	{
		return;
	}
	AdvanceTo(EOMNIQuestStage::Clear);
	if (UVEILCampaignSubsystem* CampW = GetWorld() ? GetWorld()->GetSubsystem<UVEILCampaignSubsystem>() : nullptr) { CampW->NoteKill(true); }
	Gold += 150;
	AwardXP(100, TEXT("WARDEN PRIME down"));
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI QUEST: WARDEN PRIME DOWN - CLEAR. +150 gold -> %d."), Gold);
}

// phi.claim - forge a beacon from the console, through the SAME function the
// player's interact key reaches (ClaimBeacon / ClaimBeaconNear). No second path,
// no flag: if a proof can forge one this way, a player can forge it by standing
// there and pressing the key.
#include "HAL/IConsoleManager.h"
#include "EngineUtils.h"

static FAutoConsoleCommandWithWorldAndArgs GPhiClaimBeacon(
	TEXT("phi.claim"),
	TEXT("Forge a beacon site: phi.claim <0 Keeper | 1 Seer | 2 Cartographer | all>"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UOMNIQuestSystem* Quest = World ? World->GetSubsystem<UOMNIQuestSystem>() : nullptr;
		if (!Quest)
		{
			UE_LOG(LogVEIL, Warning, TEXT("PHI BEACON: no quest system in this world."));
			return;
		}
		if (Args.Num() == 0 || Args[0].Equals(TEXT("all"), ESearchCase::IgnoreCase))
		{
			for (int32 i = 0; i < 3; ++i) { Quest->ClaimBeacon(i); }
			UE_LOG(LogVEIL, Warning, TEXT("PHI BEACON: %d of 3 forged after phi.claim all."), Quest->ClaimedBeacons.Num());
			return;
		}
		const int32 Index = FCString::Atoi(*Args[0]);
		const bool bForged = Quest->ClaimBeacon(Index);
		UE_LOG(LogVEIL, Warning, TEXT("PHI BEACON: phi.claim %d -> %s (%d of 3)."),
			Index, bForged ? TEXT("forged") : TEXT("already forged or out of range"), Quest->ClaimedBeacons.Num());
	}));
