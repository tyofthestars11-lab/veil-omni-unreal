// PHI dynamic weather implementation.
#include "Systems/PHIWeather.h"

#include "Components/AudioComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LineBatchComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/SkyLight.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/PHI_Sfx.h"
#include "VEIL.h"
#include "Systems/VEILPhi.h"

static UExponentialHeightFogComponent* PhiFogComp(AExponentialHeightFog* Fog)
{
	return Fog ? Fog->FindComponentByClass<UExponentialHeightFogComponent>() : nullptr;
}

APhiRainStreaks::APhiRainStreaks()
{
	PrimaryActorTick.bCanEverTick = false;
	Lines = CreateDefaultSubobject<ULineBatchComponent>(TEXT("RainLines"));
	SetRootComponent(Lines);
}

static uint32 PhiHash(uint32 x)
{
	x ^= x >> 16;
	x *= 0x7feb352d;
	x ^= x >> 15;
	x *= 0x846ca68b;
	x ^= x >> 16;
	return x;
}

void APhiRainStreaks::DrawStreaks(const FVector& Center, int32 Count, float Slant, float Length)
{
	if (!Lines) return;
	const float R = 2600.f;
	const float Top = 1600.f;
	const float Fall = 4200.f;
	const float Now = GetWorld()->GetTimeSeconds();
	for (int32 i = 0; i < Count; ++i)
	{
		const uint32 h1 = PhiHash((uint32)i * 2u + 1u);
		const uint32 h2 = PhiHash((uint32)i * 2u + 7919u);
		const float Ang = (h1 % 628u) / 100.f;
		const float Rad = R * (0.15f + 0.85f * ((h2 % 1000u) / 1000.f));
		float Z = Top - FMath::Fmod(Now * Fall + (h1 % 2000u), Top + 400.f);
		const FVector P(Center.X + Rad * FMath::Cos(Ang), Center.Y + Rad * FMath::Sin(Ang), Center.Z + Z);
		const FVector Q = P + FVector(Slant * 160.f, 0.f, -Length);
		Lines->DrawLine(P, Q, FLinearColor(0.55f, 0.7f, 0.9f, 0.5f), 0, 1.5f, 0.05f);
	}
}

void UPHIWeather::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogVEIL, Log, TEXT("PHI WEATHER ONLINE: the sky keeps its own time."));
}

void UPHIWeather::FindStage()
{
	UWorld* W = GetWorld();
	if (!W) return;
	if (!Sun)
	{
		for (TActorIterator<ADirectionalLight> It(W); It; ++It) { Sun = *It; break; }
		if (Sun && Sun->GetLightComponent()) { SunBase = Sun->GetLightComponent()->Intensity; }
	}
	if (!Sky)
	{
		for (TActorIterator<ASkyLight> It(W); It; ++It) { Sky = *It; break; }
	}
	if (!Fog)
	{
		for (TActorIterator<AExponentialHeightFog> It(W); It; ++It)
		{
			Fog = *It;
			if (UExponentialHeightFogComponent* Fc = PhiFogComp(Fog)) { FogBase = Fc->FogDensity; }
			break;
		}
	}
}

float UPHIWeather::TargetDim() const
{
	switch (Target)
	{
	case EPHIWeather::Clear: return 1.f;
	case EPHIWeather::Overcast: return 0.88f;
	case EPHIWeather::Rain: return 0.78f;
	default: return 0.68f; // PHI LAW: weather moods, never gloom - the real world stays lit
	}
}

float UPHIWeather::TargetFog() const
{
	switch (Target)
	{
	case EPHIWeather::Clear: return 1.f;
	case EPHIWeather::Overcast: return 1.2f;
	case EPHIWeather::Rain: return 1.35f;
	default: return 1.5f; // PHI LAW: fog breathes, never soup
	}
}

void UPHIWeather::AdvanceHours(float Hours)
{
	if (!FMath::IsFinite(Hours) || FMath::IsNearlyZero(Hours)) { return; }
	// Wrap through the day instead of clamping: the clock is a circle, and the
	// sky it drives is already reading Hour every tick, so nothing else changes.
	Hour = FMath::Fmod(Hour + Hours + 24.f, 24.f);
	bWasNight = IsNight();
	LastHourLogged = -1;   // make the next tick log the hour we just set
	UE_LOG(LogVEIL, Log, TEXT("PHI SKY: clock moved %+.0f h -> %02d:%02d, daylight %.2f."),
		Hours, FMath::FloorToInt(Hour), FMath::RoundToInt((Hour - FMath::FloorToFloat(Hour)) * 60.f), DayFactor());
}

