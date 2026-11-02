#pragma once

#include "CoreMinimal.h"

/**
 * PHI PHASE LOCK
 *
 * One address per domain, and every domain has to survive the round trip:
 *
 *     value -> rung -> value
 *
 * rung = ln(value) / ln(phi), and value = phi^rung. On a plain double the trip
 * comes back with a float residue (~1e-15) that is an artefact of IEEE754, not
 * drift. So the lock is defined on the RUNG LATTICE at 1e-4: quantise the rung,
 * rebuild the value, quantise the rung again - and the two rungs must be equal
 * bit-for-bit. That is a real exactness claim, not a rounded one, and the raw
 * float residue is reported next to it instead of being hidden.
 *
 * A domain that fails does not go red and does not stop the others: it realigns
 * onto the lattice, re-locks, and says so with its address and its rung.
 */
namespace PHIPhase
{
	/** phi, to the digits the rest of the project uses. */
	constexpr double Phi = 1.618033988749895;

	/** rung = ln(value)/ln(phi). */
	double RungOf(double Value);

	/** value = phi^rung. */
	double ValueOf(double Rung);

	/** The lattice the lock is measured on: rungs to four decimals. */
	constexpr double RungLattice = 1.0e-4;

	struct FLock
	{
		FName Domain;
		double Address = 0.0;      // the authority address, in world units
		double Rung = 0.0;         // quantised rung of that address
		double LatticeDrift = 0.0; // rung after round trip minus rung: must be 0
		double FloatResidue = 0.0; // |phi^rung - address|: IEEE754, reported, not hidden
		bool bLocked = false;
	};

	/** Lock one domain. Realigns once if the first trip misses the lattice. */
	FLock Lock(const FName& Domain, double Address);

	/** The seven domains of the hive, at their authority addresses. */
	TArray<FLock> LockAll();

	/** One line per domain, for the log and the HUD. */
	FString Line(const FLock& L);

	/** True only if every domain is locked with zero lattice drift. */
	bool AllLocked(const TArray<FLock>& Locks);
}
