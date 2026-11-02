#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILWilds.generated.h"

class AVEILEnemy;
class UPointLightComponent;
class USphereComponent;
class UStaticMeshComponent;
class USceneComponent;

/**
 * A MINE IS A PLACE WITH A NAME, NOT A CONE ON A HILL.
 *
 * Every mine in the wilds is authored: it has a name, an owner, an ore, a depth
 * and a reserve, and it is built as a headframe - four posts, two beams, a
 * shaft, and the seam itself piled at the mouth. Nothing here is a placeholder
 * standing in for a mine, and nothing is rolled at random: the twenty-four
 * names below are the world's mines, and a run always finds the same ones.
 *
 * Owning matters. The owner is whose ground it is, so stripping a seam is a
 * claim on someone's property, and the reply names them.
 */
UCLASS()
class VEIL_API AVEILMineNode : public AActor
{
	GENERATED_BODY()

public:
	AVEILMineNode();
	virtual void Tick(float DeltaSeconds) override;

	/** Give the mine the identity of its entry in the mine book. */
	void Init(int32 InIndex);

	UFUNCTION()
	void OnEnter(UPrimitiveComponent* OverlappedComp, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& Sweep);

	UPROPERTY() USceneComponent* Root = nullptr;
	UPROPERTY() UPointLightComponent* Lamp = nullptr;
	UPROPERTY() USphereComponent* Trigger = nullptr;

	const FString& MineName() const { return Name; }
	const FString& MineOwner() const { return Owner; }
	const FString& MineOre() const { return Ore; }
	float MineDepth() const { return DepthM; }
	int32 MineReserve() const { return Reserve; }

private:
	/** Posts, beams, shaft and seam - the headframe, built once. */
	void BuildHead();

	FString Name;
	FString Owner;
	FString Ore;
	FLinearColor OreTint = FLinearColor(0.35f, 0.62f, 1.f);
	float DepthM = 0.f;
	int32 Reserve = 0;
	int32 Gold = 40;
	int32 Index = 0;
	float RespawnAt = 0.f;
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
	/** Desired heading per member, refreshed on the decide clock and applied every frame. */
	TArray<FVector> Dir;
	float Phase = 0.f;
	/** The phinary decide digit, 0..1. */
	float Intent = 0.f;
	float BaseDamage = 10.f;
	float BaseSpeed = 300.f;
	/** Where the camp stands: the place its members retreat to. */
	FVector Center = FVector::ZeroVector;
	bool bResting = false;
	/** Last order issued, as an int so the enum stays out of the header. */
	int32 Order = -1;
	/** An order that differs from the current one, and how many decisions in a
	 *  row it has held. Two in a row before the camp changes what it is doing. */
	int32 Pending = -1;
	int32 PendingCount = 0;
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
