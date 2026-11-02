#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILWilds.generated.h"

class AVEILEnemy;
class UPointLightComponent;
class USphereComponent;
class UStaticMeshComponent;

/**
 * A MINE. Ore standing on the hill as a lit cluster, with a trigger you can walk
 * into. Harvesting pays gold and experience into the one quest book, puts the node
 * out for two minutes and brings it back.
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
 * A CAMP IS NOT A SPAWN POINT, IT IS A MIND.
 *
 * Every camp carries one phinary numeral - the fractional part of its clock times
 * phi - and acts on that digit. Below 1/phi^2 (0.382) the camp stands down; above
 * it the camp hunts. The numeral is not a random roll and not a timer: it is the
 * same continued-fraction rhythm the rest of the framework runs on, so two camps
 * placed one golden step apart never share a mood.
 *
 * The same clock breathes the bodies: damage rises as speed falls, on opposite
 * phases, so a camp at full aggression is quick and light and a camp at rest is
 * slow and heavy. Nothing in the loop is a fixed constant.
 */
USTRUCT()
struct FVEILCamp
{
	GENERATED_BODY()

	UPROPERTY() TArray<TWeakObjectPtr<AVEILEnemy>> Members;
	float Phase = 0.f;
	/** The phinary decide digit, 0..1. */
	float Intent = 0.f;
	float BaseDamage = 10.f;
	float BaseSpeed = 300.f;
	bool bResting = false;
};

/**
 * THE WILDS HAVE INHABITANTS.
 *
 * Camps of six species on the golden spiral out past the authority coordinates,
 * plus mines on the same rule. One machine class, six bodies, and each camp run
 * by its own phi clock.
 */
UCLASS()
class VEIL_API AVEILWildsPopulation : public AActor
{
	GENERATED_BODY()

public:
	AVEILWildsPopulation();

	virtual void Tick(float DeltaSeconds) override;

	/** Spawn camps and mines. Safe to call once. */
	void Populate(int32 Camps, int32 Mines);

	int32 EnemiesSpawned() const { return SpawnedEnemies; }
	int32 MinesPlaced() const { return SpawnedMines; }

	/** The last decide digit of a camp, for the HUD or a proof. */
	float IntentOf(int32 Camp) const;

private:
	UPROPERTY() TArray<FVEILCamp> Camps;

	int32 SpawnedEnemies = 0;
	int32 SpawnedMines = 0;
	float EcoClock = 0.f;
	float DecideClock = 0.f;
};
