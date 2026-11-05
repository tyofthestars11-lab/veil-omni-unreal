#include "VeilHud.h"
#include "PHI_Constants.h"
#include "VeilPlayerController.h"
#include "VeilRunner.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/PlayerController.h"

namespace
{
	const FLinearColor Gold(0.83f, 0.62f, 0.22f, 0.95f);
	const FLinearColor GoldDim(0.83f, 0.62f, 0.22f, 0.28f);
	const FLinearColor SignalBlue(0.247f, 0.663f, 1.0f, 0.95f);
	const FLinearColor Navy(0.02f, 0.045f, 0.14f, 0.72f);
	const FLinearColor VitRed(0.74f, 0.13f, 0.12f, 0.92f);
	const FLinearColor StmGreen(0.20f, 0.66f, 0.36f, 0.92f);
	const FLinearColor Ink(0.95f, 0.93f, 0.86f, 1.0f);
	const FLinearColor Stone(0.62f, 0.58f, 0.48f, 0.9f);

	/* North is +Y in the world, so screen Y is negated: the map reads north-up. */
	FVector2D ToMap(const FVector& World, const FVector& Centre, float Scale)
	{
		return FVector2D((World.X - Centre.X) * Scale, -(World.Y - Centre.Y) * Scale);
	}
}

void AVeilHud::GoldLine(float X1, float Y1, float X2, float Y2, float T, const FLinearColor& C)
{
	DrawLine(X1, Y1, X2, Y2, C, T);
}

void AVeilHud::Diamond(float CX, float CY, float R, const FLinearColor& C, float T)
{
	GoldLine(CX, CY - R, CX + R, CY, T, C);
	GoldLine(CX + R, CY, CX, CY + R, T, C);
	GoldLine(CX, CY + R, CX - R, CY, T, C);
	GoldLine(CX - R, CY, CX, CY - R, T, C);
}

void AVeilHud::Bar(float X, float Y, float W, float H, float Frac, const FLinearColor& Fill)
{
	DrawRect(GoldDim, X - 1.f, Y - 1.f, W + 2.f, H + 2.f);
	DrawRect(Navy, X, Y, W, H);
	DrawRect(Fill, X, Y, W * FMath::Clamp(Frac, 0.f, 1.f), H);
}

/** A filled disc: the canvas has no circle primitive, so it is drawn in strips. */
void AVeilHud::Disc(float CX, float CY, float R, const FLinearColor& C)
{
	for (float DY = -R; DY < R; DY += 2.f)
	{
		const float Half = FMath::Sqrt(FMath::Max(0.f, R * R - DY * DY));
		DrawRect(C, CX - Half, CY + DY, Half * 2.f, 2.2f);
	}
}

void AVeilHud::CollectMapGeometry()
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}
	const float Now = World->GetTimeSeconds();
	if (MapStamp >= 0.f && Now - MapStamp < 0.35f)
	{
		return;
	}
	MapStamp = Now;

	MapBlips.Reset();
	MapSizes.Reset();
	MapHeights.Reset();

	for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
	{
		const AStaticMeshActor* A = *It;
		const UStaticMeshComponent* Mesh = A ? A->GetStaticMeshComponent() : nullptr;
		if (!Mesh || !Mesh->IsVisible())
		{
			continue;
		}
		const FBox Bounds = A->GetComponentsBoundingBox(true);
		const float Size = static_cast<float>(Bounds.GetExtent().Size2D());
		// The landmass and mesa skirt are the backdrop, not map furniture.
		if (Size > 3000.f || Size < 20.f)
		{
			continue;
		}
		const FVector L = A->GetActorLocation();
		MapBlips.Add(FVector2D(L.X, L.Y));
		MapSizes.Add(Size);
		MapHeights.Add(static_cast<float>(L.Z));
	}
}

void AVeilHud::NotifyHitBoxClick(FName BoxName)
{
	Super::NotifyHitBoxClick(BoxName);
	if (BoxName == FName(TEXT("VeilPause")))
	{
		if (AVeilPlayerController* PC = Cast<AVeilPlayerController>(GetOwningPlayerController()))
		{
			PC->OnPauseToggle();
		}
	}
}

