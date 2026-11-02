#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILActivityMarker.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;
class USphereComponent;
class UVEILCampaignSubsystem;

/**
 * ONE OF THE FIFTY, IN THE WORLD.
 *
 * A number in a log is not a game. Each activity gets a marker at a place the
 * runner can walk to: a gold sphere with a lamp, on the golden angle around its
 * region's anchor. Walking into it records the visit; the activity still only
 * closes when its counter is met, so "hunt three echoes in the Hub" is a hunt
 * and then a walk, not a checkbox.
 */
UCLASS()
class VEIL_API AVEILActivityMarker : public AActor
{
	GENERATED_BODY()

public:
	AVEILActivityMarker();

	/** Which activity this stands for, and who owns it. */
	void Bind(UVEILCampaignSubsystem* InCampaign, int32 InIndex, const FString& Title);

	UPROPERTY() UStaticMeshComponent* Mesh = nullptr;
	UPROPERTY() UPointLightComponent* Lamp = nullptr;
	UPROPERTY() USphereComponent* Trigger = nullptr;

	UFUNCTION()
	void OnEnter(UPrimitiveComponent* OverlappedComp, AActor* Other,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
		const FHitResult& Sweep);

private:
	TWeakObjectPtr<UVEILCampaignSubsystem> Campaign;
	int32 Index = INDEX_NONE;
};
