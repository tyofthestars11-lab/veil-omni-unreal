#include "World/VEILSky.h"

#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "VEIL.h"

void VEILSky::Dress(UWorld* World)
{
	if (!World) { return; }

	// Gold, not white: the reference frames are lit by a low sun, and every stone surface in
	// this project is chosen to read warm.
	const FLinearColor SunColor(1.0f, 0.80f, 0.52f);
	const FRotator SunAngle(-16.f, 38.f, 0.f);
	const FLinearColor FogColor(0.62f, 0.48f, 0.30f);
	const FLinearColor SkyTint(0.62f, 0.64f, 0.78f);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	int32 Spawned = 0;
	int32 Retuned = 0;

	// 1. THE SUN. Reuse the map's own directional light when it has one - two suns is a bug
	// that looks like a lighting taste.
	ADirectionalLight* Sun = nullptr;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It) { Sun = *It; break; }
	const bool bSpawnedSun = (Sun == nullptr);
	if (!Sun)
	{
		Sun = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector(0.f, 0.f, 12000.f), SunAngle, Params);
		++Spawned;
	}
	else { ++Retuned; }

	if (Sun)
	{
		if (bSpawnedSun) { Sun->SetActorRotation(SunAngle); }
		if (UDirectionalLightComponent* Light = Cast<UDirectionalLightComponent>(Sun->GetLightComponent()))
		{
			if (bSpawnedSun) { Light->SetMobility(EComponentMobility::Movable); }
			Light->SetIntensity(7.0f);
			Light->SetLightColor(SunColor);
			Light->SetCastShadows(true);
		}
	}

	// 2. THE SKY LIGHT. Captured from the scene so the warm sun lands on stone from above
	// instead of leaving every shadow face black.
	ASkyLight* Sky = nullptr;
	for (TActorIterator<ASkyLight> It(World); It; ++It) { Sky = *It; break; }
	const bool bSpawnedSky = (Sky == nullptr);
	if (!Sky)
	{
		Sky = World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FVector(0.f, 0.f, 8000.f), FRotator::ZeroRotator, Params);
		++Spawned;
	}
	else { ++Retuned; }

	if (Sky)
	{
		if (USkyLightComponent* Light = Sky->GetLightComponent())
		{
			if (bSpawnedSky) { Light->SetMobility(EComponentMobility::Movable); }
			Light->SourceType = ESkyLightSourceType::SLS_CapturedScene;
			Light->SetIntensity(1.6f);
			Light->SetLightColor(SkyTint);
			Light->RecaptureSky();
		}
	}

	// 3. THE AIR. Fog is what makes distance read as distance; without it the crystal range
	// sits on the terraces like wallpaper.
	AExponentialHeightFog* Fog = nullptr;
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It) { Fog = *It; break; }
	if (!Fog)
	{
		Fog = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FVector(0.f, 0.f, 0.f), FRotator::ZeroRotator, Params);
		++Spawned;
	}
	else { ++Retuned; }

	if (Fog && Fog->GetComponent())
	{
		UExponentialHeightFogComponent* Air = Fog->GetComponent();
		Air->SetFogDensity(0.018f);
		Air->SetFogHeightFalloff(0.18f);
		Air->SetFogInscatteringColor(FogColor);
		Air->SetStartDistance(150.f);
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI SKY: sun %s at %.0f lux (gold %.2f,%.2f,%.2f), sky light %s, fog %s - %d spawned, %d retuned, no second sun."),
		Sun ? TEXT("stands") : TEXT("missing"),
		7.0f, SunColor.R, SunColor.G, SunColor.B,
		Sky ? TEXT("captured") : TEXT("missing"),
		Fog ? TEXT("set") : TEXT("missing"),
		Spawned, Retuned);
}