void UPHIWeather::ForceWeather(EPHIWeather W)
{
	// Setting Target alone would wait for the next roll; a hand on the sky sets
	// both ends, so the mood is what the runner asked for on this frame.
	Target = W;
	Shown = W;
	Blend = 1.f;
	StateTime = 0.f;
	UE_LOG(LogVEIL, Log, TEXT("PHI WEATHER: %s by hand (was waiting on intent %.3f)."),
		W == EPHIWeather::Clear ? TEXT("clear light")
		: W == EPHIWeather::Overcast ? TEXT("overcast")
		: W == EPHIWeather::Rain ? TEXT("rain") : TEXT("storm"), Turbulence);
}

float UPHIWeather::DayFactor() const
{
	// Elevation runs -1 at midnight, 0 at 6 and 18, +1 at noon.
	const float Elev = FMath::Sin((Hour - 6.f) / 24.f * 2.f * PI);
	// NEVER A BLACK HOLE: the old 0.06 floor is what made night frames illegible. 0.30 keeps
	// an unlit plaza readable and the shadows blue instead of void.
	// MEASURED, NOT GUESSED: at a 0.30 floor the dawn frames came back at mean 46 with
	// 46.6% of the picture near-black and 66% of the top band black. A dawn is dimmer
	// than noon, but 1/phi of the noon sky still reads as air, not as a void - and it is
	// the golden section, so the darkest hour is the same law as every other scale here.
	return FMath::Clamp(Elev * 1.6f + 0.30f, (float)VeilPhi::InvPhi, 1.f);
}