void AVeilHud::DrawHUD()
{
	Super::DrawHUD();
	AVeilPlayerController* PC = Cast<AVeilPlayerController>(GetOwningPlayerController());
	if (!PC || !Canvas)
	{
		return;
	}
	const float W = static_cast<float>(Canvas->SizeX);
	const float H = static_cast<float>(Canvas->SizeY);

	// The pause button sits inside the minimap ring and is clickable.
	PC->bEnableClickEvents = true;

	AVeilRunner* R = PC->Runner();
	const float Fill = (R && R->Mito.MaxATP > 0.f) ? R->Mito.ATP / R->Mito.MaxATP : 0.f;

	Ribbon(W, H, Fill);
	Vitals(PC, W, H);
	Minimap(PC, W, H);
	QuestBanner(PC, W, H);
	Abilities(PC, W, H);
	Prompt(PC, W, H);
	Dialogue(PC, W, H);

	// Pause panel: the state is real, so the screen says so.
	if (PC->bPauseMenu)
	{
		DrawRect(FLinearColor(0.02f, 0.03f, 0.08f, 0.88f), W * 0.32f, H * 0.34f, W * 0.36f, H * 0.22f);
		DrawText(TEXT("PAUSED"), Gold, W * 0.44f, H * 0.39f, nullptr, 1.7f, false);
		DrawText(TEXT("[P] resume"), Ink, W * 0.44f, H * 0.47f, nullptr, 0.95f, false);
	}
}

void AVeilHud::Ribbon(float W, float H, float Fill)
{
	// phi-tapered arc: thick at the crown, thinning to nothing at both ends
	const FVector2D Centre(W * 0.5f, -H * static_cast<float>(PHI_Constants::PhiPow(-4)));
	const float Radius = H * static_cast<float>(PHI_Constants::PhiPow(-1.9));
	const int32 Segments = FMath::RoundToInt(PHI_Constants::PhiPow(8));  // 46
	for (int32 i = 0; i < Segments; ++i)
	{
		const float T0 = static_cast<float>(i) / Segments;
		const float T1 = static_cast<float>(i + 1) / Segments;
		const float A0 = FMath::DegreesToRadians(202.f + 136.f * T0);
		const float A1 = FMath::DegreesToRadians(202.f + 136.f * T1);
		const float Taper = FMath::Sin(PI * T0);
		const float X0 = Centre.X + FMath::Cos(A0) * Radius;
		const float Y0 = Centre.Y + FMath::Sin(A0) * Radius;
		const float X1 = Centre.X + FMath::Cos(A1) * Radius;
		const float Y1 = Centre.Y + FMath::Sin(A1) * Radius;
		FLinearColor C = Gold;
		C.A = static_cast<float>(PHI_Constants::PhiPow(-4) + PHI_Constants::PhiPow(-0.45) * Taper);
		GoldLine(X0, Y0, X1, Y1, 3.f + 13.f * Taper, C);

		// the ATP charge rides inside the ribbon as a thinner gold thread
		FLinearColor Inner = SignalBlue;
		Inner.A = (T0 <= Fill) ? static_cast<float>(PHI_Constants::PhiPow(-0.25))
			: static_cast<float>(PHI_Constants::PhiPow(-3.3));
		const float R2 = Radius - 12.f;
		GoldLine(Centre.X + FMath::Cos(A0) * R2, Centre.Y + FMath::Sin(A0) * R2,
			Centre.X + FMath::Cos(A1) * R2, Centre.Y + FMath::Sin(A1) * R2, 4.f, Inner);
	}
	Diamond(W * 0.5f, H * static_cast<float>(PHI_Constants::PhiPow(-5.123)), 20.f, Gold, 2.f);
	DrawText(TEXT("LV 1"), Ink, W * 0.5f - 17.f, H * 0.085f - 7.f, nullptr, 0.95f, false);
}

void AVeilHud::Vitals(AVeilPlayerController* PC, float W, float H)
{
	const float X = W * 0.5f - 300.f;
	const float Y = H * static_cast<float>(PHI_Constants::PhiPow(-4.25));
	Bar(X, Y, 230.f, 11.f, PC->VIT / FMath::Max(1.f, PC->VITMax), VitRed);
	DrawText(TEXT("VIT"), Gold, X, Y - 15.f, nullptr, 0.9f, false);

	Bar(X, Y + 22.f, 178.f, 9.f, PC->STM / FMath::Max(1.f, PC->STMMax), StmGreen);
	DrawText(TEXT("STM"), Gold, X, Y + 33.f, nullptr, 0.9f, false);

	AVeilRunner* R = PC->Runner();
	if (R)
	{
		Bar(X, Y + 56.f, 150.f, 7.f, R->Mito.ATP / FMath::Max(1.f, R->Mito.MaxATP), Gold);
		DrawText(R->Mito.Ledger(), GoldDim, X, Y + 74.f, nullptr, 0.86f, false);
	}
}

