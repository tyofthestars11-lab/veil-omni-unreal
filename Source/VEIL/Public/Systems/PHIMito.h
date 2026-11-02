// PHI mitochondria law: one rung address space for every number in the game.
// rung(x) = ln(x) / ln(phi); phi^rung(x) returns x (TON618 NV center,
// Tyree's standing record). Header-only so every system speaks it natively.
#pragma once

#include "CoreMinimal.h"
#include "PHIMito.generated.h"

UCLASS()
class VEIL_API UPHIMito : public UObject
{
	GENERATED_BODY()

public:
	static constexpr float Phi = 1.618033988749895f;
	static constexpr float LnPhi = 0.48121182505960347f;
	static constexpr float GoldenAngleDeg = 137.50776405003785f;

	/** Rung address of any positive quantity. Returns 0 for x <= 0. */
	static float Rung(float x)
	{
		if (x <= 0.f) return 0.f;
		return FMath::Loge(x) / LnPhi;
	}

	/** Quantity at a rung address. Round-trips Rung within float error. */
	static float PhiPow(float r) { return FMath::Pow(Phi, r); }

	/** The camp is seated on 950 x phi^(i%3), so 950 cm IS its inner ring.
	 *  Inside that ring the camp holds station and never strikes - the door is a
	 *  seat, not a kill box. BattleMesh research note: a null nav area alone does
	 *  not stop AAIController::MoveToActor or direct movement input, so the gate
	 *  lives in the brain AND the radius comes from the camp's own geometry. */
	static float SanctuaryRadius() { return 950.f * PhiPow(0.f); }

	static bool IsInsideSanctuary(const FVector& Location)
	{
		return FVector::Dist2D(Location, FVector::ZeroVector) <= SanctuaryRadius();
	}

	/** Phase of a rung in degrees, for variety that never repeats exactly. */
	static float PhaseDeg(float r)
	{
		const float f = r - FMath::FloorToFloat(r);
		return f * 360.f;
	}
};

// ---------------------------------------------------------------------------
// FPhiDynamic - the clock every AI rides.
//
// An AI is not a static number. A stat is given as a seed, the seed's rung is
// its address, and the rung breathes on a phi clock:
//
//     rung   = Rung(seed) + phi^-4 * sin((phase + offset) * 2*pi)
//     value  = phi^rung                     (recomputed, never stored as truth)
//     phase += dt / phi                     (accumulated, so it cannot drift)
//
// phi^-4 is 0.1459 of a rung, so a quantity breathes about +/-15.7% around the
// seed - the same rung space as the ledger, no typed swing constants.
// Each stat takes its own phase offset, so speed, reach and cadence never rise
// and fall together as one frozen pattern.
// ---------------------------------------------------------------------------
struct FPhiDynamic
{
	/** Elapsed phi-clock phase in cycles. Advanced by dt / phi. */
	float Phase = 0.f;

	void Advance(float DeltaSeconds)
	{
		Phase += DeltaSeconds / UPHIMito::Phi;
	}

	/** Rung address of a seed, displaced by the clock. Offset and scale are rung-space. */
	template <typename T>
	static float Rung(T Seed, float InPhase, float PhaseOffset = 0.f, float AmplitudeScale = 1.f)
	{
		const float Wave = FMath::Sin((InPhase + PhaseOffset) * 2.f * PI);
		const float Amplitude = UPHIMito::PhiPow(-4.f) * AmplitudeScale;
		return UPHIMito::Rung(static_cast<float>(Seed)) + Amplitude * Wave;
	}

	/** The quantity this stat actually is right now. */
	template <typename T>
	float Value(T Seed, float PhaseOffset = 0.f, float AmplitudeScale = 1.f) const
	{
		return UPHIMito::PhiPow(Rung(Seed, Phase, PhaseOffset, AmplitudeScale));
	}

	/** Round-trip check: phi^Rung(v) - v. This is how a rung is proven, not claimed. */
	static float RoundTripError(float Value)
	{
		return FMath::Abs(UPHIMito::PhiPow(UPHIMito::Rung(Value)) - Value);
	}
};
