#pragma once
#include "CoreMinimal.h"

class PHI_MitochondrialEngine
{
public:
	PHI_MitochondrialEngine();

	float ATPGradient;
	float NDnaLedger;
	float GoldenPitch;
	float EquilibriumRung;
	float EchoGap;
	float CoherenceDissolve;

	void PumpProtons(float amount);
	void RestoreEquilibrium();
	bool IntegrateNode(float echoGap);
	float BranchAngle(int step);
};
