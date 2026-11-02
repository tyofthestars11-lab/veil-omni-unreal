// VEIL: THE LAST SIGNAL - physical loot.
//
// A machine that falls leaves something in the world: a lit drop that sits on
// the ground where it fell, and is only yours when the runner walks over it.
// No floating reward, no silent inventory edit - the district asks you to move.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILLootPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class VEIL_API AVEILLootPickup : public AActor
{
	GENERATED_BODY()

public:
	AVEILLootPickup();

	/** What this drop is: its name in the ledger and what it is worth. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Loot")
	FName ItemName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Loot")
	int32 GoldValue = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Loot")
	int32 Tier = 0;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnTouch(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& SweepResult);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere) USphereComponent* Touch;
	UPROPERTY(VisibleAnywhere) UStaticMeshComponent* Core;
	UPROPERTY(VisibleAnywhere) UPointLightComponent* Light;

private:
	float Spin = 0.f;
	bool bTaken = false;
};
