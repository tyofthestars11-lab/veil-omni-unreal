#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Systems/VEILInteractable.h"
#include "VEILVault.generated.h"

class AVEILVault;
class UStaticMeshComponent;
class UPointLightComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVaultOpened);

// One of the 4 tetrahedral anchors. Interact once to align it.
UCLASS()
class VEIL_API AVEILVaultAnchor : public AActor, public IVEILVaultInteractable
{
	GENERATED_BODY()
public:
	AVEILVaultAnchor();
	virtual void Interact_Implementation(AActor* Instigator) override;

	UPROPERTY() AVEILVault* Vault = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL") int32 Index = 0;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL") bool bAligned = false;

protected:
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere) UPointLightComponent* Glow;
};

// The Vault: a tetrahedral lattice at the world centre. Four anchors, four faces.
// It opens only when all 4 anchors are aligned (4/4 = 1.0, otherwise 0)
// and the artifact has been built.
UCLASS()
class VEIL_API AVEILVault : public AActor
{
	GENERATED_BODY()
public:
	AVEILVault();
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;

	// Tetrahedral angle between any two anchors as seen from the centre: acos(-1/3) = 109.471221 deg.
	static constexpr float TetraAngleDeg = 109.471221f;

	UPROPERTY(EditAnywhere, Category = "VEIL|Vault") float AnchorRadius = 900.f;
	UPROPERTY(EditAnywhere, Category = "VEIL|Vault") float AnchorHeight = 700.f; // centre of the tetrahedron above the floor
	UPROPERTY(EditAnywhere, Category = "VEIL|Vault") TSubclassOf<AVEILVaultAnchor> AnchorClass;
	UPROPERTY(EditAnywhere, Category = "VEIL|Vault") UStaticMesh* LatticeMesh = nullptr;  // set to your gold Vault mesh
	UPROPERTY(EditAnywhere, Category = "VEIL|Vault") UStaticMesh* AnchorMesh = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "VEIL|Vault") bool bArtifactBuilt = false;

	/** The district camp must fall before the Vault answers: four anchors and an
	 *  artifact are not enough while the machines that own the plaza still stand. */
	UPROPERTY(BlueprintReadWrite, Category = "VEIL|Vault") bool bCampCleared = false;
	UPROPERTY(BlueprintReadWrite, Category = "VEIL|Vault") int32 CampStanding = 0;
	UPROPERTY(BlueprintAssignable) FVaultOpened OnVaultOpened;

	// 4 anchors aligned -> 1.0 else 0.0
	UFUNCTION(BlueprintPure, Category = "VEIL|Vault") float OpenFactor() const;
	UFUNCTION(BlueprintPure, Category = "VEIL|Vault") int32 AlignedCount() const;
	UFUNCTION(BlueprintCallable, Category = "VEIL|Vault") void NotifyAnchorAligned();
	UFUNCTION(BlueprintPure, Category = "VEIL|Vault") bool IsOpen() const { return bOpen; }

	// Unit vector of anchor i from the tetrahedron centre (pairwise dot = -1/3).
	static FVector TetraDirection(int32 i);

protected:
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Lattice;
	UPROPERTY(VisibleAnywhere) UPointLightComponent* Core;
	UPROPERTY() TArray<AVEILVaultAnchor*> Anchors;
	bool bOpen = false;
	void TryOpen();
};