void AVeilHud::Minimap(AVeilPlayerController* PC, float W, float H)
{
	AVeilRunner* R = PC->Runner();
	const float CX = W - 170.f;
	const float CY = H - 178.f;
	const float Rad = 130.f;
	const float Range = 5600.f;                       // world units the ring covers
	const float Scale = Rad / Range;
	// The ring is always drawn: a HUD that vanishes with the pawn is not a minimap.
	const FVector Centre = R ? R->GetActorLocation()
		: (PC->PlayerCameraManager ? PC->PlayerCameraManager->GetCameraLocation() : FVector::ZeroVector);

	CollectMapGeometry();

	// The map itself: dark disc, then every real piece of the town inside the ring.
	Disc(CX, CY, Rad, FLinearColor(0.012f, 0.030f, 0.085f, 0.76f));

	// phi rings and crosshair, the surveyor's grid
	for (int32 Ring = 1; Ring <= 2; ++Ring)
	{
		const float RR = Rad * static_cast<float>(PHI_Constants::PhiPow(-Ring));
		FLinearColor C = GoldDim;
		C.A = 0.22f;
		for (int32 i = 0; i < 40; ++i)
		{
			const float A0 = (2.f * PI * i) / 40.f;
			const float A1 = (2.f * PI * (i + 1)) / 40.f;
			GoldLine(CX + FMath::Cos(A0) * RR, CY + FMath::Sin(A0) * RR,
				CX + FMath::Cos(A1) * RR, CY + FMath::Sin(A1) * RR, 1.f, C);
		}
	}
	GoldLine(CX - Rad, CY, CX + Rad, CY, 1.f, FLinearColor(0.83f, 0.62f, 0.22f, 0.13f));
	GoldLine(CX, CY - Rad, CX, CY + Rad, 1.f, FLinearColor(0.83f, 0.62f, 0.22f, 0.13f));

	for (int32 i = 0; i < MapBlips.Num(); ++i)
	{
		const FVector2D D = MapBlips[i] - FVector2D(Centre.X, Centre.Y);
		const float Len = static_cast<float>(D.Size());
		if (Len > Range)
		{
			continue;
		}
		const FVector2D P(CX + D.X * Scale, CY - D.Y * Scale);
		const float Sz = FMath::Clamp(MapSizes[i] * Scale * 1.7f, 1.6f, 11.f);
		const float Fade = 1.f - 0.45f * (Len / Range);
		FLinearColor C = (MapHeights[i] > Centre.Z + 140.f) ? Gold : Stone;
		C.A = 0.30f + 0.50f * Fade;
		DrawRect(C, P.X - Sz * 0.5f, P.Y - Sz * 0.5f, Sz, Sz);
	}

	// Objectives. Inside the ring they sit where they really are; beyond it they
	// pin to the rim in their true direction, with the range beneath the map.
	float Nearest = BIG_NUMBER;
	FString NearestName = TEXT("SIGNAL");

	auto Objective = [&](const FVector& World, const FLinearColor& C, float Size, const FString& Name)
	{
		const FVector2D D = ToMap(World, Centre, Scale);
		const float Len = static_cast<float>(D.Size());
		const bool bBeyond = Len > Rad * 0.93f;
		const FVector2D Dir = bBeyond ? D.GetSafeNormal() : FVector2D(0.f, 0.f);
		const FVector2D P = bBeyond ? FVector2D(CX + Dir.X * Rad * 0.93f, CY + Dir.Y * Rad * 0.93f)
			: FVector2D(CX + D.X, CY + D.Y);
		Diamond(P.X, P.Y, Size, C, 2.f);
		if (bBeyond)
		{
			GoldLine(P.X, P.Y, CX + Dir.X * Rad * 0.80f, CY + Dir.Y * Rad * 0.80f, 1.f, GoldDim);
		}
		const float Metres = static_cast<float>(FVector::Dist(World, Centre)) * 0.01f;
		if (Metres < Nearest)
		{
			Nearest = Metres;
			NearestName = Name;
		}
	};

	Objective(AVeilPlayerController::KeeperAt, Gold, 6.f, TEXT("KEEPER"));
	Objective(AVeilPlayerController::SpireAt, Gold, 8.f, TEXT("SPIRE"));
	Objective(AVeilPlayerController::DeepAt, SignalBlue, 6.f, TEXT("DEEP"));
	for (int32 i = 0; i < 3; ++i)
	{
		Objective(AVeilPlayerController::BeaconAt(i), SignalBlue, 5.f,
			FString::Printf(TEXT("BEACON %d"), i + 1));
	}

	// Heading needle, then the Runner at the centre.
	if (R)
	{
		const FVector Fw = R->GetActorForwardVector();
		FVector2D Dir(Fw.X, -Fw.Y);
		if (!Dir.Normalize())
		{
			Dir = FVector2D(0.f, -1.f);
		}
		GoldLine(CX - Dir.X * 8.f, CY - Dir.Y * 8.f, CX + Dir.X * Rad * 0.40f, CY + Dir.Y * Rad * 0.40f,
			2.f, FLinearColor(0.83f, 0.62f, 0.22f, 0.55f));
		Diamond(CX + Dir.X * Rad * 0.40f, CY + Dir.Y * Rad * 0.40f, 5.f, Gold, 2.f);
	}
	Diamond(CX, CY, 9.f, Ink, 2.f);
	DrawRect(Gold, CX - 3.f, CY - 3.f, 6.f, 6.f);

	// Bezel: double gold ring with N/E/S/W ticks.
	for (int32 i = 0; i < 48; ++i)
	{
		const float A = (2.f * PI * i) / 48.f;
		const float Len = (i % 12 == 0) ? 11.f : 4.f;
		const FLinearColor C = (i % 12 == 0) ? Gold : GoldDim;
		GoldLine(CX + FMath::Cos(A) * Rad, CY + FMath::Sin(A) * Rad,
			CX + FMath::Cos(A) * (Rad - Len), CY + FMath::Sin(A) * (Rad - Len),
			(i % 12 == 0) ? 2.5f : 1.2f, C);
	}
	{
		FLinearColor C = Gold;
		C.A = 0.9f;
		for (int32 i = 0; i < 64; ++i)
		{
			const float A0 = (2.f * PI * i) / 64.f;
			const float A1 = (2.f * PI * (i + 1)) / 64.f;
			GoldLine(CX + FMath::Cos(A0) * Rad, CY + FMath::Sin(A0) * Rad,
				CX + FMath::Cos(A1) * Rad, CY + FMath::Sin(A1) * Rad, 2.4f, C);
		}
	}

	const float LX = Rad - 20.f;
	DrawText(TEXT("N"), Gold, CX - 5.f, CY - LX, nullptr, 1.05f, false);
	DrawText(TEXT("S"), Gold, CX - 5.f, CY + LX - 12.f, nullptr, 1.05f, false);
	DrawText(TEXT("E"), Gold, CX + LX - 8.f, CY - 6.f, nullptr, 1.05f, false);
	DrawText(TEXT("W"), Gold, CX - LX, CY - 6.f, nullptr, 1.05f, false);

	// Pause button inside the ring: two bars, and it really pauses.
	const float PX = CX + Rad * 0.55f;
	const float PY = CY - Rad * 0.55f;
	const float PR = 17.f;
	Disc(PX, PY, PR, FLinearColor(0.012f, 0.030f, 0.085f, 0.95f));
	for (int32 i = 0; i < 28; ++i)
	{
		const float A0 = (2.f * PI * i) / 28.f;
		const float A1 = (2.f * PI * (i + 1)) / 28.f;
		GoldLine(PX + FMath::Cos(A0) * PR, PY + FMath::Sin(A0) * PR,
			PX + FMath::Cos(A1) * PR, PY + FMath::Sin(A1) * PR, 2.f, Gold);
	}
	DrawRect(Gold, PX - 6.f, PY - 7.f, 4.f, 14.f);
	DrawRect(Gold, PX + 2.f, PY - 7.f, 4.f, 14.f);
	AddHitBox(FVector2D(PX - PR, PY - PR), FVector2D(PR * 2.f, PR * 2.f), FName(TEXT("VeilPause")), true);

	// Range under the map, from the real distance to the nearest objective.
	DrawText(FString::Printf(TEXT("%s   %.0f m"), *NearestName, Nearest), Ink,
		CX - 78.f, CY + Rad + 10.f, nullptr, 1.0f, false);
	DrawText(FString::Printf(TEXT("BEACONS %d/%d"), PC->BeaconsLit, PC->BeaconsTotal), Gold,
		CX - 78.f, CY + Rad + 28.f, nullptr, 0.92f, false);
}

