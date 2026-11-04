// PHI dynamic weather: Clear -> Overcast -> Rain -> Storm, blending on
// phi-timed intervals. Drives the sun, sky and fog already in the level,
// draws rain as streaks around the Runner (no asset needed), answers
// lightning with light and thunder. No pause, no freeze, ever.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PHIWeather.generated.h"

class ULineBatchComponent;

UENUM(BlueprintType, meta = (ScriptName = "EPHIWeather"))
enum class EPHIWeather : uint8
{
	Clear,
	Overcast,
	Rain,
	Storm
};

/** Rain streak renderer: a plain actor carrying a line batcher. */
UCLASS()
class VEIL_API APhiRainStreaks : public AActor
{
	GENERATED_BODY()

public:
	APhiRainStreaks();

	/** Draws Count streaks falling around Center. Slant leans with wind. */
	void DrawStreaks(const FVector& Center, int32 Count, float Slant, float Length);

	UPROPERTY()
	TObjectPtr<ULineBatchComponent> Lines;
};

UCLASS()
class VEIL_API UPHIWeather : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Driven from the GameMode tick (same road as the voice pump). */
	void TickWorld(float DeltaSeconds);

	UFUNCTION(BlueprintPure, Category = "VEIL|Weather")
	EPHIWeather GetWeather() const { return Shown; }

	/** Day / night. One real minute is one world hour, so a whole cycle is 24
	 *  minutes of play: the sun's pitch follows the hour and the weather dims
	 *  on top of the daylight instead of replacing it. */
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Sky")
	// GOLDEN HOUR, NOT WHATEVER THE CLOCK DRIFTED TO. The cycle used to open at 08:30 and run
	// a world hour every real minute, so a session was night inside five minutes and every
	// frame that survived to be measured was a blue one. 17.27 h is the hour whose sun pitch
	// is the vision's own 17.0324 deg down - the light every reference painting is lit by.
	float Hour = 16.5f;

	/** The world's own intent, folded from measured chaos by UPHIIntent. Above
	 *  0.618 the sky leans toward storm, below 0.382 it clears. */
	void SetTurbulence(float T) { Turbulence = FMath::Clamp(T, 0.f, 1.f); }
	float GetTurbulence() const { return Turbulence; }

	/** Menu hook: move the sky's own clock. Wraps at 24 hours and re-lights. */
	void AdvanceHours(float Hours);

	/** Menu hook: hand the sky a mood now, instead of waiting for its own roll. */
	void ForceWeather(EPHIWeather W);

	UFUNCTION(BlueprintPure, Category = "VEIL|Sky")
	float DayFactor() const;

	UFUNCTION(BlueprintPure, Category = "VEIL|Sky")
	bool IsNight() const { return DayFactor() < 0.35f; }

private:
	EPHIWeather Target = EPHIWeather::Clear;
	EPHIWeather Shown = EPHIWeather::Clear;
	float StateTime = 0.f;
	float Blend = 1.f;
	float StrikeTimer = 8.f;
	float Flash = 0.f;
	/** Measured, not scripted: what the world itself is leaning toward. */
	float Turbulence = 0.618f;
	TObjectPtr<class ADirectionalLight> Sun;
	TObjectPtr<class ASkyLight> Sky;
	TObjectPtr<class AExponentialHeightFog> Fog;
	TObjectPtr<APhiRainStreaks> Rain;
	TObjectPtr<class UAudioComponent> WindBed;
	float SunBase = 3.f;
	float FogBase = 0.02f;
	float SkyBase = 1.f;
	bool bWasNight = false;
	int32 LastHourLogged = -1;

	void FindStage();
	float TargetDim() const;
	float TargetFog() const;
};
