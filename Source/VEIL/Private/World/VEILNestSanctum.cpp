#include "World/VEILNestSanctum.h"

#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/PointLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "VEIL.h"

namespace
{
	constexpr float GoldenDeg = 137.50776405f;

	// The project's own palette, the same paths the district already uses. Stone is the kit's
	// cut stone: the gold is trim, never a body.
	const TCHAR* StonePath = TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone");
	const TCHAR* GoldPath = TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic");
	const TCHAR* GlowPath = TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive");
	const TCHAR* WaterPath = TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue");
}

AVEILNestSanctum::AVEILNestSanctum()
{
	PrimaryActorTick.bCanEverTick = false;
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

void AVEILNestSanctum::AddLamp(const FVector& At, float Intensity, float Radius,
	const FLinearColor& Colour)
{
	UWorld* W = GetWorld();
	if (!W) { return; }
	if (APointLight* Lamp = W->SpawnActor<APointLight>(APointLight::StaticClass(), At,
		FRotator::ZeroRotator))
	{
		Lamp->Tags.Add(FName(TEXT("VEIL_BAKE")));
		if (UPointLightComponent* L = Cast<UPointLightComponent>(Lamp->GetLightComponent()))
		{
			L->SetMobility(EComponentMobility::Movable);
			L->SetIntensity(Intensity);
			L->SetAttenuationRadius(Radius);
			L->SetLightColor(Colour);
			// The sun already casts. One shadowed light per beacon is a shadow pass per beacon.
			L->SetCastShadows(false);
			L->SetAffectGlobalIllumination(false);
		}
	}
}

bool AVEILNestSanctum::BuildSanctum()
{
	if (bBuilt) { return true; }

	UWorld* W = GetWorld();
	if (!W) { return false; }

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Cyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Ball = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UMaterialInterface* Stone = LoadObject<UMaterialInterface>(nullptr, StonePath);
	UMaterialInterface* Gold = LoadObject<UMaterialInterface>(nullptr, GoldPath);
	UMaterialInterface* Glow = LoadObject<UMaterialInterface>(nullptr, GlowPath);
	UMaterialInterface* Water = LoadObject<UMaterialInterface>(nullptr, WaterPath);

	if (!Cube || !Cyl || !Stone)
	{
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI SANCTUM: engine shapes or the stone material are missing - nothing was raised."));
		return false;
	}

	// THE DECK SITS ON THE SURFACE THE HUB ACTUALLY HAS.
	//
	// At the plaza centre the first surface under the sky is the stacked mass at 500 cm, not the
	// street at 0. A deck at street level would be inside it, which is the state the runner was
	// already waking in. So the surface is measured, and the deck is raised on it - and the log
	// says which height it found, so nobody has to guess where the floor went.
	{
		FHitResult Floor;
		FCollisionQueryParams Query(SCENE_QUERY_STAT(VeilSanctumFloor), false);
		// NOT THE FIRST THING UNDER THE SKY.
		//
		// A single trace from 60 km struck the landmark proxy at 4,302 cm and raised the deck in
		// orbit, thirty-four pieces of platform floating above a district nobody could see. The
		// surface that matters is the one a body can stand on under the hub's roofline, so the
		// trace steps past anything above 1,200 cm and takes the first floor below it.
		FVector From(0.f, 0.f, 60000.f);
		float Found = 98.f;
		for (int32 Step = 0; Step < 32; ++Step)
		{
			if (!W->LineTraceSingleByChannel(Floor, From, FVector(0.f, 0.f, -4000.f),
				ECC_Visibility, Query))
			{
				break;
			}
			if (Floor.ImpactPoint.Z <= 1200.f)
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
		const float Angle = FMath::DegreesToRadians(i * GoldenDeg);
		const FVector At(FMath::Cos(Angle) * 1400.f, FMath::Sin(Angle) * 1400.f, DeckZ + 80.f);
		SpawnPiece(Cube, At, FVector(60.f, 60.f, 160.f), Gold, true);
	}

	// ---- THREE PILLARS, STONE BODY AND GOLD CROWN -----------------------------
	// On 120 degrees with the pitch folded in, so the trio is not axis-aligned.
	for (int32 i = 0; i < 3; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * 120.f + 17.0324f);
		const FVector Base(FMath::Cos(Angle) * 2050.f, FMath::Sin(Angle) * 2050.f, DeckZ);
		SpawnPiece(Cyl, Base + FVector(0.f, 0.f, 550.f), FVector(150.f, 150.f, 1100.f), Stone, true);
		SpawnPiece(Cyl, Base + FVector(0.f, 0.f, 1160.f), FVector(190.f, 190.f, 120.f), Gold, false);
	}

	// ---- THE SPIRE: THE ONE THING VISIBLE FROM EVERYWHERE --------------------
	SpawnPiece(Cyl, FVector(0.f, 0.f, DeckZ + 900.f), FVector(180.f, 180.f, 1800.f), Gold, true);
	if (Ball)
	{
		SpawnPiece(Ball, FVector(0.f, 0.f, DeckZ + 1880.f), FVector(220.f, 220.f, 220.f), Glow, false);
	}
	AddLamp(FVector(0.f, 0.f, DeckZ + 1700.f), 6000.f, 2200.f, FLinearColor(1.f, 0.86f, 0.55f));

	// ---- THREE BEACON SHAFTS --------------------------------------------------
	// The Bible's three beacon addresses, lifted onto the deck's height: a shaft of light the
	// runner can see from the platform even when the ground beneath it is buried.
	const FVector BeaconPlan[3] = {
		FVector(2400.f, 430.f, 0.f),
		FVector(-1710.f, -770.f, 0.f),
		FVector(510.f, -1800.f, 0.f) };

	for (int32 i = 0; i < 3; ++i)
	{
		const FVector Foot = BeaconPlan[i] + FVector(0.f, 0.f, DeckZ);
		SpawnPiece(Cyl, Foot + FVector(0.f, 0.f, 150.f), FVector(240.f, 240.f, 300.f), Stone, true);
		SpawnPiece(Cyl, Foot + FVector(0.f, 0.f, 1500.f), FVector(90.f, 90.f, 2400.f), Glow, false);
		AddLamp(Foot + FVector(0.f, 0.f, 700.f), 9000.f, 1600.f, FLinearColor(1.f, 0.78f, 0.32f));
	}

	// ---- THE FALL OF WATER ----------------------------------------------------
	// A sheet at the deck's edge, falling past it. Deep blue, emissive through the palette, no
	// particle system and no shader authoring: the district's water reads as water by colour and
	// motion, and this is the half that can be built in code tonight.
	if (Water)
	{
		SpawnPiece(Cube, FVector(0.f, 1500.f, DeckZ - 620.f), FVector(620.f, 24.f, 1240.f), Water, false);
		SpawnPiece(Cube, FVector(0.f, 1520.f, DeckZ - 1240.f), FVector(700.f, 200.f, 40.f), Water, false);
	}

	bBuilt = true;

	// THE RUNNER STANDS ON THE DECK, NOT INSIDE THE HUB.
	//
	// The seat probe resolved him onto the slab at 598, inside the mass, because that is where
	// the collision was at the time. He is placed on the deck one golden-angle step from the
	// Spire instead: on the surface that was just built, with the ring and the shafts in view.
	if (APawn* Runner = UGameplayStatics::GetPlayerPawn(W, 0))
	{
		const float Step = FMath::DegreesToRadians(GoldenDeg);
		const FVector Stand(FMath::Cos(Step) * 700.f, FMath::Sin(Step) * 700.f, DeckZ + 100.f);
		Runner->SetActorLocation(Stand, false, nullptr, ETeleportType::TeleportPhysics);
		if (AController* C = Runner->GetController())
		{
			const FRotator ToSpire = (FVector(0.f, 0.f, DeckZ + 900.f) - Stand).Rotation();
			C->SetControlRotation(FRotator(-6.f, ToSpire.Yaw, 0.f));
		}
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI SANCTUM: the runner stands on the deck at %s, %.0f cm above it, facing the Spire."),
			*Stand.ToCompactString(), 100.f);
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI SANCTUM: deck raised at Z=%.0f on the surface the hub actually has - %d pieces "
			"(ring of 13, three pillars, the Spire, three beacon shafts, the fall of water)."),
		DeckZ, Pieces);
	return true;
}
