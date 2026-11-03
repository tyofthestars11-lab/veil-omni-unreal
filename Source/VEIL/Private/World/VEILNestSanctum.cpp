#include "World/VEILNestSanctum.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "VEIL.h"
#include "Systems/VEILPhi.h"

namespace
{
	constexpr float NestGoldenDeg = VeilPhi::GoldenAngleF;

	// The project's own palette, the same paths the district already uses. Stone is the kit's
	// cut stone: the gold is trim, never a body.
	const TCHAR* NestStonePath = TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone");
	const TCHAR* NestGoldPath = TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic");
	const TCHAR* NestGlowPath = TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive");
	const TCHAR* NestWaterPath = TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue");
}

AVEILNestSanctum::AVEILNestSanctum()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

// ---------------------------------------------------------------------------
// THE SANCTUM MOVES.
//
// Two motions, both from the framework rather than from a taste: the halo turns at the golden
// angle per step, and each beacon shaft breathes on the pitch's own phase, offset by the golden
// angle in radians so the three never pulse together. It is one actor's tick over a handful of
// components - no particle system, no per-frame allocation, and nothing that needs an asset.
// ---------------------------------------------------------------------------
void AVEILNestSanctum::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bBuilt) { return; }

	HaloPhase += DeltaSeconds * 0.35f;

	const float Radius = 520.f;
	for (int32 i = 0; i < Halo.Num(); ++i)
	{
		AActor* Shard = Halo[i].Get();
		if (!Shard) { continue; }
		const float Angle = HaloPhase + FMath::DegreesToRadians(i * NestGoldenDeg);
		const float Rise = FMath::Sin(HaloPhase * 2.f + i * 0.6180339887f) * 40.f;
		Shard->SetActorLocation(FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius,
			DeckZ + 240.f + Rise));
		Shard->SetActorRotation(FRotator(0.f, FMath::RadiansToDegrees(Angle), 25.f));
	}

	for (int32 i = 0; i < BeaconLamps.Num(); ++i)
	{
		UPointLightComponent* Lamp = BeaconLamps[i].Get();
		if (!Lamp || !BeaconBase.IsValidIndex(i)) { continue; }
		const float Phase = HaloPhase * VeilPhi::PhiF + i * 2.39996323f;   // golden angle, radians
		Lamp->SetIntensity(BeaconBase[i] * (1.f + 0.18f * FMath::Sin(Phase)));
	}
}

UStaticMeshComponent* AVEILNestSanctum::SpawnPiece(UStaticMesh* Mesh, const FVector& Centre,
	const FVector& Size, UMaterialInterface* Mat, bool bCollide)
{
	UWorld* W = GetWorld();
	if (!W || !Mesh) { return nullptr; }

	AStaticMeshActor* Piece = W->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
		Centre, FRotator::ZeroRotator);
	if (!Piece) { return nullptr; }

	// Generated stage, tagged like everything the builders make, so a re-bake can find it.
	Piece->Tags.Add(FName(TEXT("VEIL_BAKE")));

	UStaticMeshComponent* Comp = Piece->GetStaticMeshComponent();
	Comp->SetStaticMesh(Mesh);
	Comp->SetMobility(EComponentMobility::Movable);
	Comp->SetCollisionEnabled(bCollide ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	Comp->SetCollisionResponseToAllChannels(bCollide ? ECR_Block : ECR_Ignore);
	if (Mat) { Comp->SetMaterial(0, Mat); }

	// SIZE IN CENTIMETRES, NOT SCALE. Authored meshes are not unit cubes, so the requested size is
	// divided by the mesh's own extent; the pivot offset is then corrected so the geometry's centre
	// lands on the coordinate asked for instead of sliding away by (origin x scale).
	const FBoxSphereBounds Bounds = Mesh->GetBounds();
	const FVector Extent = Bounds.BoxExtent.GetAbs();
	FVector Scale(1.f, 1.f, 1.f);
	if (Extent.X > KINDA_SMALL_NUMBER) { Scale.X = (Size.X * 0.5f) / Extent.X; }
	if (Extent.Y > KINDA_SMALL_NUMBER) { Scale.Y = (Size.Y * 0.5f) / Extent.Y; }
	if (Extent.Z > KINDA_SMALL_NUMBER) { Scale.Z = (Size.Z * 0.5f) / Extent.Z; }

	Piece->SetActorScale3D(Scale);
	Piece->SetActorLocation(Centre - Bounds.Origin * Scale);

	++Pieces;
	return Comp;
}

