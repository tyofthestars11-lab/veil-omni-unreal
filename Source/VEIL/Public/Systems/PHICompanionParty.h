// PHI companion party: five slots beside the Runner. Slot 1 is GR MUSE
// (sage), slot 2 is the DRONE. Test muster via phi.party (0 off, else count).
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PHICompanionParty.generated.h"

class AVEILCompanion;
class APawn;

UCLASS()
class VEIL_API UPHICompanionParty : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Muster from phi.party and prune the fallen. Driven from GameMode tick. */
	void TickWorld(float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = "VEIL|Party")
	AVEILCompanion* Recruit(uint8 Role, FName CallSign);
	UFUNCTION(BlueprintCallable, Category = "VEIL|Party")
	void NotePlayable(APawn* P);
	UFUNCTION(BlueprintCallable, Category = "VEIL|Party")
	void CyclePlayable();
	UFUNCTION(BlueprintPure, Category = "VEIL|Party")
	FString LeadName() const;

	UFUNCTION(BlueprintPure, Category = "VEIL|Party")
	int32 Count() const;

private:
	TArray<TWeakObjectPtr<AVEILCompanion>> Members;
	TWeakObjectPtr<APawn> RunnerPawn;
	TWeakObjectPtr<APawn> CountessPawn;
	bool bVesper = false;
	bool bQuill = false;
	bool bBastion = false;
	bool bHome = false;
	void NoteRunner();
	void FollowTick(float DT);
	void BeatsTick(float DT);
	bool PlayableAlive(APawn* P) const;
	void Beat(bool& Flag, const TCHAR* Who, const TCHAR* Deed);
	bool bMustered = false;
	void Muster(int32 Wanted);
	void Prune();
};
