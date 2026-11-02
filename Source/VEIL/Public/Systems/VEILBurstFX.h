// VEIL: THE LAST SIGNAL - coded impact.

// No Niagara asset, no Fab download: a burst is an expanding emissive shell, a
// ring on the golden angle and a light that dies with it. Unreal's own sphere
// and cylinder are the mesh library; phi is the timing.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILBurstFX.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class VEIL_API AVEILBurstFX : public AActor
{
	GENERATED_BODY()

public:
	AVEILBurstFX();

	/** Bigger kills throw a bigger burst: 0 shard, 3 signet. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|FX")
	int32 Tier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|FX")
	float LifeSeconds = 0.55f;

	/** Spawn a burst already aimed at the world, and let it clean itself up. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|FX")
	static AVEILBurstFX* Fire(UWorld* World, const FVector& At, int32 InTier);

	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Shell;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Ring;
	UPROPERTY(VisibleAnywhere) UPointLightComponent* Flash;

private:
	float Age = 0.f;
	FLinearColor Colour = FLinearColor::White;
};
