// AVEILGoldenSpawner: raises Count cubes in a golden sunflower spiral.
// Defaults are Hark's: Count 34, Spacing 100, Ccbe scale 0.3, snapped to ground
// with a line trace so the spiral sits on the floor instead of floating.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILGoldenSpawner.generated.h"

class UStaticMeshComponent;
class UStaticMesh;
class UMaterialInterface;

UCLASS()
class VEIL_API AVEILGoldenSpawner : public AActor
{
	GENERATED_BODY()

public:
	AVEILGoldenSpawner();

	/** How many cubes in the spiral. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Golden")
	int32 Count = 34;

	/** Radius step of the spiral, in cm. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Golden")
	float Spacing = 100.f;

	/** Uniform scale of each cube. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Golden")
	float CubeScale = 0.3f;

	/** Draw the yellow proof lines for i = 1, 2, 5. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Golden")
	bool bDrawProofLines = true;

	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

private:
	void BuildSpiral();

	UPROPERTY()
	TArray<TObjectPtr<UStaticMeshComponent>> Cubes;

	UPROPERTY()
	TObjectPtr<UStaticMesh> CubeMesh;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> CubeMaterial;
};
