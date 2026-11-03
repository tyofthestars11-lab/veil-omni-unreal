#include "Systems/PHIIntent.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/PHIWeather.h"
#include "Systems/VEILEnemy.h"
#include "Systems/VEILTime.h"
#include "VEIL.h"
#include "Systems/VEILPhi.h"

namespace
{
	// 1/phi^2 = 0.381966: the contraction factor, and also the window the cycle
	// runs on, so the law and the clock are literally the same number.
	constexpr float GContract = 0.381966011250105f;
	constexpr float GPhi = VeilPhi::PhiF;
	/** Measured chaos above this feeds the world; below 1/phi^2 it rests. */
	constexpr float GHigh = VeilPhi::InvPhiF;
	constexpr float GLow = 0.381966011250105f;
	constexpr float GWindowSeconds = VeilPhi::Phi2F; // phi^2
	/** How many frame samples one cycle averages. */
	constexpr int32 GFrames = 48;
}

TStatId UPHIIntent::GetStatId() const
{
	return TStatId();
}

float UPHIIntent::GetRung() const
{
	// Computed from the live intent, never typed. Guarded at 1/phi^13 so the log
	// can never divide by zero or print an infinity.
	const float V = FMath::Max(Intent, 0.001f);
	return FMath::Loge(V) / FMath::Loge(GPhi);
}

FString UPHIIntent::IntentLine() const
{
	static const TCHAR* Lean[3] = { TEXT("RESTING"), TEXT("STEADY"), TEXT("LEANING IN") };
	const int32 L = Intent > GHigh ? 2 : (Intent < GLow ? 0 : 1);
	return FString::Printf(TEXT("PHI INTENT   chaos %.3f   intent %.3f   rung %.4f   %s"),
		Chaos, Intent, GetRung(), Lean[L]);
}

float UPHIIntent::MeasureJitter()
{
	// A frame that is merely slow is not chaos; a frame that is UNEVEN is. This
	// is the spread against the mean, scaled by phi so the golden band sits at
	// the middle of the scale instead of the top.
	if (Frames.Num() < 8 || AvgMs <= 0.f)
	{
		return 0.f;
	}
	const float Spread = FMath::Max(0.f, P95Ms - AvgMs);
	return FMath::Clamp(Spread / (AvgMs * GPhi), 0.f, 1.f);
}

float UPHIIntent::MeasureCombat()
{
	const UWorld* W = GetWorld();
	const APawn* P = UGameplayStatics::GetPlayerPawn(W, 0);
	if (!W || !P)
	{
		return 0.f;
	}
	int32 Near = 0;
	const FVector At = P->GetActorLocation();
	for (TActorIterator<AVEILEnemy> It(const_cast<UWorld*>(W)); It; ++It)
	{
		if (FVector::DistSquared(At, It->GetActorLocation()) < FMath::Square(3000.f))
		{
			++Near;
		}
	}
	// Eight hostiles at once is a full push; Fibonaccis 3, 5 and 8 are the steps.
	return FMath::Clamp(Near / 8.f, 0.f, 1.f);
}

float UPHIIntent::MeasureMotion()
{
	const APawn* P = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!P)
	{
		return 0.f;
	}
	// Against sprint speed, so walking sits around 1/phi^2 and sprinting at 1.
	return FMath::Clamp((float)P->GetVelocity().Size2D() / 750.f, 0.f, 1.f);
}

float UPHIIntent::MeasureTemporal()
{
	const UWorld* W = GetWorld();
	const UVEILTime* Clock = W ? W->GetSubsystem<UVEILTime>() : nullptr;
	if (!Clock)
	{
		return 0.f;
	}
	// Standing outside linear time is the strongest signal the game has: three
	// beats ahead reads as full temporal chaos.
	const float Drift = FMath::Clamp(Clock->GetPhaseDrift() / 3.f, 0.f, 1.f);
	// A field simply being open is ordinary - every beat boundary opens one. It
	// counts for 1/phi, while actually standing outside time counts in full.
	const float Open = Clock->IsSuperposed() ? (1.f / GPhi) : 0.f;
	return FMath::Clamp(FMath::Max(Drift, Open), 0.f, 1.f);
}

