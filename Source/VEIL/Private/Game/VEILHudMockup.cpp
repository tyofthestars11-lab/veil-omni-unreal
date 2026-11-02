// The reference mockup, built from shapes the canvas already has.
//
//   top-left      gold sweep: two tapered ribbons curling round the plate
//   bottom-left   QUEST banner, slanted ends, navy under gold
//   bottom-centre diamond hotbar, centre slot larger and lit
//   bottom-right  COMBAT MODE pill with a red dot, only while machines are close
//
// Every number here is live state: the rungs come from the one OS, the quest
// line from the loop's own stage, the combat pill from the enemies actually
// within reach. Nothing is a label pretending to be data.
#include "Game/VEILHudMockup.h"

#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/OMNIQuestSystem.h"
#include "Systems/VEILEnemy.h"
#include "Systems/VEILOS.h"

namespace
{
	const FLinearColor MockGold(0.788f, 0.635f, 0.290f, 1.f);
	const FLinearColor MockGoldLit(0.965f, 0.851f, 0.478f, 1.f);
	const FLinearColor MockNavy(0.055f, 0.078f, 0.141f, 0.88f);
	const FLinearColor MockWhite(0.949f, 0.949f, 0.949f, 1.f);
	const FLinearColor MockDim(0.62f, 0.67f, 0.75f, 1.f);
	const FLinearColor MockRed(0.851f, 0.204f, 0.227f, 1.f);
	const FLinearColor MockBlue(0.247f, 0.663f, 1.f, 1.f);

	void Txt(AHUD* H, const FString& S, const FLinearColor& C, float X, float Y, UFont* F, float Scale)
	{
		if (H && F && !S.IsEmpty()) { H->DrawText(S, C, X, Y, F, Scale); }
	}

	/** A banner with cut-back ends: the silhouette the mockup's quest plate has. */
	void Banner(AHUD* H, const FLinearColor& Fill, float X, float Y, float W, float Ht,
		const FLinearColor& Trim)
	{
		const int32 Rows = FMath::Max(3, (int32)Ht);
		for (int32 i = 0; i < Rows; ++i)
		{
			const float T = (float)i / (float)(Rows - 1);
			const float Inset = (Ht * 0.55f) * FMath::Abs(T - 0.5f) * 2.f;
			FLinearColor C = Fill;
			C.A = Fill.A * FMath::Lerp(1.f, 0.86f, T);
			H->DrawRect(C, X + Inset, Y + i, W - 2.f * Inset, 1.f);
		}
		H->DrawRect(Trim, X + Ht * 0.55f, Y, W - 1.1f * Ht, 1.f);
		H->DrawRect(FLinearColor(Trim.R, Trim.G, Trim.B, 0.45f), X + Ht * 0.55f, Y + Ht - 1.f, W - 1.1f * Ht, 1.f);
	}

	/** Four strokes. A diamond, at any size, with no new drawing path. */
	void Diamond(AHUD* H, float CX, float CY, float R, const FLinearColor& C, float Thick)
	{
		H->DrawLine(CX, CY - R, CX + R, CY, C, Thick);
		H->DrawLine(CX + R, CY, CX, CY + R, C, Thick);
		H->DrawLine(CX, CY + R, CX - R, CY, C, Thick);
		H->DrawLine(CX - R, CY, CX, CY - R, C, Thick);
	}
}

