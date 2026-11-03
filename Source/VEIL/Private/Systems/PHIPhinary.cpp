#include "Systems/PHIPhinary.h"
#include "Systems/VEILPhi.h"

namespace VEILPhinary
{
	const double Phi = VeilPhi::Phi;

	namespace
	{
		// Highest place written: phi^10 covers every drive this game holds. The
		// fraction depth is the caller's; 32 places reaches ~2e-7 of a unit.
		constexpr int32 TopExponent = 16;

		double Place(int32 Exponent)
		{
			return FMath::Pow(Phi, (double)Exponent);
		}
	}

	FString Encode(double Value, int32 FracDigits)
	{
		const bool bNegative = Value < 0.0;
		double V = FMath::Abs(Value);

		TArray<int32> Digits;
		const int32 Count = TopExponent + 1 + FMath::Max(0, FracDigits);
		Digits.SetNumZeroed(Count);

		// Greedy place-taking, high to low. In base phi this is the canonical
		// spelling by construction: taking the largest place that fits can never
		// leave the next place also taken, because that pair would be a larger
		// single place (phi^n = phi^(n-1) + phi^(n-2)).
		for (int32 i = 0; i < Count; ++i)
		{
			const int32 Exponent = TopExponent - i;
			const double P = Place(Exponent);
			if (V + 1e-12 >= P)
			{
				Digits[i] = 1;
				V -= P;
			}
		}

		// The point belongs where the place values turn negative, whatever the
		// leading zeros did. Trimming must never move it: a numeral below 1 starts
		// with its own "0." so the reader knows where the units place is.
		FString IntPart, FracPart;
		for (int32 i = 0; i <= TopExponent; ++i) { IntPart += Digits[i] ? TEXT("1") : TEXT("0"); }
		for (int32 i = TopExponent + 1; i < Count; ++i) { FracPart += Digits[i] ? TEXT("1") : TEXT("0"); }

		int32 Lead = 0;
		while (Lead < IntPart.Len() - 1 && IntPart[Lead] == TEXT('0')) { ++Lead; }
		IntPart = IntPart.RightChop(Lead);
		while (!FracPart.IsEmpty() && FracPart.EndsWith(TEXT("0"))) { FracPart.LeftChopInline(1); }

		FString Out;
		if (bNegative) { Out += TEXT("-"); }
		Out += IntPart;
		if (!FracPart.IsEmpty()) { Out += TEXT("."); Out += FracPart; }
		return Out;
	}

	double Decode(const FString& Digits)
	{
		double Sign = 1.0;
		int32 Start = 0;
		if (Digits.StartsWith(TEXT("-")))
		{
			Sign = -1.0;
			Start = 1;
		}

		// Encode trims leading zeros, so the first digit's place is set by how many
		// digits stand before the point: "100.0" starts at phi^2, ".10" at phi^-1.
		const int32 PointAt = Digits.Find(TEXT("."), ESearchCase::IgnoreCase, ESearchDir::FromStart, Start);
		const int32 IntegerDigits = (PointAt == INDEX_NONE ? Digits.Len() : PointAt) - Start;
		double Value = 0.0;
		int32 Exponent = IntegerDigits - 1;
		for (int32 i = Start; i < Digits.Len(); ++i)
		{
			const TCHAR C = Digits[i];
			if (C == TEXT('.')) { continue; }
			if (C == TEXT('1')) { Value += Place(Exponent); }
			--Exponent;
		}
		return Sign * Value;
	}

	bool IsCanonical(const FString& Digits)
	{
		bool bPreviousWasOne = false;
		for (int32 i = 0; i < Digits.Len(); ++i)
		{
			const TCHAR C = Digits[i];
			if (C == TEXT('.') || C == TEXT('-')) { continue; }
			const bool bOne = (C == TEXT('1'));
			if (bOne && bPreviousWasOne) { return false; }
			bPreviousWasOne = bOne;
		}
		return true;
	}

	double RoundTripError(double Value, int32 FracDigits)
	{
		return FMath::Abs(Value - Decode(Encode(Value, FracDigits)));
	}
}
