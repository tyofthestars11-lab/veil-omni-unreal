// AVEILPhiTetraSpawner: a chain of tetrahedra, under one of Hark's two rules.
//
// Chain60 (the default, Hark's newer result):
//   size(0)   = BaseSize
//   size(i)   = BaseSize * phi^i
//   spot(i+1) = spot(i) + size(i) * (cos(60*i), sin(60*i))   (steps forward
//               by the size it already has, then grows by 1.618)
//
// GoldenSpiral (Hark's first result):
//   position = GoldenLoc(i, Spacing),  size = BaseSize * phi^(i-1)
//
// Either way the position and size of every step are saved into SavedPositions /
// SavedSizes and logged, so the chain can be checked without a screenshot.
//
// COUNT HAS NO CEILING, BECAUSE PHI DOES NOT SET ONE. Growth is a rung, not a
// size: step 34 is 33 rungs above step 1, which is 4.6 million times the anchor
// if it is measured in one band. So the chain is read in RUNG SPACE instead of
// capping the count - RungOffset lifts the whole chain into whichever band the
// world can hold, geometry and all, while SavedSizes keeps the TRUE BaseSize *
// phi^i for every step. Nothing is truncated; only the band is chosen.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILPhiTetraSpawner.generated.h"

class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

/** Which of Hark's two rules lays the tetrahedra out. */
UENUM(BlueprintType)
enum class EPhiTetraRule : uint8
{
	/** Grow and step forward by the previous size, 60 degrees around. */
	Chain60,
	/** Hark's golden phyllotaxis / sunflower spiral. */
	GoldenSpiral
};

UCLASS()
class VEIL_API AVEILPhiTetraSpawner : public AActor
{
	GENERATED_BODY()

public:
	AVEILPhiTetraSpawner();

	/** Which rule lays the chain out. Chain60 is Hark's newer tetra result. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Phi")
	EPhiTetraRule Rule = EPhiTetraRule::Chain60;

	/** Number of tetrahedra in the chain. No ceiling - a long chain is a band
	 *  choice, not an impossibility. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Phi")
	int32 Count = 6;

	/** Which rung band the chain is laid in: positions and drawn sizes are
	 *  divided by phi^RungOffset, so 34 steps stand in the same world span a
	 *  6-step chain does. SavedSizes always holds the true BaseSize * phi^i. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Phi")
	float RungOffset = 0.f;

	/** GoldenLoc radius step, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Phi")
	float Spacing = 140.f;

	/** Size of the first tetrahedron, in cm (its own height). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Phi")
	float BaseSize = 20.f;

	/** Growth per step. phi^2 = phi + 1, so 1.618033988749895. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Phi")
	float Growth = 1.618033988749895f;

	/** Yaw advance per step, in degrees. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Phi")
	float YawStepDegrees = 60.f;

	/** Line the chain so the sequence is readable in Play. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Phi")
	bool bDrawChain = true;

	/** Saved world position of each step, written as the chain is built. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Phi")
	TArray<FVector> SavedPositions;

	/** Saved size (height in cm) of each step. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Phi")
	TArray<float> SavedSizes;

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

private:
	void BuildChain();

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Tetras;

	UPROPERTY()
	TObjectPtr<UStaticMesh> TetraMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> TetraMaterial;
};
