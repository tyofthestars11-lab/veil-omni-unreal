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
// ---------------------------------------------------------------------------
namespace VeilPhi
{
	constexpr double Phi = 1.618033988749895;              // the one typed literal

	constexpr double Phi2 = Phi * Phi;                     // phi^2  == phi + 1
	constexpr double InvPhi = 1.0 / Phi;                   // 1/phi  == phi - 1
	constexpr double GoldenAngleDeg = 360.0 / Phi2;        // 137.5077640500378
	constexpr double GoldenPitchDeg = 17.0324;             // given, rung 5.8916

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
}