void UPHIWeather::TickWorld(float DeltaSeconds)
{
	UWorld* W = GetWorld();
	if (!W) return;
	FindStage();

	// DAY / NIGHT: one real minute is one world hour.
	// ONE WORLD HOUR PER TEN REAL MINUTES, not per minute. At the old rate the golden window
	// lasted about ninety seconds of play; a cycle nobody can stand inside is not a day cycle.
	Hour = FMath::Fmod(Hour + DeltaSeconds / 600.f, 24.f);
	if (bWasNight != IsNight())
	{
		bWasNight = IsNight();
		UE_LOG(LogVEIL, Log, TEXT("PHI SKY: %s at hour %.2f (daylight %.2f)."),
			bWasNight ? TEXT("night falls") : TEXT("dawn breaks"), Hour, DayFactor());
	}
	const int32 WholeHour = FMath::FloorToInt(Hour);
	if (Sun)
	{
		const float Elev = FMath::Sin((Hour - 6.f) / 24.f * 2.f * PI);
		// THE SKY IN THE OWNER'S ART IS NEVER BLACK. Physically the sun sets, and the
		// atmosphere then renders true black - measured as an entire black half-frame at
		// hour 12. The clock still runs its full day, but the key light is held at ten
		// degrees of elevation, so the worst hour is a navy dusk instead of a void.
		const float SunPitch = FMath::Min(-Elev * 90.f, -10.f);
		Sun->SetActorRotation(FRotator(SunPitch, VeilPhi::GoldenAngleF, 0.f));
		if (WholeHour != LastHourLogged)
		{
			LastHourLogged = WholeHour;
			UE_LOG(LogVEIL, Log, TEXT("PHI SKY: %02d:00, daylight %.2f, sun pitch %.1f (raw %.1f)."),
				WholeHour, DayFactor(), SunPitch, -Elev * 90.f);
		}
	}
	if (Sky && Sky->GetLightComponent())
	{
		if (SkyBase <= 0.f) { SkyBase = Sky->GetLightComponent()->Intensity; }
		// THE AIR IS NEVER A CELLAR. SkyBase was captured before the golden-hour stamp
		// landed (weather came online at :49, the stamp at :51), so it held 3.0 and dawn
		// was lit by 0.9 lux. The floor is phi^4 of sky - the same weight the light pass
		// stamps - so this per-tick write can no longer undo the golden hour.
		SkyBase = FMath::Max(SkyBase, (float)(VeilPhi::Phi2 * VeilPhi::Phi2));
		Sky->GetLightComponent()->SetIntensity(SkyBase * DayFactor());
	}

	StateTime += DeltaSeconds;
	const float Hold = (Target == EPHIWeather::Clear) ? 150.f
		: (Target == EPHIWeather::Overcast) ? 90.f
		: (Target == EPHIWeather::Rain) ? 120.f : 60.f;
	if (StateTime >= Hold)
	{
		StateTime = 0.f;
		// The sky answers the world's intent instead of a fixed rotation: chaos
		// above the golden mean feeds the heavier weather, calm clears it.
		const int32 Step = Turbulence > 0.618f ? 1 : (Turbulence < 0.382f ? -1 : 0);
		Target = Step == 0
			? (EPHIWeather)(((int32)Target + 1) % 4)
			: (EPHIWeather)FMath::Clamp((int32)Target + Step, 0, 3);
		UE_LOG(LogVEIL, Log, TEXT("PHI WEATHER: %s rolls in (intent %.3f)."),
			Target == EPHIWeather::Clear ? TEXT("clear light")
			: Target == EPHIWeather::Overcast ? TEXT("overcast")
			: Target == EPHIWeather::Rain ? TEXT("rain") : TEXT("storm"), Turbulence);
	}
	Blend = FMath::Min(1.f, Blend + DeltaSeconds / 12.f);
	if (Shown != Target && StateTime <= DeltaSeconds * 1.5f) { Blend = 0.f; Shown = Target; }

	// THE SUN'S INTENSITY IS NOT THE WEATHER'S TO WRITE. This block multiplied the sun by the
	// weather mood and the clock's daylight every tick, so two runs of the same build measured
	// mean 123 and mean 21 - the light was a race, not an author. The weather now turns the sun
	// and nothing else; AVEILGameMode sets its intensity once.
	if (Sun && Sun->GetLightComponent() && FMath::IsNearlyZero(Sun->GetLightComponent()->Intensity))
	{
		Sun->GetLightComponent()->SetIntensity(FMath::Max(SunBase, 1600.f));
	}
	if (UExponentialHeightFogComponent* Fc = PhiFogComp(Fog.Get()))
	{
		// THE AIR IS CAPPED, BECAUSE THE ART HAS NO HAZE. The owner's paintings are flat
		// colour with a crisp horizon; the level's own fog density multiplied by a storm
		// mood is what pulled every palette colour toward the same grey - a frame that
		// measured mean (124,121,111), near-neutral, from a palette of gold, cream and
		// navy. Weather still breathes through the cap; it can no longer erase the palette.
		const float Want = FMath::Min(FogBase * TargetFog(), 0.02f);
		Fc->FogDensity = FMath::Lerp(Fc->FogDensity, Want, FMath::Min(1.f, DeltaSeconds * 0.4f));
	}

	if ((Shown == EPHIWeather::Rain || Shown == EPHIWeather::Storm) && Blend > 0.25f)
	{
		if (!Rain)
		{
			Rain = W->SpawnActor<APhiRainStreaks>(APhiRainStreaks::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		}
		if (Rain)
		{
			FVector Center = FVector::ZeroVector;
			if (APawn* P = UGameplayStatics::GetPlayerPawn(W, 0)) { Center = P->GetActorLocation(); }
			const int32 Count = (Shown == EPHIWeather::Storm ? 420 : 260);
			Rain->DrawStreaks(Center, (int32)(Count * Blend), Shown == EPHIWeather::Storm ? 0.5f : 0.25f, 220.f);
		}
		if (!WindBed || !WindBed->IsPlaying())
		{
			WindBed = UPHI_Sfx::PlayLoop(W, TEXT("/Game/SFX/wind_loop"), Shown == EPHIWeather::Storm ? 0.5f : 0.3f);
		}
	}

	if (Shown == EPHIWeather::Storm)
	{
		StrikeTimer -= DeltaSeconds;
		if (StrikeTimer <= 0.f)
		{
			StrikeTimer = 6.f + (PhiHash((uint32)(W->GetTimeSeconds() * 10.f)) % 90u) / 10.f;
			Flash = 0.22f;
			UPHI_Sfx::Thunder(W);
			UE_LOG(LogVEIL, Log, TEXT("PHI WEATHER: lightning answers."));
		}
	}
}

