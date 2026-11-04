#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILHubMarket.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

/**
 * THE HUB MARKET, BUILT FROM THE REFERENCE.
 *
 * The vision image is a cliff market: gold phi pillars, eight stalls, and five
 * peak sites standing over the roofs, with the golden Spire behind. The families pass only
 * builds its market when the composition is absent, so on the baked district
 * nothing places these; this pass owns them, once, at the Hub's authority point
 * (0,0), on real ground, from seed 1618 and the golden angle. It uses project meshes only;
 * missing crystal and canopy art is reported, not replaced with engine primitives.
 * No random numbers, no floaters:
 * every piece is grounded by a downward trace and skipped (and counted) if the
 * ground does not answer.
 */
UCLASS()
class VEIL_API AVEILHubMarket : public AActor
{
	GENERATED_BODY()

public:
	AVEILHubMarket();

	/** Build the twelve pillars, eight stalls and five crystal peaks. Idempotent per actor.
	 *  BlueprintCallable so the CMD bake can compose the market INTO the level file instead of
	 *  leaving an empty actor that only fills in at runtime. */
	UFUNCTION(BlueprintCallable, Category = "VEIL")
	void Build();

	int32 Pillars = 0;
	int32 Stalls = 0;
	int32 Peaks = 0;

private:
	UStaticMeshComponent* Part(UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FRotator& Rot, UMaterialInterface* Mat, bool bSolid);
	UStaticMeshComponent* PartAtGround(UStaticMesh* Mesh, const FVector& Ground, const FVector& Size,
		const FRotator& Rot, UMaterialInterface* Mat, bool bSolid);

	/** The pavement at X,Y, or false when nothing solid answers. */
	bool GroundAt(float X, float Y, FVector& Out) const;

	void BuildPillar(const FVector& At, float YawDeg);
	void BuildStall(const FVector& At, float YawDeg);
	void BuildPeak(const FVector& At, float YawDeg);

	UPROPERTY() UStaticMesh* ColumnMesh = nullptr;
	UPROPERTY() UStaticMesh* ArchMesh = nullptr;
	UPROPERTY() UStaticMesh* PlatformMesh = nullptr;
	UPROPERTY() UStaticMesh* StandMesh = nullptr;
	UPROPERTY() UStaticMesh* MonolithMesh = nullptr;
	UPROPERTY() UMaterialInterface* Gold = nullptr;
	UPROPERTY() UMaterialInterface* Blue = nullptr;
	UPROPERTY() UMaterialInterface* Stone = nullptr;
	UPROPERTY() TArray<UStaticMeshComponent*> Parts;
};
