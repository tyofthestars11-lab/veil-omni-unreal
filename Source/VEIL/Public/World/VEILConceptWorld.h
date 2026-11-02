// VEIL - THE CONCEPT WORLD.
//
// The composition the vision asks for is not a set of bespoke assets, it is a LAYOUT:
// three gold phi pillars standing over a cliffside market, tents on the terraces,
// waterfalls falling off the shelves, bridges between them, the gold Spire behind,
// blue crystal peaks on one flank, and the Megastructure Deep glowing under the
// district. Every one of those is a rule about where a thing goes relative to
// another thing, and the rule is the golden ratio.
//
// So this actor places them by phi and nothing else: successive landmarks step by
// 137.50776405 degrees, radii scale by 1.618 per ring, heights are rungs of the same
// ladder. It uses only engine shapes and the project's own PHI materials, so it is
// real geometry in the level without a single new asset.
//
// It also gives the runner wings: two gold blades that beat on the golden pitch.
// They are code, not a skeletal mesh, because the runner's body cannot animate one.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILConceptWorld.generated.h"

class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS()
class VEIL_API AVEILConceptWorld : public AActor
{
	GENERATED_BODY()

public:
	AVEILConceptWorld();

	virtual void Tick(float DeltaSeconds) override;

	/** Lay the composition down once. Safe to call twice; builds only once. */
	bool Build();

	int32 GetPieces() const { return Pieces; }

private:
	UStaticMeshComponent* SpawnPiece(UStaticMesh* Mesh, const FVector& Centre, const FVector& Size,
		UMaterialInterface* Mat, const FRotator& Rot, bool bCollide);

	void BuildPhiPillars(UStaticMesh* Cube, UStaticMesh* Cylinder, UStaticMesh* Sphere,
		UMaterialInterface* Gold, UMaterialInterface* Stone, UMaterialInterface* Glow);
	void BuildMarket(UStaticMesh* Cube, UStaticMesh* Cone, UStaticMesh* Cylinder,
		UMaterialInterface* Gold, UMaterialInterface* Stone);
	void BuildWaterAndBridges(UStaticMesh* Cube, UStaticMesh* Cylinder,
		UMaterialInterface* Water, UMaterialInterface* Stone, UMaterialInterface* Gold);
	void BuildCrystals(UStaticMesh* Cone, UStaticMesh* Cylinder, UMaterialInterface* Water,
		UMaterialInterface* Glow);
	void BuildSpire(UStaticMesh* Cube, UStaticMesh* Cylinder, UStaticMesh* Sphere,
		UMaterialInterface* Gold, UMaterialInterface* Stone, UMaterialInterface* Glow);
	void BuildTerraces(UStaticMesh* Cube, UStaticMesh* Cylinder,
		UMaterialInterface* Stone, UMaterialInterface* Water, UMaterialInterface* Gold);
	void BuildCrystalRange(UStaticMesh* Cone, UStaticMesh* Cylinder,
		UMaterialInterface* Water, UMaterialInterface* Glow);
	void BuildBiomes(UStaticMesh* Cube, UStaticMesh* Cone, UStaticMesh* Cylinder,
		UStaticMesh* Sphere, UMaterialInterface* Stone, UMaterialInterface* Water,
		UMaterialInterface* Gold);
	void BuildDeep(UStaticMesh* Cube, UStaticMesh* Cylinder, UMaterialInterface* Water,
		UMaterialInterface* Glow);
	void BuildWings(UStaticMesh* Cube, UMaterialInterface* Gold);

	UPROPERTY() TArray<TWeakObjectPtr<AActor>> Wings;
	float WingPhase = 0.f;
	int32 Pieces = 0;
	bool bBuilt = false;
};
