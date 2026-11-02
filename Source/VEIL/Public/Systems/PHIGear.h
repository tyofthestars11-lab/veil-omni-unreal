// PHI gear: pickups with real stats. Walk over, feel it forever.
// Damage multiplies bolt hits, Speed adds walk pace, Health raises the ceiling.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PHIGear.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UPointLightComponent;

UENUM(BlueprintType)
enum class EPHIGearStat : uint8
{
	Damage,
	Speed,
	Health,
	Rapid
};

UCLASS()
class VEIL_API APhiGear : public AActor
{
	GENERATED_BODY()

public:
	APhiGear();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Gear")
	EPHIGearStat Stat = EPHIGearStat::Damage;

	/** 1-3. Damage +10%/tier, Speed +40 pace/tier, Health +25 ceiling/tier, Rapid cooldown x0.92^tier. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Gear")
	int32 Tier = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Gear")
	FName GearName = NAME_None;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY() TObjectPtr<USphereComponent> Touch;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Relic;
	UPROPERTY() TObjectPtr<UPointLightComponent> Shine;

	UFUNCTION()
	void OnGearTouch(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
};