void UPHIIntent::ForceCycle()
{
	SinceCycle = 0.f;
	Cycle();
}

void UPHIIntent::Cycle()
{
	++Cycles;

	PartJitter = MeasureJitter();
	PartCombat = MeasureCombat();
	PartMotion = MeasureMotion();
	PartTemporal = MeasureTemporal();

	// Folded the way chaos actually adds up: sources combine, they do not
	// average away. One source at full strength is full chaos; several partial
	// sources reinforce each other. The phi weights put the sharpest signal on
	// the shortest path (1) and let the rest fade down the spiral.
	const float W1 = 1.f, W2 = 1.f / GPhi, W3 = 1.f / (GPhi * GPhi), W4 = 1.f / (GPhi * GPhi * GPhi);
	Chaos = FMath::Clamp(1.f
		- FMath::Pow(FMath::Max(0.f, 1.f - PartJitter), W1)
		* FMath::Pow(FMath::Max(0.f, 1.f - PartCombat), W2)
		* FMath::Pow(FMath::Max(0.f, 1.f - PartMotion), W3)
		* FMath::Pow(FMath::Max(0.f, 1.f - PartTemporal), W4), 0.f, 1.f);

	const float Before = Intent;
	// THE GOLDEN CONTRACTION. 1/phi^2 < 1, so this can only converge.
	Intent = FMath::Clamp(Intent + (Chaos - Intent) * GContract, 0.f, 1.f);

	const int32 Lean = Intent > GHigh ? 2 : (Intent < GLow ? 0 : 1);

	// Feed the world with what was measured, not with a script.
	if (UWorld* W = GetWorld())
	{
		if (UPHIWeather* Sky = W->GetSubsystem<UPHIWeather>())
		{
			Sky->SetTurbulence(Intent);
		}
		if (UVEILTime* Clock = W->GetSubsystem<UVEILTime>())
		{
			Clock->SetChaosPressure(Intent);
		}
	}

	UE_LOG(LogVEIL, Log,
		TEXT("PHI INTENT cycle %d: chaos %.3f (jitter %.2f, combat %.2f, motion %.2f, temporal %.2f) "
			 "-> intent %.3f (was %.3f), rung %.4f, %s."),
		Cycles, Chaos, PartJitter, PartCombat, PartMotion, PartTemporal,
		Intent, Before, GetRung(),
		Lean == 2 ? TEXT("LEANING IN") : (Lean == 0 ? TEXT("RESTING") : TEXT("STEADY")));

	if (Lean != LastLean)
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI INTENT: the world turns %s - the sky and the field follow it."),
			Lean == 2 ? TEXT("toward intensity") : (Lean == 0 ? TEXT("toward rest") : TEXT("back to the middle")));
		LastLean = Lean;
	}
}

void UPHIIntent::Tick(float DeltaTime)
{
	// Measure the machine every frame; fold it once per phi^2 seconds. The
	// measurement is continuous, the decision is golden-spaced.
	Frames.Add(DeltaTime * 1000.f);
	if (Frames.Num() > GFrames)
	{
		Frames.RemoveAt(0, Frames.Num() - GFrames, EAllowShrinking::No);
	}
	if (Frames.Num() >= 8)
	{
		TArray<float> Sorted = Frames;
		Sorted.Sort();
		float Sum = 0.f;
		for (const float M : Sorted) { Sum += M; }
		AvgMs = Sum / (float)Sorted.Num();
		P95Ms = Sorted[FMath::Clamp((int32)(Sorted.Num() * 0.95f), 0, Sorted.Num() - 1)];
	}

	SinceCycle += DeltaTime;
	if (SinceCycle >= GWindowSeconds)
	{
		SinceCycle = 0.f;
		Cycle();
	}
}