void AVeilHud::QuestBanner(AVeilPlayerController* PC, float W, float H)
{
	const float X = W * static_cast<float>(PHI_Constants::PhiPow(-6.444));
	const float Y = H * static_cast<float>(PHI_Constants::PhiPow(-4.495));
	const float BW = 470.f;
	const float BH = 38.f;
	const float Skew = 16.f;

	// angled gold banner, drawn as a skewed slab rather than a rectangle
	DrawRect(Navy, X, Y, BW, BH);
	GoldLine(X + Skew, Y, X + BW, Y, 3.f, Gold);
	GoldLine(X + BW, Y, X + BW - Skew, Y + BH, 3.f, Gold);
	GoldLine(X + BW - Skew, Y + BH, X, Y + BH, 3.f, Gold);
	GoldLine(X, Y + BH, X + Skew, Y, 3.f, Gold);
	// the tapered tail that makes it read as a ribbon, not a box
	GoldLine(X, Y, X - 26.f, Y + BH * 0.5f, 2.f, GoldDim);
	GoldLine(X - 26.f, Y + BH * 0.5f, X, Y + BH, 2.f, GoldDim);

	FString Line;
	if (!PC->bKeeperCounted)
	{
		Line = TEXT("WALK TO THE KEEPER");
	}
	else if (PC->BeaconsLit < PC->BeaconsTotal)
	{
		Line = FString::Printf(TEXT("KEEPER BEACONS   %d/%d"), PC->BeaconsLit, PC->BeaconsTotal);
	}
	else
	{
		Line = TEXT("THE SIGNAL ANSWERS  3/3");
	}
	DrawText(Line, Ink, X + 24.f, Y + 11.f, nullptr, PHI_Constants::Reach(-10.280), false);
}

