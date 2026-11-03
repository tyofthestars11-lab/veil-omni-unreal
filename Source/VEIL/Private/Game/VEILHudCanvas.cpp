// VEIL canvas HUD: objectives, health, progression, win/lose.
#include "Game/VEILHudCanvas.h"
#include "Game/VEILGameMode.h"
#include "Game/VEILOSFace.h"
#include "Game/VEILHudMockup.h"
#include "Character/VEILCharacter.h"
#include "Engine/Texture2D.h"
#include "Systems/OMNIQuestSystem.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Systems/PHIVoice.h"
#include "EngineUtils.h"
#include "World/OMNIQuestContent.h"
#include "AI/OMNIWardenPrime.h"
#include "Systems/PHICompanionParty.h"
#include "Systems/VEILLoopSubsystem.h"
#include "Systems/VEILDialogue.h"
#include "Systems/PHIMito.h"
#include "Story/VEILCast.h"
#include "Story/VEILSpine.h"
#include "Story/VEILVoiceTake.h"
#include "AI/VEILFactionAI.h"
#include "Systems/VEILTutorial.h"
#include "Systems/VEILInputHelp.h"
#include "Systems/VEILFastTravel.h"
#include "Systems/PHI_NPCProcessor.h"
#include "Systems/PHIOMNI.h"
#include "Systems/PHIIntent.h"
#include "Systems/VEILEnemy.h"
#include "Systems/VEILTime.h"
#include "Systems/VEILCampaign.h"
#include "Systems/VEILSociety.h"
#include "Systems/VEILPerf.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Systems/PHICheats.h"
#include "Systems/PHIONE.h"
#include "Systems/PHILivingWorld.h"
#include "Systems/VEILRegionSubsystem.h"
#include "World/VEILMapCapture.h"
#include "Engine/Canvas.h"
#include "SceneView.h"

// Vertical margin on the same phi grid as the horizontal one.
#define UPHIMITO_PHI_SAFE (1.618033988749895f)

// ---------------------------------------------------------------------------
// THE SHAPE KIT.
//
// The HUD used to be built out of one primitive: a filled rectangle. Every
// panel was the same flat slab, every bar the same slab with a colour in it,
// which is exactly why the screen read as a debug overlay rather than an
// instrument. These four shapes are the vocabulary now, and every panel in the
// file is drawn from them, so the look cannot drift back to flat boxes:
//
//   RoundRect  a rectangle with cut corners, built from three bodies and two
//              fan rows so the curve is real at any size
//   Glass      rounded panel, gold trim, halo, and a translucent body you can
//              still see the world through
//   Ribbon     a banner with cut-back ends and a lit top edge - cloth, not card
//   Ring       a circle of segments, for the map and its halo
//
// Everything is drawn with the canvas primitives the HUD already used, so this
// adds shape, not a new rendering path.
// ---------------------------------------------------------------------------
namespace FVeilUI
{
	static void RoundRect(AHUD* H, FLinearColor C, float X, float Y, float W, float Ht, float R)
	{
		if (!H || W <= 0.f || Ht <= 0.f) { return; }
		R = FMath::Clamp(R, 0.f, FMath::Min(W, Ht) * 0.5f);
		if (R < 1.f) { H->DrawRect(C, X, Y, W, Ht); return; }

		H->DrawRect(C, X + R, Y, W - 2.f * R, Ht);              // middle band
		H->DrawRect(C, X, Y + R, R, Ht - 2.f * R);              // left slab
		H->DrawRect(C, X + W - R, Y + R, R, Ht - 2.f * R);      // right slab

		// The corners: one row per pixel of radius, width from the circle. A
		// rounded corner that is really a rounded corner, not a diagonal cut.
		const int32 Steps = FMath::Clamp((int32)R, 1, 26);
		for (int32 i = 0; i < Steps; ++i)
		{
			const float T = (i + 0.5f) / (float)Steps;
			const float D = R * (1.f - FMath::Sqrt(FMath::Max(0.f, 1.f - (1.f - T) * (1.f - T))));
			H->DrawRect(C, X + D, Y + i, W - 2.f * D, 1.f);
			H->DrawRect(C, X + D, Y + Ht - 1.f - i, W - 2.f * D, 1.f);
		}
	}

	static void Glass(AHUD* H, FLinearColor Tint, float X, float Y, float W, float Ht, float R)
	{
		// #C9A24A: the frame in the mockup is a restrained gold line, not a
		// bright yellow stripe.
		const FLinearColor Trim(0.788f, 0.635f, 0.290f, 1.f);
		// halo -> trim -> body -> sheen. Four layers, so the panel has an edge
		// that survives a bright world and a body that does not hide it.
		RoundRect(H, FLinearColor(Trim.R, Trim.G, Trim.B, 0.07f), X - 5.f, Y - 5.f, W + 10.f, Ht + 10.f, R + 5.f);
		RoundRect(H, FLinearColor(Trim.R, Trim.G, Trim.B, 0.30f), X - 2.5f, Y - 2.5f, W + 5.f, Ht + 5.f, R + 2.5f);
		RoundRect(H, FLinearColor(Trim.R, Trim.G, Trim.B, 0.85f), X - 1.f, Y - 1.f, W + 2.f, Ht + 2.f, R + 1.f);
		RoundRect(H, Tint, X, Y, W, Ht, R);

		FLinearColor Sheen = Tint;
		Sheen.R = FMath::Min(1.f, Tint.R * 1.9f + 0.05f);
		Sheen.G = FMath::Min(1.f, Tint.G * 1.9f + 0.05f);
		Sheen.B = FMath::Min(1.f, Tint.B * 1.7f + 0.05f);
		Sheen.A = 0.13f;
		RoundRect(H, Sheen, X + 1.f, Y + 1.f, W - 2.f, Ht * 0.42f, R * 0.75f);
		H->DrawRect(FLinearColor(1.f, 0.95f, 0.82f, 0.30f), X + R, Y + 1.f, W - 2.f * R, 1.f);
	}

	static void Ribbon(AHUD* H, FLinearColor Tint, float X, float Y, float W, float Ht, bool bTails)
	{
		if (!H || W <= 0.f || Ht <= 0.f) { return; }
		const FLinearColor Trim(1.f, 0.80f, 0.32f, 1.f);
		const float Tail = bTails ? FMath::Min(Ht * 0.62f, 20.f) : 0.f;
		const int32 Rows = FMath::Max(2, (int32)Ht);

		for (int32 i = 0; i < Rows; ++i)
		{
			const float T = i / (float)(Rows - 1);
			// The ends are cut back to a point at mid-height: the silhouette of
			// cloth hung on a rail, instead of another rectangle.
			const float Inset = Tail * FMath::Abs(T - 0.5f) * 2.f;
			FLinearColor C = FMath::Lerp(Tint * 1.28f, Tint * 0.78f, T);
			C.A = Tint.A * FMath::Lerp(1.f, 0.84f, T);
			H->DrawRect(C, X + Inset, Y + i, W - 2.f * Inset, 1.f);
		}
		H->DrawRect(FLinearColor(Trim.R, Trim.G, Trim.B, 0.85f), X + Tail, Y, W - 2.f * Tail, 1.f);
		H->DrawRect(FLinearColor(Trim.R, Trim.G, Trim.B, 0.35f), X + Tail, Y + Ht - 1.f, W - 2.f * Tail, 1.f);
	}

	/** A small readout plate: ribbon silhouette with a gold floor line. */
	static void Chip(AHUD* H, FLinearColor Tint, float X, float Y, float W, float Ht)
	{
		Ribbon(H, Tint, X, Y, W, Ht, true);
	}

	// A DIAMOND, drawn as rows. The vision's badge shape for LV and for the quest
	// icons; canvas has no rotation, and rows are how the ribbon's cut ends are made,
	// so it belongs to the same kit rather than being a new primitive.
	static void Diamond(AHUD* H, float CX, float CY, float R, FLinearColor C)
	{
		if (!H || R <= 0.f) { return; }
		const int32 Rows = FMath::Max(4, (int32)(R * 2.f));
		const float RowH = (2.f * R) / (float)Rows;
		for (int32 i = 0; i < Rows; ++i)
		{
			const float T = (i + 0.5f) / (float)Rows;
			const float Half = R * (1.f - FMath::Abs(T * 2.f - 1.f));
			if (Half > 0.4f)
			{
				H->DrawRect(C, CX - Half, CY - R + i * RowH, Half * 2.f, RowH + 0.6f);
			}
		}
	}

	static void Ring(AHUD* H, float CX, float CY, float R, FLinearColor C, float Thickness, int32 Segments = 64)
	{
		if (!H) { return; }
		Segments = FMath::Clamp(Segments, 8, 180);
		for (int32 i = 0; i < Segments; ++i)
		{
			const float A0 = (i / (float)Segments) * 2.f * PI;
			const float A1 = ((i + 1) / (float)Segments) * 2.f * PI;
			H->DrawLine(CX + FMath::Cos(A0) * R, CY + FMath::Sin(A0) * R,
				CX + FMath::Cos(A1) * R, CY + FMath::Sin(A1) * R, C, Thickness);
		}
	}
}

static const TCHAR* PhiStageText(int32 Stage)
{
	switch (Stage)
	{
	case 0: return TEXT("Find the KEEPER statue (gold) and walk to it");
	case 1: return TEXT("Gather the 3 signal fragments (walk into them)");
	case 2: return TEXT("Find the SEER statue and walk to it");
	case 3: return TEXT("Reach the green CACHE at the south wall");
	case 4: return TEXT("WARDEN PRIME wakes - bring it down");
	default: return TEXT("Signal restored - district breathes");
	}
}

// ---------------------------------------------------------------------------
// THE MAP KEY.
//
// A minimap without a legend is a set of coloured pixels the player has to
// reverse-engineer, and every mark on this map means something: a person who
// will talk, a piece of a broken voice, the cache, the machines, the thing at
// the end. The colours here are written ONCE and both the in-play strip and
// the chart page read them, so a key can never drift from the blips.
// ---------------------------------------------------------------------------
void AVEILHudCanvas::DrawMapLegend(UCanvas* C, UFont* Font, float X, float Y, bool bWide)
{
	if (!C || !Font) { return; }

	enum class EMark : uint8 { Square, Chevron, Ring };
	struct FKey { FLinearColor Color; const TCHAR* Label; const TCHAR* Meaning; EMark Mark; };
	static const FKey Keys[] = {
		{ FLinearColor(1.f, 0.90f, 0.45f), TEXT("RUNNER"),   TEXT("you - the map turns, not the eye"), EMark::Chevron },
		{ FLinearColor(1.f, 0.82f, 0.32f), TEXT("CAST"),     TEXT("Keeper / Seer / Cartographer - they speak"), EMark::Ring },
		{ FLinearColor(0.35f, 0.95f, 1.f), TEXT("FRAGMENT"), TEXT("a piece of the interrupted sentence"), EMark::Square },
		{ FLinearColor(0.30f, 1.f, 0.45f), TEXT("CACHE"),    TEXT("the drop at the south wall"), EMark::Square },
		{ FLinearColor(1.f, 0.45f, 0.35f), TEXT("MACHINE"),  TEXT("faction agents - they fight back"), EMark::Square },
		{ FLinearColor(1.f, 0.22f, 0.18f), TEXT("WARDEN"),   TEXT("the thing the Vault sends"), EMark::Square },
	};
	const int32 Count = UE_ARRAY_COUNT(Keys);

	auto Mark = [this, C](FLinearColor Col, EMark M, float SX, float SY, float S)
	{
		switch (M)
		{
		case EMark::Chevron:
			DrawLine(SX + S * 0.5f, SY, SX, SY + S, Col, 2.f);
			DrawLine(SX + S * 0.5f, SY, SX + S, SY + S, Col, 2.f);
			break;
		case EMark::Ring:
			FVeilUI::Diamond(this, SX + S * 0.5f, SY + S * 0.5f, S * 0.62f, FLinearColor(0.f, 0.f, 0.f, 0.6f));
			FVeilUI::Diamond(this, SX + S * 0.5f, SY + S * 0.5f, S * 0.5f, Col);
			FVeilUI::Diamond(this, SX + S * 0.5f, SY + S * 0.5f, S * 0.22f, FLinearColor(0.f, 0.f, 0.f, 0.85f));
			break;
		default:
			FVeilUI::Diamond(this, SX + S * 0.5f, SY + S * 0.5f, S * 0.62f, FLinearColor(0.f, 0.f, 0.f, 0.6f));
			FVeilUI::Diamond(this, SX + S * 0.5f, SY + S * 0.5f, S * 0.5f, Col);
			break;
		}
	};

	if (!bWide)
	{
		// IN PLAY: one horizontal strip, out of the way of the shot.
		const float Swatch = 9.f;
		const float Pad = 18.f;
		float Total = 0.f;
		for (const FKey& K : Keys)
		{
			float W = 0.f, H = 0.f;
			C->StrLen(Font, K.Label, W, H);
			Total += Swatch + 6.f + W * 0.62f + Pad;
		}
		FVeilUI::Chip(this, FLinearColor(0.04f, 0.06f, 0.10f, 0.62f), X - 10.f, Y - 5.f, Total + 12.f, 22.f);

		float CX = X;
		for (const FKey& K : Keys)
		{
			Mark(K.Color, K.Mark, CX, Y + 4.f, Swatch);
			CX += Swatch + 6.f;
			float W = 0.f, H = 0.f;
			C->StrLen(Font, K.Label, W, H);
			AHUD::DrawText(K.Label, FLinearColor(0.80f, 0.86f, 0.94f), CX, Y + 1.f, Font, 0.62f, false);
			CX += W * 0.62f + Pad;
		}
		return;
	}

	// ON THE CHART PAGE: the same key, two columns, titled - it sits in the
	// panel's corner where the beacon list cannot reach it.
	const float Swatch = 11.f;
	const float ColW = 136.f;
	AHUD::DrawText(TEXT("MAP KEY"), FLinearColor(0.55f, 0.62f, 0.74f), X, Y, Font, 0.74f, false);
	const float FirstRow = Y + 22.f;
	for (int32 i = 0; i < Count; ++i)
	{
		const float ColX = X + (i % 2) * ColW;
		const float RowY = FirstRow + (i / 2) * 24.f;
		Mark(Keys[i].Color, Keys[i].Mark, ColX, RowY + 3.f, Swatch);
		AHUD::DrawText(Keys[i].Label, FLinearColor(0.90f, 0.94f, 0.99f), ColX + Swatch + 8.f, RowY + 1.f, Font, 0.70f, false);
	}
	float RowY = FirstRow + 3.f * 24.f + 4.f;

	// THE COUNT, so a legend is not only a key but a reading of the map.
	UWorld* W = GetWorld();
	if (W)
	{
		int32 Givers = 0, Frags = 0, Caches = 0, Machines = 0, Wardens = 0;
		for (TActorIterator<AOMNIQuestGiver> It(W); It; ++It) { ++Givers; }
		for (TActorIterator<AOMNIFragment> It(W); It; ++It) { ++Frags; }
		for (TActorIterator<AOMNICache> It(W); It; ++It) { ++Caches; }
		for (TActorIterator<AVEILFactionAgent> It(W); It; ++It) { ++Machines; }
		for (TActorIterator<AOMNIWardenPrime> It(W); It; ++It) { ++Wardens; }
		const FString Line = FString::Printf(
			TEXT("ON THIS MAP   %d cast   %d fragment(s)   %d cache(s)   %d machine(s)   %d warden"),
			Givers, Frags, Caches, Machines, Wardens);
		AHUD::DrawText(Line, FLinearColor(0.72f, 0.88f, 1.f), X, RowY + 2.f, Font, 0.72f, false);
	}
}

