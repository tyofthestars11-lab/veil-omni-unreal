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
 * bridges, zero crystal geometry and no particle system in 8,858 assets. None of
 * that is a reason for the district to stay empty: a crystal is four faceted
 * shards, a tree is a trunk and three canopy cones, a stall is a counter with an
 * awning, a bridge is planks and two ropes. All of it is real geometry placed by
 * the golden angle on ground that exists - not a placeholder cone on a hill.
 */
UCLASS()
class VEIL_API AVEILContentPass : public AActor
{
	GENERATED_BODY()

public:
	AVEILContentPass();

	/** Build every missing family once. Counts accumulate so the log can be read. */
	void Populate();

	int32 Crystals = 0;
	int32 Trees = 0;
	int32 Stalls = 0;
	int32 Tents = 0;
	int32 Bridges = 0;

private:
	UStaticMeshComponent* Part(UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FRotator& Rot, UMaterialInterface* Mat, bool bSolid);

	bool GroundAt(float X, float Y, float& OutZ) const;

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
};
