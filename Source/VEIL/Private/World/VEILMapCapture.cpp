#include "World/VEILMapCapture.h"

#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/VEILPerf.h"
#include "VEIL.h"

namespace
{
	// The phi ladder of map extents. Each step is 1/phi of the previous, so a
	// zoom never jumps a screen's worth: 24000 / 1.618 = 14833 / 9167 / 5666.
	constexpr float MapExtentMax = 24000.f;
	constexpr float MapExtentMin = 5666.f;

	// The cadence is adaptive, because a fixed one is either wasteful or janky:
	// the map starts at 2 s and backs off by phi (2 -> 3.2 -> 5.2 -> 8.4) each
	// time a capture is followed by the phase lock giving render scale away.
	// 4 s to start, and it can back off to 10. A minimap does not need to be a
	// video: the terrain only changes when the runner moves, and the blips on top
	// of it are drawn live every frame by the HUD regardless.
	constexpr float CaptureIntervalMin = 4.f;
	constexpr float CaptureIntervalMax = 10.f;
	// 384 px is drawn at 220 px on screen - more than enough, and a quarter of
	// the pixels of a 768 shot.
	constexpr int32 MapResolution = 320;
}

TWeakObjectPtr<UVEILMapCapture> UVEILMapCapture::ActiveInstance;

UVEILMapCapture* UVEILMapCapture::GetActive()
{
	return ActiveInstance.IsValid() ? ActiveInstance.Get() : nullptr;
}

void UVEILMapCapture::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	MapTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("VEILMapTarget"));
	if (!MapTarget)
	{
		return;
	}
	MapTarget->RenderTargetFormat = RTF_RGBA8;
	MapTarget->ClearColor = FLinearColor(0.02f, 0.025f, 0.04f, 1.f);
	MapTarget->bAutoGenerateMips = false;
	MapTarget->InitAutoFormat(MapResolution, MapResolution);
	MapTarget->UpdateResourceImmediate(true);

	// A scene capture actor spawned at runtime: nothing authored, nothing bought.
	const FVector Start = FVector(0.f, 0.f, 60000.f);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.ObjectFlags |= RF_Transient;
	CaptureActor = World->SpawnActor<ASceneCapture2D>(ASceneCapture2D::StaticClass(), Start, FRotator::ZeroRotator, Params);
	if (!CaptureActor)
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI MAP: scene capture could not be spawned; the map stays dark."));
		return;
	}

	USceneCaptureComponent2D* Cam = CaptureActor->GetCaptureComponent2D();
	if (!Cam)
	{
		return;
	}
	Cam->TextureTarget = MapTarget;
	// BASE COLOUR, not the lit frame. A map that goes black at night is a
	// placeholder with better manners; base colour keeps the district's own hues
	// (gold pillars, blue rock, stone ground) readable at any hour, which is how
	// a Witcher or Elden Ring map reads at a glance.
	Cam->CaptureSource = ESceneCaptureSource::SCS_BaseColor;
	Cam->ProjectionType = ECameraProjectionMode::Orthographic;
	Cam->OrthoWidth = MapExtent * 2.f;
	Cam->FOVAngle = 90.f;
	Cam->bCaptureEveryFrame = false;   // the tick below decides when
	Cam->bCaptureOnMovement = false;
	Cam->bAlwaysPersistRenderingState = true;
	// A map needs colour and shape, nothing else. Everything below is work the
	// capture would otherwise pay for and the minimap cannot show.
	Cam->ShowFlags.SetDynamicShadows(false);
	Cam->ShowFlags.SetParticles(false);
	Cam->ShowFlags.SetTranslucency(false);
	Cam->ShowFlags.SetAmbientOcclusion(false);
	Cam->ShowFlags.SetAntiAliasing(false);
	Cam->ShowFlags.SetPostProcessing(false);
	// The map is bathed in the same light as the world, so it reads as terrain
	// rather than a silhouette: ambient only, no post-process tinting.
	Cam->ShowFlags.SetAtmosphere(false);
	Cam->ShowFlags.SetFog(false);
	Cam->ShowFlags.SetBloom(false);
	Cam->ShowFlags.SetMotionBlur(false);
	Cam->ShowFlags.SetTemporalAA(false);
	CaptureActor->SetActorRotation(FRotator(-90.f, 0.f, 0.f)); // straight down, north up

	ActiveInstance = this;

	UE_LOG(LogVEIL, Log, TEXT("PHI MAP: live capture online - %d px, ortho %.0f cm, %.1f s interval."),
		MapResolution, MapExtent * 2.f, CaptureInterval);
}

void UVEILMapCapture::Deinitialize()
{
	if (CaptureActor)
	{
		CaptureActor->Destroy();
		CaptureActor = nullptr;
	}
	MapTarget = nullptr;
	Super::Deinitialize();
}

