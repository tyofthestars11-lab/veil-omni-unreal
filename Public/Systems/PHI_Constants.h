#pragma once
#include "CoreMinimal.h"

/**
 * VEIL Bible §1. Single source of truth for phi. Nothing else in the project
 * may hard-code 1.618..., 137.5077... or 137.5.
 *
 * VerifyBoot() is called once on boot, never per frame (Bible §9: log only on change).
 */
class PHI_Constants
{
public:
	// Bible §1, exact values. Do not round these.
	static constexpr double Phi = 1.618033988749895;
	static constexpr double GoldenAngleDeg = 137.5077640500378;

	// Tolerances required by the Bible.
	static constexpr double BootTolerance = 1e-6;   // phi^2 = phi + 1
	static constexpr double ProofTolerance = 1e-4;  // 4 dp proof values at Spacing 10

	// GoldenLoc(i): angle = i x 137.5077640500378 deg, radius = Spacing x sqrt(i).
	// Unreal units are centimetres. Returns XY in centimetres.
	static FVector2D GoldenLoc(int32 Index, double Spacing);

	static double GoldenAngleRad();

	// |phi^2 - (phi + 1)| <= 1e-6
	static bool VerifyPhiIdentity(double& OutDelta);

	// Full boot gate: identity, the golden-angle/phi relation, and the three Bible
	// proof values at Spacing 10. Logs one line per check, returns true only if all pass.
	static bool VerifyBoot();
};
