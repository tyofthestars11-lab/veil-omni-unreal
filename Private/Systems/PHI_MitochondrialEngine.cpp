#include "PHI_MitochondrialEngine.h"

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
	float a = step * 137.5;
	while (a >= 360.0)
	{
		a -= 360.0;
	}
	return a;
}