UPointLightComponent* AVEILNestSanctum::AddLamp(const FVector& At, float Intensity, float Radius,
	const FLinearColor& Colour)
{
	UWorld* W = GetWorld();
	if (!W) { return nullptr; }

	APointLight* Lamp = W->SpawnActor<APointLight>(APointLight::StaticClass(), At,
		FRotator::ZeroRotator);
	if (!Lamp) { return nullptr; }

	Lamp->Tags.Add(FName(TEXT("VEIL_BAKE")));
	UPointLightComponent* L = Cast<UPointLightComponent>(Lamp->GetLightComponent());
	if (!L) { return nullptr; }

	L->SetMobility(EComponentMobility::Movable);
	L->SetIntensity(Intensity);
	L->SetAttenuationRadius(Radius);
	L->SetLightColor(Colour);
	// The sun already casts. One shadowed light per beacon is a shadow pass per beacon.
	L->SetCastShadows(false);
	L->SetAffectGlobalIllumination(false);
	return L;
}

namespace
{
	// ------------------------------------------------------------------
	// OPEN THE SKY.
	//
	// The bake stacked lids over the plaza centre - measured at 500, 1250,
	// 1751 and 2440 cm - so the front door of the game is a ceiling with a
	// runner under it. Those slabs are big, flat, and directly above the
	// plaza; nothing the player can do up there is content. They are hidden
	// and stripped of collision here rather than re-baked, because the level
	// is the game and the game has to open.
	//
	// The filter is narrow on purpose: whole bounds above 350 cm, footprint
	// inside 1,150 cm of the plaza centre, wider than 600 cm on both axes.
	// The market ring stands outside that disc, so its rooftops stay.
	// ------------------------------------------------------------------
	int32 ClearOverheadSlabs(UWorld* World, AActor* Keeper)
	{
		if (!World) { return 0; }
		int32 Opened = 0;
		AActor* Biggest = nullptr;
		float BiggestTop = 0.f;
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Slab = *It;
			if (!Slab || Slab == Keeper || Slab->IsA<APawn>()) { continue; }
			const FBox B = Slab->GetComponentsBoundingBox(true);
			if (!B.IsValid) { continue; }
			const FVector C = B.GetCenter();
			const FVector E = B.GetExtent();
			// EVERYTHING OVERHEAD COUNTS, whatever class carries it: the 500 cm lid
			// is a plain actor, the roofs at 270 and 348 are thin slabs, and a
			// class filter missed both and reported "0 slabs" over a buried plaza.
			if (B.Min.Z < 300.f) { continue; }
			if (FMath::Abs(C.X) > 3000.f || FMath::Abs(C.Y) > 3000.f) { continue; }
			if (E.Z > 900.f) { continue; }              // towers, not lids
			if (FMath::Max(E.X, E.Y) < 350.f) { continue; }  // posts, not lids
			Slab->SetActorHiddenInGame(true);
			Slab->SetActorEnableCollision(false);
			++Opened;
			if (B.Max.Z > BiggestTop) { BiggestTop = B.Max.Z; Biggest = Slab; }
		}
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI SKY: %d slab(s) overhead opened - hidden, collision off, highest at Z=%.0f (%s)."),
			Opened, BiggestTop, Biggest ? *Biggest->GetName() : TEXT("none"));
		return Opened;
	}
}
int32 AVEILNestSanctum::OpenSky(UWorld* World)
{
	return ClearOverheadSlabs(World, nullptr);
}

