#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILCrowd.generated.h"

class UInstancedStaticMeshComponent;
class USkeletalMeshComponent;
class UStaticMesh;
class UMaterialInterface;

/** One body in the market: an orbit, a speed, and a bob. */
USTRUCT()
struct FVEILWalker
{
	GENERATED_BODY()
	float Angle = 0.f;
	float Radius = 0.f;
	float Speed = 0.f;
	float Phase = 0.f;
	float GroundZ = 0.f;
	float Scale = 1.f;
};

/**
 * THE MARKET HAS PEOPLE IN IT.
 *
 * Landmarks and tents are a set; a market is a crowd. This actor walks bodies
 * around the plaza on golden-angle orbits between the tents, drawn as two
 * instanced meshes (bodies and heads) so fifty people cost two draw calls and
 * one tick instead of fifty actors and fifty ticks.
 *
 * The orbits are not random: each walker starts on i * 137.50776405 degrees at a
 * radius that grows with the square root of i, and turns at the speed that keeps
 * its arc rate equal to its neighbours', so the crowd drifts the way a crowd
 * drifts - never in step, never in collision.
 */
UCLASS()
class VEIL_API AVEILCrowd : public AActor
{
	GENERATED_BODY()

public:
	AVEILCrowd();

	virtual void Tick(float DeltaSeconds) override;

	/** Spawn the walkers. Safe to call once; count is clamped to what the ring holds. */
	void Populate(int32 Count);

	UFUNCTION(BlueprintPure, Category = "VEIL|Crowd")
	int32 CrowdCount() const { return Walkers.Num(); }

private:
	UPROPERTY() UInstancedStaticMeshComponent* Bodies = nullptr;
	UPROPERTY() TArray<USkeletalMeshComponent*> People;
	UPROPERTY() UInstancedStaticMeshComponent* Heads = nullptr;
	UPROPERTY() TArray<FVEILWalker> Walkers;

	float Clock = 0.f;
	bool bReady = false;
};
