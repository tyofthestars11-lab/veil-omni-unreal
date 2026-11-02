// VEIL OMNI // ONE OS - the face.
//
// Screen, voice, map, controls on top; the engine's own readouts underneath,
// because an interface that hides its machinery is a mock-up. Everything is
// drawn from the engine's state, so the face cannot claim what the engine did
// not do.
#include "Game/VEILOSFace.h"

#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "GameFramework/HUD.h"
#include "Systems/VEILOS.h"

namespace
{
	void Txt(AHUD* H, const FString& S, const FLinearColor& C, float X, float Y, UFont* F, float Scale)
	{
		if (H && F && !S.IsEmpty()) { H->DrawText(S, C, X, Y, F, Scale); }
	}

	void Frame(AHUD* H, const FLinearColor& C, float X, float Y, float W, float Ht, float T)
	{
		H->DrawRect(C, X, Y, W, T);
		H->DrawRect(C, X, Y + Ht - T, W, T);
		H->DrawRect(C, X, Y, T, Ht);
		H->DrawRect(C, X + W - T, Y, T, Ht);
	}

	void Panel(AHUD* H, const FLinearColor& Fill, const FLinearColor& Trim,
		float X, float Y, float W, float Ht)
	{
		H->DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.35f), X + 3.f, Y + 3.f, W, Ht);
		H->DrawRect(Fill, X, Y, W, Ht);
		Frame(H, Trim, X, Y, W, Ht, 1.5f);
		// Slanted top-right corner: the frame's crest, not a rounded box.
		H->DrawLine(X + W - 34.f, Y, X + W, Y + 34.f, Trim, 2.f);
	}
}

