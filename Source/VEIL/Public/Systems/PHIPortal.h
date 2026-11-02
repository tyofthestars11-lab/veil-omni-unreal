// PHI portals: District hub <-> OMNIWorld <-> VEILWorld. Walk in, travel.
// Gold light towers, chime on use, 3s cooldown so arrival never bounces back.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PHIPortal.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class VEIL_API APhiPortal : public AActor
{
	GENERATED_BODY()

public:
	APhiPortal();

	/** Map to open, e.g. /Game/Maps/OMNIWorld. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Portal")
	FName TargetMap = NAME_None;

	/** Region word for the log and HUD, e.g. OMNIWorld. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Portal")
	FName RegionTag = NAME_None;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY() TObjectPtr<USphereComponent> Trigger;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Tower;
	UPROPERTY() TObjectPtr<UPointLightComponent> Glow;

	float LastUseTime = -100.f;

	UFUNCTION()
	void OnPortalTouch(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
};
