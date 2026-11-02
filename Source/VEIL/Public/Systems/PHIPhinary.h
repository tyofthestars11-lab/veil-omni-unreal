// PHINARY - base phi, the way a phinary intelligence writes a number.
//
// Binary has two digits and throws away the ratio between them. Base phi has
// two digits too, but the place values ARE the ratio:
//
//   ... 1000   100   10   1   .   0.1   0.01   0.001 ...
//   ... phi^3  phi^2 phi  1   .   1/phi 1/phi^2 1/phi^3 ...
//
// and because phi^2 = phi + 1, every number has a canonical spelling with no
// two adjacent 1s ("100" = "011"). That is not a formatting rule - it is the
// same law the ledger uses, so a drive, a mood and a decision can be written as
// one numeral and read back to the value they came from.
//
// This is what a phinary AI is: not a flag that is on or off, but a numeral that
// carries its own weight in every place. Nothing here rounds to "true".
#pragma once

#include "CoreMinimal.h"

namespace VEILPhinary
{
	/** phi, computed once. */
	extern const double Phi;

	/**
	 * Writes Value in base phi. Greedy place-taking produces the canonical form
	 * (no two adjacent 1s) directly; the part past the point is truncated at
	 * FracDigits places. Sign is a leading '-'.
	 */
	FString Encode(double Value, int32 FracDigits = 32);

	/** Reads a base-phi numeral back to a double. */
	double Decode(const FString& Digits);

	/** True when the numeral has no two adjacent 1s - the canonical spelling. */
	bool IsCanonical(const FString& Digits);

	/** |value - decode(encode(value))|. The round-trip the ledger asks for. */
	double RoundTripError(double Value, int32 FracDigits = 32);
}
