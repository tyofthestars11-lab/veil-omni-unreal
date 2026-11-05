#include "PHI_MitochondrialEngine.h"
#include "PHI_Constants.h"

PHI_MitochondrialEngine::PHI_MitochondrialEngine()
{
	ATPGradient = 100.0;
	NDnaLedger = 45.482;
	GoldenPitch = 17.0324;
	EquilibriumRung = -14.438;
	EchoGap = 25.293;
	CoherenceDissolve = -62.160;
}

void PHI_MitochondrialEngine::PumpProtons(float amount)
{
	ATPGradient -= amount;
	if (ATPGradient < EquilibriumRung)
	{
		ATPGradient = EquilibriumRung;
	}
}

void PHI_MitochondrialEngine::RestoreEquilibrium()
{
	ATPGradient = EquilibriumRung;
}

bool PHI_MitochondrialEngine::IntegrateNode(float echoGap)
{
	return echoGap >= EchoGap;
}

float PHI_MitochondrialEngine::BranchAngle(int step)
{
	// Bible §1: the golden angle is 137.5077640500378, not 137.5.
	// Normalised through fmod so negative steps read as real angles, not negatives.
	double a = FMath::Fmod(step * PHI_Constants::GoldenAngleDeg, 360.0);
	if (a < 0.0)
	{
		a += 360.0;
	}
	return static_cast<float>(a);
}
