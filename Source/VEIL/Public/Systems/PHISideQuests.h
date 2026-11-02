// PHI side quests: Echoes (foes down), Beacons (sites touched), CacheSweep,
// WardenDown. Real counters, real rewards, toasted on screen when done.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PHISideQuests.generated.h"

UCLASS()
class VEIL_API UPHISideQuests : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	void AddEcho();
	void TouchBeacon(FName Site);
	void OpenCache();
	void WardenDown();

	UFUNCTION(BlueprintPure, Category = "VEIL|Side")
	int32 Echoes() const { return EchoCount; }

	UFUNCTION(BlueprintPure, Category = "VEIL|Side")
	int32 Beacons() const { return BeaconSites.Num(); }

private:
	int32 EchoCount = 0;
	TSet<FName> BeaconSites;
	bool bEchoesDone = false;
	bool bBeaconsDone = false;
	bool bCacheDone = false;
	bool bWardenToasted = false;

	void Toast(const FString& Msg) const;
	void Reward(int32 Gold, int32 XP, const TCHAR* Why);
};