void AVeilHud::Abilities(AVeilPlayerController* PC, float W, float H)
{
	const float CY = H - 62.f;
	const float Space = 34.f * PHI_Constants::Reach(-9.570);
	const float CX = W * 0.5f;
	for (int32 i = 0; i < 4; ++i)
	{
		const float X = CX + (i - PHI_Constants::Reach(-9.727)) * Space;
		const bool bReady = (i == 0);
		Diamond(X, CY, 13.f, bReady ? Gold : GoldDim, bReady ? PHI_Constants::Reach(-8.666) : PHI_Constants::Reach(-10.191));
	}
	if (PC->bCombat)
	{
		DrawText(TEXT("COMBAT MODE"), Gold, CX - 52.f, CY - 44.f, nullptr, 1.0f, false);
	}
	DrawText(TEXT("[E] interact    SHIFT sprint    SPACE glide    C combat"), Ink,
		CX - 250.f, CY - 24.f, nullptr, static_cast<float>(PHI_Constants::PhiPow(-0.338)), false);
}

void AVeilHud::Prompt(AVeilPlayerController* PC, float W, float H)
{
	if (!PC->bNearKeeper || PC->bDialogueOpen)
	{
		return;
	}
	const FString T = TEXT("[E]   Speak with Keeper Sera Vahl");
	DrawText(T, Gold, W * 0.5f - 165.f, H - 150.f, nullptr, PHI_Constants::Reach(-10.372), false);
}

void AVeilHud::Dialogue(AVeilPlayerController* PC, float W, float H)
{
	if (!PC->bDialogueOpen)
	{
		return;
	}
	const float X = W * static_cast<float>(PHI_Constants::PhiPow(-4.406));
	const float Y = H - 168.f;
	const float BW = W * static_cast<float>(PHI_Constants::PhiPow(-0.570));
	const float BH = 132.f;

	DrawRect(Navy, X, Y, BW, BH);
	GoldLine(X, Y, X + BW, Y, 3.f, Gold);
	GoldLine(X, Y + BH, X + BW, Y + BH, 3.f, Gold);
	GoldLine(X, Y, X, Y + BH, 2.f, GoldDim);
	GoldLine(X + BW, Y, X + BW, Y + BH, 2.f, GoldDim);

	DrawText(PC->DialogueSpeaker, Gold, X + 22.f, Y + 14.f, nullptr, PHI_Constants::Reach(-10.191), false);
	DrawText(PC->DialogueLine, Ink, X + 22.f, Y + 48.f, nullptr, PHI_Constants::Reach(-10.280), false);
	DrawText(TEXT("[E] continue"), GoldDim, X + BW - 110.f, Y + BH - 26.f, nullptr, 0.95f, false);
}
