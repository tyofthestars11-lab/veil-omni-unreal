#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VEILLoopSubsystem.generated.h"

UENUM(BlueprintType)
enum class EVEILStage : uint8
{
	FindSignal,   // wake in Hub, talk to Keeper, find the Vault signal
	Fragments,    // recover 10 fragments across the 5 regions
	Artifact,     // build the artifact at the Spire forge
	FinalWave,    // defend: 5 waves, then WARDEN PRIME
	Beacon,       // activate the beacon
	Won
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVEILStageChanged, EVEILStage, NewStage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVEILFragmentCount, int32, Count);

// The 0% -> 100% loop: signal -> 10 fragments -> artifact -> final wave -> beacon -> win.
UCLASS()
class VEIL_API UVEILLoopSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	static constexpr int32 FragmentsNeeded = 10;
	static constexpr int32 TotalWaves = 5;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Loop") EVEILStage Stage = EVEILStage::FindSignal;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Loop") int32 Fragments = 0;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Loop") int32 WavesCleared = 0;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Loop") bool bBossDefeated = false;

	UPROPERTY(BlueprintAssignable) FVEILStageChanged OnStageChanged;
	UPROPERTY(BlueprintAssignable) FVEILFragmentCount OnFragmentCollected;

	UFUNCTION(BlueprintCallable, Category = "VEIL|Loop") void SignalFound();          // step 1
	UFUNCTION(BlueprintCallable, Category = "VEIL|Loop") void CollectFragment();      // step 2 (x10)
	UFUNCTION(BlueprintCallable, Category = "VEIL|Loop") bool BuildArtifact();        // step 3, needs 10/10
	UFUNCTION(BlueprintCallable, Category = "VEIL|Loop") bool StartFinalWave();       // step 4, needs the artifact
	UFUNCTION(BlueprintCallable, Category = "VEIL|Loop") void WaveCleared();          // call after each wave; boss after wave 5
	UFUNCTION(BlueprintCallable, Category = "VEIL|Loop") void BossDefeated();
	UFUNCTION(BlueprintCallable, Category = "VEIL|Loop") bool ActivateBeacon();       // step 5 -> Won

	// 0..100 for the HUD.
	UFUNCTION(BlueprintPure, Category = "VEIL|Loop") float ProgressPercent() const;
	UFUNCTION(BlueprintPure, Category = "VEIL|Loop") bool ArtifactBuilt() const { return Stage >= EVEILStage::FinalWave; }

	// Fibonacci enemy counts per wave: 3, 5, 8, 13, 21.
	UFUNCTION(BlueprintPure, Category = "VEIL|Loop") static int32 DronesForWave(int32 WaveIndex /*0..4*/);

private:
	void SetStage(EVEILStage S);
};
