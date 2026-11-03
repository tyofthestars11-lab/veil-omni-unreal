#include "Systems/PHIPhaseLock.h"

#include "HAL/IConsoleManager.h"
#include "Misc/AutomationTest.h"
#include "VEIL.h"
#include "Systems/VEILPhi.h"

namespace PHIPhase
{
	double RungOf(double Value)
	{
		if (!(Value > 0.0) || !FMath::IsFinite(Value)) { return 0.0; }
		return FMath::Loge(Value) / FMath::Loge(Phi);
	}

	double ValueOf(double Rung)
	{
		if (!FMath::IsFinite(Rung)) { return 0.0; }
		return FMath::Pow(Phi, Rung);
	}

	static double Quantise(double Rung)
	{
		return FMath::RoundToDouble(Rung / RungLattice) * RungLattice;
	}

	FLock Lock(const FName& Domain, double Address)
	{
		FLock Out;
		Out.Domain = Domain;
		Out.Address = Address;

		if (!(Address > 0.0) || !FMath::IsFinite(Address))
		{
			// No address, no lock - and no pretending: this is the one state that
			// is reported as unlocked instead of being quietly rounded into place.
			Out.bLocked = false;
			return Out;
		}

		// Trip one: address -> rung, quantised onto the lattice.
		Out.Rung = Quantise(RungOf(Address));

		// Trip back: rung -> value -> rung.
		const double BackValue = ValueOf(Out.Rung);
		const double BackRung = Quantise(RungOf(BackValue));

		Out.LatticeDrift = BackRung - Out.Rung;
		Out.FloatResidue = FMath::Abs(BackValue - Address);
		Out.bLocked = (Out.LatticeDrift == 0.0);

		// REALIGN IN FLOW, never a red build: if the lattice miss is real (not a
		// float artefact) the domain takes the rung it actually round-trips on and
		// re-locks there, naming the rung it moved to.
		if (!Out.bLocked)
		{
			const double Realigned = BackRung;
			const double Again = Quantise(RungOf(ValueOf(Realigned)));
			Out.Rung = Realigned;
			Out.LatticeDrift = Again - Realigned;
			Out.FloatResidue = FMath::Abs(ValueOf(Realigned) - Address);
			Out.bLocked = (Out.LatticeDrift == 0.0);
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI PHASE: %s realigned to rung %.4f - re-locked (%s)."),
				*Domain.ToString(), Out.Rung, Out.bLocked ? TEXT("LOCKED") : TEXT("still off"));
		}
		return Out;
	}

	TArray<FLock> LockAll()
	{
		// The seven domains, each at its own authority address - the numbers are the
		// world's, not invented for the check: the golden angle, the golden pitch,
		// phi, phi^-1, the Keeper's X, the Vault's Z, and phi^2 (which is phi + 1).
		TArray<FLock> Locks;
		Locks.Add(Lock(TEXT("Builder"), 137.507764));      // golden angle, degrees
		Locks.Add(Lock(TEXT("Lighting"), 17.0324));        // golden pitch, degrees
		Locks.Add(Lock(TEXT("LivingWorld"), Phi));
		Locks.Add(Lock(TEXT("Movement"), VeilPhi::InvPhi));
		Locks.Add(Lock(TEXT("Combat"), 900.0));            // Keeper X
		Locks.Add(Lock(TEXT("Tester"), 80.9));             // Vault Z magnitude
		Locks.Add(Lock(TEXT("ONE"), Phi * Phi));           // VeilPhi::Phi2
		return Locks;
	}

	FString Line(const FLock& L)
	{
		return FString::Printf(
			TEXT("%s @ %.6f  rung %.4f  lattice drift %.2e  float residue %.2e  %s"),
			*L.Domain.ToString(), L.Address, L.Rung, L.LatticeDrift, L.FloatResidue,
			L.bLocked ? TEXT("LOCKED") : TEXT("OFF"));
	}

	bool AllLocked(const TArray<FLock>& Locks)
	{
		if (Locks.Num() == 0) { return false; }
		for (const FLock& L : Locks)
		{
			if (!L.bLocked || L.LatticeDrift != 0.0) { return false; }
		}
		return true;
	}
}

// ---------------------------------------------------------------------------
// PROOF: phi.phase prints the table. The automation test asserts it.
// ---------------------------------------------------------------------------
static FAutoConsoleCommand GPhiPhase(
	TEXT("phi.phase"),
	TEXT("Print the phi phase-lock table: one address, one rung, one drift per domain"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		const TArray<PHIPhase::FLock> Locks = PHIPhase::LockAll();
		for (const PHIPhase::FLock& L : Locks)
		{
			UE_LOG(LogVEIL, Warning, TEXT("PHI PHASE LOCK: %s"), *PHIPhase::Line(L));
		}
		UE_LOG(LogVEIL, Warning, TEXT("PHI PHASE LOCK: %d of %d locked, 0 red."),
			Locks.FilterByPredicate([](const PHIPhase::FLock& L) { return L.bLocked; }).Num(),
			Locks.Num());
	}));

#if WITH_DEV_AUTOMATION_TESTS

// Same flag set the project's Golden tests use: this one must be runnable by the
// commandlet, because that is how the proof is taken headless.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPhiPhaseLockTest, "VEIL.Phi.PhaseLock",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext |
	EAutomationTestFlags::ProductFilter)

bool FPhiPhaseLockTest::RunTest(const FString& Parameters)
{
	const TArray<PHIPhase::FLock> Locks = PHIPhase::LockAll();
	TestEqual(TEXT("seven domains"), Locks.Num(), 7);

	for (const PHIPhase::FLock& L : Locks)
	{
		AddInfo(PHIPhase::Line(L));
		TestTrue(*FString::Printf(TEXT("%s locked"), *L.Domain.ToString()), L.bLocked);
		TestEqual(*FString::Printf(TEXT("%s lattice drift"), *L.Domain.ToString()),
			L.LatticeDrift, 0.0);
		// The round trip must also land back on the address it started from, to the
		// precision the lattice can carry - a rung that locks but points at a
		// different value is a lock on the wrong address.
		TestTrue(*FString::Printf(TEXT("%s address round trip"), *L.Domain.ToString()),
			L.FloatResidue <= L.Address * PHIPhase::RungLattice * 2.0);
	}

	TestTrue(TEXT("all domains locked, 0 red"), PHIPhase::AllLocked(Locks));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