void UVEILMapCapture::Tick(float DeltaTime)
{
	if (!CaptureActor || !MapTarget)
	{
		return;
	}

	CaptureTimer += DeltaTime;
	if (CaptureTimer < CaptureInterval)
	{
		return;
	}
	CaptureTimer = 0.f;

	FVector Focus = bHasView ? ViewFocus : MapCenter;
	float Yaw = bHasView ? ViewYaw : MapYaw;
	if (!bHasView)
	if (const UWorld* World = GetWorld())
	{
		// GetPlayerPawn first: on a rebuilt world the first controller can exist a
		// beat before its pawn is possessed, and capturing at the origin put the
		// map on the plaza's own dark floor instead of on the runner.
		const APawn* P = UGameplayStatics::GetPlayerPawn(World, 0);
		const APlayerController* PC = World->GetFirstPlayerController();
		if (!P && PC) { P = PC->GetPawn(); }
		if (P)
		{
			Focus = P->GetActorLocation();
			Yaw = FMath::DegreesToRadians((float)(PC ? PC->GetControlRotation().Yaw : P->GetActorRotation().Yaw));
		}
	}

	// THE MAP YIELDS TO THE FRAME. A capture costs a scene pass, and measured it
	// was enough to pull the lock's render scale down and make it chase
	// 45 -> 53 -> 49 %. So the map watches what its own last shot cost: if the
	// lock handed back render scale afterwards, the next shot waits longer.
	// (Gating on "is there headroom" does not work under a 60 fps cap - a capped
	// frame always reads 16.67 ms, so that test blocked every capture.)
	if (const UWorld* World = GetWorld())
	{
		if (const UVEILPerf* Perf = World->GetSubsystem<UVEILPerf>())
		{
			const float ScaleNow = Perf->GetLockScale();
			if (bHasCaptured)
			{
				if (ScaleNow < ScaleAtLastCapture - 1.5f)
				{
					CaptureInterval = FMath::Min(CaptureIntervalMax, CaptureInterval * 1.618033988749895f);
					UE_LOG(LogVEIL, Log, TEXT("PHI MAP: the shot cost render scale (%.1f -> %.1f%%) - next map shot in %.1f s."),
						ScaleAtLastCapture, ScaleNow, CaptureInterval);
				}
				else if (ScaleNow >= ScaleAtLastCapture - 0.1f)
				{
					CaptureInterval = FMath::Max(CaptureIntervalMin, CaptureInterval / 1.618033988749895f);
				}
			}
			ScaleAtLastCapture = ScaleNow;
		}
	}

	// Captured on the interval, not only on movement: a live map that freezes
	// the moment the runner stands still is the static placeholder with a new
	// label - enemies walk, the light turns, the world changes under a standing
	// player.
	CaptureNow(Focus, Yaw);
	bHasCaptured = true;
}

void UVEILMapCapture::CaptureNow(const FVector& Focus, float YawRadians)
{
	if (!CaptureActor || !MapTarget)
	{
		return;
	}
	MapCenter = FVector(Focus.X, Focus.Y, 0.f);
	ActiveInstance = this;
	LastFocus = Focus;
	MapYaw = YawRadians;
	LastYaw = YawRadians;
	bHasFocus = true;

	// Sit the camera high above the runner and look straight down, yawed with the
	// runner so their facing is up in the image. Height is irrelevant under an
	// orthographic projection; it only has to clear the tallest thing around.
	CaptureActor->SetActorLocation(FVector(MapCenter.X, MapCenter.Y, 60000.f));
	CaptureActor->SetActorRotation(FRotator(-90.f, FMath::RadiansToDegrees(YawRadians), 0.f));
	if (USceneCaptureComponent2D* Cam = CaptureActor->GetCaptureComponent2D())
	{
		Cam->OrthoWidth = MapExtent * 2.f;
		Cam->CaptureScene();
	}
	++CaptureCount;

	// Every capture is logged while the map is young, then every tenth: a live
	// map that quietly stops re-shooting is a static texture with a live label.
	if (CaptureCount <= 3 || (CaptureCount % 10) == 0)
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI MAP: capture %lld at %s, yaw %.1f deg, %.0f cm across."),
			CaptureCount, *MapCenter.ToCompactString(), FMath::RadiansToDegrees(MapYaw), MapExtent * 2.f);
	}
}

void UVEILMapCapture::ZoomIn()
{
	MapExtent = FMath::Max(MapExtentMin, MapExtent / 1.618033988749895f);
	CaptureNow(bHasFocus ? LastFocus : MapCenter, MapYaw);
	UE_LOG(LogVEIL, Log, TEXT("PHI MAP: zoom in - %.0f cm across."), MapExtent * 2.f);
}

void UVEILMapCapture::ZoomOut()
{
	MapExtent = FMath::Min(MapExtentMax, MapExtent * 1.618033988749895f);
	CaptureNow(bHasFocus ? LastFocus : MapCenter, MapYaw);
	UE_LOG(LogVEIL, Log, TEXT("PHI MAP: zoom out - %.0f cm across."), MapExtent * 2.f);
}

TStatId UVEILMapCapture::GetStatId() const
{
	return TStatId();
}
