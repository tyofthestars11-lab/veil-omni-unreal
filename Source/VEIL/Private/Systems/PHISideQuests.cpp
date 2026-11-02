// PHI side quests implementation.
#include "Systems/PHISideQuests.h"

#include "Engine/Engine.h"
#include "Systems/OMNIQuestSystem.h"
#include "VEIL.h"

void UPHISideQuests::Toast(const FString& Msg) const
{
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Cyan, Msg); }
	UE_LOG(LogVEIL, Log, TEXT("PHI SIDE: %s"), *Msg);
}

void UPHISideQuests::Reward(int32 Gold, int32 XP, const TCHAR* Why)
{
	if (UWorld* W = GetWorld())
	{
		if (UOMNIQuestSystem* Q = W->GetSubsystem<UOMNIQuestSystem>())
		{
			Q->Gold += Gold;
			Q->AwardXP(XP, Why); // side XP climbs the ladder (level-ups live in AwardXP)
			if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Yellow, FString::Printf(TEXT("SIDE DONE %s: +%dG +%dXP"), Why, Gold, XP)); }
		}
	}
}

void UPHISideQuests::AddEcho()
{
	++EchoCount;
	if (!bEchoesDone && EchoCount >= 8)
	{
		bEchoesDone = true;
		Toast(TEXT("ECHOES: the field goes quiet. 8 down."));
		Reward(150, 150, TEXT("ECHOES"));
	}
}

void UPHISideQuests::TouchBeacon(FName Site)
{
	if (BeaconSites.Contains(Site)) return; // idempotent: each site pays once
	BeaconSites.Add(Site);
	int32 N = BeaconSites.Num();
	Toast(FString::Printf(TEXT("BEACON %d/3: %s hums."), N, *Site.ToString()));
	if (!bBeaconsDone && N >= 3)
	{
		bBeaconsDone = true;
		Toast(TEXT("KEPT SIGNALS: all three beacons burn."));
		Reward(200, 200, TEXT("KEPT-SIGNALS"));
	}
}

void UPHISideQuests::OpenCache()
{
	if (bCacheDone) return;
	bCacheDone = true;
	Toast(TEXT("CACHE SWEEP: every shelf read."));
	Reward(120, 120, TEXT("CACHE-SWEEP"));
}

void UPHISideQuests::WardenDown()
{
	if (bWardenToasted) return;
	bWardenToasted = true;
	Toast(TEXT("LAST SIGNAL: the Warden is down. The path holds."));
}

