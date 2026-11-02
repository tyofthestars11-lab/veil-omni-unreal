// THE LIVING MAP.
//
// The minimap used to be dots on a black square - a placeholder, not a map.
// This captures the real district: an orthographic camera looking straight
// down, rendering the game's own colours (gold pillars, blue rock, the stone
// ground) into a render target the HUD draws. So the map cannot disagree with
// the world, because it IS the world, re-photographed a few times a second.
//
// Cost is held down deliberately: 512 px, captured twice a second and only
// when the player has moved, so it can never be the reason the 60 fps phase
// lock slips. The camera is a plain ASceneCapture2D spawned at runtime - no
// asset, no plugin, nothing to buy.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VEILMapCapture.generated.h"

class ASceneCapture2D;
class UTextureRenderTarget2D;

UCLASS()
class VEIL_API UVEILMapCapture : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/**
	 * The instance that actually owns the live capture this session. A -game
	 * session can hold more than one world subsystem (the boot world and the
	 * play world), and the HUD was reading the copy that never photographed
	 * anything while the other one shot the frames. One registry fixes that: the
	 * instance that captures announces itself, and the HUD asks for it by name.
	 */
	static UVEILMapCapture* GetActive();

	/** The live top-down image of the world. Null until the first capture. */
	UTextureRenderTarget2D* GetMapTexture() const { return MapTarget; }

	/** Centre of the captured square, in world units. */
	FVector GetMapCenter() const { return MapCenter; }

	/** Half-width of the captured square, in world units - the icon scale too. */
	float GetMapExtent() const { return MapExtent; }

	/**
	 * The yaw the capture was taken at, in radians. The camera turns with the
	 * runner, so the runner's facing is always UP in the image and the compass
	 * ring turns around it - the map reads the way a GTA or Witcher minimap
	 * reads, not like a north-locked diagram you have to translate in your head.
	 * Icons use this same yaw, so a blip cannot lie about where a thing is.
	 */
	float GetMapYaw() const { return MapYaw; }

	/** How many captures have actually landed. Proof the map is live. */
	int64 GetCaptureCount() const { return CaptureCount; }

	/**
	 * The HUD tells the capture where it is looking, every frame it draws the
	 * map. The subsystem used to hunt for a pawn and could end up in a world
	 * whose controller had not possessed one yet, so it photographed the origin
	 * and the map showed the plaza's own dark floor instead of the runner's
	 * ground. Being handed the view means the picture and the frame are the same
	 * place by construction.
	 */
	void SetViewFocus(const FVector& Location, float YawRadians)
	{
		ViewFocus = Location;
		ViewYaw = YawRadians;
		bHasView = true;
	}

	/** 6000 cm (close) .. 24000 cm (whole region). Steps by 1/phi, not by taste. */
	void ZoomIn();
	void ZoomOut();

private:
	void CaptureNow(const FVector& Focus, float YawRadians);

	UPROPERTY() TObjectPtr<UTextureRenderTarget2D> MapTarget = nullptr;
	UPROPERTY() TObjectPtr<ASceneCapture2D> CaptureActor = nullptr;

	FVector MapCenter = FVector::ZeroVector;
	/** Seconds between shots: adaptive, so the map never costs the frame its scale. */
	float CaptureInterval = 4.f;
	float ScaleAtLastCapture = 100.f;
	bool bHasCaptured = false;
	/**
	 * 6000 cm = 60 m across the panel: local ground, close enough that terrain
	 * fills it. The first cut opened at 240 m, which is mostly sky and reads as an
	 * empty rectangle rather than a map.
	 */
	float MapExtent = 6000.f;
	float CaptureTimer = 0.f;
	int64 CaptureCount = 0;
	/** Only re-shoot when the player has actually gone somewhere. */
	FVector LastFocus = FVector::ZeroVector;
	float MapYaw = 0.f;
	float LastYaw = 0.f;
	bool bHasFocus = false;
	FVector ViewFocus = FVector::ZeroVector;
	float ViewYaw = 0.f;
	bool bHasView = false;
	static TWeakObjectPtr<UVEILMapCapture> ActiveInstance;
};