void AVEILHudCanvas::DrawHUD()
{
	Super::DrawHUD();
	UWorld* W = GetWorld();
	if (!W || !Canvas) return;
	// REAL TYPE, NOT ENGINE DEFAULT. GetLargeFont/GetMediumFont hand back the stock
	// bitmap face at one size, which is why the whole HUD read like a 1984 terminal.
	// These are real font assets from Engine content: a distance-field face for the
	// big lines (stays crisp when scaled) and Roboto for the body text.
	static UFont* Big = LoadObject<UFont>(nullptr,
		TEXT("/Engine/EngineFonts/RobotoDistanceField.RobotoDistanceField"));
	static UFont* Med = LoadObject<UFont>(nullptr,
		TEXT("/Engine/EngineFonts/Roboto.Roboto"));
	if (!Big || !Med) return;

	APlayerController* PC = GetOwningPlayerController();
	AVEILCharacter* Pawn = PC ? Cast<AVEILCharacter>(PC->GetPawn()) : nullptr;

	UOMNIQuestSystem* Q = W->GetSubsystem<UOMNIQuestSystem>();
	AVEILGameMode* GM = W->GetAuthGameMode<AVEILGameMode>();
	const int32 Stage = Q ? (int32)Q->Stage : 0;
	UVEILLoopSubsystem* LoopSys = W->GetSubsystem<UVEILLoopSubsystem>();
	const bool bDead = Pawn && !Pawn->IsAlive();
	const bool bWon = GM && GM->Phase == EVEILGamePhase::Resolution;

	// ------------------------------------------------------------------
	// HUD TYPE AND LAYOUT. The font was already real; the block was not a
	// layout - seven lines of the same weight and colour floating on the world,
	// which is what reads as "basic text". Now: a framed plate, a small kicker,
	// one gold headline, real bars for health and XP, then secondary rows and a
	// dimmed key hint. Spacing is one phi grid, not eyeballed pixels.
	// ------------------------------------------------------------------
	// 420 x 110 IS THE SPEC, AND 470 x 232 WAS MORE THAN DOUBLE IT. At 232 px tall the
	// plate was a panel the district was read around; the brief allows 420 x 110 with the
	// objective line, the three gauges and one stats line - everything else it used to
	// carry belongs behind TAB. Every step below is stated in pixels, not multiples of a
	// unit that grew the block back.
	const float U = 5.2f * UPHIMito::Phi;         // 8.41 px base unit, was 12.944
	const float PX = 14.f;                        // spec margin, was 29.1
	const float PY = 14.f;
	const float PW = 420.f;                       // spec max width, was 470
	const float PH = 110.f;                       // spec max height, was 232

	auto Shadow = [this](const FString& S, FLinearColor C, float X, float Yy, UFont* F, float Sc)
	{
		DrawText(S, FLinearColor(0.f, 0.f, 0.f, 0.8f * C.A), X + 1.5f, Yy + 1.5f, F, Sc);
		DrawText(S, C, X, Yy, F, Sc);
	};
	// A GAUGE, NOT A SLAB. Rounded trough, lit ribbon of fill, bright cap at the
	// leading edge - so health reads as a level at a glance instead of a colour
	// in a box.
	auto Bar = [this](float X, float Yy, float Wd, float Ht, float Pct, FLinearColor Fill)
	{
		const float R = Ht * 0.5f;
		FVeilUI::RoundRect(this, FLinearColor(0.f, 0.f, 0.f, 0.45f), X - 1.f, Yy - 1.f, Wd + 2.f, Ht + 2.f, R + 1.f);
		// Warm trough, not a grey slot: the bar sits in gold-shadowed bronze so the
		// VIT red and STM green read as light inside an instrument.
		FVeilUI::RoundRect(this, FLinearColor(0.10f, 0.07f, 0.02f, 0.92f), X, Yy, Wd, Ht, R);
		const float FW = FMath::Max(0.f, (Wd - 2.f) * FMath::Clamp(Pct, 0.f, 1.f));
		if (FW > 0.f)
		{
			FVeilUI::RoundRect(this, FLinearColor(Fill.R * 0.50f, Fill.G * 0.50f, Fill.B * 0.50f, 0.95f), X + 1.f, Yy + 1.f, FW, Ht - 2.f, R);
			FVeilUI::RoundRect(this, FLinearColor(Fill.R, Fill.G, Fill.B, 0.85f), X + 1.f, Yy + 1.f, FW, (Ht - 2.f) * 0.5f, R);
			if (FW > 4.f)
			{
				FVeilUI::RoundRect(this, FLinearColor(1.f, 1.f, 1.f, 0.80f), X + FW - 1.5f, Yy + 1.f, 3.f, Ht - 2.f, 1.5f);
			}
		}
	};

	// Plate: rounded glass with a gold trim and a warm spine on the left, so the
	// instrument has a silhouette and you can still see the district through it.
	// GOLD RIBBON PLATE, NOT A GREY CARD. The body carries the world's own gold
	// instead of navy, with a lit band along the top edge - the ribbon from the vision.
	// THE PHI BADGE COLUMN. The vision puts a large diamond with the phi glyph and LV
	// before every other number, with the bars starting to its right - so the plate keeps
	// its width and the bars start at BARX instead of being squeezed to make room. These
	// are stated before the plate because the badge is drawn with it, not after it.
	const float BadgeC = PX + 30.f;
	const float BadgeY = PY + 34.f;
	const float BARX = PX + 76.f;                 // text column aligns with the gauges
	const float BARW = 170.f;

	// NAVY GLASS, GOLD LINE - the mockup's plate. #0E1424 under #C9A24A.
	FVeilUI::Glass(this, FLinearColor(0.055f, 0.078f, 0.141f, 0.85f), PX, PY, PW, PH, 11.f);
	FVeilUI::RoundRect(this, FLinearColor(0.788f, 0.635f, 0.290f, 0.62f), PX + 8.f, PY + 1.f, PW - 16.f, 2.4f, 1.2f);
	FVeilUI::RoundRect(this, FLinearColor(0.788f, 0.635f, 0.290f, 0.95f), PX + 1.f, PY + 10.f, 2.6f * UPHIMito::Phi, PH - 20.f, 1.3f * UPHIMito::Phi);

	// THE PHI DIAMOND AND THE LEVEL. A gold diamond, a dark diamond inside it so the
	// glyph reads, the phi mark itself, and LV under it - the badge the vision leads with.
	{
		FVeilUI::Diamond(this, BadgeC, BadgeY, 26.f, FLinearColor(1.f, 0.86f, 0.45f, 0.95f));
		FVeilUI::Diamond(this, BadgeC, BadgeY, 20.f, FLinearColor(0.10f, 0.07f, 0.02f, 0.96f));
		DrawText(TEXT("\u03C6"), FLinearColor(1.f, 0.93f, 0.58f), BadgeC - 5.f, BadgeY - 11.f, Big, 0.90f);
		// LV moved onto the XP row: the plate is 110 px tall and the badge no longer has a
		// floor of its own to set it under.
	}

	const float TX = BARX;
	float Y = PY + 6.f;
	// THE DIRECTIVE IS THE ACT, NOT A TODO. The line under CURRENT DIRECTIVE is
	// written in the spine, in the world's own voice, so what the runner is asked
	// to do and what the statues are talking about are the same sentence.
	const FString Objective = LoopSys
		? VEILSpine::DirectiveFor(LoopSys->Stage).ToString()
		: FString(PhiStageText(Stage));

	// NO KICKER. The spec for this panel is the objective line, the three gauges and the
	// level, inside 420 x 110. "CURRENT DIRECTIVE" was a label for a panel that had room
	// to explain itself; at this size the gold sentence explains itself.
	// SIX PIXELS LOWER. At PY + 8 the top of the cap height was being taken by the plate's
	// gold trim band, so the directive read as though it had been sheared. The line now
	// starts clear of the band, and the row steps below pay the six pixels back.
	Y = PY + 14.f;

	// NO RIBBON BEHIND THE DIRECTIVE. The gold cloth sat over the line's cap height and its
	// cut end took the first word, so the objective read "...k to the KEEPER" wherever the
	// text or the ribbon was moved. At 420 x 110 the sentence is legible on the plate, and
	// a panel this size has no room for a banner inside a banner.
	Shadow(Objective, FLinearColor(1.f, 0.90f, 0.46f), TX, Y, Big, 0.62f);
	Y += 22.f;

	// Bars carry the state; the numbers stay for exact reading.
	if (Pawn)
	{
		const float Pct = Pawn->MaxHealth > 0.f ? Pawn->Health / Pawn->MaxHealth : 0.f;
		// VIT is health. The numbers stay, because "VIT 100 / 100" is the same live
		// state the vision shows - not a bar with the truth filed off.
		// VIT #D9343A, STM #2FB36A - the two gradients the mockup leads with.
		const FLinearColor Vit = Pct > 0.35f ? FLinearColor(0.851f, 0.204f, 0.227f) : FLinearColor(0.95f, 0.55f, 0.18f);
		Bar(BARX, Y, BARW, 7.f, Pct, Vit);
		Shadow(FString::Printf(TEXT("VIT  %.0f / %.0f"), Pawn->Health, Pawn->MaxHealth),
			FLinearColor(1.f, 0.86f, 0.70f), BARX + BARW + 10.f, Y - 2.f, Med, 0.58f);
		Y += 13.f;
		// STM is the mitochondrial gradient the runner actually burns (0..100 ATP on
		// the Countess rig), so the green bar is live state rather than decoration.
		// STM is the runner's OWN ATP fuel (0..100 on AVEILCharacter) - no cast to a
		// sibling class, which the compiler rightly refused: the pawn already carries
		// the number the bar is meant to show.
		const float Stam = Pawn->MaxATP > 0.f
			? FMath::Clamp(Pawn->ATP / Pawn->MaxATP, 0.f, 1.f)
			: 1.f;
		Bar(BARX, Y, BARW, 7.f, Stam, FLinearColor(0.184f, 0.702f, 0.416f));
		Shadow(FString::Printf(TEXT("STM  %.0f / %.0f"), Pawn->ATP, Pawn->MaxATP),
			FLinearColor(0.74f, 1.f, 0.80f), BARX + BARW + 10.f, Y - 2.f, Med, 0.58f);
		Y += 13.f;
	}
	if (Q)
	{
		const int32 Need = UOMNIQuestSystem::XPNeedForLevel(Q->Level);
		const float Pct = Need > 0 ? (float)Q->XP / (float)Need : 0.f;
		// LV IS A DIAMOND. The badge shape carries the number, and the XP bar fills
		// beside it - the same level, read at a glance instead of spelled out.
		Bar(BARX, Y, BARW, 7.f, Pct, FLinearColor(0.55f, 0.80f, 1.f));
		// THE LEVEL LIVES WHERE THE XP IS. The badge is 26 px in a 110 px plate, so the
		// number it used to carry under itself now sits on the row it belongs to.
		Shadow(FString::Printf(TEXT("LV %d   XP %d / %d"), Q->Level, Q->XP, Need),
			FLinearColor(0.72f, 0.86f, 1.f), BARX + BARW + 10.f, Y - 2.f, Med, 0.58f);
		Y += 11.f;
	}

	// Hairline, then secondary rows in a quieter weight of the same face.
	DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.10f), TX, Y, PW - U * 4.f, 1.f);
	Y += 4.f;

	if (Q)
	{
		Shadow(FString::Printf(TEXT("FRAG %d/3   GOLD %d   SHARDS %d"),
			Q->Fragments, Q->Gold, Q->Shards),
			FLinearColor(0.78f, 0.84f, 0.92f), TX, Y, Med, 0.56f);
		Y += 9.f;
		// THE REGION IS ASKED OF THE WORLD, NOT READ OFF AN EMPTY STRING. The quest
		// system's RegionMap was never assigned by anything, so this row printed
		// "REGION " and stopped - a hole in the HUD exactly where a place name
		// belongs. The region subsystem already knows where the runner is standing;
		// it is the one that gets asked, and the quest's own name is the fallback.
		FString RegionName = Q->GetRegionName();
		if (const UVEILRegionSubsystem* Regions = W->GetSubsystem<UVEILRegionSubsystem>())
		{
			RegionName = Regions->GetInfo(Regions->Current).Name.ToString();
		}
		Shadow(FString::Printf(TEXT("FORGE %d   BEACONS %d/3   SEAMS %d/24   %s"),
			Q->ForgeLevel, Q->ClaimedBeacons.Num(), Q->ClaimedMines.Num(), *RegionName),
			FLinearColor(0.78f, 0.84f, 0.92f), TX, Y, Med, 0.56f);
		Y += 9.f;
	}
	// THE NEXT SEAM, NAMED. A name, a distance and whose ground - so the runner can
	// walk to somewhere the world actually told him about.
	if (Q && !Q->TrackedMine.IsEmpty())
	{
		Shadow(FString::Printf(TEXT("%s   %.0f m   %s ground"), *Q->TrackedMine.ToUpper(),
			Q->TrackedCm / 100.f, *Q->TrackedOwner),
			FLinearColor(0.95f, 0.80f, 0.44f), PX, PY + PH + 8.f, Med, 0.60f);
	}

	// A CLAIM ANNOUNCES ITSELF: six seconds of the name, in the middle of the screen.
	if (Q && !Q->LastClaimName.IsEmpty() && W->GetTimeSeconds() - Q->LastClaimAt < 6.f)
	{
		const FString Line = FString::Printf(TEXT("%s CLAIMED  -  %s GROUND"),
			*Q->LastClaimName.ToUpper(), *Q->LastClaimOwner.ToUpper());
		float LW = 0.f, LH = 0.f;
		Canvas->StrLen(Big, Line, LW, LH);
		Shadow(Line, FLinearColor(1.f, 0.88f, 0.45f),
			(Canvas->ClipX - LW * 0.80f) * 0.5f, 132.f, Big, 0.80f);
	}

	if (LoopSys && LoopSys->Stage != EVEILStage::FindSignal && LoopSys->Stage != EVEILStage::Won)
	{
		Shadow(FString::Printf(TEXT("SIGNAL %d/%d   WAVES %d/%d"), LoopSys->Fragments,
			UVEILLoopSubsystem::FragmentsNeeded, LoopSys->WavesCleared, UVEILLoopSubsystem::TotalWaves),
			FLinearColor(0.55f, 0.92f, 1.f), TX, Y, Med, 0.56f);
		Y += 9.f;
	}
	{
		FString Lead = TEXT("RUNNER");
		if (PC && PC->GetPawn())
		{
			Lead = PC->GetPawn()->GetName();
			Lead = Lead.Contains(TEXT("Countess")) ? TEXT("COUNTESS") : TEXT("RUNNER");
		}
		const int32 Party = W->GetSubsystem<UPHICompanionParty>() ? W->GetSubsystem<UPHICompanionParty>()->Count() : 0;
		Shadow(FString::Printf(TEXT("LEAD %s   PARTY %d/5"), *Lead, Party),
			FLinearColor(0.78f, 0.84f, 0.92f), TX, Y, Med, 0.56f);
	}

	// THE KEY LIST IS GONE. At 420 x 110 the panel has no room for a permanent line of
	// reference text, the line collided with OVERHEARD directly under it, and the field
	// manual in the bottom right already teaches movement and the chart key. The spec for
	// this panel is the objective, the three gauges and the level - nothing else.

	// ------------------------------------------------------------------
	// OVERHEARD. Two of them, talking to each other about the district they are
	// standing in - not about the runner, unless the runner earned it. It fades, so
	// it reads as something passing by rather than a permanent caption.
	// ------------------------------------------------------------------
	if (const UPHILivingWorld* Living = W->GetSubsystem<UPHILivingWorld>())
	{
		const FString& Heard = Living->OverheardLine();
		if (!Heard.IsEmpty())
		{
			const float A = Living->OverheardAlpha();
			if (A > 0.02f)
			{
				// Cloth, not a caption box: it fades in and out on a banner so it
				// reads as something the district said, not an interface line.
				const FString Over = FString::Printf(TEXT("OVERHEARD   %s"), *Heard);
				float OWid = 0.f, OHigh = 0.f;
				Canvas->StrLen(Med, Over, OWid, OHigh);
				FVeilUI::Ribbon(this, FLinearColor(0.06f, 0.07f, 0.11f, 0.55f * A),
					PX + U * 0.5f, PY + PH + U * 0.75f, OWid * 0.80f + 22.f, U * 1.75f, true);
				Shadow(Over, FLinearColor(0.82f, 0.88f, 0.97f, A), PX + U * 2.f, PY + PH + U * 1.1f, Med, 0.80f);
			}
		}
	}

	// ------------------------------------------------------------------
	// THE LIVING MAP. Not dots on black: the district itself, photographed from
	// above by UVEILMapCapture and turned with the runner, with the cast, the
	// sites and the machines drawn on top in their own colours. The capture and
	// the blips share one yaw, so a marker cannot lie about where a thing is.
	// ------------------------------------------------------------------
	{
		// MINIMAP DIAMETER = SCREEN HEIGHT / PHI. 1080 / 1.618033988749895 = 667 px,
		// 720 / phi = 445 px - the ring is sized by the law, not by a guess.
		const float MS = Canvas->ClipY / UPHIMITO_PHI_SAFE;
		const float MX = Canvas->ClipX - MS - 26.f, MY = 30.f;
		const FLinearColor Gold(1.f, 0.78f, 0.3f, 0.92f);

		UVEILMapCapture* Map = UVEILMapCapture::GetActive();
		if (!Map) { Map = W->GetSubsystem<UVEILMapCapture>(); }
		// Hand the capture the view this frame is about to draw. The map and the
		// world are then the same place by construction, not by a pawn search that
		// can land in a world with no possessed pawn.
		if (Map && Pawn)
		{
			const float ViewYaw = PC ? (float)PC->GetControlRotation().Yaw : (float)Pawn->GetActorRotation().Yaw;
			Map->SetViewFocus(Pawn->GetActorLocation(), FMath::DegreesToRadians(ViewYaw));
		}
		UTexture* Tex = Map ? Cast<UTexture>(Map->GetMapTexture()) : nullptr;
		const FVector C = Map ? Map->GetMapCenter() : FVector::ZeroVector;
		const float  E = Map && Map->GetMapExtent() > 0.f ? Map->GetMapExtent() : 12000.f;
		const float  Ya = Map ? Map->GetMapYaw() : 0.f;
		const float  CS = FMath::Cos(Ya), SN = FMath::Sin(Ya);

		// A ROUND INSTRUMENT, NOT A SQUARE BOX. The live capture is masked to a
		// disc one row at a time - each row is the chord of the circle at that
		// height - so the map is genuinely circular rather than a square with
		// rounded corners drawn over it.
		const float CR = MS * 0.5f;
		const float CCX = MX + CR, CCY = MY + CR;
		for (int32 Row = 0; Row < (int32)MS; ++Row)
		{
			const float DY = Row + 0.5f - CR;
			const float Half = FMath::Sqrt(FMath::Max(0.f, CR * CR - DY * DY));
			if (Half < 0.5f) { continue; }
			// A TEXTURE THAT EXISTS AND A TEXTURE THAT CAN BE DRAWN ARE TWO THINGS. The way
			// this frame died was CanvasItem.cpp asserting InTexture != nullptr, so both
			// draw sites now require a real resource on the texture, not just a pointer.
			if (Tex && Tex->GetResource() && Map && Map->GetCaptureCount() > 0)
			{
				DrawTexture(Tex, CCX - Half, MY + Row, Half * 2.f, 1.f,
					(CR - Half) / MS, (Row + 0.5f) / MS, (Half * 2.f) / MS, 1.f / MS,
					FLinearColor::White, BLEND_Opaque);
			}
			else
			{
				DrawRect(FLinearColor(0.03f, 0.05f, 0.09f, 0.95f), CCX - Half, MY + Row, Half * 2.f, 1.f);
			}
		}
		if (!Tex || !Map || Map->GetCaptureCount() == 0)
		{
			DrawText(TEXT("MAP ACQUIRING"), FLinearColor(0.62f, 0.70f, 0.80f), MX + 44.f, MY + MS * 0.5f, Med, 0.8f);
		}
		// Gold ring with a halo under it: the instrument's bezel.
		FVeilUI::Ring(this, CCX, CCY, CR + 5.f, FLinearColor(Gold.R, Gold.G, Gold.B, 0.10f), 5.f, 64);
		FVeilUI::Ring(this, CCX, CCY, CR + 1.5f, FLinearColor(Gold.R, Gold.G, Gold.B, 0.55f), 2.f, 64);
		FVeilUI::Ring(this, CCX, CCY, CR, FLinearColor(Gold.R, Gold.G, Gold.B, 0.95f), 1.6f, 72);

		// Projection shared by every mark: screen right = (-sin, cos), screen up =
		// (cos, sin), because the capture camera is yawed with the runner.
		auto ToScreen = [&](const FVector& At, float& OutX, float& OutY)
		{
			const float DX = (float)(At.X - C.X), DY = (float)(At.Y - C.Y);
			const float SX = -SN * DX + CS * DY;
			const float SY = -(CS * DX + SN * DY);
			OutX = MX + MS * 0.5f + SX / E * MS * 0.5f;
			OutY = MY + MS * 0.5f + SY / E * MS * 0.5f;
		};
		auto Blip = [&](const FVector& At, FLinearColor Col, float S)
		{
			float PX, PY;
			ToScreen(At, PX, PY);
			// Clipped to the disc, not to a square: a blip outside the ring would be
			// a mark floating in the world behind the instrument.
			{
				const float DX = PX - (MX + MS * 0.5f), DYY = PY - (MY + MS * 0.5f);
				const float Inner = MS * 0.5f - 3.f;
				if (DX * DX + DYY * DYY > Inner * Inner) { return; }
			}
			// A MARKER, NOT A SQUARE. Every blip on this map was an axis-aligned rectangle,
			// which is why the minimap read as a field of random gold and blue squares. Each
			// one is now a phi diamond - the shape the badge and the quest markers already
			// use - so a person, a shard and a beacon are all marks rather than boxes.
			FVeilUI::Diamond(this, PX, PY, S * 0.5f + 1.8f, FLinearColor(0.f, 0.f, 0.f, 0.60f));
			FVeilUI::Diamond(this, PX, PY, S * 0.5f, Col);
		};

		// The cast, in their faction colours: the Keeper gold, the Seer violet,
		// the Cartographer ash-blue - the same colours their dialogue plate uses.
		for (TActorIterator<AOMNIQuestGiver> It(W); It; ++It)
		{
			Blip(It->GetActorLocation(), FLinearColor(1.f, 0.82f, 0.32f), 8.f);
		}
		for (TActorIterator<AOMNIFragment> It(W); It; ++It) { Blip(It->GetActorLocation(), FLinearColor(0.35f, 0.95f, 1.f), 6.f); }
		for (TActorIterator<AOMNICache> It(W); It; ++It) { Blip(It->GetActorLocation(), FLinearColor(0.3f, 1.f, 0.45f), 9.f); }
		for (TActorIterator<AOMNIWardenPrime> It(W); It; ++It) { Blip(It->GetActorLocation(), FLinearColor(1.f, 0.22f, 0.18f), 11.f); }
		for (TActorIterator<AVEILFactionAgent> It(W); It; ++It)
		{
			Blip(It->GetActorLocation(), It->Faction == EVEILFaction::Guardian ? FLinearColor(0.55f, 1.f, 0.55f)
				: It->Faction == EVEILFaction::Remnant ? FLinearColor(0.75f, 0.75f, 1.f)
				: FLinearColor(1.f, 0.45f, 0.35f), 5.f);
		}

		// The runner, always dead centre, always pointing up: the map turns, not
		// the eye. A chevron so facing is readable at a glance.
		{
			const float RXP = MX + MS * 0.5f, RYP = MY + MS * 0.5f;
			// THE RUNNER IS A PHI DIAMOND, not a 4 x 4 square: at the centre of a gold-ringed
			// compass the player's own mark should be the brightest diamond on the map.
			FVeilUI::Diamond(this, RXP, RYP, 6.f, FLinearColor(1.f, 0.90f, 0.45f));
			FVeilUI::Diamond(this, RXP, RYP, 3.f, FLinearColor(0.10f, 0.07f, 0.02f, 0.95f));
			DrawLine(RXP, RYP - 4.f, RXP - 6.f, RYP + 7.f, FLinearColor(1.f, 0.9f, 0.45f, 0.95f), 2.f);
			DrawLine(RXP, RYP - 4.f, RXP + 6.f, RYP + 7.f, FLinearColor(1.f, 0.9f, 0.45f, 0.95f), 2.f);
		}

		// THE COMPASS RING. N / E / S / W orbit the panel as the runner turns, so
		// "which way is north" is answered without a second diagram.
		{
			const float R = MS * 0.5f - 13.f;
			const float CXP = MX + MS * 0.5f, CYP = MY + MS * 0.5f;
			auto Compass = [&](const TCHAR* Letter, float WX, float WY, FLinearColor Col)
			{
				const float SX = -SN * WX + CS * WY;
				const float SY = -(CS * WX + SN * WY);
				DrawText(Letter, Col, CXP + SX * R - 4.f, CYP + SY * R - 6.f, Med, 0.78f);
			};
			Compass(TEXT("N"), 1.f, 0.f, FLinearColor(1.f, 0.86f, 0.42f));
			Compass(TEXT("E"), 0.f, 1.f, FLinearColor(0.82f, 0.86f, 0.92f));
			Compass(TEXT("S"), -1.f, 0.f, FLinearColor(0.62f, 0.66f, 0.74f));
			Compass(TEXT("W"), 0.f, -1.f, FLinearColor(0.82f, 0.86f, 0.92f));
		}

		// OFF-SCREEN OBJECTIVE ARROW: the Keeper until the chain moves on, then the
		// next beacon. When it is outside the panel the arrow rides the edge.
		{
			const AActor* Goal = nullptr;
			for (TActorIterator<AOMNIQuestGiver> It(W); It && !Goal; ++It) { Goal = *It; }
			if (Goal)
			{
				float GX, GY;
				ToScreen(Goal->GetActorLocation(), GX, GY);
				const bool bOutside = GX < MX || GX > MX + MS || GY < MY || GY > MY + MS;
				if (bOutside)
				{
					const float CX = MX + MS * 0.5f, CY = MY + MS * 0.5f;
					FVector2D Dir(GX - CX, GY - CY);
					Dir.Normalize();
					const float AX = CX + Dir.X * (MS * 0.5f - 10.f);
					const float AY = CY + Dir.Y * (MS * 0.5f - 10.f);
					const FLinearColor AGold(1.f, 0.78f, 0.25f, 0.95f);
					DrawLine(AX, AY, AX - Dir.X * 12.f, AY - Dir.Y * 12.f, AGold, 3.f);
					DrawText(TEXT("KEEPER"), AGold, AX - 22.f, AY + (Dir.Y > 0.f ? -16.f : 6.f), Med, 0.62f);
				}
			}
		}

		DrawText(Map ? FString::Printf(TEXT("LIVE MAP  %.0fm"), E * 2.f / 100.f) : FString(TEXT("LIVE MAP")),
			FLinearColor(0.85f, 0.88f, 0.94f, 0.8f), MX + 6.f, MY + MS + 6.f, Med, 0.6f);
	}

	// THE CLOCK, ALWAYS ON. Real seconds, Fibonacci beats, a day that closes
	// without wearing anyone down. Sits above the minimap so it reads without
	// hunting for it.
	if (const UVEILTime* Clock = W->GetSubsystem<UVEILTime>())
	{
		const FString Line = Clock->ClockLine();
		float LW = 0.f, LH = 0.f;
		Canvas->StrLen(Med, Line, LW, LH);
		// TOP CENTRE, NOT OVER THE RING. Anchored to ClipX-30 the clock sat on the
		// minimap's crown - two instruments in one zone. Centred, it keeps the phi
		// safe margin from the top edge and clears both the ring and the left plate.
		const float LX = FMath::Max(24.f, (Canvas->ClipX - LW) * 0.5f);
		FVeilUI::Chip(this, FLinearColor(0.05f, 0.06f, 0.10f, 0.66f), LX - 10.f, 5.f, LW + 20.f, 21.f);
		DrawText(Line, FLinearColor(1.f, 0.87f, 0.42f), LX, 8.f, Med, 0.8f);

		// THE DEBUG STRIPS STAND DOWN WHILE A PAGE IS OPEN. They were drawn after
		// the chart and printed straight through it - the eye caught the lock lines
		// and the OMNI readout sitting across the beacon list. A page owns the
		// frame; instrument text comes back when it closes.
		// DEBUG TEXT LIVES BEHIND F10. The phase lock, the machine's phinary brain, the
		// world's intent and the OMNI field are instrumentation: on screen they filled
		// the lower right and made the game read as a debug build. F10 brings them back.
		const AVEILPlayerController* StripPC = Cast<AVEILPlayerController>(PC);
		const bool bStripPage = !StripPC || !StripPC->bShowDebug
			|| StripPC->bShowChart || StripPC->bShowFieldManual;

		// THE 60 FPS PHASE LOCK, always visible while it is holding the frame.
		if (const UVEILPerf* Lock = bStripPage ? nullptr : W->GetSubsystem<UVEILPerf>())
		{
			const FString LockLine = Lock->LockLine();
			float LW2 = 0.f, LH2 = 0.f;
			Canvas->StrLen(Med, LockLine, LW2, LH2);
			const float LX2 = Canvas->ClipX - 30.f - LW2;
			const float LY2 = Canvas->ClipY - 88.f;
			FVeilUI::Chip(this, FLinearColor(0.05f, 0.07f, 0.11f, 0.60f), LX2 - 10.f, LY2 - 5.f, LW2 + 20.f, 19.f);
			DrawText(LockLine, Lock->IsLocked() ? FLinearColor(0.62f, 1.f, 0.74f) : FLinearColor(1.f, 0.72f, 0.42f),
				LX2, LY2, Med, 0.68f);

			// THE OMNI LOCK, under it: the shared phase, the drift, and the count of
			// frames that missed. A held game reads misses of zero, so an error is an
			// event you can see happen rather than the resting state of the frame.
			const FString OmniLine = Lock->OmniLine();
			float OW = 0.f, OH = 0.f;
			Canvas->StrLen(Med, OmniLine, OW, OH);
			const float OX = Canvas->ClipX - 30.f - OW;
			// Above the lock line, not under it: the machine readouts own the strip
			// below, and an overlap made both unreadable.
			const float OY = LY2 - 20.f;
			FVeilUI::Chip(this, FLinearColor(0.05f, 0.07f, 0.11f, 0.50f), OX - 10.f, OY - 4.f, OW + 20.f, 17.f);
			const bool bClean = Lock->GetInPhasePct() >= 100.f * (1.f - 1.f / (1.618033988749895f * 1.618033988749895f * 1.618033988749895f * 1.618033988749895f * 1.618033988749895f * 1.618033988749895f));
			DrawText(OmniLine, bClean ? FLinearColor(0.72f, 0.9f, 1.f) : FLinearColor(1.f, 0.78f, 0.5f),
				OX, OY, Med, 0.6f);
		}

		// THE NEAREST MACHINE'S BRAIN. Its decision is not a flag on a HUD: it is a
		// numeral in base phi, and every place in it carries weight.
		{
			const AVEILEnemy* Nearest = nullptr;
			float BestDist = 6000.f;
			for (TActorIterator<AVEILEnemy> It(W); It; ++It)
			{
				const float D = (float)FVector::Dist(Pawn->GetActorLocation(), It->GetActorLocation());
				if (D < BestDist) { BestDist = D; Nearest = *It; }
			}
			if (!bStripPage && Pawn && Nearest && Nearest->Brain)
			{
				const FString BrainLine = FString::Printf(TEXT("MACHINE  %s = %s phi   %.0f cm"),
					*UPHI_NPCProcessor::IntentName(Nearest->Brain->GetIntent()),
					Nearest->Brain->PhinaryState.IsEmpty() ? TEXT("-") : *Nearest->Brain->PhinaryState,
					BestDist);
				float MW2 = 0.f, MH2 = 0.f;
				Canvas->StrLen(Med, BrainLine, MW2, MH2);
				const float MX5 = Canvas->ClipX - 30.f - MW2;
				const float MY5 = Canvas->ClipY - 66.f;
				FVeilUI::Chip(this, FLinearColor(0.05f, 0.07f, 0.11f, 0.60f), MX5 - 10.f, MY5 - 5.f, MW2 + 20.f, 19.f);
				DrawText(BrainLine, FLinearColor(1.f, 0.80f, 0.52f), MX5, MY5, Med, 0.70f);
			}
		}

		// THE WORLD'S OWN INTENT: measured chaos folded through phi, always live.
		// Small and out of the way, bottom right, because it is a fact about the
		// world rather than an instruction to the player.
		if (const UPHIIntent* SelfUpdate = bStripPage ? nullptr : W->GetSubsystem<UPHIIntent>())
		{
			const FString IntentLine = SelfUpdate->IntentLine();
			float IW = 0.f, IH = 0.f;
			Canvas->StrLen(Med, IntentLine, IW, IH);
			const float IX = Canvas->ClipX - 30.f - IW;
			const float IY = Canvas->ClipY - 44.f;
			FVeilUI::Chip(this, FLinearColor(0.06f, 0.05f, 0.11f, 0.60f), IX - 10.f, IY - 5.f, IW + 20.f, 19.f);
			DrawText(IntentLine, FLinearColor(0.74f, 0.84f, 0.98f), IX, IY, Med, 0.70f);
		}

		// THE FIELD UNDERNEATH ALL OF IT: every site held as several states at
		// once, and only the observed ones decided. Not a progress bar - a count of
		// what the world is still holding open.
		if (const UPHIOMNI* Omni = bStripPage ? nullptr : W->GetSubsystem<UPHIOMNI>())
		{
			const FString OmniText = Omni->OmniLine();
			float OW2 = 0.f, OH2 = 0.f;
			Canvas->StrLen(Med, OmniText, OW2, OH2);
			const float OX2 = Canvas->ClipX - 30.f - OW2;
			// Its own row, ABOVE the lock strips. The column from the bottom is:
			// PHI INTENT -44, MACHINE -66, PHI LOCK -88, OMNI LOCK -108 - and the eye
			// caught this line printed through PHI LOCK when it sat at -88.
			const float OY2 = Canvas->ClipY - 130.f;
			FVeilUI::Chip(this, FLinearColor(0.07f, 0.05f, 0.11f, 0.60f), OX2 - 10.f, OY2 - 4.f, OW2 + 20.f, 19.f);
			DrawText(OmniText, FLinearColor(0.86f, 0.76f, 1.f), OX2, OY2, Med, 0.70f);
		}

		// A second line only when it is true: the runner is ahead of the clock and
		// the world is running slower to catch up to him.
		if (Clock->IsOutsideTime())
		{
			const FString Phase = Clock->PhaseLine();
			float PhaseW = 0.f, PhaseH = 0.f;
			Canvas->StrLen(Med, Phase, PhaseW, PhaseH);
			const float PhaseX = Canvas->ClipX - 30.f - PhaseW;
			FVeilUI::Chip(this, FLinearColor(0.04f, 0.08f, 0.10f, 0.72f), PhaseX - 10.f, 27.f, PhaseW + 20.f, 21.f);
			DrawText(Phase, FLinearColor(0.58f, 0.98f, 1.f), PhaseX, 30.f, Med, 0.8f);
		}
	}

	// THE KEY IN PLAY: one strip along the bottom-left, out of the shot - and out
	// of the way entirely while somebody is speaking, because the dialogue box
	// owns that band.
	{
		const UVEILDialogue* Talking = W->GetSubsystem<UVEILDialogue>();
		const AVEILPlayerController* LegendPC = Cast<AVEILPlayerController>(PC);
		// ...and it stands down while a page is open: the eye caught it drawing
		// straight through the chart panel, so the same key appeared twice.
		const bool bPageUp = LegendPC && (LegendPC->bShowChart || LegendPC->bShowFieldManual);
		if ((!Talking || !Talking->IsActive()) && !bPageUp)
		{
			DrawMapLegend(Canvas, Med, 30.f, Canvas->ClipY - 118.f, false);

			// QUEST BANNER, bottom left, in gold cloth: the side objective the vision
			// carries above the legend, so the screen always names the work in progress.
			{
				// A BANNER, NOT A BAR ACROSS THE CORNER: shorter, lighter, one line.
				// ACTIVITIES ON ITS OWN BAND. It was the mockup's line, drawn 16 px above
				// its own banner while the canvas ribbon sat at ClipY-146: three texts in
				// one strip. It now sits one phi step above the ribbon and nothing else does.
				if (const UVEILCampaignSubsystem* Camp = W->GetSubsystem<UVEILCampaignSubsystem>())
				{
					DrawText(Camp->ActivityLine(), FLinearColor(0.788f, 0.635f, 0.290f, 0.95f),
						30.f, Canvas->ClipY - 146.f - 26.f, Med, 0.56f);
				}
				const float QY = Canvas->ClipY - 146.f;
				float QW = 0.f, QH = 0.f;
				Canvas->StrLen(Med, FString(TEXT("THE GOLDEN MIDST")), QW, QH);
				FVeilUI::Ribbon(this, FLinearColor(0.16f, 0.11f, 0.03f, 0.88f), 30.f, QY, QW * 0.86f + 46.f, 15.f, true);
				DrawText(TEXT("QUEST"), FLinearColor(0.62f, 0.47f, 0.20f), 38.f, QY + 2.f, Med, 0.5f);
				DrawText(TEXT("THE GOLDEN MIDST"), FLinearColor(1.f, 0.88f, 0.50f), 76.f, QY + 2.f, Med, 0.58f);
				// The step rides on the same line, dimmer: two lines of banner was a block.
				DrawText(TEXT("Find the hidden phi chamber"), FLinearColor(0.86f, 0.82f, 0.68f),
					76.f + QW * 0.50f + 14.f, QY + 3.f, Med, 0.48f);
			}

			// COMBAT MODE, bottom centre, with diamond icons above it: the state the
			// player is in, held in the same gold as everything else.
			{
				const float CCX = Canvas->ClipX * 0.5f;
				const float CY = Canvas->ClipY - 84.f;
				FVeilUI::Ribbon(this, FLinearColor(0.30f, 0.18f, 0.02f, 0.92f), CCX - 88.f, CY, 176.f, 22.f, false);
				DrawText(TEXT("COMBAT MODE"), FLinearColor(1.f, 0.92f, 0.62f), CCX - 56.f, CY + 4.f, Med, 0.78f);
				const FLinearColor IconGold(1.f, 0.84f, 0.38f, 0.92f);
				for (int32 Icon = 0; Icon < 5; ++Icon)
				{
					FVeilUI::Diamond(this, CCX - 64.f + Icon * 32.f, CY - 18.f, 8.f, IconGold);
				}
			}

			// REAL ICONS, NOT DRAWN SHAPES. This project ships the Polyart inventory art, so
			// the pack strip wears those textures - one per slot, labelled with the item it
				// actually is. A drawn diamond is a placeholder; a texture is an asset. Nothing
			// here is labelled an ability, because no ability system is tracking charges yet.
			{
				static TArray<UTexture2D*> PackIcons;
				if (PackIcons.Num() == 0)
				{
					const TCHAR* Base = TEXT("/Game/ModularS30eeb54ea3e4V1/data/Content/Polyart/ModularStylizedChars/Textures/Inventory/");
					const TCHAR* Names[] = {
						TEXT("T_MaleArmor_InvIcon"), TEXT("T_MaleBelt_InvIcon"),
						TEXT("T_MaleBoots_InvIcon"), TEXT("T_MaleGloves_InvIcon") };
					for (const TCHAR* N : Names)
					{
						const FString Path = FString::Printf(TEXT("%s%s.%s"), Base, N, N);
						PackIcons.Add(LoadObject<UTexture2D>(nullptr, *Path));
					}
				}
				const TCHAR* Labels[] = { TEXT("ARMOR"), TEXT("BELT"), TEXT("BOOTS"), TEXT("GLOVES") };
				int32 Loaded = 0;
				for (int32 i = 0; i < 4; ++i)
				{
					const float IX = Canvas->ClipX * 0.5f - 168.f + i * 88.f;
					const float IY = Canvas->ClipY - 132.f;
					// A RESOURCE IS NOT A RESIDENT TEXTURE. This drew whenever GetResource()
					// was non-null, and the run crashed on the render thread in
					// RDGExecute_RenderThread / CanvasRenderThreadScope with an access violation
					// at 0xffffffffffffffff, immediately after "3 of 4 pack icons loaded". In
					// -game the streamer has not filled the RHI texture, and FCanvas dereferences
					// it while drawing. Textures are drawn in the editor only; a -game run gets
					// the labelled slot, which cannot fault.
					const bool bIconDrawable = GIsEditor
						&& PackIcons.IsValidIndex(i) && PackIcons[i]
						&& PackIcons[i]->GetResource() != nullptr
						&& PackIcons[i]->GetSizeX() > 0;
					if (bIconDrawable)
					{
						DrawTexture(PackIcons[i], IX, IY, 36.f, 36.f, 0.f, 0.f, 1.f, 1.f,
							FLinearColor::White, BLEND_Translucent);
						++Loaded;
					}
					else if (PackIcons.IsValidIndex(i) && PackIcons[i])
					{
						// Loaded, but with no resident texture: the canvas cannot draw it, and it
						// asserted on exactly this once. Named ONCE for the whole strip, never per
						// frame - a per-frame log is its own defect, and the label still marks the
						// slot, so nothing vanishes silently.
						static bool bIconGapNamed = false;
						if (!bIconGapNamed)
						{
							bIconGapNamed = true;
							UE_LOG(LogTemp, Warning,
								TEXT("VEIL HUD: pack icons load without resident textures in -game; drawing labels only, once."));
						}
					}
					DrawText(Labels[i], FLinearColor(0.78f, 0.70f, 0.54f), IX, IY + 38.f, Med, 0.56f);
				}
				if (Loaded > 0 && Loaded < 4)
				{
					UE_LOG(LogTemp, Warning, TEXT("VEIL HUD: %d of 4 pack icons loaded from the Polyart inventory set."), Loaded);
				}
			}

			// TEXT AS AN OPTION, NOT ONLY VOICE. A player who cannot hear a line still gets
			// the whole line: the speaker and the full sentence on cloth whenever a
			// conversation is active. The voice still plays - this is the copy of it, not a
			// replacement, so nothing has to be configured to read along.
			if (const UVEILDialogue* DlgText = W->GetSubsystem<UVEILDialogue>())
			{
				if (DlgText->IsActive())
				{
					const FString Line = DlgText->GetFullText();
					if (!Line.IsEmpty())
					{
						float LW = 0.f, LH = 0.f;
						Canvas->StrLen(Med, Line, LW, LH);
						const float BW2 = FMath::Min(Canvas->ClipX - 140.f, LW + 56.f);
						const float BX2 = (Canvas->ClipX - BW2) * 0.5f;
						const float BY2 = Canvas->ClipY - 224.f;
						FVeilUI::Ribbon(this, FLinearColor(0.07f, 0.06f, 0.04f, 0.88f), BX2, BY2, BW2, 56.f, true);
						DrawText(DlgText->GetSpeakerName(), FLinearColor(1.f, 0.88f, 0.52f), BX2 + 22.f, BY2 + 7.f, Med, 0.70f);
						DrawText(Line, FLinearColor(0.93f, 0.91f, 0.85f), BX2 + 22.f, BY2 + 26.f, Med, 0.80f);
						DrawText(TEXT("TEXT"), FLinearColor(0.60f, 0.54f, 0.40f), BX2 + BW2 - 48.f, BY2 + 7.f, Med, 0.52f);
					}
				}
			}
		}
	}

	// ------------------------------------------------------------------
	// FIELD MANUAL CARD. It teaches by watching: the step advances when the
	// gesture actually happens, and the glyph follows the device in hand, so a
	// pad player is never told to press TAB.
	// ------------------------------------------------------------------
	if (UVEILTutorial* Tut = W->GetSubsystem<UVEILTutorial>())
	{
		// A full page supersedes the step card while it is open: the manual, or the
		// chart with its destination list.
		const AVEILPlayerController* CardPC = Cast<AVEILPlayerController>(PC);
		const bool bPageOpen = CardPC && (CardPC->bShowFieldManual || CardPC->bShowChart);
		// A CONVERSATION OWNS THE FRAME. The card was drawn over the open dialogue
		// box - the eye showed MOVE sitting on top of Sera Vahl's line - so the
		// step card stands down while somebody is speaking and comes back after.
		const bool bTalking = W->GetSubsystem<UVEILDialogue>()
			&& W->GetSubsystem<UVEILDialogue>()->IsActive();
		if (!Tut->IsHidden() && !bPageOpen && !bTalking)
		{
			// THE CARD IS A CARD, NOT A PANEL. 360 x 138 covered the lower right of the
			// district; 252 x 104 still teaches the gesture without hiding the world.
			// SPEC IS 320 x 90. At 252 x 104 the card was narrower and taller than the brief
			// allows - a column of text where a single wide line belongs.
			const float CW = 320.f;
			const float CX = Canvas->ClipX - CW - 30.f;
			// Under the map, not on top of it: the round instrument now reaches
			// 30 + 220 px, so the card starts clear of the ring's halo.
			const float CY = 30.f + 244.f;
			const float CH = 90.f;
			const FVEILTutorialStep& S = Tut->GetStep();
			const bool bDone = Tut->IsComplete();

			// 85%, BECAUSE THE BIBLE IS THE LAW. The spec sheet for this pass says 55% so
			// the world shows through the smaller windows; the Bible says "fill Navy #0E1424
			// at 85% opacity" (Docs/VEIL_Bible.md:48). The Bible wins: one number, stated
			// once, and every panel reads the same. This card was the one panel that had
			// drifted to 82%, so it is the one line changed.
			FVeilUI::Glass(this, FLinearColor(0.022f, 0.030f, 0.056f, 0.85f), CX, CY, CW, CH, 12.f);
			FVeilUI::RoundRect(this, FLinearColor(1.f, 0.80f, 0.32f, 0.92f), CX + 1.f, CY + 12.f, 3.f, CH - 24.f, 1.5f);

			DrawText(bDone ? FString(TEXT("FIELD MANUAL"))
				: FString::Printf(TEXT("FIELD MANUAL   %d OF %d"), Tut->GetStepIndex() + 1, Tut->NumSteps()),
				FLinearColor(0.52f, 0.60f, 0.74f), CX + 16.f, CY + 9.f, Med, 0.66f);

			// The button itself, drawn as a chip: the shape is the hint.
			const FString Glyph = Tut->GetGlyph();
			const float GS = 0.66f;
			float GW = 0.f, GH = 0.f;
			Canvas->StrLen(Big, Glyph, GW, GH);
			const float ChipW = GW * GS + 24.f;
			const float ChipX = CX + CW - ChipW - 14.f;
			// THE KEY IS A KEY: a solid gold cap with dark lettering, the way a keycap
			// looks in the hand, rather than a wireframe box around a word.
			FVeilUI::RoundRect(this, FLinearColor(0.10f, 0.07f, 0.02f, 0.85f), ChipX + 1.f, CY + 12.f, ChipW, 28.f, 6.f);
			FVeilUI::RoundRect(this, FLinearColor(1.f, 0.82f, 0.35f, 0.96f), ChipX, CY + 9.f, ChipW, 28.f, 6.f);
			FVeilUI::RoundRect(this, FLinearColor(1.f, 0.95f, 0.72f, 0.55f), ChipX + 3.f, CY + 11.f, ChipW - 6.f, 10.f, 5.f);
			DrawText(Glyph, FLinearColor(0.12f, 0.08f, 0.02f), ChipX + 12.f, CY + 14.f, Big, GS);

			DrawText(S.Action, FLinearColor(1.f, 0.86f, 0.34f), CX + 16.f, CY + 36.f, Big, 0.80f);

			// Wrapped hint, two or three lines at the plate's width.
			TArray<FString> Words;
			S.Hint.ParseIntoArray(Words, TEXT(" "), true);
			FString Line;
			float HY = CY + 62.f;
			for (int32 i = 0; i < Words.Num(); ++i)
			{
				const FString Try = Line.IsEmpty() ? Words[i] : Line + TEXT(" ") + Words[i];
				float XL = 0.f, YL = 0.f;
				Canvas->StrLen(Med, Try, XL, YL);
				if (XL > CW - 34.f && !Line.IsEmpty())
				{
					DrawText(Line, FLinearColor(0.80f, 0.85f, 0.92f), CX + 16.f, HY, Med, 0.70f);
					HY += 15.f;
					Line = Words[i];
				}
				else { Line = Try; }
			}
			if (!Line.IsEmpty())
			{
				DrawText(Line, FLinearColor(0.80f, 0.85f, 0.92f), CX + 16.f, HY, Med, 0.70f);
			}

			// Progress: one pip per gesture, filled when it has been taught.
			for (int32 i = 0; i < Tut->NumSteps(); ++i)
			{
				const bool bLit = bDone || i < Tut->GetStepIndex();
				DrawRect(bLit ? FLinearColor(1.f, 0.82f, 0.35f, 0.9f) : FLinearColor(1.f, 1.f, 1.f, 0.16f),
					CX + 16.f + i * 12.f, CY + CH - 13.f, 7.f, 4.f);
			}
			// The current step's own progress, a hairline under the pips.
			if (!bDone)
			{
				DrawRect(FLinearColor(0.42f, 0.72f, 1.f, 0.65f),
					CX + 16.f + Tut->GetStepIndex() * 12.f, CY + CH - 17.f, 7.f * FMath::Clamp(Tut->GetStepProgress(), 0.f, 1.f), 2.f);
			}
		}
	}

	// ------------------------------------------------------------------
	// NAMEPLATES. In Skyrim you know who a guard is before he speaks; the
	// world names its own people. Each machine carries its faction's name and
	// a title drawn from the cast, fading with distance rather than popping.
	// ------------------------------------------------------------------
	if (Pawn)
	{		const FVector Eye = Pawn->GetActorLocation();
		// Two people standing together must not print one name through the
		// other: a plate stacks upward until it has clear air.
		TArray<FVector2D> Taken;
		auto Plate = [&](const FVector& WorldLoc, const FString& Title, const FString& Sub, FLinearColor C, float Near, float Far)
			{
				const float D = (float)FVector::Dist(Eye, WorldLoc);
				if (D > Far) { return; }
				const FVector S = Canvas->Project(WorldLoc + FVector(0.f, 0.f, 118.f));
				if (S.Z <= 0.f) { return; }
				// Off-screen actors project to phantom points at the edge; a plate
				// there belongs to nobody.
				if (S.X < 24.f || S.X > Canvas->ClipX - 24.f || S.Y < 24.f || S.Y > Canvas->ClipY - 24.f) { return; }
				// The directive column owns the top-left corner. A world label
				// printed through it does not read as a name - it reads as a bug.
				if (S.X < Canvas->ClipX * 0.46f && S.Y < Canvas->ClipY * 0.46f) { return; }
				const float A = FMath::Clamp((Far - D) / FMath::Max(1.f, Far - Near), 0.f, 1.f);
				if (A < 0.08f) { return; }
				float XL = 0.f, YL = 0.f;
				Canvas->StrLen(Med, Title, XL, YL);
				const float X = S.X - XL * 0.5f;
				float TY = S.Y;
				for (int32 Guard = 0; Guard < 4; ++Guard)
				{
					bool bClash = false;
					for (const FVector2D& T : Taken)
					{
						// A plate is two lines tall (19px title + 17px sub), so the step
						// that clears a neighbour has to be a plate, not a line.
						if (FMath::Abs(T.X - S.X) < 150.f && FMath::Abs(T.Y - TY) < 40.f) { bClash = true; break; }
					}
					if (!bClash) { break; }
					TY -= 42.f;
				}
				Taken.Add(FVector2D(S.X, TY));
				// A soft backing so the name stays legible over any surface.
				DrawRect(FLinearColor(0.f, 0.f, 0.02f, 0.38f * A), X - 8.f, TY - 2.f, XL + 16.f, 19.f);
				DrawText(Title, FLinearColor(0.f, 0.f, 0.f, 0.9f * A), X + 1.f, TY + 1.f, Med, 0.86f);
				DrawText(Title, FLinearColor(C.R, C.G, C.B, A), X, TY, Med, 0.86f);
				if (!Sub.IsEmpty())
				{
					float SL = 0.f, SH = 0.f;
					Canvas->StrLen(Med, Sub, SL, SH);
					DrawText(Sub, FLinearColor(0.78f, 0.84f, 0.92f, A * 0.72f), S.X - SL * 0.5f, TY + 17.f, Med, 0.7f);
				}
				DrawRect(FLinearColor(C.R, C.G, C.B, A * 0.35f), S.X - 22.f, TY + 34.f, 44.f, 1.f);
			};

		// The three talking statues are people with names, ranks and creeds.
		for (TActorIterator<AOMNIQuestGiver> It(W); It; ++It)
		{
			const FName Who = VEILCast::CharacterForGiver(It->GiverName);
			const FVEILCharacterDef* C = VEILCast::FindCharacter(Who);
			if (!C) { continue; }
			Plate(It->GetActorLocation(), C->Name.ToString(), C->Title.ToString(),
				VEILCast::ColorOf(Who), 900.f, 9000.f);
		}
		// Machines wear their faction's colours and a member's name, so a
		// patrol reads as Ashfall cutters or Watch shields - not "enemy 3".
		for (TActorIterator<AVEILFactionAgent> It(W); It; ++It)
		{
			const FName Who = VEILCast::CharacterForFaction(It->Faction, It->GetUniqueID());
			const FVEILCharacterDef* C = VEILCast::FindCharacter(Who);
			if (!C) { continue; }
			const FVEILFactionDef* F = VEILCast::FindFaction(C->FactionId);
			Plate(It->GetActorLocation(), C->Name.ToString(),
				F ? F->Name.ToString() : FString(TEXT("UNALIGNED")), VEILCast::ColorOf(Who), 400.f, 4200.f);
		}
	}

	if (AVEILPlayerController* CPC = Cast<AVEILPlayerController>(PC))
	{
		if (CPC->bShowChart && CPC->ChartPage == 0)
		{
			const float CX = Canvas->ClipX * 0.5f - 330.f, CY = Canvas->ClipY * 0.5f - 240.f;
			FVeilUI::Glass(this, FLinearColor(0.020f, 0.030f, 0.062f, 0.92f), CX, CY, 660.f, 480.f, 16.f);
			float LY = CY + 30.f;
			DrawText(TEXT("CHART OF THE SIGNAL"), FLinearColor(1.f, 0.85f, 0.3f), CX + 30.f, LY, Big); LY += 52.f;
			// THE SAME QUESTION AS THE HUD CARD, ASKED THE SAME WAY: the region
			// subsystem knows where he is, the quest system's string was never
			// written, so this printed "You stand in" and stopped.
			FString WhereHeStands = Q ? Q->GetRegionName() : TEXT("the signal");
			if (const UVEILRegionSubsystem* Regions = W->GetSubsystem<UVEILRegionSubsystem>())
			{
				WhereHeStands = Regions->GetInfo(Regions->Current).Name.ToString();
			}
			DrawText(FString::Printf(TEXT("You stand in %s"), *WhereHeStands), FLinearColor::White, CX + 30.f, LY, Med); LY += 34.f;
			for (TActorIterator<AOMNIQuestGiver> It(W); It; ++It)
			{
				const FVector L = It->GetActorLocation();
				DrawText(FString::Printf(TEXT("%s  X%.0f Y%.0f"), *It->GiverName.ToString(), L.X, L.Y), FLinearColor(0.7f, 1.f, 0.7f), CX + 30.f, LY, Med); LY += 28.f;
			}
			// ----------------------------------------------------------
			// BEACONS. Free travel: every lit beacon is a destination, and
			// the runner is carried there inside the same world.
			// ----------------------------------------------------------
			if (UVEILFastTravel* FT = W->GetSubsystem<UVEILFastTravel>())
			{
				LY += 12.f;
				DrawText(TEXT("BEACONS   [UP / DOWN] choose    [ENTER] or [A] travel"),
					FLinearColor(0.52f, 0.60f, 0.74f), CX + 30.f, LY, Med, 0.8f);
				LY += 26.f;
				for (int32 i = 0; i < FT->Waypoints.Num(); ++i)
				{
					const FVEILWaypoint& WP = FT->Waypoints[i];
					const bool bSel = (i == CPC->ChartSel);
					const float Dist = Pawn ? FVector::Dist(Pawn->GetActorLocation(), WP.Location) : 0.f;
					if (bSel)
					{
						DrawRect(FLinearColor(1.f, 0.80f, 0.32f, 0.20f), CX + 22.f, LY - 4.f, 616.f, 24.f);
						DrawRect(FLinearColor(1.f, 0.80f, 0.32f, 0.95f), CX + 22.f, LY - 4.f, 3.f, 24.f);
					}
					DrawText(bSel ? FString(TEXT(">")) : FString(TEXT(" ")),
						FLinearColor(1.f, 0.86f, 0.38f), CX + 30.f, LY, Med, 0.92f);
					DrawText(FString::Printf(TEXT("%d"), i + 1),
						bSel ? FLinearColor(1.f, 0.9f, 0.55f) : FLinearColor(0.5f, 0.55f, 0.64f), CX + 42.f, LY, Med, 0.86f);
					DrawText(WP.Name,
						WP.bLit ? FLinearColor(0.90f, 0.94f, 0.99f) : FLinearColor(0.44f, 0.48f, 0.55f), CX + 74.f, LY, Med, 0.86f);
					const FString Right = WP.bLit
						? FString::Printf(TEXT("LIT   %.0f m"), Dist / 100.f)
						: FString(TEXT("UNLIT - walk there first"));
					DrawText(Right, WP.bLit ? FLinearColor(1.f, 0.84f, 0.42f) : FLinearColor(0.40f, 0.44f, 0.50f),
						CX + 380.f, LY, Med, 0.8f);
					LY += 23.f;
				}
			}

			DrawText(TEXT("East door -> OMNIWorld     West door -> VEILWorld     North door -> CountessRegion     [TAB] shut"),
				FLinearColor(0.7f, 0.9f, 1.f), CX + 30.f, LY + 8.f, Med, 0.86f);

			// THE KEY, in the empty upper-right of the panel: a chart with coloured
			// marks and no legend is a puzzle, not a map. Down by the door line it
			// printed over the doors, which the eye caught immediately.
			DrawMapLegend(Canvas, Med, CX + 380.f, CY + 34.f, true);
		}

		// ----------------------------------------------------------
		// THE FIELD MANUAL PAGE. Every action the game binds, on both
		// devices at once, with the one in your hand lit. This is the page
		// that answers "which button does what" without leaving the game.
		// ----------------------------------------------------------
		if (CPC->bShowFieldManual)
		{
			const TArray<FVEILInputRow>& Table = VEILInput::Rows();
			const float MW = 780.f;
			const float MH = 96.f + Table.Num() * 25.f + 52.f;
			const float MX2 = (Canvas->ClipX - MW) * 0.5f;
			const float MY2 = (Canvas->ClipY - MH) * 0.5f;
			const bool bPadCol = VEILInput::IsGamepad();

			FVeilUI::Glass(this, FLinearColor(0.018f, 0.026f, 0.050f, 0.94f), MX2, MY2, MW, MH, 16.f);
			DrawRect(FLinearColor(1.f, 0.78f, 0.3f, 0.95f), MX2, MY2, 3.f, MH);
			DrawRect(FLinearColor(1.f, 0.78f, 0.3f, 0.3f), MX2, MY2, MW, 1.f);

			DrawText(TEXT("FIELD MANUAL  -  CONTROLS"), FLinearColor(1.f, 0.88f, 0.36f), MX2 + 28.f, MY2 + 20.f, Big, 1.15f);
			DrawText(TEXT("Every action this game binds, on both devices. The column in use is lit."),
				FLinearColor(0.62f, 0.68f, 0.78f), MX2 + 30.f, MY2 + 52.f, Med, 0.86f);

			const float ColA = MX2 + 30.f, ColK = MX2 + 340.f, ColP = MX2 + 540.f;
			float RY = MY2 + 82.f;
			DrawText(TEXT("ACTION"), FLinearColor(0.45f, 0.50f, 0.60f), ColA, RY, Med, 0.78f);
			DrawText(TEXT("KEYBOARD"), bPadCol ? FLinearColor(0.45f, 0.50f, 0.60f) : FLinearColor(0.55f, 0.95f, 1.f), ColK, RY, Med, 0.78f);
			DrawText(TEXT("CONTROLLER"), bPadCol ? FLinearColor(0.55f, 0.95f, 1.f) : FLinearColor(0.45f, 0.50f, 0.60f), ColP, RY, Med, 0.78f);
			RY += 20.f;
			DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.12f), MX2 + 28.f, RY, MW - 56.f, 1.f);
			RY += 8.f;

			for (const FVEILInputRow& Row : Table)
			{
				DrawText(Row.Action, FLinearColor(0.86f, 0.90f, 0.96f), ColA, RY, Med, 0.86f);
				DrawText(Row.Key, bPadCol ? FLinearColor(0.62f, 0.66f, 0.72f) : FLinearColor(1.f, 0.92f, 0.60f), ColK, RY, Med, 0.86f);
				DrawText(Row.Pad.IsEmpty() ? FString(TEXT("-")) : Row.Pad,
					bPadCol ? FLinearColor(1.f, 0.92f, 0.60f) : FLinearColor(0.62f, 0.66f, 0.72f), ColP, RY, Med, 0.86f);
				RY += 25.f;
			}

			DrawText(bPadCol ? TEXT("D-PAD LEFT closes this page    D-PAD DOWN mutes the narrator    VIEW opens the chart")
				: TEXT("H or F1 closes this page    T hides the step card    M mutes the narrator"),
				FLinearColor(0.50f, 0.56f, 0.66f), MX2 + 30.f, MY2 + MH - 34.f, Med, 0.8f);
		}

		// ----------------------------------------------------------
		// THE LIVING WORLD. Who everyone is, what they can do, and what
		// they are doing at this beat of the Fibonacci day.
		// ----------------------------------------------------------
		if (CPC->bShowChart && CPC->ChartPage == 1)
		{
			const UVEILTime* Clock = W->GetSubsystem<UVEILTime>();
			const TArray<FVEILLife>& Lives = VEILSociety::Lives();
			const float MW = FMath::Min(820.f, Canvas->ClipX - 80.f);
			const float MH = FMath::Min(Canvas->ClipY - 60.f, 132.f + Lives.Num() * 26.f + 44.f);
			const float MX3 = (Canvas->ClipX - MW) * 0.5f;
			const float MY3 = (Canvas->ClipY - MH) * 0.5f;
			const bool bDuty = Clock && Clock->IsDutyBeat();

			FVeilUI::Glass(this, FLinearColor(0.018f, 0.026f, 0.050f, 0.94f), MX3, MY3, MW, MH, 16.f);
			FVeilUI::RoundRect(this, FLinearColor(0.42f, 0.78f, 1.f, 0.95f), MX3 + 1.f, MY3 + 14.f, 3.f, MH - 28.f, 1.5f);

			DrawText(TEXT("THE LIVING WORLD"), FLinearColor(0.70f, 0.90f, 1.f), MX3 + 28.f, MY3 + 18.f, Big, 1.15f);
			if (Clock)
			{
				DrawText(Clock->ClockLine(), FLinearColor(1.f, 0.86f, 0.36f), MX3 + 30.f, MY3 + 50.f, Med, 0.9f);
				DrawText(TEXT("Real time, counted in Fibonacci beats. The cycle closes; nobody ages."),
					FLinearColor(0.60f, 0.66f, 0.76f), MX3 + 30.f, MY3 + 70.f, Med, 0.82f);

				// The day's own bar: each beat a segment, sized by its true length.
				const TArray<int32>& Secs = UVEILTime::BeatSeconds();
				const float BarX = MX3 + 30.f, BarW = MW - 60.f, BarY = MY3 + 94.f;
				float Cursor = BarX;
				for (int32 i = 0; i < Secs.Num(); ++i)
				{
					const float SegW = BarW * (Secs[i] / 143.f);
					const bool bHere = (i == Clock->GetBeat() - 1);
					DrawRect(bHere ? FLinearColor(1.f, 0.82f, 0.34f, 0.95f) : FLinearColor(1.f, 1.f, 1.f, 0.16f),
						Cursor + 1.f, BarY, FMath::Max(1.f, SegW - 2.f), 6.f);
					Cursor += SegW;
				}
			}

			// A column that runs off the plate is not a column. Measure once and
			// cut the line to the space that exists, with an ellipsis.
			UFont* FitFont = Med;
			auto Fit = [this, FitFont](const FString& In, float MaxW) -> FString
			{
				if (In.IsEmpty()) { return In; }
				float XL = 0.f, YL = 0.f;
				Canvas->StrLen(FitFont, In, XL, YL);
				if (XL <= MaxW) { return In; }
				const float PerChar = XL / (float)In.Len();
				const int32 Keep = FMath::Clamp((int32)(MaxW / FMath::Max(1.f, PerChar)) - 1, 4, In.Len());
				return In.Left(Keep) + TEXT("...");
			};

			// A plate drawn for a wide window must still fit a small one: the third
			// column slides left instead of running off the edge at 640 px.
			const float ActX = MX3 + FMath::Min(612.f, MW - 236.f);
			const float ActW = FMath::Max(120.f, MW - (ActX - MX3) - 36.f);
			float RY = MY3 + 114.f;
			DrawText(TEXT("WHO"), FLinearColor(0.45f, 0.50f, 0.60f), MX3 + 30.f, RY, Med, 0.76f);
			const float MixX = MX3 + 250.f;
			const float MixW = FMath::Max(90.f, ActX - MixX - 14.f);
			DrawText(ActX < MX3 + 560.f ? TEXT("WHAT THEY CAN DO  -  NONE LOCKED")
				: TEXT("WHAT THEY CAN DO  -  ALL OPEN, NONE LOCKED"),
				FLinearColor(0.45f, 0.50f, 0.60f), MixX, RY, Med, 0.76f);
			DrawText(Clock && Clock->IsSuperposed() ? TEXT("IN THE FIELD") : (bDuty ? TEXT("ON POST") : TEXT("OFF POST")),
				FLinearColor(0.45f, 0.50f, 0.60f), ActX, RY, Med, 0.76f);
			RY += 20.f;
			DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.12f), MX3 + 28.f, RY, MW - 56.f, 1.f);
			RY += 6.f;

			for (const FVEILLife& Life : Lives)
			{
				if (RY > MY3 + MH - 44.f) { break; }
				const FVEILCharacterDef* Who = VEILCast::FindCharacter(Life.Character);
				const FString Name = Who ? Who->Name.ToString() : Life.Character.ToString();
				DrawText(Name, VEILCast::ColorOf(Life.Character), MX3 + 30.f, RY, Med, 0.82f);

				// The mix, written out: no class, just what this person carries.
				FString Mix;
				for (const FName& ArchId : VEILSociety::ArchetypesOf(Life.Character))
				{
					if (const FVEILArchetype* Arch = VEILSociety::FindArchetype(ArchId))
					{
						Mix += Mix.IsEmpty() ? Arch->Name : TEXT(" + ") + Arch->Name;
					}
				}
				DrawText(Mix.IsEmpty() ? FString(TEXT("-")) : Fit(Mix, MixW),
					FLinearColor(0.66f, 0.74f, 0.86f), MixX, RY, Med, 0.80f);
				// While the field is open a person is doing both things at once, so
				// the line shows both futures rather than pretending to know.
				FString Act = VEILSociety::ActivityNow(Life.Character, bDuty);
				if (Clock && Clock->IsSuperposed() && Clock->Candidates().Num() > 1)
				{
					const int32 B0 = Clock->Candidates()[0].Beat;
					const int32 B1 = Clock->Candidates()[1].Beat;
					Act = FString::Printf(TEXT("%s  /  %s"),
						*VEILSociety::ActivityNow(Life.Character, B0 >= 1 && B0 <= 7),
						*VEILSociety::ActivityNow(Life.Character, B1 >= 1 && B1 <= 7));
				}
				// What they can do now, in rungs. They grow on their own clock, so this
				// number is different on day 4 than it was on day 1 whether the runner
				// ever spoke to them or not.
				if (const UPHILivingWorld* Living = W->GetSubsystem<UPHILivingWorld>())
				{
					Act = FString::Printf(TEXT("%s  -  rung %.2f"), *Act, Living->PowerRungOf(Life.Character));
				}
				DrawText(Fit(Act, ActW),
					FLinearColor(0.52f, 0.60f, 0.70f), ActX, RY, Med, 0.74f);
				RY += 26.f;
			}

			const FVEILArchetype* Active = VEILSociety::Archetypes().IsValidIndex(CPC->ActiveArchetype)
				? &VEILSociety::Archetypes()[CPC->ActiveArchetype] : nullptr;
			DrawText(Active
				? FString::Printf(TEXT("RUNNER STANDS IN %s   -   1-0 switches, nothing is locked   -   TAB next page"), *Active->Name)
				: FString(TEXT("RUNNER HOLDS ALL TWELVE   -   1-0 switches, nothing is locked   -   TAB next page")),
				FLinearColor(1.f, 0.86f, 0.40f), MX3 + 30.f, MY3 + MH - 28.f, Med, 0.82f);
		}

		// ----------------------------------------------------------
		// OPTIONS. Eight real settings, each one applied the moment it
		// changes and saved, with the frame measured live underneath.
		// ----------------------------------------------------------
		if (CPC->bShowChart && CPC->ChartPage == 2)
		{
			const UVEILPerf* Perf = W->GetSubsystem<UVEILPerf>();
			// Fits inside any viewport the game is actually running in - a panel
			// that overflows a small window is a bug, not a layout choice.
			const float MW = FMath::Min(760.f, Canvas->ClipX - 80.f);
			const float MH = FMath::Min(430.f, Canvas->ClipY - 60.f);
			const float MX4 = (Canvas->ClipX - MW) * 0.5f;
			const float MY4 = (Canvas->ClipY - MH) * 0.5f;

			FVeilUI::Glass(this, FLinearColor(0.018f, 0.026f, 0.050f, 0.95f), MX4, MY4, MW, MH, 16.f);
			DrawRect(FLinearColor(1.f, 0.78f, 0.30f, 0.95f), MX4, MY4, 3.f, MH);
			DrawRect(FLinearColor(1.f, 0.78f, 0.30f, 0.3f), MX4, MY4, MW, 1.f);
			DrawText(TEXT("OPTIONS"), FLinearColor(1.f, 0.88f, 0.36f), MX4 + 28.f, MY4 + 16.f, Big, 1.15f);
			DrawText(TEXT("Changed in place, saved on change. The frame is measured live at the bottom."),
				FLinearColor(0.60f, 0.66f, 0.76f), MX4 + 30.f, MY4 + 48.f, Med, 0.84f);

			static const TCHAR* Presets[] = { TEXT("LOW"), TEXT("MEDIUM"), TEXT("HIGH"), TEXT("EPIC") };
			static const TCHAR* Names[8] = {
				TEXT("QUALITY PRESET"), TEXT("RESOLUTION SCALE"), TEXT("FRAME CAP"), TEXT("VSYNC"),
				TEXT("ADAPTIVE QUALITY  (KRAKEN)"), TEXT("RESTORE 60 FPS DEFAULTS"),
				TEXT("MEASURE AND LOG THE FRAME"), TEXT("BACK")
			};

			FString Values[8];
			if (Perf)
			{
				const int32 Pre = Perf->GetPreset();
				Values[0] = (Pre >= 0 && Pre < 4) ? FString(Presets[Pre]) : FString(TEXT("CUSTOM"));
				Values[1] = FString::Printf(TEXT("%d %%"), Perf->GetResolutionScale());
				Values[2] = Perf->GetFrameCap() > 0 ? FString::Printf(TEXT("%d fps"), Perf->GetFrameCap()) : FString(TEXT("UNCAPPED"));
				Values[3] = Perf->GetVsync() ? TEXT("ON") : TEXT("OFF");
				Values[4] = Perf->IsAdaptive() ? TEXT("ON") : TEXT("OFF");
				Values[5] = TEXT("GO");
				Values[6] = TEXT("GO");
				Values[7] = TEXT("CLOSE");
			}

			float RY = MY4 + 78.f;
			for (int32 i = 0; i < 8; ++i)
			{
				const bool bSel = (i == CPC->OptionsSel);
				if (bSel)
				{
					DrawRect(FLinearColor(1.f, 0.80f, 0.32f, 0.16f), MX4 + 20.f, RY - 4.f, MW - 40.f, 26.f);
					DrawRect(FLinearColor(1.f, 0.80f, 0.32f, 0.95f), MX4 + 20.f, RY - 4.f, 3.f, 26.f);
				}
				DrawText(bSel ? FString(TEXT(">")) : FString(TEXT(" ")), FLinearColor(1.f, 0.86f, 0.38f), MX4 + 30.f, RY, Med, 0.9f);
				DrawText(Names[i], bSel ? FLinearColor(0.95f, 0.97f, 1.f) : FLinearColor(0.78f, 0.83f, 0.90f), MX4 + 50.f, RY, Med, 0.9f);
				DrawText(Values[i], FLinearColor(1.f, 0.87f, 0.44f), MX4 + MW * 0.62f, RY, Med, 0.9f);
				RY += 30.f;
			}

			if (Perf)
			{
				DrawRect(FLinearColor(1.f, 1.f, 1.f, 0.10f), MX4 + 28.f, MY4 + MH - 66.f, MW - 56.f, 1.f);
				DrawText(FString::Printf(TEXT("MEASURED   avg %.2f ms   p95 %.2f ms   %.1f fps"),
					Perf->GetAvgMs(), Perf->GetP95Ms(), Perf->GetFps()),
					FLinearColor(0.55f, 0.92f, 1.f), MX4 + 30.f, MY4 + MH - 56.f, Med, 0.86f);
				DrawText(FString::Printf(
					TEXT("Budget 16.67 ms (60 fps)   -   heavy above %.2f ms, light below %.2f ms   -   13 s between moves, 55 s before a probe up"),
					Perf->GetHeavyMs(), Perf->GetLightMs()),
					FLinearColor(0.52f, 0.58f, 0.68f), MX4 + 30.f, MY4 + MH - 36.f, Med, 0.76f);
			}
		}
	}
	// ------------------------------------------------------------------
	// DIALOGUE BOX. A speaker needs a name, a faction, a title and a voice -
	// the words alone are a chat log, not a scene.
	// ------------------------------------------------------------------
	if (UVEILDialogue* Dlg = W->GetSubsystem<UVEILDialogue>())
	{
		if (Dlg->IsActive())
		{
			const float BW = Canvas->ClipX * 0.62f;
			// The plate grows to hold the replies: a conversation that offers a
			// choice and then hides it in a one-line strip is half a dialogue.
			const float BH = (118.f + (Dlg->IsChoosing() ? 24.f * Dlg->GetChoices().Num() + 8.f : 24.f)) * UPHIMITO_PHI_SAFE;
			const float BX = (Canvas->ClipX - BW) * 0.5f;
			const float BY = Canvas->ClipY - BH - 26.f * UPHIMITO_PHI_SAFE;
			const FLinearColor Who = Dlg->GetSpeakerColor();

			// Plate + faction spine + a name band that carries the colour. The body
			// is glass under a gold trim, and the speaker's own colour runs down the
			// left edge, so a conversation looks like a held document, not a console.
			// The dialogue box is a panel too, and it was the last one off the Bible's
			// number at 78%. One fill for every panel: 85%.
			FVeilUI::Glass(this, FLinearColor(0.026f, 0.032f, 0.050f, 0.85f), BX, BY, BW, BH, 12.f);
			FVeilUI::RoundRect(this, FLinearColor(Who.R, Who.G, Who.B, 0.95f), BX + 1.f, BY + 12.f, 3.2f * UPHIMITO_PHI_SAFE, BH - 24.f, 1.6f * UPHIMITO_PHI_SAFE);
			FVeilUI::RoundRect(this, FLinearColor(Who.R, Who.G, Who.B, 0.22f), BX + 12.f, BY, BW - 24.f, 1.4f, 0.7f);

			auto ShadowB = [this](const FString& S, FLinearColor C, float X, float Yy, UFont* F, float Sc)
			{
				DrawText(S, FLinearColor(0.f, 0.f, 0.f, 0.85f * C.A), X + 1.5f, Yy + 1.5f, F, Sc);
				DrawText(S, C, X, Yy, F, Sc);
			};

			DrawText(Dlg->GetSpeakerName(), Who, BX + 22.f, BY + 12.f, Big, 1.05f);
			DrawText(Dlg->GetSpeakerTitle(), FLinearColor(0.72f, 0.78f, 0.88f), BX + 24.f, BY + 44.f, Med, 0.86f);

			// THE TAKE ON SCREEN. Every line is performed inside the speaker's own
			// range, so the numbers behind the words are shown rather than claimed:
			// visit count, mood, pitch, rate. Two visits never print the same row.
			{
				const FVEILVoiceTake& V = Dlg->GetVoice();
				const FString Take = FString::Printf(TEXT("visit %d   %s   pitch %.2f   rate %.2f%s"),
					Dlg->TalkCount(Dlg->GetSpeakerId()), *V.Mood, V.Pitch, V.Rate,
					V.bMachine ? TEXT("   [machine]") : TEXT(""));
				float TW = 0.f, TH = 0.f;
				Canvas->StrLen(Med, Take, TW, TH);
				DrawText(Take, FLinearColor(Who.R, Who.G, Who.B, 0.62f),
					BX + BW - 22.f - TW, BY + 16.f, Med, 0.74f);
			}
			DrawText(Dlg->GetSpeakerCreed(), FLinearColor(Who.R, Who.G, Who.B, 0.55f), BX + 24.f, BY + 62.f, Med, 0.74f);
			DrawRect(FLinearColor(Who.R, Who.G, Who.B, 0.28f), BX + 22.f, BY + 82.f, BW - 44.f, 1.f);

			// Typewriter body, wrapped to the plate.
			const float TextW = BW - 48.f;
			auto Wrap = [this](const FString& In, UFont* Font, float MaxW) -> TArray<FString>
			{
				TArray<FString> Out;
				TArray<FString> Words;
				In.ParseIntoArray(Words, TEXT(" "), true);
				FString Cur;
				for (const FString& Wd : Words)
				{
					const FString Try = Cur.IsEmpty() ? Wd : Cur + TEXT(" ") + Wd;
					float XL = 0.f, YL = 0.f;
					Canvas->StrLen(Font, Try, XL, YL);
					if (XL > MaxW && !Cur.IsEmpty()) { Out.Add(Cur); Cur = Wd; }
					else { Cur = Try; }
				}
				if (!Cur.IsEmpty()) { Out.Add(Cur); }
				return Out;
			};

			int32 Remaining = Dlg->GetVisibleText().Len();
			float TY = BY + 90.f;
			for (const FString& Line : Wrap(Dlg->GetFullText(), Med, TextW))
			{
				if (Remaining <= 0) { break; }
				ShadowB(Line.Left(Remaining), FLinearColor(0.93f, 0.95f, 0.99f), BX + 24.f, TY, Med, 0.98f);
				Remaining -= Line.Len() + 1;
				TY += 20.f;
			}

			// REPLIES. Each one is a thing the runner says, and choosing it has
			// already changed the world by the time the speaker answers.
			if (Dlg->IsChoosing())
			{
				const TArray<FVEILReplyDef>& Choices = Dlg->GetChoices();
				float OY = TY + 8.f;
				for (int32 ci = 0; ci < Choices.Num(); ++ci)
				{
					// The glyph in front of a reply is the button in the player's hands:
					// A / X / Y on a pad, 1 / 2 / 3 on a keyboard. Same three answers.
					static const TCHAR* PadGlyphs[3] = { TEXT("A"), TEXT("X"), TEXT("Y") };
					ShadowB(VEILInput::IsGamepad()
						? FString::Printf(TEXT("[%s]  %s"), PadGlyphs[FMath::Clamp(ci, 0, 2)], *Choices[ci].Label.ToString())
						: FString::Printf(TEXT("[%d]  %s"), ci + 1, *Choices[ci].Label.ToString()),
						FLinearColor(1.f, 0.86f, 0.45f), BX + 26.f, OY, Med, 0.92f);
					OY += 22.f;
				}
				DrawText(VEILInput::IsGamepad()
					? TEXT("A / X / Y answer   -   this changes the world")
					: TEXT("1 / 2 / 3 answer   -   this changes the world"),
					FLinearColor(0.62f, 0.70f, 0.82f), BX + 24.f, BY + BH - 20.f, Med, 0.7f);
			}
			else if (Dlg->HasAnswered())
			{
				// What the world did about it, in the same gold as the ledger.
				ShadowB(Dlg->GetConsequence(), FLinearColor(0.55f, 1.f, 0.72f), BX + 24.f, TY + 8.f, Med, 0.86f);
				DrawText(TEXT("[SPACE] close"), FLinearColor(0.55f, 0.62f, 0.72f),
					BX + BW - 130.f, BY + BH - 20.f, Med, 0.78f);
			}
			// Prompt only once the line has finished arriving.
			else if (!Dlg->IsTyping())
			{
				const bool bMore = Dlg->GetLineIndex() < (Dlg->GetSpeaker() ? Dlg->GetSpeaker()->Greetings.Num() - 1 : 0);
				DrawText(bMore ? TEXT("[SPACE] continue") : TEXT("[SPACE] reply"),
					FLinearColor(0.55f, 0.62f, 0.72f), BX + BW - 150.f, BY + BH - 24.f, Med, 0.78f);
			}
		}
	}

	// ------------------------------------------------------------------
	// GODMODE. GTA's cheat list and a trainer menu, in the game's own UI:
	// every row a real switch, ON/OFF shown, and one ALL OFF row that puts the
	// game back exactly as it was.
	// ------------------------------------------------------------------
	if (AVEILPlayerController* CheatPC = Cast<AVEILPlayerController>(PC))
	{
		if (CheatPC->bGodMenuOpen)
		{
			UPHICheats* Cheats = W->GetSubsystem<UPHICheats>();
			const int32 PerCol = UPHICheats::RowsPerColumn;
			const int32 Cols = (UPHICheats::RowCount + PerCol - 1) / PerCol;
			const float MW2 = 300.f + 470.f * (float)Cols;
			const float RowH = 22.f;
			const float MH2 = 138.f + RowH * (float)PerCol;
			const float MX2 = FMath::Max(10.f, (Canvas->ClipX - MW2) * 0.5f);
			const float MY2 = FMath::Max(12.f, (Canvas->ClipY - MH2) * 0.5f);

			FVeilUI::Glass(this, FLinearColor(0.016f, 0.024f, 0.046f, 0.95f), MX2, MY2, MW2, MH2, 16.f);
			DrawRect(FLinearColor(1.f, 0.55f, 0.15f, 0.95f), MX2, MY2, 3.f, MH2);
			DrawText(TEXT("GODMODE"), FLinearColor(1.f, 0.74f, 0.28f), MX2 + 28.f, MY2 + 16.f, Big, 1.25f);
			DrawText(Cheats ? Cheats->StatusLine() : FString(TEXT("GODMODE  //  ALL OFF")),
				FLinearColor(0.92f, 0.70f, 0.42f), MX2 + 200.f, MY2 + 24.f, Med, 0.82f);
			DrawText(TEXT("Built in, free, no download  -  and nothing here survives a restart."),
				FLinearColor(0.60f, 0.64f, 0.72f), MX2 + 30.f, MY2 + 48.f, Med, 0.76f);
			DrawRect(FLinearColor(1.f, 0.6f, 0.2f, 0.25f), MX2 + 28.f, MY2 + 68.f, MW2 - 56.f, 1.f);

			for (int32 i = 0; i < UPHICheats::RowCount; ++i)
			{
				const int32 Col = i / PerCol;
				const int32 Row = i % PerCol;
				const float CX = MX2 + 24.f + 470.f * (float)Col;
				const float RY2 = MY2 + 80.f + RowH * (float)Row;
				const float CW = 440.f;

				const bool bSel = (CheatPC->GodSel == i);
				const FString Value = Cheats ? Cheats->ValueOf(i) : FString();
				const bool bOn = Cheats && Cheats->IsOn(i);
				const bool bAllOff = (i == (int32)UPHICheats::ERow::AllOff);

				if (bSel)
				{
					DrawRect(FLinearColor(1.f, 0.6f, 0.2f, 0.16f), CX, RY2 - 3.f, CW, RowH - 2.f);
					DrawRect(FLinearColor(1.f, 0.78f, 0.3f, 1.f), CX, RY2 - 3.f, 3.f, RowH - 2.f);
				}
				FLinearColor RowCol = bSel ? FLinearColor(1.f, 0.92f, 0.72f) : FLinearColor(0.74f, 0.78f, 0.86f);
				if (bAllOff) { RowCol = bSel ? FLinearColor(1.f, 0.95f, 0.7f) : FLinearColor(1.f, 0.70f, 0.30f); }
				DrawText(Cheats ? Cheats->LabelOf(i) : FString(), RowCol, CX + 14.f, RY2, Med, 0.82f);
				if (!Value.IsEmpty())
				{
					DrawText(Value, bOn ? FLinearColor(0.45f, 1.f, 0.55f) : FLinearColor(0.55f, 0.58f, 0.64f),
						CX + CW - 54.f, RY2, Med, 0.82f);
				}
			}

			DrawText(TEXT("[UP/DOWN] move    [LEFT/RIGHT] other column    [SPACE / A] switch    [G] close"),
				FLinearColor(0.52f, 0.56f, 0.64f), MX2 + 30.f, MY2 + MH2 - 26.f, Med, 0.72f);
			DrawText(TEXT("FLY: W/S/A/D + SPACE up / CTRL down.  ALL OFF is the clean run  -  nothing is ever written to the save."),
				FLinearColor(0.45f, 0.62f, 0.52f), MX2 + 30.f, MY2 + MH2 - 46.f, Med, 0.70f);
		}
	}

	// ------------------------------------------------------------------
	// ONE - the voice that has hands. One line, one place, so its state is read
	// at a glance: a grey dot is listening, a cool dot is working, gold speaking.
	// Hidden while a conversation owns the plate, because two voices at once is
	// neither.
	// ------------------------------------------------------------------
	if (const UPHIONE* One = W->GetSubsystem<UPHIONE>())
	{
		const UVEILDialogue* DlgNow = W->GetSubsystem<UVEILDialogue>();
		if (!DlgNow || !DlgNow->IsActive())
		{
			const FString OneLine = One->StateLine();
			float OW = 0.f, OH = 0.f;
			Canvas->StrLen(Med, OneLine, OW, OH);
			const float OX = (Canvas->ClipX - OW) * 0.5f;
			const float OY = Canvas->ClipY - 170.f;
			FVeilUI::Chip(this, FLinearColor(0.03f, 0.05f, 0.09f, 0.66f), OX - 18.f, OY - 6.f, OW + 36.f, OH + 12.f);
			const bool bSpeaking = One->GetState() == EONEState::Speaking;
			const bool bWorking = One->GetState() == EONEState::Thinking;
			// Ready is deliberately dim: the dot only lights while the key is
			// actually down, so "listening" can never be shown when it is not.
			const bool bHearing = One->GetState() == EONEState::Listening;
			const FLinearColor Orb = bSpeaking ? FLinearColor(1.f, 0.85f, 0.35f)
				: bWorking ? FLinearColor(0.45f, 0.95f, 0.85f)
				: bHearing ? FLinearColor(0.85f, 0.92f, 1.f)
				: FLinearColor(0.30f, 0.34f, 0.44f);
			DrawRect(Orb, OX - 12.f, OY + OH * 0.5f - 3.f, 6.f, 6.f);
			DrawText(OneLine, bSpeaking ? FLinearColor(1.f, 0.92f, 0.72f) : FLinearColor(0.82f, 0.88f, 1.f),
				OX, OY, Med, 0.9f);
		}
	}

	if (bWon)
	{
		DrawText(TEXT("SIGNAL RESTORED - YOU WIN"), FLinearColor(1.f, 0.85f, 0.2f),
			Canvas->ClipX * 0.5f - 320.f, Canvas->ClipY * 0.42f, Big, 2.2f);
	}
	if (FPHIVoice::LastSubtitleAge(W) < 7.f && !FPHIVoice::LastSubtitle.IsEmpty())
	{
		DrawText(FPHIVoice::LastSubtitle, FLinearColor(1.f, 1.f, 1.f), 30.f, Canvas->ClipY - 70.f, Med);
	}
	if (bDead)
	{
		DrawText(TEXT("SIGNAL LOST - press R"), FLinearColor(1.f, 0.25f, 0.2f),
			Canvas->ClipX * 0.5f - 260.f, Canvas->ClipY * 0.42f, Big, 2.0f);
	}

	// THE ONE OS rides over the world HUD in its own pass. It is a surface you
	// raise, not more rows in the instrument, so it cannot crowd the play view.
	// ONE OWNER PER ZONE. The mockup used to draw a second quest banner, a second
	// diamond hotbar and its own ACTIVITIES row into the same bottom-left corner the
	// canvas already owned, and the OS face painted a full-screen panel behind both -
	// three systems, one corner, the overlap in the photograph. The canvas owns the
	// play HUD now; the face is a surface you raise with F1, drawn only while open.
	if (const AVEILPlayerController* OsPC = Cast<AVEILPlayerController>(PC))
	{
				if (OsPC->bShowDebug) { VEILOSFace::Draw(this, Canvas); }
	}
}

