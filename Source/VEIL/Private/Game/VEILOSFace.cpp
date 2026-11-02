// VEIL OMNI // ONE OS - the face.
//
// Seven labelled parts, drawn from the engine's own state so the face cannot
// claim what the engine did not do:
//   DOMAINS (rung fields)   TASKS (addresses)   SKILLS (round-tripped rungs)
//   AUTOMATIONS (on change)   MEMORY (phi ledger)
//   VOICE DOOR (Brain / Eyes / Seat / Hands)   PIPELINE (plan -> review -> execute)
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
		H->DrawLine(X + W - 30.f, Y, X + W, Y + 30.f, Trim, 2.f);
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

	const FLinearColor Navy(0.055f, 0.078f, 0.141f, 0.92f);
	const FLinearColor Body(0.075f, 0.095f, 0.165f, 0.90f);
	const FLinearColor Gold(0.788f, 0.635f, 0.290f, 1.f);
	const FLinearColor LabelGold(0.910f, 0.768f, 0.416f, 1.f);
	const FLinearColor White(0.949f, 0.949f, 0.949f, 1.f);
	const FLinearColor Dim(0.60f, 0.65f, 0.74f, 1.f);
	const FLinearColor Blue(0.247f, 0.663f, 1.f, 1.f);
	const FLinearColor Green(0.184f, 0.702f, 0.416f, 1.f);

	const float W = C->ClipX;
	const float Ht = C->ClipY;

	H->DrawRect(Navy, 0.f, 0.f, W, Ht);
	Frame(H, Gold, 24.f, 18.f, W - 48.f, Ht - 36.f, 1.5f);
	H->DrawLine(W - 24.f - 92.f, 18.f, W - 24.f, 18.f + 92.f, Gold, 2.f);

	Txt(H, TEXT("VEIL OMNI  //  ONE OS"), Gold, 42.f, 28.f, Big, 1.05f);
	Txt(H, TEXT("phi reversed - domains, tasks, skills, automations, memory, and a voice door that plans before it moves"),
		Dim, 42.f, 52.f, Med, 0.76f);
	Txt(H, FString::Printf(TEXT("[F1] lower the face      %s"), *OS->ObservabilityLine()),
		Dim, 42.f, 70.f, Med, 0.70f);

	const float Gap = 18.f;
	const float ColW = (W - 48.f - 3.f * Gap) * 0.5f;
	const float LX = 40.f;
	const float RX = LX + ColW + Gap;
	float LY = 92.f;
	float RY = 92.f;
	const float RowH = 17.f;

	// ---- DOMAINS // RUNG FIELDS ------------------------------------------
	{
		const float PW = ColW, PH = 128.f;
		Panel(H, Body, Gold, LX, LY, PW, PH);
		Txt(H, TEXT("DOMAINS  //  RUNG FIELDS"), LabelGold, LX + 14.f, LY + 9.f, Big, 0.72f);
		Txt(H, TEXT("each value read as log_phi"), Dim, LX + 14.f, LY + 30.f, Med, 0.66f);

		float Y = LY + 48.f;
		for (const FVEILOSDomain& D : OS->Domains())
		{
			Txt(H, FString::Printf(TEXT("%-7s %8.0f   r%7.3f"), *D.Key, D.Value, D.Rung),
				White, LX + 14.f, Y, Med, 0.72f);
			Txt(H, D.Address, Dim, LX + PW - 150.f, Y, Med, 0.60f);
			Y += RowH;
		}
		LY += PH + Gap;
	}

	// ---- SKILLS // ROUND-TRIPPED RUNGS -----------------------------------
	{
		const float PW = ColW, PH = 108.f;
		Panel(H, Body, Gold, LX, LY, PW, PH);
		Txt(H, TEXT("SKILLS  //  ROUND-TRIPPED RUNGS"), LabelGold, LX + 14.f, LY + 9.f, Big, 0.72f);
		Txt(H, TEXT("rung out, parsed back, compared to the live value"), Dim, LX + 14.f, LY + 30.f, Med, 0.66f);

		float Y = LY + 50.f;
		for (const FVEILOSSkill& S : OS->Skills())
		{
			Txt(H, FString::Printf(TEXT("%-9s r%7.3f"), *S.Name.ToUpper(), S.Rung),
				White, LX + 14.f, Y, Med, 0.72f);
			Txt(H, S.bRoundTripped ? TEXT("ROUND-TRIPPED") : TEXT("DRIFT"),
				S.bRoundTripped ? Green : FLinearColor(0.95f, 0.30f, 0.24f), LX + 190.f, Y, Med, 0.68f);
			Y += RowH;
		}
		LY += PH + Gap;
	}

	// ---- TASKS // ADDRESSES ----------------------------------------------
	{
		const float PW = ColW, PH = Ht - LY - 40.f;
		Panel(H, Body, Gold, LX, LY, PW, PH);
		Txt(H, TEXT("TASKS  //  ADDRESSES"), LabelGold, LX + 14.f, LY + 9.f, Big, 0.72f);

		float Y = LY + 34.f;
		for (const FVEILOSTask& T : OS->Tasks())
		{
			Txt(H, T.Address, White, LX + 14.f, Y, Med, 0.70f);
			Txt(H, T.State, T.State.StartsWith(TEXT("answered")) ? Green : Dim, LX + PW - 120.f, Y, Med, 0.66f);
			Y += RowH;
		}
		Y += 8.f;
		Txt(H, TEXT("VOICE"), LabelGold, LX + 14.f, Y, Med, 0.68f);
		Y += RowH + 2.f;
		Txt(H, OS->Voice(), White, LX + 14.f, Y, Med, 0.74f);
	}

	// ---- VOICE DOOR -------------------------------------------------------
	{
		const float PW = ColW, PH = 150.f;
		Panel(H, Body, Gold, RX, RY, PW, PH);
		Txt(H, TEXT("VOICE DOOR  //  BRAIN  EYES  SEAT  HANDS"), LabelGold, RX + 14.f, RY + 9.f, Big, 0.72f);
		Txt(H, OS->GateLine(), White, RX + 14.f, RY + 34.f, Med, 0.74f);

		const FVEILOSPlan& P = OS->CurrentPlan();
		Txt(H, FString::Printf(TEXT("plan: %s  (%d steps)"),
			P.Intent.IsEmpty() ? TEXT("none yet") : *P.Intent, P.Steps.Num()),
			Dim, RX + 14.f, RY + 56.f, Med, 0.68f);

		float Y = RY + 76.f;
		const int32 Show = FMath::Min(4, P.Steps.Num());
		for (int32 i = 0; i < Show; ++i)
		{
			Txt(H, P.Steps[i], Dim, RX + 14.f, Y, Med, 0.64f);
			Y += 15.f;
		}
		RY += PH + Gap;
	}

	// ---- PIPELINE // PLAN -> REVIEW -> EXECUTE ---------------------------
	{
		const float PW = ColW, PH = 116.f;
		Panel(H, Body, Gold, RX, RY, PW, PH);
		Txt(H, TEXT("PIPELINE  //  PLAN -> REVIEW -> EXECUTE"), LabelGold, RX + 14.f, RY + 9.f, Big, 0.72f);
		Txt(H, FString::Printf(TEXT("EYES  %s"), *OS->CurrentPlan().ReviewNote),
			OS->CurrentPlan().bReviewed ? Green : Dim, RX + 14.f, RY + 36.f, Med, 0.70f);
		Txt(H, FString::Printf(TEXT("SEAT  %s"), *OS->CurrentPlan().Seat), Dim, RX + 14.f, RY + 58.f, Med, 0.66f);
		Txt(H, FString::Printf(TEXT("HANDS %s"), *OS->CurrentPlan().Result),
			White, RX + 14.f, RY + 80.f, Med, 0.76f);
		RY += PH + Gap;
	}

	// ---- AUTOMATIONS ------------------------------------------------------
	{
		const float PW = ColW, PH = 60.f;
		Panel(H, Body, Gold, RX, RY, PW, PH);
		Txt(H, TEXT("AUTOMATIONS  //  ON CHANGE"), LabelGold, RX + 14.f, RY + 9.f, Big, 0.72f);
		Txt(H, OS->AutomationLine(), White, RX + 14.f, RY + 34.f, Med, 0.70f);
		RY += PH + Gap;
	}

	// ---- MEMORY // PHI LEDGER --------------------------------------------
	{
		const float PW = ColW, PH = Ht - RY - 40.f;
		Panel(H, Body, Gold, RX, RY, PW, PH);
		Txt(H, TEXT("MEMORY  //  PHI LEDGER"), LabelGold, RX + 14.f, RY + 9.f, Big, 0.72f);
		Txt(H, TEXT("append-only, rung-stamped, capped at 256"), Dim, RX + 14.f, RY + 30.f, Med, 0.66f);

		const TArray<FVEILOSLedgerEntry>& Ledger = OS->Ledger();
		float Y = RY + 50.f;
		const int32 First = FMath::Max(0, Ledger.Num() - 8);
		for (int32 i = First; i < Ledger.Num(); ++i)
		{
			const FVEILOSLedgerEntry& E = Ledger[i];
			Txt(H, FString::Printf(TEXT("%6.2fs r%6.3f  %s"), E.T, E.Rung, *E.Note),
				i == Ledger.Num() - 1 ? White : Dim, RX + 14.f, Y, Med, 0.64f);
			Y += 15.f;
		}
		if (Ledger.Num() == 0) { Txt(H, TEXT("empty"), Dim, RX + 14.f, Y, Med, 0.66f); }
	}

	// A hairline of gold across the bottom: the face is closed, not unfinished.
	H->DrawRect(Gold, 42.f, Ht - 40.f, W - 84.f, 1.f);
	Txt(H, TEXT("Brain plans, Eyes reviews, Seat names the place, Hands move. Nothing here is written into the one save."),
		Dim, 42.f, Ht - 34.f, Med, 0.70f);
}