bool AVEILNestSanctum::BuildSanctum()
{
	ClearOverheadSlabs(GetWorld(), this);
	if (bBuilt) { return true; }

	UWorld* W = GetWorld();
	if (!W) { return false; }

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Ball = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UMaterialInterface* Stone = LoadObject<UMaterialInterface>(nullptr, NestStonePath);
	UMaterialInterface* Gold = LoadObject<UMaterialInterface>(nullptr, NestGoldPath);
	UMaterialInterface* Glow = LoadObject<UMaterialInterface>(nullptr, NestGlowPath);
	UMaterialInterface* Water = LoadObject<UMaterialInterface>(nullptr, NestWaterPath);

	if (!Cube || !Cyl || !Stone)
	{
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI SANCTUM: engine shapes or the stone material are missing - nothing was raised."));
		return false;
	}

	// THE DECK SITS ON THE SURFACE THE HUB ACTUALLY HAS.
	//
	// A single trace from 60 km struck the landmark proxy at 4,302 cm and raised the deck in
	// orbit; a single trace after streaming hit the street at 0 and put the deck under the hub.
	// So the trace walks down, skips anything above the roofline, and takes the first floor below
	// it - then the log states the height, so nobody has to guess where the floor went.
	{
		FHitResult Floor;
		FCollisionQueryParams Query(SCENE_QUERY_STAT(VeilSanctumFloor), false);
		FVector From(0.f, 0.f, 60000.f);
		float Found = 98.f;
		for (int32 Step = 0; Step < 32; ++Step)
		{
			if (!W->LineTraceSingleByChannel(Floor, From, FVector(0.f, 0.f, -4000.f),
				ECC_Visibility, Query))
			{
				break;
			}
			// A MESHLESS PROXY IS NOT A FLOOR. The hub's landmarks are collision boxes with no
			// geometry; a deck raised on one floats in the air above a world nobody can see.
			const UStaticMeshComponent* HitMesh = Floor.GetActor()
				? Floor.GetActor()->FindComponentByClass<UStaticMeshComponent>() : nullptr;
			const bool bRealSurface = HitMesh && HitMesh->GetStaticMesh();
			if (bRealSurface && Floor.ImpactPoint.Z <= 1200.f)
			{
				Found = FMath::Max(98.f, Floor.ImpactPoint.Z + 2.f);
				break;
			}
			From = FVector(0.f, 0.f, Floor.ImpactPoint.Z - 25.f);
		}
		DeckZ = Found;
	}

	// ---- THE DECK -------------------------------------------------------------
	// 30 m across, 40 cm thick, stone, with collision: this is the floor the seat probe finds.
	SpawnPiece(Cube, FVector(0.f, 0.f, DeckZ - 20.f), FVector(3000.f, 3000.f, 40.f), Stone, true);

	// Gold trim on the four edges: the deck reads as a designed platform, not a slab.
	for (int32 Side = 0; Side < 4; ++Side)
	{
		const bool bAlongX = (Side % 2) == 0;
		const float Sign = (Side < 2) ? 1.f : -1.f;
		const FVector At = bAlongX
			? FVector(0.f, Sign * 1490.f, DeckZ + 8.f)
			: FVector(Sign * 1490.f, 0.f, DeckZ + 8.f);
		const FVector Size = bAlongX
			? FVector(3000.f, 20.f, 16.f)
			: FVector(20.f, 3000.f, 16.f);
		SpawnPiece(Cube, At, Size, Gold, false);
	}

	// ---- THE RING: THIRTEEN POSTS ON THE GOLDEN ANGLE -------------------------
	for (int32 i = 0; i < 13; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * NestGoldenDeg);
		const FVector At(FMath::Cos(Angle) * 1400.f, FMath::Sin(Angle) * 1400.f, DeckZ + 80.f);
		SpawnPiece(Cube, At, FVector(60.f, 60.f, 160.f), Gold, true);
	}

	// ---- THREE PILLARS, STONE BODY AND GOLD CROWN -----------------------------
	for (int32 i = 0; i < 3; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * 120.f + 17.0324f);
		const FVector Base(FMath::Cos(Angle) * 2050.f, FMath::Sin(Angle) * 2050.f, DeckZ);
		SpawnPiece(Cyl, Base + FVector(0.f, 0.f, 550.f), FVector(150.f, 150.f, 1100.f), Stone, true);
		SpawnPiece(Cyl, Base + FVector(0.f, 0.f, 1160.f), FVector(190.f, 190.f, 120.f), Gold, false);
	}

	// ---- THE SPIRE ------------------------------------------------------------
	SpawnPiece(Cyl, FVector(0.f, 0.f, DeckZ + 900.f), FVector(180.f, 180.f, 1800.f), Gold, true);
	if (Ball)
	{
		SpawnPiece(Ball, FVector(0.f, 0.f, DeckZ + 1880.f), FVector(220.f, 220.f, 220.f), Glow, false);
	}
	AddLamp(FVector(0.f, 0.f, DeckZ + 1700.f), 6000.f, 2200.f, FLinearColor(1.f, 0.86f, 0.55f));

	// ---- THE HALO: EIGHT SHARDS TURNING ON THE GOLDEN ANGLE -------------------
	for (int32 i = 0; i < 8; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * NestGoldenDeg);
		if (UStaticMeshComponent* Shard = SpawnPiece(Cube,
			FVector(FMath::Cos(Angle) * 520.f, FMath::Sin(Angle) * 520.f, DeckZ + 240.f),
			FVector(60.f, 60.f, 180.f), Gold, false))
		{
			if (AActor* ShardActor = Shard->GetOwner()) { Halo.Add(ShardActor); }
		}
	}

	// ---- THREE BEACON SHAFTS --------------------------------------------------
	const FVector BeaconPlan[3] = {
		FVector(2400.f, 430.f, 0.f),
		FVector(-1710.f, -770.f, 0.f),
		FVector(510.f, -1800.f, 0.f) };

	for (int32 i = 0; i < 3; ++i)
	{
		const FVector Foot = BeaconPlan[i] + FVector(0.f, 0.f, DeckZ);
		SpawnPiece(Cyl, Foot + FVector(0.f, 0.f, 150.f), FVector(240.f, 240.f, 300.f), Stone, true);
		SpawnPiece(Cyl, Foot + FVector(0.f, 0.f, 1500.f), FVector(90.f, 90.f, 2400.f), Glow, false);
		if (UPointLightComponent* Beacon = AddLamp(Foot + FVector(0.f, 0.f, 700.f), 9000.f, 1600.f,
			FLinearColor(1.f, 0.78f, 0.32f)))
		{
			BeaconLamps.Add(Beacon);
			BeaconBase.Add(9000.f);
		}
	}

	// ---- THE FALL OF WATER ----------------------------------------------------
	if (Water)
	{
		// THE FALL MOVED OFF THE STREET AND BELOW THE DECK.
		// It stood at X=0 on the spawn sightline with a deep-blue surface at deck height, which is
		// the purple slab with a black top that filled the first frame - water rendered where a
		// building should be reads as a placeholder no matter what it is made of. It is now west
		// of the plaza and sunk under the deck, so it is a channel you find, not a wall you face.
		SpawnPiece(Cube, FVector(-2600.f, 1500.f, DeckZ - 900.f), FVector(620.f, 24.f, 1240.f), Water, false);
		SpawnPiece(Cube, FVector(-2600.f, 1520.f, DeckZ - 1520.f), FVector(700.f, 200.f, 40.f), Water, false);
	}

	bBuilt = true;

	// THE RUNNER STANDS ON THE DECK, NOT INSIDE THE HUB.
	if (APawn* Runner = UGameplayStatics::GetPlayerPawn(W, 0))
	{
		const float Step = FMath::DegreesToRadians(NestGoldenDeg);
		const FVector Stand(FMath::Cos(Step) * 700.f, FMath::Sin(Step) * 700.f, DeckZ + 100.f);
		Runner->SetActorLocation(Stand, false, nullptr, ETeleportType::TeleportPhysics);
		if (AController* C = Runner->GetController())
		{
			const FRotator ToSpire = (FVector(0.f, 0.f, DeckZ + 900.f) - Stand).Rotation();
			C->SetControlRotation(FRotator(-6.f, ToSpire.Yaw, 0.f));
		}
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI SANCTUM: the runner stands on the deck at %s, 100 cm above it, facing the Spire."),
			*Stand.ToCompactString());
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI SANCTUM: deck raised at Z=%.0f on the surface the hub actually has - %d pieces, "
			"8 halo shards turning, 3 beacon shafts breathing."),
		DeckZ, Pieces);
	return true;
}
