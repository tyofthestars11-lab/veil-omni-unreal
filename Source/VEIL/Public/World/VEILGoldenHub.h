// VEIL OMNI - THE GOLDEN HUB, built from installed authored meshes.
//
// The reference is a specific place: gold pillars framing the plaza, a walled city rising in
// tiers behind, rope bridges and a gold spire. The market owns its 25-site layout separately.
// Missing canopy, crystal and waterfall art is reported rather than imitated with stretched meshes.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILGoldenHub.generated.h"

class UStaticMesh;
class UStaticMeshComponent;
class UMaterialInterface;

UCLASS()
class VEIL_API AVEILGoldenHub : public AActor
{
	GENERATED_BODY()

public:
	AVEILGoldenHub();

	static constexpr int32 CurrentCompositionVersion = 3;

	/** Builds the whole hub. Deterministic: same call, same district, every time. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Hub")
	void Build();

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Hub")
	int32 CompositionVersion = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Hub")
	int32 PartCount = 0;

	/** The point the player should stand to see the painting: on the plaza, facing the spire. */
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Hub")
	FVector ViewPoint = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Hub")
	FVector ViewTarget = FVector::ZeroVector;

private:
	UStaticMeshComponent* Part(UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
		const FRotator& Rot, UMaterialInterface* Mat, bool bSolid);
	UStaticMeshComponent* PartFitted(UStaticMesh* Mesh, const FVector& Center, const FVector& Size,
		const FRotator& Rot, UMaterialInterface* Mat, bool bSolid);

	void Pillar(const FVector& At, float Height, float Radius, float YawDeg);
	void House(const FVector& At, float YawDeg, float Width, float Depth, float Height);
	void Bridge(const FVector& A, const FVector& B);
	void Spire(const FVector& At, float Height, float Radius);

	// Authored meshes from the installed ancient kit; no engine-shape fallbacks.
	UPROPERTY() TObjectPtr<UStaticMesh> KitWall;
	UPROPERTY() TObjectPtr<UStaticMesh> KitFloor;
	UPROPERTY() TObjectPtr<UStaticMesh> KitColumn;
	UPROPERTY() TObjectPtr<UStaticMesh> KitDecor;
	UPROPERTY() TObjectPtr<UStaticMesh> KitMonolith;

	UPROPERTY() TObjectPtr<UMaterialInterface> Gold;
	UPROPERTY() TObjectPtr<UMaterialInterface> Blue;
	UPROPERTY() TObjectPtr<UMaterialInterface> Cream;

	/** THE HUB WAS NOT BEING SAVED, AND THIS IS WHY.
	 *
	 * Part() creates components with NewObject and RegisterComponent, and nothing held a
	 * reference to them. A component referenced only by the actor's component list is not
	 * serialized into a level, so the bake logged "HUB parts: 1061" and the level reloaded
	 * with 0 - the composition existed for exactly one session. AVEILHubMarket has carried
	 * this very array from the start, which is why its 45 parts survived the same round trip.
	 * Holding them here makes the hub level data instead of a session effect. */
	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	/** The hub's axis: the direction the reference is seen along. */
	FVector Fwd = FVector(1.f, 0.f, 0.f);
	FVector Right = FVector(0.f, 1.f, 0.f);
};
