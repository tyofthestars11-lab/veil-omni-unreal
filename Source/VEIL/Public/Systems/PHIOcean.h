// PHI ocean: one swim volume + drifting sea life + glowing flora.
// Blue box of water the Runner can enter; fish orbit inside it.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PHIOcean.generated.h"

UCLASS()
class VEIL_API APhiOcean : public AActor
{
	GENERATED_BODY()

public:
	APhiOcean();

	UPROPERTY(EditAnywhere, Category = "VEIL|Ocean")
	FVector WaterSize = FVector(3000.f, 3000.f, 400.f);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DT) override;

private:
	UPROPERTY()
	class UBoxComponent* Water = nullptr;

	UPROPERTY()
	class UStaticMeshComponent* Skin = nullptr;

	UPROPERTY()
	TArray<AActor*> Fish;

	UFUNCTION()
	void OnSplash(UPrimitiveComponent* Overlapped, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 BodyIndex, bool bFromSweep, const FHitResult& Sweep);

	void GrowFish(int32 Count);
	void GrowFlora(int32 Count);
	void DriftFish(float DT);
};
