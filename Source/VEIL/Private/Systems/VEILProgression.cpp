// VEIL OMNI progression + save implementation.
#include "Systems/VEILProgression.h"

#include "Kismet/GameplayStatics.h"
#include "Systems/OMNIQuestSystem.h"
#include "VEIL.h"
#include "Systems/VEILPhi.h"

const FString UVEILProgression::SaveSlotName = TEXT("VEILOMNI_Save0");

int32 UVEILProgression::XPNeedForLevel(int32 InLevel)
{
	// round(20 * phi^(level-1)), phi = 1.61803398875. Level 1 -> 20.
	const double Phi = VeilPhi::Phi;
	return FMath::RoundToInt(20.0 * FMath::Pow(Phi, (double)FMath::Max(1, InLevel) - 1.0));
}

void UVEILProgression::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);
	VerifyAnchors();
	// ONE SAVE, ONE DOOR: the slot was written by the same class that reads it,
	// but nothing ever called either side, so every run started from zero and
	// every run's progress died with the process. The book is opened at boot.
	// THE OMNI SLOT IS THE ONE SAVE. When it exists, its own reader loads the run
	// and hands it over; reading the older base slot as well would let a stale
	// file win the race and overwrite a newer run with old numbers.
	if (!UGameplayStatics::DoesSaveGameExist(TEXT("VEILOMNI_Omni0"), 0))
	{
		LoadGame();
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI PROGRESSION: level=%d xp=%d gold=%d fragments=%d items=%d slot=%s"),
		Level, XP, Gold, Fragments, Inventory.Num(), *SaveSlotName);
}

void UVEILProgression::Deinitialize()
{
	// Leaving the world writes the book: the run is never lost on exit.
	SaveGame();
	Super::Deinitialize();
}

void UVEILProgression::AwardXP(int32 Amount, const FString& Reason)
{
	if (Amount <= 0) return;
	XP += Amount;
	UE_LOG(LogVEIL, Log, TEXT("PHI XP: +%d (%s) total=%d level=%d need_next=%d"),
		Amount, *Reason, XP, Level, XPNeedForLevel(Level));
	bool bLevelled = false;
	while (XP >= XPNeedForLevel(Level))
	{
		XP -= XPNeedForLevel(Level);
		++Level;
		bLevelled = true;
		UE_LOG(LogVEIL, Log, TEXT("PHI LEVEL UP: level=%d xp_left=%d"), Level, XP);
	}
	// A level change is a milestone: write the book right there, so a crash or
	// a closed window never costs a rung that was actually earned.
	if (bLevelled) { SaveGame(); }
	PushToQuest();
}

void UVEILProgression::AddLoot(FName ItemName, int32 GoldValue)
{
	if (ItemName.IsNone()) return;
	Inventory.AddUnique(ItemName);
	Gold += GoldValue;
	UE_LOG(LogVEIL, Log, TEXT("PHI LOOT: %s gold=+%d total_gold=%d items=%d"),
		*ItemName.ToString(), GoldValue, Gold, Inventory.Num());
	SaveGame();
	PushToQuest();
}

void UVEILProgression::AddFragment()
{
	++Fragments;
	UE_LOG(LogVEIL, Log, TEXT("PHI FRAGMENT: %d / 3"), Fragments);
	PushToQuest();
}

bool UVEILProgression::SaveGame()
{
	UWorld* World = GetWorld();
	if (!World) return false;
	// Never write the book from a passage world. The engine boots through an
	// Entry/Untitled world on the way to the game map, and its teardown wrote an
	// EMPTY save over a real run - the one save must keep the run, not erase it.
	const FString MapName = World->GetMapName();
	if (MapName.Contains(TEXT("Entry")) || MapName.Contains(TEXT("Untitled")))
	{
		return false;
	}
	UVEILSaveGame* Save = Cast<UVEILSaveGame>(UGameplayStatics::CreateSaveGameObject(UVEILSaveGame::StaticClass()));
	if (!Save) return false;
	Save->Level = Level;
	Save->XP = XP;
	Save->Gold = Gold;
	Save->Fragments = Fragments;
	Save->ClaimedMines = ClaimedMines;
	Save->Inventory = Inventory;
	if (APawn* Player = UGameplayStatics::GetPlayerPawn(World, 0))
	{
		Save->PlayerLocation = Player->GetActorLocation();
	}
	Save->CurrentMap = FName(*World->GetMapName());
	const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, 0);
	UE_LOG(LogVEIL, Log, TEXT("PHI SAVE: slot=%s result=%d level=%d map=%s"),
		*SaveSlotName, bSaved ? 1 : 0, Level, *Save->CurrentMap.ToString());
	return bSaved;
}

bool UVEILProgression::LoadGame()
{
	UWorld* World = GetWorld();
	if (!World) return false;
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI LOAD: no save in slot %s - fresh run, not an error."), *SaveSlotName);
		return false;
	}
	UVEILSaveGame* Save = Cast<UVEILSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0));
	if (!Save) return false;
	Level = Save->Level;
	XP = Save->XP;
	Gold = Save->Gold;
	Fragments = Save->Fragments;
	ClaimedMines = Save->ClaimedMines;
	Inventory = Save->Inventory;
	UE_LOG(LogVEIL, Log, TEXT("PHI LOAD: slot=%s level=%d xp=%d gold=%d fragments=%d"),
		*SaveSlotName, Level, XP, Gold, Fragments);
	PushToQuest();
	return true;
}

bool UVEILProgression::VerifyAnchors() const
{
	// Constants are the authority lock itself; this log line is the
	// code-side check. Live-map verification (exactly 1 Vault, exactly
	// 1 Countess, RUNNER at start) is done by Tools/whole_verify_goal.py
	// in the editor + a photo/log - never claimed from this log alone.
	const bool bVault = (VEILAnchors::VaultX == 0.f && VEILAnchors::VaultY == 0.f && VEILAnchors::VaultZ == -80.9f);
	const bool bCountess = (VEILAnchors::CountessX == 809.f && VEILAnchors::CountessY == -309.f && VEILAnchors::CountessZ == 64.7f);
	const bool bRunner = (VEILAnchors::RunnerX == 0.f && VEILAnchors::RunnerY == 0.f && VEILAnchors::RunnerZ == 100.f);
	UE_LOG(LogVEIL, Log, TEXT("VERIFY Vault:%s Countess:%s Runner:%s FloorNav:%.0f"),
		bVault ? TEXT("True") : TEXT("False"),
		bCountess ? TEXT("True") : TEXT("False"),
		bRunner ? TEXT("True") : TEXT("False"),
		VEILAnchors::FloorNav);
	return bVault && bCountess && bRunner;
}

// ---------------------------------------------------------------------------
// ONE BOOK, ONE HUD. The progression system writes the file; the quest system
// is the live copy the panel draws. These two calls are the join, so a number
// on screen and a number in the save are the same number by construction.
// ---------------------------------------------------------------------------
void UVEILProgression::PushToQuest()
{
	UWorld* World = GetWorld();
	if (!World) { return; }
	UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();
	if (!Quest) { return; }
	Quest->Level = Level;
	Quest->XP = XP;
	Quest->Gold = Gold;
	Quest->Fragments = Fragments;
	Quest->ClaimedMines = ClaimedMines;
}

void UVEILProgression::PullFromQuest()
{
	UWorld* World = GetWorld();
	if (!World) { return; }
	const UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();
	if (!Quest) { return; }
	Level = Quest->Level;
	XP = Quest->XP;
	Gold = Quest->Gold;
	Fragments = Quest->Fragments;
	ClaimedMines = Quest->ClaimedMines;
}
