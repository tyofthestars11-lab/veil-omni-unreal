#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILWilds.generated.h"

class UPointLightComponent;
class USphereComponent;
class UStaticMeshComponent;

/**
 * A MINE. Ore standing on the hill as a lit cluster, with a trigger you can walk
 * into. Harvesting pays gold and experience into the one quest book, puts the node
 * out for two minutes and brings it back: a resource you strip once and never
 * return to is a prop, not a mine.
 */
UCLASS()
class VEIL_API AVEILMineNode : public AActor
{
	GENERATED_BODY()

public:
	AVEILMineNode();
	virtual void Tick(float DeltaSeconds) override;

	void Init(int32 InIndex, int32 InGold);

	UFUNCTION()
	void OnEnter(UPrimitiveComponent* OverlappedComp, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& Sweep);

	UPROPERTY() UStaticMeshComponent* Ore = nullptr;
	UPROPERTY() UPointLightComponent* Lamp = nullptr;
	UPROPERTY() USphereComponent* Trigger = nullptr;

private:
	float RespawnAt = 0.f;
	int32 Gold = 40;
	int32 Index = 0;
	bool bSpent = false;
};

/**
 * THE WILDS HAVE INHABITANTS.
 *
 * Camps of six species, each with its own body of stats, on the golden spiral out
 * past the authority coordinates, plus mines on the same rule. The species are the
 * point: a Hollow is numerous and fragile, a Stoneborn is slow and hard, a Warden
 * Spawn is a champion with champion spoils. One engine class, six bodies, so the
 * country fights differently in different places.
 */
UCLASS()
class VEIL_API AVEILWildsPopulation : public AActor
{
	GENERATED_BODY()

public:
	AVEILWildsPopulation();

	/** Spawn camps and mines. Safe to call once. */
	void Populate(int32 Camps, int32 Mines);

	int32 EnemiesSpawned() const { return SpawnedEnemies; }
	int32 MinesPlaced() const { return SpawnedMines; }

private:
	int32 SpawnedEnemies = 0;
	int32 SpawnedMines = 0;
};
