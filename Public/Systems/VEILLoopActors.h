#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILInteractable.h"
#include "VEILLoopActors.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

// Fragment: interact to collect (10 in the world: Nest 3, Spire 2, Omni 3, Deep 2).
UCLASS()
class VEIL_API AVEILFragment : public AActor, public IVEILInteractable
{
	GENERATED_BODY()
public:
	AVEILFragment();
	UPROPERTY(EditAnywhere, Category = "VEIL") FName RegionId; // Nest / Spire / Omni / Deep
	UPROPERTY(EditAnywhere, Category = "VEIL") FText LoreLine;
	virtual void Interact_Implementation(AActor* Instigator) override;
protected:
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere) UPointLightComponent* Glow;
};

// Spire forge: with 10/10 fragments, interact to build the artifact (unlocks the Vault).
UCLASS()
class VEIL_API AVEILForge : public AActor, public IVEILInteractable
{
	GENERATED_BODY()
public:
	AVEILForge();
	virtual void Interact_Implementation(AActor* Instigator) override;
protected:
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
};

// Beacon: after WARDEN PRIME falls, interact to activate it -> win.
UCLASS()
class VEIL_API AVEILBeacon : public AActor, public IVEILInteractable
{
	GENERATED_BODY()
public:
	AVEILBeacon();
	virtual void Interact_Implementation(AActor* Instigator) override;
protected:
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Mesh;
	UPROPERTY(VisibleAnywhere) UPointLightComponent* Light;
};
