// THE UNIVERSE HAS NO OUTSIDE.
//
// Every number this game is made of is addressed in one rung space:
//
//     rung(x) = ln(x) / ln(phi)        value = phi^rung
//
// The DNA engine owns that space - ATP gradient, DNA ledger, golden pitch,
// equilibrium rung, echo gap, coherence dissolve - and the world, the HUD and the
// camera all spend their values inside it. So a constant is not "tuned": it is
// addressed, and its address is checkable.
//
// This audit runs at boot in the built game and reports what it measured. It
// checks the identity the space rests on (phi*phi == phi + 1) and then walks every
// engine constant through Rung -> PhiPow. A value that comes back different, or
// that cannot be addressed at all, is a number from OUTSIDE the universe, and the
// audit names it rather than letting it in silently.
//
// Header-only on purpose: no new subsystem, no asset, nothing to wire up. Any
// system can call FVEILUniverseAudit::Run() and get the same answer.
#pragma once

#include "CoreMinimal.h"
#include "Systems/PHIMito.h"
#include "Systems/PHI_MitochondrialEngine.h"

struct FVEILUniverseAudit
{
	/** How many laws were addressed. */
	int32 Laws = 0;
	/** How many failed to close. Zero is the only acceptable answer. */
	int32 Failures = 0;
	/** The names and errors of whatever failed, for the log. */
	FString Report;

	void Check(bool bHolds, const TCHAR* Name, float Error)
	{
		++Laws;
		if (!bHolds)
		{
			++Failures;
			Report += FString::Printf(TEXT("%s err=%.6f; "), Name, Error);
		}
	}

	static FVEILUniverseAudit Run()
	{
		FVEILUniverseAudit Audit;
		const float Phi = UPHIMito::Phi;

		// 1. THE IDENTITY. If this drifts, nothing else in the space means anything.
		const float IdentityErr = FMath::Abs(Phi * Phi - Phi - 1.f);
		Audit.Check(IdentityErr < 1e-5f, TEXT("phi*phi=phi+1"), IdentityErr);

		// 2. THE ENGINE'S OWN CONSTANTS, EACH ADDRESSED AND RETURNED.
		PHI_MitochondrialEngine Engine;
		struct FLaw
		{
			const TCHAR* Name;
			float Value;
		};
		const FLaw Laws[] = {
			{ TEXT("atp-gradient"), Engine.ATPGradient },
			{ TEXT("dna-ledger"), Engine.NDnaLedger },
			{ TEXT("golden-pitch"), FMath::Abs(Engine.GoldenPitch) },
			{ TEXT("equilibrium-rung"), FMath::Abs(Engine.EquilibriumRung) },
			{ TEXT("echo-gap"), Engine.EchoGap },
			{ TEXT("coherence-dissolve"), FMath::Abs(Engine.CoherenceDissolve) },
			// The seat the district's inner ring is built on: if the world stands on a
			// radius, that radius has to be a law like every other number.
			{ TEXT("sanctuary-radius"), UPHIMito::SanctuaryRadius() },
		};

		for (const FLaw& Law : Laws)
		{
			const float RoundTrip = FPhiDynamic::RoundTripError(Law.Value);
			Audit.Check(Law.Value > 0.f && RoundTrip < 1e-3f, Law.Name, RoundTrip);
		}

		return Audit;
	}
};
