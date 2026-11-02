// PHI self update implementation: audit at boot, rescue live.
#include "Systems/PHISelfUpdate.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundWave.h"
#include "VEIL.h"

int32 UPHISelfUpdate::AuditAsset(const TCHAR* Path) const
{
	UObject* O = StaticLoadObject(UObject::StaticClass(), nullptr, Path);
	if (!O)
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI SELFUPDATE MISS: %s"), Path);
		return 0;
	}
	return 1;
}

void UPHISelfUpdate::BootAudit()
{
	if (bAudited) return;
	bAudited = true;
	UWorld* W = GetWorld();
	if (!W) return;

	int32 Voices = 0;
	for (int32 i = 1; i <= 19; ++i)
	{
		Voices += AuditAsset(*FString::Printf(TEXT("/Game/Voice/v%02d"), i));
	}
	for (int32 i = 1; i <= 18; ++i)
	{
		Voices += AuditAsset(*FString::Printf(TEXT("/Game/Voice/o%02d"), i));
	}
	const TCHAR* Sfx[] = { TEXT("/Game/SFX/bolt_fire"), TEXT("/Game/SFX/hit"),
		TEXT("/Game/SFX/foe_down"), TEXT("/Game/SFX/fragment_chime"), TEXT("/Game/SFX/cache_claim"),
		TEXT("/Game/SFX/warden_roar"), TEXT("/Game/SFX/win_fanfare"), TEXT("/Game/SFX/ui_click"),
		TEXT("/Game/SFX/jump_whoosh"), TEXT("/Game/SFX/land_thud"), TEXT("/Game/SFX/footstep_a"),
		TEXT("/Game/SFX/footstep_b"), TEXT("/Game/SFX/thunder"), TEXT("/Game/SFX/wind_loop") };
	int32 Effects = 0;
	for (const TCHAR* S : Sfx) { Effects += AuditAsset(S); }

	int32 Starts = 0, Vaults = 0, Runners = 0;
	for (TActorIterator<APlayerStart> It(W); It; ++It) { ++Starts; }
	for (TActorIterator<AActor> It(W); It; ++It)
	{
		const FString Cls = It->GetClass()->GetName();
		if (Cls.Contains(TEXT("VEILVault"))) { ++Vaults; }
		if (Cls.Contains(TEXT("VEILCharacter"))) { ++Runners; }
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI SELFUPDATE AUDIT: voices %d/37, effects %d/14, starts %d, vaults %d, runners %d."),
		Voices, Effects, Starts, Vaults, Runners);
	if (Voices < 37 || Effects < 14 || Starts < 1 || Vaults < 1 || Runners < 1)
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI SELFUPDATE: faults found above - the game plays on with what stands."));
	}
	else
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI SELFUPDATE: whole - every board seated."));
	}
}

void UPHISelfUpdate::TickWorld(float DeltaSeconds)
{
	UWorld* W = GetWorld();
	if (!W) return;
	RescueTimer += DeltaSeconds;
	if (RescueTimer < 2.f) return;
	RescueTimer = 0.f;
	APawn* P = UGameplayStatics::GetPlayerPawn(W, 0);
	if (!P) return;
	const FVector L = P->GetActorLocation();
	// PHI: catch the fall above KillZ, alive - never let death take him first.
	if (L.Z < -800.f)
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI SELFUPDATE RESCUE: the Runner fell at %s and was walked home alive."), *L.ToString());
		P->SetActorLocation(FVector(0.f, 0.f, 150.f), false, nullptr, ETeleportType::TeleportPhysics);
	}
}

