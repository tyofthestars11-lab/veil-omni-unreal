#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILLoopSubsystem.h"
#include "VEILEnemy.h"
#include "VEILWaveSpawner.generated.h"

// Runs the final defence at the Vault: 5 waves (drones 3,5,8,13,21 plus soldiers), then WARDEN PRIME.
// Reports to UVEILLoopSubsystem: WaveCleared() after each wave, BossDefeated() when the boss falls.
UCLASS()
class VEIL_API AVEILWaveSpawner : public AActor
{
	GENERATED_BODY()
public:
	AVEILWaveSpawner();
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "VEIL|Waves") TSubclassOf<AVEILEnemy> EnemyClass; // BP child with a mesh (drone/soldier/boss)
	UPROPERTY(EditAnywhere, Category = "VEIL|Waves") float SpawnRadius = 2200.f;
	UPROPERTY(EditAnywhere, Category = "VEIL|Waves") float PauseBetweenWaves = 4.f;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Waves") int32 CurrentWave = 0; // 1..5, 6 = boss
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Waves") int32 Alive = 0;

	// Golden-angle placement: step * 137.5077640500378 mod 360 (Bible §1, via PHI_Constants)
	UFUNCTION(BlueprintPure, Category = "VEIL|Waves") static float BranchAngleDeg(int32 Step);

private:
	UFUNCTION() void HandleStage(EVEILStage NewStage);
	UFUNCTION() void HandleEnemyDied(AVEILEnemy* Enemy);

	UVEILLoopSubsystem* Loop() const;
	void StartNextWave();
	void SpawnOne(EVEILEnemyKind Kind, int32 SpawnIndex);
	FTimerHandle WaveTimer;
	int32 SpawnCounter = 0;
	bool bRunning = false;
	bool bBossPhase = false;
};
