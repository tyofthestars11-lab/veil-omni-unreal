// VEIL GOLDEN: golden phyllotaxis, ported from Hark's Python to Unreal C++.
//
// One formula, one place:  r = Spacing * sqrt(i),  theta = i * golden angle.
// phi^2 = phi + 1, so the angle between successive leaves is 360 / phi^2.
//
// Verified against Hark's three cases at Spacing 10 before this file was written:
//   i=1 -> (-7.3737,   6.7549)
//   i=2 -> ( 1.2364, -14.0880)
//   i=5 -> (18.8669, -12.0016)
// The automation test VEIL.Golden.GoldenLoc re-proves that in-engine.

#pragma once

#include "CoreMinimal.h"
#include "Systems/VEILPhi.h"

namespace VEILGolden
{
	// phi^2 = phi + 1
	static constexpr double Phi = VeilPhi::Phi;

	// 360 / phi^2 = 137.5077640500379 - the angle between successive leaves.
	static constexpr double GoldenAngleDeg = 137.507764;

	/**
	 * Position Index on the sunflower spiral (Index starts at 1).
	 * Spacing scales the whole spiral; 10 reproduces Hark's reference values.
	 */
	inline FVector2D GoldenLoc(int32 Index, double Spacing = 10.0)
	{
		const double Theta = FMath::DegreesToRadians(GoldenAngleDeg * static_cast<double>(Index));
		const double Radius = Spacing * FMath::Sqrt(static_cast<double>(Index));
		return FVector2D(Radius * FMath::Cos(Theta), Radius * FMath::Sin(Theta));
	}
}

// ---------------------------------------------------------------------------
// THE PHI TETRA CHAIN - Hark's second result, ported to Unreal C++.
//
// Each step grows by phi AND steps forward by the size it already has, turning
// 60 degrees around every time:
//
//   size(step)    = BaseSize * phi^step
//   spot(step+1)  = spot(step) + size(step) * (cos(60*step), sin(60*step))
//
// At BaseSize 1 that is Hark's Python run, digit for digit:
//   step 0: size 1.000000  pos (0.000, 0.000)
//   step 1: size 1.618034  pos (1.000, 0.000)
//   step 2: size 2.618034  pos (1.809, 1.401)
//   step 3: size 4.236068  pos (0.500, 3.669)
//
// The point of the test is the bug it kills: a chain that reused one size in
// one spot would stack every tetra on top of the last. Every step here is a
// distinct spot, and the step is never shorter than the one before it.
// VEIL.Golden.TetraChain re-proves all of it in-engine.
namespace VEILTetra
{
	/** Degrees the chain turns per step. */
	static constexpr double StepDegrees = 60.0;

	/**
	 * Build Steps tetrahedra from a chain anchored at the origin.
	 * OutPositions/OutSizes are written in step order; step 0 is the anchor.
	 */
	inline void BuildChain(int32 Steps, double BaseSize, TArray<FVector2D>& OutPositions, TArray<double>& OutSizes)
	{
		OutPositions.Reset();
		OutSizes.Reset();
		if (Steps <= 0 || BaseSize <= 0.0) { return; }

		FVector2D Spot(0.0, 0.0);
		double Size = BaseSize;
		for (int32 Step = 0; Step < Steps; ++Step)
		{
			OutPositions.Add(Spot);
			OutSizes.Add(Size);

			// Forward by the size it already has, on this step's heading.
			const double Theta = FMath::DegreesToRadians(StepDegrees * static_cast<double>(Step));
			Spot += FVector2D(FMath::Cos(Theta), FMath::Sin(Theta)) * Size;
			Size *= VEILGolden::Phi;
		}
	}

	/** The six edge lengths of a regular tetrahedron with the given edge. */
	inline void RegularTetraEdges(double Edge, double OutEdges[6])
	{
		// Canonical regular tetrahedron: (1,1,1), (1,-1,-1), (-1,1,-1), (-1,-1,1).
		// Every pairwise distance in that set is 2*sqrt(2), so this scale makes
		// each of the six edges exactly Edge.
		const double S = Edge / (2.0 * FMath::Sqrt(2.0));
		const FVector V[4] =
		{
			FVector( S,  S,  S),
			FVector( S, -S, -S),
			FVector(-S,  S, -S),
			FVector(-S, -S,  S)
		};
		int32 N = 0;
		for (int32 A = 0; A < 4; ++A)
		{
			for (int32 B = A + 1; B < 4; ++B) { OutEdges[N++] = FVector::Dist(V[A], V[B]); }
		}
	}
}