void VEILHudMockup::Draw(AHUD* H, UCanvas* C)
{
	if (!H || !C) { return; }

	UWorld* World = H->GetWorld();
	if (!World) { return; }

	UVEILOS* OS = World->GetSubsystem<UVEILOS>();
	UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();
	const APawn* Body = UGameplayStatics::GetPlayerPawn(H, 0);

	static UFont* Big = LoadObject<UFont>(nullptr,
		TEXT("/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField"));
	static UFont* Med = LoadObject<UFont>(nullptr,
		TEXT("/Engine/EngineFonts/Roboto.Roboto"));
	if (!Med) { return; }
	if (!Big) { Big = Med; }

	const float W = C->ClipX;
	const float Ht = C->ClipY;

	// ---- GOLD SWEEP -------------------------------------------------------
	// Two tapered ribbons curling round the plate's corner. Drawn as rows with a
	// width that falls off by phi, so the curl is the same law as everything else.
	{
		const float BaseX = 18.f;
		const float BaseY = 14.f;
		for (int32 i = 0; i < 46; ++i)
		{
			const float T = (float)i / 45.f;
			const float A = 1.f - T;
			const float Wid = (1.f - T) * (1.f - T) * 190.f + 6.f;
			const float YOff = BaseY + T * 22.f;
			const float XOff = BaseX + T * 6.f;
			H->DrawRect(FLinearColor(MockGold.R, MockGold.G, MockGold.B, A * 0.55f),
				XOff, YOff, Wid, 1.f);
			H->DrawRect(FLinearColor(MockGoldLit.R, MockGoldLit.G, MockGoldLit.B, A * 0.30f),
				XOff + 2.f, YOff - 1.f, Wid * 0.82f, 1.f);
		}
		// The crest: a small chevron sitting on the top edge.
		H->DrawLine(BaseX + 210.f, BaseY + 2.f, BaseX + 226.f, BaseY + 14.f, MockGoldLit, 2.f);
		H->DrawLine(BaseX + 226.f, BaseY + 14.f, BaseX + 242.f, BaseY + 2.f, MockGoldLit, 2.f);
	}

	// ---- QUEST BANNER (bottom-left) --------------------------------------
	{
		const float BW = 404.f;
		const float BH = 54.f;
		const float BX = 26.f;
		const float BY = Ht - BH - 26.f;

		Banner(H, MockNavy, BX, BY, BW, BH, MockGold);
		Txt(H, TEXT("QUEST: THE GOLDEN MIDST"), MockGoldLit, BX + 34.f, BY + 10.f, Big, 0.82f);

		FString Objective = TEXT("Follow the signal");
		if (const UOMNIQuestSystem* QNow = Quest)
		{
			switch (QNow->Stage)
			{
			case EOMNIQuestStage::MeetKeeper:      Objective = TEXT("Let the Keeper count you"); break;
			case EOMNIQuestStage::GatherFragments: Objective = TEXT("Recover ten fragments of a broken voice"); break;
			case EOMNIQuestStage::MeetSeer:        Objective = TEXT("Find the Seer above the falls"); break;
			case EOMNIQuestStage::ReachCache:      Objective = TEXT("Claim the survey cache"); break;
			case EOMNIQuestStage::WardenAwake:     Objective = TEXT("Face WARDEN PRIME"); break;
			case EOMNIQuestStage::Clear:           Objective = TEXT("The signal is whole"); break;
			}
		}
		Txt(H, FString::Printf(TEXT("\u25C6  %s"), *Objective), MockWhite, BX + 34.f, BY + 30.f, Med, 0.74f);
	}

	// ---- DIAMOND HOTBAR (bottom-centre) ----------------------------------
	if (OS)
	{
		const TArray<FVEILOSDomain>& D = OS->Domains();
		auto RungOf = [&D](const TCHAR* Key) -> double
		{
			for (const FVEILOSDomain& F : D) { if (F.Key == Key) { return F.Rung; } }
			return 0.0;
		};

		const float CX = W * 0.5f;
		const float CY = Ht - 52.f;
		const float Step = 60.f;

		// Centre slot: the phi core, larger and lit.
		Diamond(H, CX, CY, 24.f, MockGoldLit, 2.4f);
		Diamond(H, CX, CY, 18.f, FLinearColor(MockNavy.R, MockNavy.G, MockNavy.B, 0.9f), 1.f);
		Txt(H, TEXT("\u03C6"), MockGoldLit, CX - 7.f, CY - 11.f, Big, 1.0f);

		struct FSlot { const TCHAR* Key; const TCHAR* Label; FLinearColor Tint; };
		static const FSlot Slots[3] =
		{
			{ TEXT("VIT"),    TEXT("5"), MockRed },
			{ TEXT("GOLD"),   TEXT("3"), MockGold },
			{ TEXT("BEACON"), TEXT("2"), MockBlue },
		};

		for (int32 i = 0; i < 3; ++i)
		{
			const float Side = (i == 0) ? -Step : ((i == 1) ? Step : -2.f * Step);
			const float SX = CX + Side;
			const float SY = CY + ((i == 2) ? -14.f : 0.f);
			Diamond(H, SX, SY, 20.f, MockGold, 1.8f);
			Diamond(H, SX, SY, 15.f, FLinearColor(Slots[i].Tint.R * 0.35f, Slots[i].Tint.G * 0.35f, Slots[i].Tint.B * 0.35f, 0.92f), 1.f);
			Txt(H, FString::Printf(TEXT("%.0f"), RungOf(Slots[i].Key)), MockWhite, SX - 10.f, SY - 7.f, Med, 0.68f);
			Txt(H, FString::Printf(TEXT("%s"), Slots[i].Key), MockDim, SX - 14.f, SY + 22.f, Med, 0.56f);
		}
	}

	// ---- COMBAT PILL (bottom-right) --------------------------------------
	if (Body)
	{
		int32 Near = 0;
		const FVector Eye = Body->GetActorLocation();
		for (TActorIterator<AVEILEnemy> It(World); It; ++It)
		{
			if (!*It) { continue; }
			if (FVector::Dist2D(It->GetActorLocation(), Eye) < 2200.f) { ++Near; }
		}

		if (Near > 0)
		{
			const float PW = 196.f;
			const float PH2 = 34.f;
			const float PX2 = W - PW - 30.f;
			const float PY2 = Ht - PH2 - 34.f;

			Banner(H, MockNavy, PX2, PY2, PW, PH2, MockGold);
			H->DrawRect(MockRed, PX2 + 20.f, PY2 + PH2 * 0.5f - 4.f, 8.f, 8.f);
			Txt(H, TEXT("COMBAT MODE"), MockGoldLit, PX2 + 38.f, PY2 + 9.f, Big, 0.74f);
			Txt(H, FString::Printf(TEXT("%d"), Near), MockWhite, PX2 + PW - 34.f, PY2 + 10.f, Med, 0.70f);
		}
	}
}
