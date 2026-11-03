#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILContentFamilies.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

/**
 * THE FAMILIES THE ASSET LIBRARY DOES NOT HAVE, BUILT BY PHI RULE.
 *
 * The audit found zero trees, zero market tents, zero stall counters, zero
 * bridges and zero crystal geometry in 8,858 assets. None of that is a reason
 * for the district to stay empty: a crystal is four faceted shards, a tree is a
 * trunk and three canopy cones, a stall is a counter with an awning, a bridge is
 * planks and two ropes.
 *
 * WHERE THEY GO IS THE WHOLE ARGUMENT. The first version of this pass planted
 * the families on a golden spiral around WORLD ORIGIN. The runner wakes at
 * 2,183,-480 in this district, so every piece landed 2,200 cm away from him,
 * inside the buildings, on whatever roof the sky trace struck first - and the
 * district read as random clutter instead of a place. Everything is now placed
 * in a ring around the RUNNER, on HIS floor, refusing any spot that has a roof
 * overhead, a wall in the way, a different floor height, or a neighbour within
 * the spacing law.
 */
UCLASS()
class VEIL_API AVEILContentPass : public AActor
{
	GENERATED_BODY()

public:
	AVEILContentPass();

	/** Build every missing family once, in a ring around the runner. */
	void Populate();

	int32 Crystals = 0;
	int32 Trees = 0;
	int32 Stalls = 0;
	int32 Tents = 0;
	int32 Bridges = 0;

private:
	UStaticMeshComponent* Part(UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FRotator& Rot, UMaterialInterface* Mat, bool bSolid);

	/**
	 * One acceptable spot on the runner's floor: open sky above, 240 cm of room,
	 * and at least MinSpacing from every spot already taken this pass.
	 */
	bool FindSpot(const FVector& Anchor, float Radius, float AngleDeg, float MinSpacing, FVector& Out);

	void BuildCrystal(const FVector& At);
	void BuildTree(const FVector& At);
	void BuildStall(const FVector& At, float YawDeg);
	void BuildTent(const FVector& At, float YawDeg);
	void BuildBridge(const FVector& A, const FVector& B);

	UPROPERTY() UStaticMesh* Cube = nullptr;
	UPROPERTY() UStaticMesh* Cylinder = nullptr;
	UPROPERTY() UStaticMesh* Cone = nullptr;
	UPROPERTY() UMaterialInterface* Gold = nullptr;
	UPROPERTY() UMaterialInterface* DeepBlue = nullptr;
	UPROPERTY() TArray<UStaticMeshComponent*> Parts;

	/** Spots already claimed this pass, so the ring does not stack up on itself. */
	TArray<FVector> Taken;
};
