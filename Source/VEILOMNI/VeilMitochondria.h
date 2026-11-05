#pragma once
#include "CoreMinimal.h"
#include "PHI_Constants.h"

/**
 * The mitochondrial engine of the Bible, as fuel and ledger rather than static mana.
 *
 * ATP is charge, not a bar that regenerates from nothing: it builds on the proton
 * gradient, a spend dips the phase, and it round-trips back to equilibrium.
 * mtDNA is the hidden address, so every organelle upgrade is permanent and written
 * to the run, never bought back with a linear tree.
 */
struct FVeilMitochondria
{
	/** ATP capacity is phi-runged: 55 rungs up the ladder from 1. */
	float MaxATP = 89.f;
	float ATP = 89.f;

	/** Proton gradient charge, 0..phi. Spends dip it below 1, rest returns it to phi. */
	float Gradient = 1.0f;

	/** Engulfed bacterial nodes. Permanent, one per node, never spent. */
	int32 Organelles = 0;

	/** The hidden ledger address. mtDNA rung 20.189 in the Bible. */
	double AddressRung = 20.189;

	void Tick(float DeltaSeconds, bool bSprinting)
	{
		const float Drain = bSprinting ? 22.0f : -6.0f * static_cast<float>(PHI_Constants::Phi);
		ATP = FMath::Clamp(ATP - Drain * DeltaSeconds, 0.f, MaxATP);
		const float Target = (ATP > 1.f) ? static_cast<float>(PHI_Constants::Phi) : 0.f;
		Gradient = FMath::FInterpTo(Gradient, Target, DeltaSeconds, 1.6f);
	}

	/** One spend. A spend that would empty the gradient is refused, not clamped. */
	bool Spend(float Cost)
	{
		if (ATP < Cost || Gradient <= 0.05f)
		{
			return false;
		}
		ATP -= Cost;
		Gradient = FMath::Max(0.f, Gradient - Cost / FMath::Max(1.f, MaxATP));
		return true;
	}

	/** Endosymbiosis: engulf a bacterial node, keep the organelle forever. */
	void Engulf(int32 NodeIndex)
	{
		++Organelles;
		MaxATP *= static_cast<float>(PHI_Constants::Phi);
		ATP = MaxATP;
		Gradient = static_cast<float>(PHI_Constants::Phi);
		AddressRung = PHI_Constants::Rung(static_cast<double>(MaxATP)) + NodeIndex * 0.001;
		UE_LOG(LogTemp, Display,
			TEXT("VEIL ATP: engulfed node %d, organelle %d, ATP now %.1f (rung %.4f)"),
			NodeIndex, Organelles, MaxATP, AddressRung);
	}

	FString Ledger() const
	{
		return FString::Printf(TEXT("mtDNA %.3f | ATP %.0f/%.0f | gradient %.2f | organelles %d"),
			AddressRung, ATP, MaxATP, Gradient, Organelles);
	}
};
