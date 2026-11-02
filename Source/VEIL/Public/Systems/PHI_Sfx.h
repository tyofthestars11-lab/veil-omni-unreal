// PHI central sound board: one call plays any baked wave or pack cue.
// Sounds are cached on first use. Nothing here allocates at play time.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PHI_Sfx.generated.h"

class USoundBase;

UCLASS()
class VEIL_API UPHI_Sfx : public UObject
{
	GENERATED_BODY()

public:
	/** Positional one-shot. No-op when the sound is missing (says so once). */
	static void PlayAt(UWorld* World, const TCHAR* Path, const FVector& Loc, float Volume = 1.f);
	/** UI 2D one-shot. */
	static void PlayUI(UWorld* World, const TCHAR* Path, float Volume = 0.7f);
	/** Looped 2D bed (wind). Returns the component so the weather can ride it. */
	static class UAudioComponent* PlayLoop(UWorld* World, const TCHAR* Path, float Volume = 0.2f);

	// Named boards, so call sites read like the design doc.
	static void Bolt(UWorld* W, const FVector& L) { PlayAt(W, TEXT("/Game/SFX/bolt_fire"), L, 0.8f); }
	static void Hit(UWorld* W, const FVector& L) { PlayAt(W, TEXT("/Game/SFX/hit"), L, 0.9f); }
	static void FoeDown(UWorld* W, const FVector& L) { PlayAt(W, TEXT("/Game/SFX/foe_down"), L, 0.9f); }
	static void Chime(UWorld* W, const FVector& L) { PlayAt(W, TEXT("/Game/SFX/fragment_chime"), L, 0.8f); }
	static void CacheClaim(UWorld* W, const FVector& L) { PlayAt(W, TEXT("/Game/SFX/cache_claim"), L, 0.9f); }
	static void Roar(UWorld* W, const FVector& L) { PlayAt(W, TEXT("/Game/SFX/warden_roar"), L, 1.f); }
	static void Fanfare(UWorld* W) { PlayUI(W, TEXT("/Game/SFX/win_fanfare"), 0.9f); }
	static void Click(UWorld* W) { PlayUI(W, TEXT("/Game/SFX/ui_click"), 0.6f); }
	static void Step(UWorld* W, const FVector& L, bool bAlt)
	{
		PlayAt(W, bAlt ? TEXT("/Game/SFX/footstep_b") : TEXT("/Game/SFX/footstep_a"), L, 0.45f);
	}
	static void Thunder(UWorld* W) { PlayAt(W, TEXT("/Game/SFX/thunder"), FVector(0.f, 0.f, 2000.f), 0.7f); }
	static void Explode(UWorld* W, const FVector& L)
	{
		PlayAt(W, TEXT("/Game/StarterContent/Audio/Explosion_Cue.Explosion_Cue"), L, 0.9f);
	}

	// The Runner's own voice: Paragon effort cues.
	static void Effort(UWorld* W, const TCHAR* Cue, const FVector& L, float Volume = 0.8f)
	{
		PlayAt(W, Cue, L, Volume);
	}
	static void EffortJump(UWorld* W, const FVector& L)
	{
		PlayAt(W, TEXT("/Game/SFX/jump_push"), L, 0.7f);
		Effort(W, TEXT("/Game/ParagonCountess/Characters/Heroes/Countess/Sounds/SoundCues/Countess_Effort_Jump.Countess_Effort_Jump"), L, 0.6f);
	}
	static void EffortLand(UWorld* W, const FVector& L)
	{
		PlayAt(W, TEXT("/Game/SFX/land_heavy"), L, 0.9f);
		Effort(W, TEXT("/Game/ParagonCountess/Characters/Heroes/Countess/Sounds/SoundCues/Countess_Effort_Land.Countess_Effort_Land"), L, 0.6f);
	}
	static void EffortPain(UWorld* W, const FVector& L)
	{
		Effort(W, TEXT("/Game/ParagonCountess/Characters/Heroes/Countess/Sounds/SoundCues/Countess_Effort_Pain.Countess_Effort_Pain"), L);
	}
	static void EffortDeath(UWorld* W, const FVector& L)
	{
		Effort(W, TEXT("/Game/ParagonCountess/Characters/Heroes/Countess/Sounds/SoundCues/Countess_Effort_Death.Countess_Effort_Death"), L);
	}
	static void EffortAttack(UWorld* W, const FVector& L)
	{
		Effort(W, TEXT("/Game/ParagonCountess/Characters/Heroes/Countess/Sounds/SoundCues/Countess_Effort_Attack.Countess_Effort_Attack"), L, 0.6f);
	}

private:
	static TMap<FString, TObjectPtr<USoundBase>> SoundCache;
	static TSet<FString> MissLogged;
	static USoundBase* Find(UWorld* World, const FString& Path);
};
