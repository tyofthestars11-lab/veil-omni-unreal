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
 * The vision image is a cliff market: gold phi pillars carrying lantern caps and
 * blue banners, eight stalls under blue canvas, and faceted blue crystal peaks
 * standing over the roofs, with the golden Spire behind. The families pass only
 * builds its market when the composition is absent, so on the baked district
 * nothing places these; this pass owns them, once, at the Hub's authority point
 * (0,0), on real ground, by the golden angle. No random numbers, no floaters:
 * every piece is grounded by a downward trace and skipped (and counted) if the
 * ground does not answer.
 */
UCLASS()
class VEIL_API AVEILHubMarket : public AActor
{
	GENERATED_BODY()

public:
	AVEILHubMarket();

	/** Build the twelve pillars, eight stalls and five crystal peaks. Idempotent per actor. */
	void Build();

	int32 Pillars = 0;
	int32 Stalls = 0;
	int32 Crystals = 0;
	int32 Banners = 0;

private:
	UStaticMeshComponent* Part(UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FRotator& Rot, UMaterialInterface* Mat, bool bSolid);

	/** The pavement at X,Y, or false when nothing solid answers. */
	bool GroundAt(float X, float Y, FVector& Out) const;

	void BuildPillar(const FVector& At, float YawDeg);
	void BuildStall(const FVector& At, float YawDeg);
	void BuildCrystal(const FVector& At);

	UPROPERTY() UStaticMesh* Cube = nullptr;
	UPROPERTY() UStaticMesh* Cylinder = nullptr;
	UPROPERTY() UStaticMesh* Cone = nullptr;
	UPROPERTY() UStaticMesh* Plane = nullptr;
	UPROPERTY() UMaterialInterface* Gold = nullptr;
	UPROPERTY() UMaterialInterface* Blue = nullptr;
	UPROPERTY() UMaterialInterface* Glow = nullptr;
	UPROPERTY() UMaterialInterface* Stone = nullptr;
	UPROPERTY() TArray<UStaticMeshComponent*> Parts;
};