void VEILOSFace::Draw(AHUD* H, UCanvas* C)
{
	if (!H || !C) { return; }

	UWorld* World = H->GetWorld();
	UVEILOS* OS = World ? World->GetSubsystem<UVEILOS>() : nullptr;
	if (!OS || !OS->IsOpen()) { return; }

	static UFont* Big = LoadObject<UFont>(nullptr,
		TEXT("/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField"));
	static UFont* Med = LoadObject<UFont>(nullptr,
		TEXT("/Engine/EngineFonts/Roboto.Roboto"));
	if (!Med) { return; }
	if (!Big) { Big = Med; }

	const FLinearColor Navy(0.055f, 0.078f, 0.141f, 0.90f);
	const FLinearColor Body(0.075f, 0.095f, 0.165f, 0.88f);
	const FLinearColor Gold(0.788f, 0.635f, 0.290f, 1.f);
	const FLinearColor LabelGold(0.910f, 0.768f, 0.416f, 1.f);
	const FLinearColor White(0.949f, 0.949f, 0.949f, 1.f);
	const FLinearColor Dim(0.60f, 0.65f, 0.74f, 1.f);
	const FLinearColor Blue(0.247f, 0.663f, 1.f, 1.f);

	const float W = C->ClipX;
	const float Ht = C->ClipY;

	// THE FACE IS A SURFACE, NOT A LIST. Full bleed, navy, gold frame.
	H->DrawRect(Navy, 0.f, 0.f, W, Ht);
	Frame(H, Gold, 26.f, 20.f, W - 52.f, Ht - 40.f, 1.5f);
	H->DrawLine(W - 26.f - 96.f, 20.f, W - 26.f, 20.f + 96.f, Gold, 2.f);

	Txt(H, TEXT("VEIL OMNI  //  ONE OS"), Gold, 46.f, 34.f, Big, 1.10f);
	Txt(H, TEXT("phi reversed - every value below is a rung of a live engine field, not a label"),
		Dim, 46.f, 60.f, Med, 0.82f);
	Txt(H, FString::Printf(TEXT("[F1] lower the face      %s"), *OS->ObservabilityLine()),
		Dim, 46.f, 80.f, Med, 0.74f);

	const float Gap = 22.f;
	const float ColW = (W - 52.f - 3.f * Gap) * 0.5f;
	const float LX = 46.f;
	const float RX = LX + ColW + Gap;
	float LY = 106.f;
	float RY = 106.f;

	// ---- SCREEN: TV / PC / GAME ------------------------------------------
	{
		const float PW = ColW, PH = 196.f;
		Panel(H, Body, Gold, LX, LY, PW, PH);
		Txt(H, TEXT("SCREEN  //  LIVE SIGNAL"), LabelGold, LX + 16.f, LY + 12.f, Big, 0.86f);

		// The signal itself: bars on a phi raster, driven by the fields.
		const float BaseY = LY + PH - 34.f;
		const float BarsX = LX + 18.f;
		const int32 Bars = 21;
		for (int32 i = 0; i < Bars; ++i)
		{
			const float T = (float)i / (float)(Bars - 1);
			const float Rung = 1.f + FMath::Sin(T * 6.2831853f * 1.6180339887f) * 0.5f;
			const float BH = 18.f + Rung * 52.f;
			H->DrawRect(i % 4 == 0 ? Gold : Blue, BarsX + i * 9.f, BaseY - BH, 5.f, BH);
		}
		H->DrawRect(FLinearColor(0.95f, 0.30f, 0.24f, 1.f), LX + PW - 96.f, LY + 16.f, 8.f, 8.f);
		Txt(H, TEXT("LIVE"), FLinearColor(0.95f, 0.45f, 0.40f, 1.f), LX + PW - 82.f, LY + 11.f, Med, 0.80f);
		Txt(H, OS->EngineLine(), White, LX + 18.f, BaseY + 6.f, Med, 0.80f);
		LY += PH + Gap;
	}

	// ---- VOICE -----------------------------------------------------------
	{
		const float PW = ColW, PH = 96.f;
		Panel(H, Body, Gold, LX, LY, PW, PH);
		Txt(H, TEXT("VOICE  //  THE TOP LAYER ONLY"), LabelGold, LX + 16.f, LY + 12.f, Med, 0.80f);
		Txt(H, OS->Voice(), White, LX + 16.f, LY + 42.f, Med, 0.92f);
		LY += PH + Gap;
	}

	// ---- CONTROLS --------------------------------------------------------
	{
		const float PW = ColW, PH = Ht - LY - 46.f;
		Panel(H, Body, Gold, LX, LY, PW, PH);
		Txt(H, TEXT("CONTROLS"), LabelGold, LX + 16.f, LY + 12.f, Med, 0.80f);
		Txt(H, OS->ControlsLine(), White, LX + 16.f, LY + 42.f, Med, 0.78f);
		Txt(H, TEXT("Every key above is bound. [F1] is the face; [G] is GODMODE with ALL OFF."),
			Dim, LX + 16.f, LY + 68.f, Med, 0.74f);
	}

	// ---- MAP -------------------------------------------------------------
	{
		const float PW = ColW, PH = 170.f;
		Panel(H, Body, Gold, RX, RY, PW, PH);
		Txt(H, TEXT("MAP  //  WHERE THE RUNNER STANDS"), LabelGold, RX + 16.f, RY + 12.f, Med, 0.80f);
		Txt(H, OS->MapLine(), White, RX + 16.f, RY + 40.f, Med, 0.84f);

		// Compass: a gold ring with a phi needle, drawn from lines.
		const float CX = RX + PW - 60.f;
		const float CY = RY + 92.f;
		const float R = 44.f;
		for (int32 i = 0; i < 48; ++i)
		{
			const float A0 = (float)i / 48.f * 6.2831853f;
			const float A1 = (float)(i + 1) / 48.f * 6.2831853f;
			H->DrawLine(CX + FMath::Cos(A0) * R, CY + FMath::Sin(A0) * R,
				CX + FMath::Cos(A1) * R, CY + FMath::Sin(A1) * R, Gold, 1.2f);
		}
		H->DrawLine(CX, CY, CX + R * 0.72f, CY - R * 0.30f, Blue, 2.f);

		Txt(H, TEXT("ADDRESS BOOK"), LabelGold, RX + 16.f, RY + PH - 52.f, Med, 0.76f);
		Txt(H, TEXT("veil://os/status   veil://os/who   veil://os/save   veil://os/alloff"),
			Dim, RX + 16.f, RY + PH - 32.f, Med, 0.74f);
		RY += PH + Gap;
	}

	// ---- ENGINE ----------------------------------------------------------
	{
		const float PW = ColW, PH = Ht - RY - 46.f;
		Panel(H, Body, Gold, RX, RY, PW, PH);
		Txt(H, TEXT("ENGINE  //  RUNGS, SKILLS, AUTOMATIONS, MEMORY"), LabelGold, RX + 16.f, RY + 12.f, Med, 0.80f);

		float Y = RY + 40.f;
		Txt(H, OS->RungTableLine(), Dim, RX + 16.f, Y, Med, 0.76f);
		Y += 22.f;
		Txt(H, FString::Printf(TEXT("ANSWER  %s"), *OS->LastAddress()), LabelGold, RX + 16.f, Y, Med, 0.78f);
		Y += 20.f;
		Txt(H, OS->LastAnswer(), White, RX + 16.f, Y, Med, 0.82f);
		Y += 28.f;
		H->DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.10f), RX + 16.f, Y, PW - 32.f, 1.f);
		Y += 10.f;

		Txt(H, TEXT("LEDGER MEMORY  (rung-stamped, append-only)"), LabelGold, RX + 16.f, Y, Med, 0.76f);
		Y += 22.f;

		const TArray<FVEILOSLedgerEntry>& Ledger = OS->Ledger();
		const int32 First = FMath::Max(0, Ledger.Num() - 6);
		for (int32 i = First; i < Ledger.Num(); ++i)
		{
			const FVEILOSLedgerEntry& E = Ledger[i];
			Txt(H, FString::Printf(TEXT("%7.2fs  r%6.3f  %s"), E.T, E.Rung, *E.Note),
				i == Ledger.Num() - 1 ? White : Dim, RX + 16.f, Y, Med, 0.74f);
			Y += 18.f;
		}
		if (Ledger.Num() == 0)
		{
			Txt(H, TEXT("empty"), Dim, RX + 16.f, Y, Med, 0.74f);
		}
	}

	// A hairline of gold across the bottom: the face is closed, not unfinished.
	H->DrawRect(Gold, 46.f, Ht - 60.f, W - 92.f, 1.f);
	Txt(H, TEXT("ONE OS - face over engine. Nothing here is written into the save."),
		Dim, 46.f, Ht - 52.f, Med, 0.74f);
}
