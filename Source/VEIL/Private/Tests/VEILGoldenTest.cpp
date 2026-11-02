// VEIL.Golden.GoldenLoc - Hark's three reference cases, re-proved in engine.
// VEIL.Golden.TetraChain - Hark's tetra chain, re-proved in engine.
// Pass line: Test Completed. Result={Success} Name={GoldenLoc}
//            Test Completed. Result={Success} Name={TetraChain}

#include "Misc/AutomationTest.h"
#include "VEILGolden.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVEILGoldenLocTest, "VEIL.Golden.GoldenLoc",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext |
	EAutomationTestFlags::ProductFilter)

bool FVEILGoldenLocTest::RunTest(const FString& Parameters)
{
	static const double Tolerance = 0.01;   // Hark's tolerance

	struct FGoldenCase
	{
		int32 Index;
		double ExpectX;
		double ExpectY;
	};

	static const FGoldenCase Cases[] =
	{
		{ 1,  -7.3737,   6.7549 },
		{ 2,   1.2364, -14.0880 },
		{ 5,  18.8669, -12.0016 },
	};

	for (const FGoldenCase& Case : Cases)
	{
		const FVector2D P = VEILGolden::GoldenLoc(Case.Index, 10.0);

		AddInfo(FString::Printf(TEXT("i=%d -> (%.4f, %.4f), expected (%.4f, %.4f)"),
			Case.Index, P.X, P.Y, Case.ExpectX, Case.ExpectY));

		TestTrue(
			FString::Printf(TEXT("GoldenLoc(%d).X within %.2f (got %.4f, want %.4f)"),
				Case.Index, Tolerance, P.X, Case.ExpectX),
			FMath::Abs(P.X - Case.ExpectX) <= Tolerance);

		TestTrue(
			FString::Printf(TEXT("GoldenLoc(%d).Y within %.2f (got %.4f, want %.4f)"),
				Case.Index, Tolerance, P.Y, Case.ExpectY),
			FMath::Abs(P.Y - Case.ExpectY) <= Tolerance);
	}

	return true;
}

// ---------------------------------------------------------------------------
// Hark's tetra chain: a size-1 tetrahedron with all six edges equal, then each
// step growing by phi while stepping forward by the size it already had, 60
// degrees around. The bug this kills is the old one: a chain that reset to the
// same size in the same spot, which stacked every tetra on the last.
// ---------------------------------------------------------------------------
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVEILTetraChainTest, "VEIL.Golden.TetraChain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext |
	EAutomationTestFlags::ProductFilter)

