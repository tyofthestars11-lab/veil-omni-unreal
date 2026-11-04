// VEIL OMNI - THE GOLDEN HUB, built to the painting.
#include "World/VEILGoldenHub.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Systems/VEILPhi.h"
#include "VEIL.h"

namespace
{
	// Use the textured, lit PHI palette so the authored meshes retain their material response.
	const TCHAR* GoldPath = TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic");
	const TCHAR* BluePath = TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue");
	const TCHAR* CreamPath = TEXT("/Game/PHI/Palette/M_PHI_CreamStone.M_PHI_CreamStone");

	// The hub is one axis, so the composition faces where the player faces.
	const float HubYawDeg = 59.f;
}

AVEILGoldenHub::AVEILGoldenHub()
{
	PrimaryActorTick.bCanEverTick = false;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("GoldenHubRoot"));
	SetRootComponent(Root);
}

UStaticMeshComponent* AVEILGoldenHub::Part(UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
	const FRotator& Rot, UMaterialInterface* Mat, bool bSolid)
{
	if (!Mesh) { return nullptr; }
	UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
	if (!C) { return nullptr; }
	C->SetupAttachment(GetRootComponent());
	C->SetStaticMesh(Mesh);
	C->SetRelativeLocation(Loc);
	C->SetRelativeRotation(Rot);
	C->SetRelativeScale3D(Scale);
	// SOLID MEANS SOLID: enabled collision alone is a picture, not an obstacle.
	C->SetCollisionEnabled(bSolid ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	if (bSolid)
	{
		C->SetCollisionObjectType(ECC_WorldStatic);
		C->SetCollisionResponseToAllChannels(ECR_Block);
	}
	if (Mat) { C->SetMaterial(0, Mat); }
	C->RegisterComponent();
	// HELD, SO IT IS SAVED. Without this the part exists only until the level is reloaded.
	Parts.Add(C);
	++PartCount;
	return C;
}

UStaticMeshComponent* AVEILGoldenHub::PartFitted(UStaticMesh* Mesh, const FVector& Center,
	const FVector& Size, const FRotator& Rot, UMaterialInterface* Mat, bool bSolid)
{
	if (!Mesh) { return nullptr; }
	const FBox Bounds = Mesh->GetBoundingBox();
	const FVector MeshSize = Bounds.GetSize();
	if (MeshSize.X <= KINDA_SMALL_NUMBER || MeshSize.Y <= KINDA_SMALL_NUMBER
		|| MeshSize.Z <= KINDA_SMALL_NUMBER)
	{
		return nullptr;
	}

	const FVector Scale(Size.X / MeshSize.X, Size.Y / MeshSize.Y, Size.Z / MeshSize.Z);
	const FVector Location = Center - Rot.RotateVector(Bounds.GetCenter() * Scale);
	return Part(Mesh, Location, Scale, Rot, Mat, bSolid);
}

void AVEILGoldenHub::Build()
{
	if (CompositionVersion == CurrentCompositionVersion && PartCount > 0) { return; }

	Gold = LoadObject<UMaterialInterface>(nullptr, GoldPath);
	Blue = LoadObject<UMaterialInterface>(nullptr, BluePath);
	Cream = LoadObject<UMaterialInterface>(nullptr, CreamPath);
	if (!Gold || !Blue || !Cream)
	{
		UE_LOG(LogVEIL, Error,
			TEXT("PHI GOLDEN HUB: required gold, blue, or cream material is missing; composition was not built."));
		return;
	}

	// The installed ancient kit supplies the hub's actual structures. Missing art is an error,
	// never a reason to silently substitute an engine primitive.
	KitWall = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Wall5_Big.SM_Wall5_Big"));
	KitFloor = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor1_Big.SM_Floor1_Big"));
	KitColumn = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big.SM_AncientColumn_Big"));
	KitDecor = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientDecor2Straight.SM_AncientDecor2Straight"));
	KitMonolith = LoadObject<UStaticMesh>(nullptr,
		TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientMonolith.SM_AncientMonolith"));
	if (!KitWall || !KitFloor || !KitColumn || !KitDecor || !KitMonolith)
	{
		UE_LOG(LogVEIL, Error,
			TEXT("PHI GOLDEN HUB: required ancient-kit wall, floor, column, decor, or monolith mesh is missing; composition was not built."));
		return;
	}

	// A placed Hub may contain parts baked by an older composition. Replace only this actor's
	// mesh components so removed placeholder-like structures cannot survive a source update.
	TInlineComponentArray<UStaticMeshComponent*> ExistingParts;
	GetComponents(ExistingParts);
	for (UStaticMeshComponent* ExistingPart : ExistingParts)
	{
		if (ExistingPart) { ExistingPart->DestroyComponent(); }
	}
	PartCount = 0;

	Fwd = FRotator(0.f, HubYawDeg, 0.f).Vector();
	Right = FVector(-Fwd.Y, Fwd.X, 0.f);

	// ---------------------------------------------------------------- the plaza
	// Separate paving blocks keep the plaza from reading as one uninterrupted slab.
	const float PlazaHalf = 7000.f;
	const int32 PlazaTilesPerSide = 5;
	const float PlazaTileSize = (PlazaHalf * 2.f) / PlazaTilesPerSide;
	const FRotator PlazaYaw(0.f, HubYawDeg, 0.f);
	for (int32 X = 0; X < PlazaTilesPerSide; ++X)
	{
		for (int32 Y = 0; Y < PlazaTilesPerSide; ++Y)
		{
			const bool bProcessionalTile = X == PlazaTilesPerSide / 2 || Y == PlazaTilesPerSide / 2;
			const float PatchSize = PlazaTileSize - 80.f;
			const float PatchX = PatchSize / 3.f;
			const float PatchY = PatchSize / 5.f;
			for (int32 TileX = 0; TileX < 3; ++TileX)
			{
				for (int32 TileY = 0; TileY < 5; ++TileY)
				{
					const FVector TileAt =
						Fwd * (-PlazaHalf + PlazaTileSize * (X + 0.5f)
							+ PatchX * (TileX - 1.f))
						+ Right * (-PlazaHalf + PlazaTileSize * (Y + 0.5f)
							+ PatchY * (TileY - 2.f))
						+ FVector(0.f, 0.f, -10.f);
					PartFitted(KitFloor, TileAt, FVector(PatchX, PatchY, 20.f), PlazaYaw,
						bProcessionalTile ? Blue : Cream, true);
				}
			}
		}
	}
	// The inner court and its entry bands use the same fitted stone and carved trim.
	for (int32 TileX = 0; TileX < 2; ++TileX)
	{
		for (int32 TileY = 0; TileY < 3; ++TileY)
		{
			const float TileXSize = 750.f;
			const float TileYSize = 500.f;
			const FVector TileAt = Fwd * ((TileX - 0.5f) * TileXSize)
				+ Right * ((TileY - 1.f) * TileYSize) + FVector(0.f, 0.f, 12.f);
			PartFitted(KitFloor, TileAt, FVector(TileXSize, TileYSize, 18.f),
				PlazaYaw, Cream, true);
		}
	}
	PartFitted(KitDecor, Fwd * 1620.f + FVector(0.f, 0.f, 2.f), FVector(760.f, 90.f, 24.f),
		PlazaYaw, Gold, false);
	PartFitted(KitDecor, Fwd * 1790.f + FVector(0.f, 0.f, -4.f), FVector(920.f, 90.f, 24.f),
		PlazaYaw, Gold, false);
	for (int32 I = -3; I <= 3; ++I)
	{
		PartFitted(KitDecor, Right * (I * 900.f) + FVector(0.f, 0.f, -8.f),
			FVector(PlazaHalf * 2.f, 24.f, 18.f), PlazaYaw, Blue, false);
	}

	// ---------------------------------------------------------------- the pillars
	// The two that frame the picture, then a second rank behind them. Height to width is the
	// painting's: a 26 m shaft on a 3 m radius, so they read as gates, not posts.
	Pillar(Fwd * 2200.f - Right * 1500.f, 2600.f, 300.f, HubYawDeg);
	Pillar(Fwd * 2200.f + Right * 1500.f, 2600.f, 300.f, HubYawDeg);
	Pillar(Fwd * 5200.f - Right * 2900.f, 2200.f, 260.f, HubYawDeg);
	Pillar(Fwd * 5200.f + Right * 2900.f, 2200.f, 260.f, HubYawDeg);

	// ---------------------------------------------------------------- the city
	// Terraces rising behind the market, cream walls under blue roofs with gold trim, the way the
	// painting stacks its town up the cliff.
	// AVEILHubMarket owns the 25 sites grouped 12/8/5. These eight houses are the architectural
	// backdrop and use the same fixed vision seed and golden angle.
	const float SiteSeedAngle = (float)(VeilPhi::VisionSeed % 360);
	for (int32 I = 0; I < 8; ++I)
	{
		const float Slot = -73.5f + (float)I * 21.f;
		const float EchoPhase = FMath::Fmod(
			(float)(VeilPhi::VisionSeed + I) * VeilPhi::GoldenAngleF, 12.f);
		const float Ang = Slot + EchoPhase - 6.f;
		const float Rad = FMath::DegreesToRadians(Ang);
		const float Radius = 9800.f + (float)(I % 3) * 1500.f;
		const FVector At = Fwd * (FMath::Cos(Rad) * Radius) + Right * (FMath::Sin(Rad) * Radius);
		House(At, HubYawDeg + Ang, 1100.f, 900.f, 1200.f + (float)(I % 3) * 260.f);
	}

	// ---------------------------------------------------------------- bridges
	Bridge(Fwd * 3000.f - Right * 5200.f, Fwd * 3600.f - Right * 9000.f);
	Bridge(Fwd * 3200.f + Right * 5200.f, Fwd * 3800.f + Right * 9200.f);

	// ---------------------------------------------------------------- the spire
	Spire(Fwd * 11000.f, 9000.f, 700.f);

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI GOLDEN HUB ART GAPS: no authored market-canopy, crystal, or waterfall/foam mesh is installed; those elements are omitted rather than represented by stretched kit meshes. The 25-site market uses the installed monolith mesh for its five peaks."));

	// The viewpoint: ON THE PAVING at its rear edge, facing the spire. Two failures measured here.
	// At -3200 the eye stood inside the plaza's own buildings; at -15000 it stood 150 m out, and the
	// frame came back as a mountain face, because the district's cliffs begin past the stone. The
	// plaza spans 7,000 either side of the axis, so the eye is placed at 6,600 - on paving, just
	// inside its edge - with the pillars, the market, the tiers and the spire all ahead of it.
	// THE EYE HAS TO CLEAR THE MARKET TO SEE THE PLACE. At 620 the frame was paving and shadow with
	// the architecture squeezed into a strip along the top. The measured heights set it: houses
	// 1,200-1,720 and pillars 2,600 - so the eye stands at 1,400, over the market and under the
	// pillar caps, aimed at the
	// spire's body (it rises 9,000 from Fwd*11000) so the city and the peaks are in the frame.
	// LEVEL, NOT UP. Aiming at the spire's body from 1,400 put the sky across the whole frame and
	// the capture measured mean (236,230,216) with 61% of pixels past white - a painted-over sky, not
	// a city. The target now sits barely above the eye, so the horizon falls mid-frame: paving and
	// market below it, the tiers and the spire above, sky only at the top edge.
	// OUTSIDE THE RING, ABOVE THE ROOFS. Two measured failures here: at -6,600 the eye stood
	// inside the ring of plaza buildings, so the frame was a cream wall corner with a navy
	// sliver down one side; at 1,400 it was level with roofs the reference looks over. The
	// painting is a wide view of the whole place - market below, tiers and spire above, sky
	// at the top - so the eye stands 40 m behind the paving edge and above every roof, and
	// aims at the spire's body rather than its cap.
	ViewPoint = -Fwd * 11000.f + FVector(0.f, 0.f, 4200.f);
	ViewTarget = Fwd * 11000.f + FVector(0.f, 0.f, 2400.f);

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI GOLDEN HUB: %d authored mesh parts; 4 pillars, 8 houses, 2 bridges, spire. The separate market owns the 25 sites 12/8/5. Seed %d, golden angle %.10f deg, opening bearing %.1f. View from V(X=%.0f,Y=%.0f,Z=%.0f)."),
		PartCount, VeilPhi::VisionSeed, VeilPhi::GoldenAngleDeg, SiteSeedAngle,
		ViewPoint.X, ViewPoint.Y, ViewPoint.Z);
	CompositionVersion = CurrentCompositionVersion;
}

void AVEILGoldenHub::Pillar(const FVector& At, float Height, float Radius, float YawDeg)
{
	const FRotator Yaw(0.f, YawDeg, 0.f);
	PartFitted(KitFloor, At + FVector(0.f, 0.f, 20.f),
		FVector(Radius * 3.f, Radius * 3.f, 40.f), Yaw, Gold, true);
	PartFitted(KitColumn, At + FVector(0.f, 0.f, Height * 0.5f),
		FVector(Radius * 2.f, Radius * 2.f, Height), Yaw, Gold, true);
}

void AVEILGoldenHub::House(const FVector& At, float YawDeg, float Width, float Depth, float Height)
{
	const FRotator Yaw(0.f, YawDeg, 0.f);
	const FVector Facing = Yaw.Vector();
	const FVector Side(-Facing.Y, Facing.X, 0.f);
	const float Phi = VeilPhi::PhiF;
	const float HalfWidth = Width * 0.5f;
	const float HalfDepth = Depth * 0.5f;
	const float WallThickness = FMath::Max(30.f, Width / (Phi * 12.f));
	const float PlinthHeight = Height / (Phi * 10.f);
	const float RoofRise = HalfWidth / Phi;
	const float WindowSill = Height * 0.42f;
	const float WindowHead = Height * 0.65f;
	const float DoorHead = Height * 0.78f;
	const float DoorHalfWidth = Width / (Phi * 8.f);
	const float WindowHalfWidth = Width / (Phi * 10.f);
	const float WindowOffset = Width / (Phi * 2.f);
	const float SideWindowHalfWidth = Depth / (Phi * 10.f);
	const float SideWindowOffset = Depth / (Phi * 2.f);

	const TArray<FVector2D> DoorOpening = { FVector2D(-DoorHalfWidth, DoorHalfWidth) };
	const TArray<FVector2D> FrontWindowOpenings = {
		FVector2D(-WindowOffset - WindowHalfWidth, -WindowOffset + WindowHalfWidth),
		FVector2D(-DoorHalfWidth, DoorHalfWidth),
		FVector2D(WindowOffset - WindowHalfWidth, WindowOffset + WindowHalfWidth)
	};
	const TArray<FVector2D> WindowOpenings = {
		FVector2D(-WindowOffset - WindowHalfWidth, -WindowOffset + WindowHalfWidth),
		FVector2D(WindowOffset - WindowHalfWidth, WindowOffset + WindowHalfWidth)
	};
	const TArray<FVector2D> SideWindowOpenings = {
		FVector2D(-SideWindowOffset - SideWindowHalfWidth, -SideWindowOffset + SideWindowHalfWidth),
		FVector2D(SideWindowOffset - SideWindowHalfWidth, SideWindowOffset + SideWindowHalfWidth)
	};

	// Each band is assembled around its openings, leaving actual empty spans in the wall.
	auto AddWallBand = [&](const FVector& WallCenter, const FVector& Along, float HalfSpan,
		float Thickness, float Bottom, float Top, const TArray<FVector2D>& Openings, bool bAlongFacing)
	{
		if (Top <= Bottom) { return; }
		float Cursor = -HalfSpan;
		auto AddPanel = [&](float Start, float End)
		{
			if (End <= Start) { return; }
			const float Span = End - Start;
			const FVector PanelAt = WallCenter + Along * ((Start + End) * 0.5f)
				+ FVector(0.f, 0.f, (Bottom + Top) * 0.5f);
			// The wall is the kit's own 880 x 100 x 400 cm stone panel fitted to the span it covers,
			// so the facade carries authored courses and coping instead of being a stretched box. The
			// panel's long axis is X, so a wall running along the side of the house needs the extra
			// 90 degrees; without it the panel lies across its own opening.
			const float PanelH = Top - Bottom;
			const FRotator PanelRot(0.f, YawDeg + (bAlongFacing ? 0.f : 90.f), 0.f);
			PartFitted(KitWall, PanelAt, FVector(Span, Thickness, PanelH),
				PanelRot, Cream, true);
		};

		for (const FVector2D& Opening : Openings)
		{
			const float OpeningStart = FMath::Clamp(Opening.X, -HalfSpan, HalfSpan);
			const float OpeningEnd = FMath::Clamp(Opening.Y, -HalfSpan, HalfSpan);
			AddPanel(Cursor, OpeningStart);
			Cursor = FMath::Max(Cursor, OpeningEnd);
		}
		AddPanel(Cursor, HalfSpan);
	};

	const float FrontOffset = HalfDepth - WallThickness * 0.5f;
	const float SideOffset = HalfWidth - WallThickness * 0.5f;
	const FVector FrontWall = At + Facing * FrontOffset;
	const FVector BackWall = At - Facing * FrontOffset;
	const FVector LeftWall = At - Side * SideOffset;
	const FVector RightWall = At + Side * SideOffset;

	// A tiled floor makes the house sit on a real stone base without closing the doorway.
	for (int32 X = 0; X < 2; ++X)
	{
		for (int32 Y = 0; Y < 2; ++Y)
		{
			const FVector FloorAt = At + Facing * ((X - 0.5f) * (Width * 0.5f))
				+ Side * ((Y - 0.5f) * (Depth * 0.5f)) + FVector(0.f, 0.f, 10.f);
			PartFitted(KitFloor, FloorAt, FVector(Width * 0.5f, Depth * 0.5f, 20.f),
				Yaw, Cream, true);
		}
	}

	// The plinth follows the footprint but leaves the entry clear.
	AddWallBand(FrontWall, Side, HalfWidth, WallThickness, 0.f, PlinthHeight, DoorOpening, false);
	AddWallBand(BackWall, Side, HalfWidth, WallThickness, 0.f, PlinthHeight, TArray<FVector2D>(), false);
	AddWallBand(LeftWall, Facing, HalfDepth, WallThickness, 0.f, PlinthHeight, TArray<FVector2D>(), true);
	AddWallBand(RightWall, Facing, HalfDepth, WallThickness, 0.f, PlinthHeight, TArray<FVector2D>(), true);

	// Street facade: door and two windows are open from the plinth to their lintels.
	AddWallBand(FrontWall, Side, HalfWidth, WallThickness, PlinthHeight, WindowSill, DoorOpening, false);
	AddWallBand(FrontWall, Side, HalfWidth, WallThickness, WindowSill, WindowHead, FrontWindowOpenings, false);
	AddWallBand(FrontWall, Side, HalfWidth, WallThickness, WindowHead, DoorHead, DoorOpening, false);
	AddWallBand(FrontWall, Side, HalfWidth, WallThickness, DoorHead, Height, TArray<FVector2D>(), false);

	// The rear and side elevations have paired, unfilled window openings as well.
	AddWallBand(BackWall, Side, HalfWidth, WallThickness, PlinthHeight, WindowSill, TArray<FVector2D>(), false);
	AddWallBand(BackWall, Side, HalfWidth, WallThickness, WindowSill, WindowHead, WindowOpenings, false);
	AddWallBand(BackWall, Side, HalfWidth, WallThickness, WindowHead, Height, TArray<FVector2D>(), false);
	AddWallBand(LeftWall, Facing, HalfDepth, WallThickness, PlinthHeight, WindowSill, TArray<FVector2D>(), true);
	AddWallBand(LeftWall, Facing, HalfDepth, WallThickness, WindowSill, WindowHead, SideWindowOpenings, true);
	AddWallBand(LeftWall, Facing, HalfDepth, WallThickness, WindowHead, Height, TArray<FVector2D>(), true);
	AddWallBand(RightWall, Facing, HalfDepth, WallThickness, PlinthHeight, WindowSill, TArray<FVector2D>(), true);
	AddWallBand(RightWall, Facing, HalfDepth, WallThickness, WindowSill, WindowHead, SideWindowOpenings, true);
	AddWallBand(RightWall, Facing, HalfDepth, WallThickness, WindowHead, Height, TArray<FVector2D>(), true);

	// Fitted stone sections close the front and rear gables beneath the pitched roof.
	for (const float EndSign : { -1.f, 1.f })
	{
		const FVector EndWall = EndSign > 0.f ? FrontWall : BackWall;
		for (int32 I = 0; I < 8; ++I)
		{
			const float Span = Width / 8.f;
			const float Offset = -HalfWidth + Span * (I + 0.5f);
			const float GableHeight = RoofRise * (1.f - FMath::Abs(Offset) / HalfWidth);
			if (GableHeight <= 0.f) { continue; }
			const FVector GableAt = EndWall + Side * Offset
				+ FVector(0.f, 0.f, Height + GableHeight * 0.5f);
			PartFitted(KitWall, GableAt, FVector(Span, WallThickness, GableHeight),
				FRotator(0.f, YawDeg + 90.f, 0.f), Cream, true);
		}
	}

	// Gold window ledges and an entry lantern add detail without filling the openings.
	for (int32 I = 0; I < 2; ++I)
	{
		const float WindowSign = I == 0 ? -1.f : 1.f;
		for (const float LedgeZ : { WindowSill - 8.f, WindowHead + 8.f })
		{
			PartFitted(KitDecor, FrontWall + Facing * (WallThickness * 0.5f)
				+ Side * (WindowSign * WindowOffset) + FVector(0.f, 0.f, LedgeZ),
				FVector(WindowHalfWidth * 2.f + 36.f, WallThickness, 18.f),
				FRotator(0.f, YawDeg + 90.f, 0.f), Gold, false);
		}
	}
	PartFitted(KitDecor,
		FrontWall + Facing * (WallThickness * 0.5f + 6.f) + FVector(0.f, 0.f, DoorHead + 35.f),
		FVector(DoorHalfWidth * 2.f, WallThickness, 70.f),
		FRotator(0.f, YawDeg + 90.f, 0.f), Gold, false);

	// Two blue roof planes meet at a gold ridge; the lower eaves extend past every wall.
	const float RoofOverhang = Depth / (Phi * 10.f);
	const float RoofHalfSpan = HalfWidth + RoofOverhang;
	const float RoofAngle = FMath::RadiansToDegrees(FMath::Atan2(RoofRise, HalfWidth));
	const float RoofSlopeLength = FMath::Sqrt(
		RoofHalfSpan * RoofHalfSpan + FMath::Square(RoofRise * RoofHalfSpan / HalfWidth));
	const float RoofRun = Depth + RoofOverhang * 2.f;
	const float RidgeZ = Height + RoofRise;
	const float RoofCenterZ = RidgeZ - RoofRise * (RoofHalfSpan * 0.5f / HalfWidth);
	const float RoofThickness = 24.f;
	// The roof planes reuse an installed stone tile; no primitive fallback is substituted.
	PartFitted(KitFloor, At + Side * (RoofHalfSpan * 0.5f) + FVector(0.f, 0.f, RoofCenterZ),
		FVector(RoofRun, RoofSlopeLength, RoofThickness),
		FRotator(0.f, YawDeg, -RoofAngle), Blue, true);
	PartFitted(KitFloor, At - Side * (RoofHalfSpan * 0.5f) + FVector(0.f, 0.f, RoofCenterZ),
		FVector(RoofRun, RoofSlopeLength, RoofThickness),
		FRotator(0.f, YawDeg, RoofAngle), Blue, true);
	PartFitted(KitDecor, At + FVector(0.f, 0.f, RidgeZ),
		FVector(RoofRun, WallThickness, WallThickness), Yaw, Gold, false);
}

void AVEILGoldenHub::Bridge(const FVector& A, const FVector& B)
{
	const FVector Delta = B - A;
	const float Len = Delta.Size2D();
	const FRotator Along(0.f, Delta.Rotation().Yaw, 0.f);
	const int32 Planks = FMath::Max(6, FMath::RoundToInt(Len / 120.f));
	for (int32 I = 0; I <= Planks; ++I)
	{
		const FVector P = A + Delta * ((float)I / Planks);
		PartFitted(KitFloor, P + FVector(0.f, 0.f, 40.f),
			FVector(120.f, 150.f, 20.f), Along, Cream, true);
	}
	const FVector Side(-Delta.Y / Len, Delta.X / Len, 0.f);
	for (const float SideSign : { -1.f, 1.f })
	{
		for (int32 I = 0; I < Planks; ++I)
		{
			const float T0 = (float)I / Planks;
			const float T1 = (float)(I + 1) / Planks;
			const FVector P0 = A + Delta * T0 + Side * (SideSign * 150.f)
				+ FVector(0.f, 0.f, 300.f - FMath::Sin(T0 * PI) * 260.f);
			const FVector P1 = A + Delta * T1 + Side * (SideSign * 150.f)
				+ FVector(0.f, 0.f, 300.f - FMath::Sin(T1 * PI) * 260.f);
			const FVector Segment = P1 - P0;
			PartFitted(KitDecor, (P0 + P1) * 0.5f,
				FVector(Segment.Size(), 24.f, 24.f), Segment.Rotation(), Gold, false);
		}
	}
}

void AVEILGoldenHub::Spire(const FVector& At, float Height, float Radius)
{
	PartFitted(KitMonolith, At + FVector(0.f, 0.f, Height * 0.5f),
		FVector(Radius * 2.f, Radius * 2.f, Height), FRotator::ZeroRotator, Gold, true);
	for (int32 I = 1; I <= 5; ++I)
	{
		const float F = I / 6.f;
		const float RingRadius = Radius * 2.f * (1.f - F * 0.55f);
		PartFitted(KitDecor, At + FVector(0.f, 0.f, Height * F),
			FVector(RingRadius, 90.f, 90.f), FRotator::ZeroRotator, Blue, false);
	}
	PartFitted(KitDecor, At + FVector(0.f, 0.f, Height + 100.f),
		FVector(250.f, 180.f, 120.f), FRotator::ZeroRotator, Gold, false);
}
