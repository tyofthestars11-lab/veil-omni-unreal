// PHILayout.h - phi as the layout framework, header-only like PHIMito.
//
// Every placement in this project used to carry its own number: a radius here, an angle there,
// a spacing constant per call site. That is how two passes ended up building in the same band.
// One framework instead: the golden angle decides direction, phi powers decide distance, and a
// spacing audit can say out loud whether a layout is crowded.
//
// Values are the locked ones: phi = 1.618033988749895, golden angle = 137.5077640500378.

#pragma once

#include "CoreMinimal.h"

namespace PHILayout
{
	/** The locked ratio. */
	constexpr double Phi = 1.618033988749895;

	/** The locked golden angle in degrees. */
	constexpr double GoldenDeg = 137.5077640500378;

	/** phi^N as a float for placement maths. */
	inline float Power(int32 N)
	{
		return (float)FMath::Pow(Phi, (double)N);
	}

	/** The direction an item takes: index * golden angle, plus a per-family offset so two
	 *  families at the same index do not sit on the same ray. */
	inline float Yaw(int32 Index, float OffsetDeg = 0.f)
	{
		return OffsetDeg + (float)((double)Index * GoldenDeg);
	}

	/** A radius inside a band: BaseCm * phi^(Index % Steps). Steps keeps a family inside its
	 *  own annulus, which is what stops one pass building inside another's ground. */
	inline float Band(int32 Index, float BaseCm, int32 Steps = 4)
	{
		if (Steps <= 0) { Steps = 1; }
		return BaseCm * Power(Index % Steps);
	}

	/** A point on the golden spiral in the ground plane. */
	inline FVector Spiral(int32 Index, float BaseCm, float OffsetDeg = 0.f, int32 Steps = 4,
		float Zcm = 0.f)
	{
		const float Ang = FMath::DegreesToRadians(Yaw(Index, OffsetDeg));
		const float Rad = Band(Index, BaseCm, Steps);
		return FVector(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, Zcm);
	}

	/** What a spacing audit found. */
	struct FSpacingReport
	{
		float MinGapCm = 1.0e9f;
		int32 Pairs = 0;
		int32 TooClose = 0;
		int32 Count = 0;

		void Measure(const TArray<FVector>& Points, float RequiredCm = 120.f)
		{
			Count = Points.Num();
			MinGapCm = 1.0e9f;
			Pairs = 0;
			TooClose = 0;
			for (int32 A = 0; A < Points.Num(); ++A)
			{
				for (int32 B = A + 1; B < Points.Num(); ++B)
				{
					const float D = (float)FVector::Dist2D(Points[A], Points[B]);
					MinGapCm = FMath::Min(MinGapCm, D);
					++Pairs;
					if (D < RequiredCm) { ++TooClose; }
				}
			}
			if (Count < 2) { MinGapCm = 0.f; }
		}

		bool IsClean() const { return TooClose == 0; }
	};
}
