#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "VeilSaveGame.generated.h"

/**
 * One save, one Runner, one Signal. Slot name is fixed by the Bible: VEILOMNI_Omni0.
 * Nothing here is a cache: beacons lit and organelles engulfed are permanent.
 */
UCLASS()
class VEILOMNI_API UVeilSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY() FString SlotName = TEXT("VEILOMNI_Omni0");
	UPROPERTY() bool bKeeperCounted = false;
	UPROPERTY() bool bQuestAccepted = false;
	UPROPERTY() int32 BeaconsLit = 0;
	UPROPERTY() float ATP = 89.f;
	UPROPERTY() int32 Organelles = 0;
	UPROPERTY() double AddressRung = 20.189;
	UPROPERTY() FVector RunnerAt = FVector(0.0, 0.0, 100.0);
};
