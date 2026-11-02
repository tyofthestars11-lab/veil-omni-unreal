// PHI central sound board implementation.
#include "Systems/PHI_Sfx.h"

#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include <excpt.h>
#include "Systems/PHIVoice.h"
#include "VEIL.h"

TMap<FString, TObjectPtr<USoundBase>> UPHI_Sfx::SoundCache;
TSet<FString> UPHI_Sfx::MissLogged;

USoundBase* UPHI_Sfx::Find(UWorld* World, const FString& Path)
{
	if (!World) return nullptr;
	if (TObjectPtr<USoundBase>* Hit = SoundCache.Find(Path))
	{
		if (IsValid(Hit->Get())) { return Hit->Get(); }
		SoundCache.Remove(Path);
	}
	USoundBase* S = LoadObject<USoundBase>(nullptr, *Path);
	if (!IsValid(S))
	{
		if (!MissLogged.Contains(Path))
		{
			MissLogged.Add(Path);
			UE_LOG(LogVEIL, Warning, TEXT("PHI SFX MISS: %s"), *Path);
		}
		return nullptr;
	}
	S->AddToRoot();
	SoundCache.Add(Path, S);
	return S;
}

// PHI: the story voice stands alone - while a narration line plays, the
// world ducks to a murmur instead of talking over it.
static float PhiDuck(UWorld* World, float Volume)
{
	if (World && FPHIVoice::IsBusy(World)) { return Volume * 0.35f; }
	return Volume;
}

namespace
{
bool PhiGuardedPlayAt(UWorld* W, USoundBase* S, FVector Loc, float Vol)
{
	__try { UGameplayStatics::PlaySoundAtLocation(W, S, Loc, Vol); }
	__except (EXCEPTION_EXECUTE_HANDLER) { return false; }
	return true;
}
bool PhiGuardedPlayUI(UWorld* W, USoundBase* S, float Vol)
{
	__try { UGameplayStatics::PlaySound2D(W, S, Vol); }
	__except (EXCEPTION_EXECUTE_HANDLER) { return false; }
	return true;
}
UAudioComponent* PhiGuardedLoop(UWorld* W, USoundBase* S, float Vol)
{
	UAudioComponent* Out = nullptr;
	__try { Out = UGameplayStatics::SpawnSound2D(W, S, Vol); }
	__except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
	return Out;
}
}

void UPHI_Sfx::PlayAt(UWorld* World, const TCHAR* Path, const FVector& Loc, float Volume)
{
	if (!World || !GEngine || !GEngine->GetMainAudioDevice() || !World->GetAudioDevice()) return;
	USoundBase* S = Find(World, Path);
	if (S == nullptr) return;
	if (USoundWave* Wv = Cast<USoundWave>(S))
	{
		if (Wv->GetDuration() <= 0.f || Wv->RawPCMDataSize == 0) return;
	}
	if (!PhiGuardedPlayAt(World, S, Loc, PhiDuck(World, Volume)))
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI SFX SURVIVED: location play refused, the loop walks on."));
	}
}

void UPHI_Sfx::PlayUI(UWorld* World, const TCHAR* Path, float Volume)
{
	if (!World || !GEngine || !GEngine->GetMainAudioDevice() || !World->GetAudioDevice()) return;
	if (USoundBase* S = Find(World, Path))
	{
		if (!PhiGuardedPlayUI(World, S, PhiDuck(World, Volume)))
		{
			UE_LOG(LogVEIL, Warning, TEXT("PHI SFX SURVIVED: ui play refused, the loop walks on."));
		}
	}
}

UAudioComponent* UPHI_Sfx::PlayLoop(UWorld* World, const TCHAR* Path, float Volume)
{
	USoundBase* S = Find(World, Path);
	if (!World || !IsValid(S) || !GEngine || !GEngine->GetMainAudioDevice() || !World->GetAudioDevice()) return nullptr;
	UAudioComponent* Comp = PhiGuardedLoop(World, S, Volume);
	if (Comp)
	{
		Comp->bIsUISound = true;
	}
	return Comp;
}

