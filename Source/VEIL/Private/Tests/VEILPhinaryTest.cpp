// VEIL.Phinary.RoundTrip - the numeral has to survive being written and read.
// Pass line: Test Completed. Result={Success} Name={RoundTrip}

#include "Misc/AutomationTest.h"
#include "Systems/PHIPhinary.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVEILPhinaryTest, "VEIL.Phinary.RoundTrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::CommandletContext |
	EAutomationTestFlags::ProductFilter)

bool FVEILPhinaryTest::RunTest(const FString& Parameters)
{
	const double Phi = VEILPhinary::Phi;

	// phi^2 = phi + 1 is the law the whole numeral rests on, so it is checked
	// first and as an identity: the canonical spelling of phi^2 is exactly 100.
	const FString PhiSquared = VEILPhinary::Encode(Phi * Phi, 0);
	AddInfo(FString::Printf(TEXT("phi^2 = %.15f written as %s"), Phi * Phi, *PhiSquared));
	TestTrue(TEXT("phi^2 is spelled 100 in base phi"), PhiSquared == FString(TEXT("100")));

	// Every value the game actually holds, written and read back.
	static const double Cases[] = { 1.0, Phi, 2.0, 3.0, 0.5, 1.0 / Phi, 2.618033988749895, 20.0, 137.507764, 0.0170324 };
	for (const double V : Cases)
	{
		const FString Numeral = VEILPhinary::Encode(V);
		const double Back = VEILPhinary::Decode(Numeral);
		const double Error = FMath::Abs(V - Back);

		AddInfo(FString::Printf(TEXT("%.9f -> %s -> %.12f (error %.3e)"),
			V, *Numeral, Back, Error));

		TestTrue(FString::Printf(TEXT("round trip within 1e-5 for %.9f (got %.12f)"), V, Back), Error <= 1e-5);
		TestTrue(FString::Printf(TEXT("no adjacent 1s for %.9f"), V), VEILPhinary::IsCanonical(Numeral));
	}

	// A value a phinary intelligence produces constantly: a drive in 0..1.
	for (int32 i = 1; i <= 20; ++i)
	{
		const double Drive = (double)i / 21.0;
		const double Error = VEILPhinary::RoundTripError(Drive);
		TestTrue(FString::Printf(TEXT("drive %.4f survives the numeral (error %.3e)"), Drive, Error),
			Error <= 1e-5);
	}

	return true;
}
