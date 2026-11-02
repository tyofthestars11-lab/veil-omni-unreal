// PHI Forge: spend 2 shards + 50 gold, raise ForgeLevel, +10% bolt damage per level.
// The Forge answers the loop with fire.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PHIForge.generated.h"

UCLASS()
class VEIL_API APhiForge : public AActor
{
	GENERATED_BODY()

public:
	APhiForge();

	// Spend 2 shards + 50 gold for +10% bolt damage. Returns the new ForgeLevel, or -1 if short.
	UFUNCTION(BlueprintCallable, Category = "VEIL|Forge")
	int32 ForgeUp();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	class UBoxComponent* Anvil = nullptr;

	UPROPERTY()
	class UStaticMeshComponent* Body = nullptr;

	UPROPERTY()
	class UPointLightComponent* Fire = nullptr;

	UFUNCTION()
	void OnForgeTouch(UPrimitiveComponent* Overlapped, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep);
};
