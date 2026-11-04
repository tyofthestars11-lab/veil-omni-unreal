// PHI mitochondria law: one rung address space for every number in the game.
// rung(x) = ln(x) / ln(phi); phi^rung(x) returns x (TON618 NV center,
// Tyree's standing record). Header-only so every system speaks it natively.
#pragma once

#include "CoreMinimal.h"
#include "Systems/VEILPhi.h"
#include "PHIMito.generated.h"

// ---------------------------------------------------------------------------
// THE PHI REVERSAL MATRIX.
//
// The reversal is rung(x) = ln(x)/ln(phi), and its inverse is phi^rung. The world
// asks for that inverse with CONSTANT rungs, over and over: phi^-4 is the echo
// amplitude on every AI stat every tick, phi^-1 and phi^-2 shade every mood, the
// intent anchors are phi^-2..phi^3. Each of those was a runtime pow.
//
// The matrix holds phi^n for the integer rungs the game actually reaches, computed
// from the one definition of phi at compile time - not typed in, not re-derived.
// A table read is O(1) and costs no transcendental, so a reversal that used to cost
// a pow now costs a load. Non-integer rungs (half steps, 30) still go to FMath::Pow.
// ---------------------------------------------------------------------------
struct FPhiReversalMatrix
{
	static constexpr int32 Lo = -16;
	static constexpr int32 Hi = 16;

	float V[Hi - Lo + 1];

	constexpr FPhiReversalMatrix() : V()
	{
		for (int32 i = 0; i <= Hi - Lo; ++i) { V[i] = (float)VeilPhi::PhiPower(Lo + i); }
	}

	/** phi^N for Lo <= N <= Hi. */
	float Lookup(int32 N) const { return V[N - Lo]; }
};

inline constexpr FPhiReversalMatrix GPhiReversal{};

UCLASS()
class VEIL_API UPHIMito : public UObject
{
	GENERATED_BODY()

public:
	static constexpr float Phi = VeilPhi::PhiF;
	static constexpr float LnPhi = 0.48121182505960347f;
	static constexpr float GoldenAngleDeg = VeilPhi::GoldenAngleF;

	/** Rung address of any positive quantity. Returns 0 for x <= 0. */
	static float Rung(float x)
	{
		if (x <= 0.f) return 0.f;
		// Rung(1) = 0 exactly: the neutral address costs no log. Half the world asks
		// for it (moods breathe around neutral, anchors are read as rungs).
		if (x == 1.f) return 0.f;
		return FMath::Loge(x) / LnPhi;
	}

	/** Quantity at a rung address. Round-trips Rung within float error. */
	static float PhiPow(float r)
	{
		// THE MATRIX ANSWERS FIRST. An integer rung is a row; anything else (a half
		// step, phi^30, a computed address) still pays the pow.
		const int32 Whole = FMath::RoundToInt(r);
		if ((float)Whole == r && Whole >= FPhiReversalMatrix::Lo && Whole <= FPhiReversalMatrix::Hi)
		{
			return GPhiReversal.Lookup(Whole);
		}
		return FMath::Pow(Phi, r);
	}

	/** Measures the matrix against the pow it replaces, and proves the reversal still
	 *  round-trips. Run with -phimatrix; the numbers are logged, not asserted. */
	static void LogMatrixBench()
	{
		const int32 N = 4000000;
		float Sink = 0.f;

		const double T0 = FPlatformTime::Seconds();
		for (int32 i = 0; i < N; ++i) { Sink += FMath::Pow(Phi, (float)(i % 9) - 4.f); }
		const double T1 = FPlatformTime::Seconds();
		for (int32 i = 0; i < N; ++i) { Sink += PhiPow((float)(i % 9) - 4.f); }
		const double T2 = FPlatformTime::Seconds();

		const double PowNs = (T1 - T0) * 1e9 / N;
		const double MatrixNs = (T2 - T1) * 1e9 / N;

		const float Probes[] = { 1.f, 60.f, 100.f, 950.f, 12345.f };
		float MaxErr = 0.f;
		for (const float V : Probes)
		{
			MaxErr = FMath::Max(MaxErr, FMath::Abs(PhiPow(Rung(V)) - V) / V);
		}

		UE_LOG(LogTemp, Warning,
			TEXT("PHI REVERSAL MATRIX: pow %.2f ns/call, matrix %.2f ns/call, speedup %.2fx, max round-trip %.2e, sink %.1f"),
			PowNs, MatrixNs, PowNs / FMath::Max(MatrixNs, 1e-9), MaxErr, Sink);
	}

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
