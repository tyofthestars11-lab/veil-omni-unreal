#include "PHI_Constants.h"

DEFINE_LOG_CATEGORY_STATIC(LogVEILPhi, Log, All);

double PHI_Constants::GoldenAngleRad()
{
	return FMath::DegreesToRadians(GoldenAngleDeg);
}

FVector2D PHI_Constants::GoldenLoc(int32 Index, double Spacing)
{
	const double AngleRad = FMath::DegreesToRadians(static_cast<double>(Index) * GoldenAngleDeg);
	const double Radius = Spacing * FMath::Sqrt(static_cast<double>(Index));
	return FVector2D(Radius * FMath::Cos(AngleRad), Radius * FMath::Sin(AngleRad));
}

bool PHI_Constants::VerifyPhiIdentity(double& OutDelta)
{
	OutDelta = FMath::Abs((Phi * Phi) - (Phi + 1.0));
	return OutDelta <= BootTolerance;
}

bool PHI_Constants::VerifyBoot()
{
	double IdentityDelta = 0.0;
	const bool bIdentity = VerifyPhiIdentity(IdentityDelta);
	UE_LOG(LogVEILPhi, Display,
		TEXT("VEIL PHI BOOT: phi^2 = phi + 1 | delta=%.18f tol=%.0e -> %s"),
		IdentityDelta, BootTolerance, bIdentity ? TEXT("PASS") : TEXT("FAIL"));

	// The golden angle is derived from phi: 360 x (1 - 1/phi) = 360 x (2 - phi).
	// If this fails, GoldenAngleDeg is a typo, not a constant.
	const double AngleDelta = FMath::Abs(GoldenAngleDeg - 360.0 * (2.0 - Phi));
	const bool bAngle = AngleDelta <= BootTolerance;
	UE_LOG(LogVEILPhi, Display,
		TEXT("VEIL PHI BOOT: golden angle=%.13f deg | delta=%.18f tol=%.0e -> %s"),
		GoldenAngleDeg, AngleDelta, BootTolerance, bAngle ? TEXT("PASS") : TEXT("FAIL"));

	// Bible §1 proof values at Spacing 10, to 4 dp.
	const double ExpectedX[3] = { -7.3737, 1.2364, 18.8669 };
	const double ExpectedY[3] = { 6.7549, -14.0880, -12.0016 };
	const int32 Indices[3] = { 1, 2, 5 };

	bool bProof = true;
	for (int32 k = 0; k < 3; ++k)
	{
		const FVector2D P = GoldenLoc(Indices[k], 10.0);
		const double DX = FMath::Abs(P.X - ExpectedX[k]);
		const double DY = FMath::Abs(P.Y - ExpectedY[k]);
		const bool bOk = (DX <= ProofTolerance) && (DY <= ProofTolerance);
		bProof = bProof && bOk;
		UE_LOG(LogVEILPhi, Display,
			TEXT("VEIL PHI BOOT: GoldenLoc(i=%d) = (%9.4f, %9.4f) expected (%9.4f, %9.4f) -> %s"),
			Indices[k], P.X, P.Y, ExpectedX[k], ExpectedY[k], bOk ? TEXT("PASS") : TEXT("FAIL"));
	}

	const bool bAll = bIdentity && bAngle && bProof;
	UE_LOG(LogVEILPhi, Display, TEXT("VEIL PHI BOOT: %s"),
		bAll ? TEXT("ALL PASS") : TEXT("FAILED - do not build on this"));
	return bAll;
}
