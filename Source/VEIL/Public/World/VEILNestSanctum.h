// VEIL - THE NEST SANCTUM.
//
// The hub is buried. The World Partition bake stacked roofs over the plaza centre and the authority
// spawn at 0,0,100 lands inside that mass: the runner wakes with the camera in a wall and the seat
// probe reporting "floor Actor_0 at 0 cm". Moving the world is not the fix (the Bible fixes the
// coordinate) and re-baking is a separate, slower job.
//
// So the sanctum is CODE. It measures the surface the hub actually has, raises a stone deck on it,
// and dresses it with the project's own palette: a gold ring on the golden angle, three stone
// pillars with gold crowns, the Spire at the centre, three beacon shafts, and a fall of water.
// Every piece is engine geometry wearing PHI materials - no Niagara asset, no authored level.
//
// It also MOVES. A platform of static cubes is a diorama; the halo turns on the golden angle and
// the beacon shafts breathe against the golden pitch, which is the part of an effect that can be
// built in code without an effects asset existing on disk.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILNestSanctum.generated.h"

class UMaterialInterface;
class UPointLightComponent;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS()
class VEIL_API AVEILNestSanctum : public AActor
{
	GENERATED_BODY()

public:
	AVEILNestSanctum();

	virtual void Tick(float DeltaSeconds) override;

	/**
	 * Raise the deck, the ring, the pillars, the Spire, the beacon shafts and the water.
	 * Builds once; safe to call again. Returns false only if the engine shapes are missing.
	 */
	bool BuildSanctum();

	/** The height the deck was raised to, in cm. Valid after BuildSanctum(). */
	float GetDeckZ() const { return DeckZ; }

	/** How many pieces the sanctum actually placed, for the log and for proofs. */
	int32 GetPieces() const { return Pieces; }

private:
	/** One placed piece: size is in CENTIMETRES, not scale, and is normalised per mesh. */
	UStaticMeshComponent* SpawnPiece(UStaticMesh* Mesh, const FVector& Centre, const FVector& Size,
		UMaterialInterface* Mat, bool bCollide);

	/** A light with no shadow and no GI: the sanctum is lit, not lit twice. */
	UPointLightComponent* AddLamp(const FVector& At, float Intensity, float Radius,
		const FLinearColor& Colour);

	float DeckZ = 98.f;
	int32 Pieces = 0;
	bool bBuilt = false;

	/** The turning halo: gold shards orbiting the Spire on the golden angle. */
	TArray<TWeakObjectPtr<AActor>> Halo;
	float HaloPhase = 0.f;

	/** Beacon shafts and the intensity each was built at, so the pulse is relative, not absolute. */
	TArray<TWeakObjectPtr<UPointLightComponent>> BeaconLamps;
	TArray<float> BeaconBase;
};