bool FVEILTetraChainTest::RunTest(const FString& Parameters)
{
	static const double Tolerance = 0.001;   // the chain is read to 3 decimals

	// 1. The anchor tetrahedron's six edges, at edge 1.0.
	{
		double Edges[6];
		VEILTetra::RegularTetraEdges(1.0, Edges);
		for (int32 i = 0; i < 6; ++i)
		{
			AddInfo(FString::Printf(TEXT("edge %d = %.6f (want 1.000000)"), i + 1, Edges[i]));
			TestTrue(FString::Printf(TEXT("tetra edge %d is 1.0 (got %.6f)"), i + 1, Edges[i]),
				FMath::Abs(Edges[i] - 1.0) <= 1.0e-9);
		}
	}

	// 2. The chain itself, at BaseSize 1 - Hark's Python run.
	struct FChainCase
	{
		int32 Step;
		double ExpectSize;
		double ExpectX;
		double ExpectY;
	};

	static const FChainCase Cases[] =
	{
		{ 0, 1.000000, 0.000, 0.000 },
		{ 1, 1.618034, 1.000, 0.000 },
		{ 2, 2.618034, 1.809, 1.401 },
		{ 3, 4.236068, 0.500, 3.669 },
	};

	TArray<FVector2D> Spots;
	TArray<double> Sizes;
	VEILTetra::BuildChain(4, 1.0, Spots, Sizes);

	TestEqual(TEXT("chain step count"), Spots.Num(), 4);
	TestEqual(TEXT("chain size count"), Sizes.Num(), 4);

	for (const FChainCase& Case : Cases)
	{
		if (!Spots.IsValidIndex(Case.Step) || !Sizes.IsValidIndex(Case.Step))
		{
			AddError(FString::Printf(TEXT("step %d missing from the chain"), Case.Step));
			continue;
		}

		const FVector2D P = Spots[Case.Step];
		const double Size = Sizes[Case.Step];

		AddInfo(FString::Printf(TEXT("step %d: size %.6f pos (%.3f, %.3f) - expected size %.6f pos (%.3f, %.3f)"),
			Case.Step, Size, P.X, P.Y, Case.ExpectSize, Case.ExpectX, Case.ExpectY));

		TestTrue(FString::Printf(TEXT("step %d size is %.6f (got %.6f)"), Case.Step, Case.ExpectSize, Size),
			FMath::Abs(Size - Case.ExpectSize) <= Tolerance);
		TestTrue(FString::Printf(TEXT("step %d X is %.3f (got %.3f)"), Case.Step, Case.ExpectX, P.X),
			FMath::Abs(P.X - Case.ExpectX) <= Tolerance);
		TestTrue(FString::Printf(TEXT("step %d Y is %.3f (got %.3f)"), Case.Step, Case.ExpectY, P.Y),
			FMath::Abs(P.Y - Case.ExpectY) <= Tolerance);
	}

	// 3. No stacking: every spot distinct, and the step never shrinks.
	for (int32 A = 0; A < Spots.Num(); ++A)
	{
		for (int32 B = A + 1; B < Spots.Num(); ++B)
		{
			const double Gap = FVector2D::Distance(Spots[A], Spots[B]);
			TestTrue(FString::Printf(TEXT("step %d and %d are distinct (gap %.3f)"), A, B, Gap), Gap > 0.5);
		}
		if (A > 0)
		{
			TestTrue(FString::Printf(TEXT("step %d is larger than step %d (%.6f > %.6f)"), A, A - 1, Sizes[A], Sizes[A - 1]),
				Sizes[A] > Sizes[A - 1]);
		}
	}

	// 4. NO CEILING. 34 steps is not "too big to express" - it is 33 rungs up.
	//    The chain itself carries every step at its true size, and reading it in
	//    one rung band brings the whole thing back into the world span a 6-step
	//    chain already uses. Nothing is truncated to make it fit.
	{
		TArray<FVector2D> LongSpots;
		TArray<double> LongSizes;
		VEILTetra::BuildChain(34, 1.0, LongSpots, LongSizes);

		TestEqual(TEXT("a 34-step chain exists"), LongSizes.Num(), 34);
		if (LongSizes.Num() == 34)
		{
			const double GrownRungs = FMath::Loge(LongSizes[33] / LongSizes[0]) / FMath::Loge(VEILGolden::Phi);
			AddInfo(FString::Printf(TEXT("34 steps: size 1 -> %.3e, %.4f rungs, band phi^33 = %.3e"),
				LongSizes[33], GrownRungs, FMath::Pow(VEILGolden::Phi, 33.0)));

			TestTrue(TEXT("34 steps is exactly 33 rungs of growth"), FMath::Abs(GrownRungs - 33.0) <= 1.0e-6);

			// Read in band 33: every spot comes back inside the same span as the
			// 4-step chain, and every size comes back to 1.0 at the anchor.
			const double Band = FMath::Pow(VEILGolden::Phi, 33.0);
			const FVector2D LastBanded = FVector2D(LongSpots[33].X / Band, LongSpots[33].Y / Band);
			const double LastSizeBanded = LongSizes[33] / Band;
			AddInfo(FString::Printf(TEXT("34 steps in band 33: last spot (%.3f, %.3f) size %.6f, span %.3f"),
				LastBanded.X, LastBanded.Y, LastSizeBanded, LastBanded.Size()));

			TestTrue(TEXT("band 33 brings the last step back to the unit size"),
				FMath::Abs(LastSizeBanded - 1.0) <= 1.0e-6);
			TestTrue(TEXT("band 33 keeps the 34-step chain inside a 4-step world span"),
				LastBanded.Size() <= 4.236068 + Tolerance);
			TestTrue(TEXT("every one of the 34 spots is distinct"),
				FVector2D::Distance(LongSpots[32], LongSpots[33]) > 1.0e-6);
		}
	}

	// 5. The same chain at world scale: 20 cm anchor, so the spawner's numbers
	//    are the unit chain scaled, not a second set of formulas.
	{
		TArray<FVector2D> WorldSpots;
		TArray<double> WorldSizes;
		VEILTetra::BuildChain(4, 20.0, WorldSpots, WorldSizes);
		if (WorldSizes.Num() == 4)
		{
			AddInfo(FString::Printf(TEXT("20cm chain: %.2f -> %.2f cm"), WorldSizes[0], WorldSizes[3]));
			TestTrue(TEXT("20cm chain grows by phi"), FMath::Abs(WorldSizes[1] / WorldSizes[0] - VEILGolden::Phi) <= 1.0e-6);
			TestTrue(TEXT("20cm chain spots are the unit chain x20"),
				FMath::Abs(WorldSpots[2].X - Spots[2].X * 20.0) <= Tolerance * 20.0);
		}
	}

	return true;
}
