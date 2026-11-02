// VEIL - THE NEST SANCTUM.
//
// The hub is buried. The World Partition bake stacked roofs over the plaza centre (500, 1250,
// 1751, 2440 cm) and the authority spawn at 0,0,100 lands INSIDE that mass: the runner wakes with
// the camera in a wall and the seat probe reporting "floor Actor_0 at 0 cm". Moving the world is
// not the fix (the Bible fixes the coordinate) and re-baking is a separate, slower job.
//
// So the sanctum is CODE. It traces for the surface the hub actually has, raises a stone deck on
// it, and dresses that deck with the project's own palette: a gold ring on the golden angle, three
// stone pillars with gold crowns, the Spire at the centre, three beacon shafts with lights, and a
// fall of water at the deck's edge. Every piece is engine geometry wearing PHI materials - no
// Niagara asset, no authored level, nothing that has to be made in the editor first.
//
// It is built from primitives on purpose: a missing asset is a missing asset, and the district
// has already been held hostage once by effects that did not exist on disk.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILNestSanctum.generated.h"

class UMaterialInterface;
class UStaticMesh;
class UStaticMeshComponent;

UCLASS()
class VEIL_API AVEILNestSanctum : public AActor
{
	GENERATED_BODY()

public:
	AVEILNestSanctum();

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
	void AddLamp(const FVector& At, float Intensity, float Radius, const FLinearColor& Colour);

	float DeckZ = 98.f;
	int32 Pieces = 0;
	bool bBuilt = false;
};
