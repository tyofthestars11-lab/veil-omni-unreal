#include "Systems/PHI_MitochondrialEngine.h"

PHI_MitochondrialEngine::PHI_MitochondrialEngine()
{
	// f-suffixed: these are floats, and a bare double literal truncates into them,
	// which is warning C4305 - a warning is a failure at this project's gate.
	ATPGradient = 100.0f;
	NDnaLedger = 45.482f;
	GoldenPitch = 17.0324f;
	EquilibriumRung = -14.438f;
	EchoGap = 25.293f;
	CoherenceDissolve = -62.160f;
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
	float a = step * 137.5;
	while (a >= 360.0)
	{
		a -= 360.0;
	}
	return a;
}


