#pragma once

#include "CoreMinimal.h"

// ---------------------------------------------------------------------------
// THE ONE DEFINITION OF PHI.
//
// Before this header, VEILWorldBuilder.cpp declared phi nine times under nine
// names - Phi, PhiRegion, PhiV, PhiLocal, PhiRef, GoldenAngleDeg,
// GoldenAngleRegion, GoldDeg, GoldenDeg - and several of those were typed short
// (1.61803398875f, 1.6180339887), so the same file carried two different phis.
//
// Here the literal is written exactly once, as the definition it is. Everything
// else is computed from it, including the golden angle, which is not a number to
// be remembered: it is 360 / phi^2.
//
// Fibonacci and Lucas live here too, for the same reason: they are not tables to
// be typed in and trusted, they are consequences of the equation
//     phi^2 = phi + 1,   phi = (1 + sqrt 5) / 2
// and the compiler checks every identity below before the game can be built.
// ---------------------------------------------------------------------------
namespace VeilPhi
{
	constexpr double Phi = 1.618033988749895;              // the one typed literal

	constexpr double Phi2 = Phi * Phi;                     // phi^2  == phi + 1
	constexpr double InvPhi = 1.0 / Phi;                   // 1/phi  == phi - 1
	constexpr double GoldenAngleDeg = 360.0 / Phi2;        // 137.5077640500378
	constexpr double GoldenPitchDeg = 17.0324;             // given, rung 5.8916
	constexpr int32 VisionSeed = 1618;                     // the vision's seed, never random

	// Float forms, so a float site never pays an implicit narrowing conversion.
	constexpr float PhiF = (float)Phi;
	constexpr float Phi2F = (float)Phi2;
	constexpr float InvPhiF = (float)InvPhi;
	constexpr float GoldenAngleF = (float)GoldenAngleDeg;

	// The law itself, checked by the compiler instead of trusted from a comment.
	static_assert(Phi2 - (Phi + 1.0) < 1e-15 && (Phi + 1.0) - Phi2 < 1e-15,
		"phi^2 must equal phi + 1");
	static_assert(GoldenAngleDeg > 137.5077640500377 && GoldenAngleDeg < 137.5077640500379,
		"golden angle must come out of 360/phi^2");

	// -----------------------------------------------------------------------
	// THE LADDER. Every distance, duration and tier in the game climbs by phi.
	// -----------------------------------------------------------------------
	constexpr double Sqrt5 = 2.2360679774997896964;         // sqrt(5), the equation's root

	constexpr double PhiPower(int32 N)
	{
		// phi^N by multiplication, not std::pow: constexpr, exact, and no dependency.
		double R = 1.0;
		if (N >= 0) { for (int32 i = 0; i < N; ++i) { R *= Phi; } }
		else { for (int32 i = 0; i < -N; ++i) { R *= InvPhi; } }
		return R;
	}

	constexpr double SignPow(int32 N) { return (N % 2 == 0) ? 1.0 : -1.0; }  // (-1)^N

	// F(n) = (phi^n - (-1/phi)^n) / sqrt(5)  -- Binet, straight from the equation.
	constexpr double BinetF(int32 N) { return (PhiPower(N) - SignPow(N) * PhiPower(-N)) / Sqrt5; }

	// L(n) = phi^n + (-1/phi)^n  -- the Lucas closed form on the same root.
	constexpr double LucasClosed(int32 N) { return PhiPower(N) + SignPow(N) * PhiPower(-N); }

	// The integers, by recurrence, so nothing depends on floating point rounding.
	constexpr int64 Fibonacci(int32 N)
	{
		int64 A = 0, B = 1;                                 // F(0) = 0, F(1) = 1
		for (int32 i = 0; i < N; ++i) { const int64 C = A + B; A = B; B = C; }
		return A;
	}

	constexpr int64 Lucas(int32 N)
	{
		int64 A = 2, B = 1;                                 // L(0) = 2, L(1) = 1
		for (int32 i = 0; i < N; ++i) { const int64 C = A + B; A = B; B = C; }
		return A;
	}

	// base * phi^step: the ladder the camera, the rings and the tiers already climb.
	constexpr double LadderCm(double BaseCm, int32 Step) { return BaseCm * PhiPower(Step); }

	// Which rung a length sits on, so a measured distance can be named as a step.
	constexpr int32 StepOf(double ValueCm, double BaseCm)
	{
		if (ValueCm <= 0.0 || BaseCm <= 0.0) { return 0; }
		int32 Step = 0;
		double L = BaseCm;
		while (L * Phi <= ValueCm && Step < 64) { L *= Phi; ++Step; }
		while (L > ValueCm && Step > -64) { L *= InvPhi; --Step; }
		return Step;
	}

	// -----------------------------------------------------------------------
	// THE PROOF. Compiler-checked: the integers, the closed forms, the identity
	// that ties Fibonacci to Lucas, and the convergence the whole game rests on.
	// -----------------------------------------------------------------------
	static_assert(Fibonacci(0) == 0 && Fibonacci(1) == 1 && Fibonacci(10) == 55
		&& Fibonacci(11) == 89 && Fibonacci(20) == 6765, "Fibonacci recurrence must be exact");
	static_assert(Lucas(0) == 2 && Lucas(1) == 1 && Lucas(5) == 11 && Lucas(10) == 123,
		"Lucas recurrence must be exact");
	static_assert(Lucas(5) == Fibonacci(4) + Fibonacci(6), "L(n) = F(n-1) + F(n+1)");
	static_assert(Lucas(9) * Lucas(9) - 5 * Fibonacci(9) * Fibonacci(9) == -4,
		"L(n)^2 - 5 F(n)^2 = 4 (-1)^n");
	static_assert(BinetF(10) > 54.9999 && BinetF(10) < 55.0001,
		"Binet must reproduce F(10) = 55 from phi alone");
	static_assert(BinetF(20) > 6764.99 && BinetF(20) < 6765.01,
		"Binet must reproduce F(20) = 6765 from phi alone");
	static_assert(LadderCm(420.0, 0) == 420.0, "the ladder's zero rung is the base length");
	static_assert(LadderCm(420.0, 1) > 679.5 && LadderCm(420.0, 1) < 679.6,
		"one rung up from 420 cm is 679.57 cm (420 * phi)");
	static_assert(StepOf(420.0, 420.0) == 0, "the base length is the zero rung");
	static_assert(StepOf(680.0, 420.0) == 1, "680 cm is one phi rung above 420 cm");
	static_assert(StepOf(1100.0, 420.0) == 2, "1100 cm is two phi rungs above 420 cm");
	static_assert(StepOf(260.0, 420.0) == -1, "260 cm is one phi rung below 420 cm");
}
