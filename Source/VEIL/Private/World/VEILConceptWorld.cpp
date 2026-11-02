#include "World/VEILConceptWorld.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "VEIL.h"

namespace
{
	constexpr float ConceptPhi = 1.6180339887f;
	constexpr float ConceptGoldenDeg = 137.50776405f;

	// The project's palette. Stone is the kit's cut stone; gold is trim and light.
	const TCHAR* StonePath = TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone");
	const TCHAR* GoldPath = TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic");
	const TCHAR* GlowPath = TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive");
	const TCHAR* WaterPath = TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue");

	// The world's addresses, as the Bible fixes them.
	const FVector Plaza(0.f, 0.f, 0.f);
	const FVector SpireAt(2800.f, 500.f, 0.f);
	const FVector DeepAt(0.f, -6200.f, 0.f);
}

AVEILConceptWorld::AVEILConceptWorld()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

UStaticMeshComponent* AVEILConceptWorld::SpawnPiece(UStaticMesh* Mesh, const FVector& Centre,
	const FVector& Size, UMaterialInterface* Mat, const FRotator& Rot, bool bCollide)
{
	UWorld* W = GetWorld();
	if (!W || !Mesh) { return nullptr; }

	AStaticMeshActor* Piece = W->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
		Centre, Rot);
	if (!Piece) { return nullptr; }
	Piece->Tags.Add(FName(TEXT("VEIL_BAKE")));

	UStaticMeshComponent* Comp = Piece->GetStaticMeshComponent();
	Comp->SetStaticMesh(Mesh);
	Comp->SetMobility(EComponentMobility::Movable);
	Comp->SetCollisionEnabled(bCollide ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	Comp->SetCollisionResponseToAllChannels(bCollide ? ECR_Block : ECR_Ignore);
	if (Mat) { Comp->SetMaterial(0, Mat); }

	// SIZE IN CENTIMETRES: authored shapes are not unit cubes, so divide by the
	// mesh's own extent, then correct the pivot so the geometry lands where asked.
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

bool AVEILConceptWorld::Build()
{
	if (bBuilt) { return true; }

	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* Cone = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (!Cube || !Cylinder) { return false; }

	UMaterialInterface* Stone = LoadObject<UMaterialInterface>(nullptr, StonePath);
	UMaterialInterface* Gold = LoadObject<UMaterialInterface>(nullptr, GoldPath);
	UMaterialInterface* Glow = LoadObject<UMaterialInterface>(nullptr, GlowPath);
	UMaterialInterface* Water = LoadObject<UMaterialInterface>(nullptr, WaterPath);

	BuildPhiPillars(Cube, Cylinder, Sphere, Gold, Stone, Glow);
	BuildMarket(Cube, Cone, Cylinder, Gold, Stone);
	BuildWaterAndBridges(Cube, Cylinder, Water, Stone, Gold);
	BuildCrystals(Cone, Cylinder, Water, Glow);
	BuildDeep(Cube, Cylinder, Water, Glow);
	BuildWings(Cube, Gold);

	bBuilt = true;
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI CONCEPT: composition laid from the golden ratio - %d pieces (pillars, market, falls, bridges, crystals, the Deep, wings)."),
		Pieces);
	return true;
}

// ---------------------------------------------------------------------------
// THE THREE PHI PILLARS. The image's foreground: three gold-topped columns over
// the market, each carrying the phi glyph. They stand on the golden angle at
// radius 1,100, 1,100*phi and 1,100*phi^2 - the same rule as the district ring,
// so the foreground reads as part of the world rather than decoration on it.
// ---------------------------------------------------------------------------
void AVEILConceptWorld::BuildPhiPillars(UStaticMesh* Cube, UStaticMesh* Cylinder,
	UStaticMesh* Sphere, UMaterialInterface* Gold, UMaterialInterface* Stone,
	UMaterialInterface* Glow)
{
	for (int32 i = 0; i < 3; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * ConceptGoldenDeg + 30.f);
		const float Radius = 1100.f * FMath::Pow(ConceptPhi, (float)i);
		const FVector At(Plaza.X + FMath::Cos(Angle) * Radius,
			Plaza.Y + FMath::Sin(Angle) * Radius, 0.f);

		// Shaft, base and gold crown: stone body, metallic trim.
		SpawnPiece(Cylinder, At + FVector(0.f, 0.f, 850.f), FVector(420.f, 420.f, 1700.f), Stone, FRotator::ZeroRotator, true);
		SpawnPiece(Cube, At + FVector(0.f, 0.f, 60.f), FVector(620.f, 620.f, 120.f), Stone, FRotator::ZeroRotator, true);
		SpawnPiece(Cube, At + FVector(0.f, 0.f, 1760.f), FVector(700.f, 700.f, 120.f), Gold, FRotator::ZeroRotator, true);

		// THE PHI GLYPH, built from the shapes: a flattened sphere for the loop and a
		// vertical bar through it. Two pieces, both gold, facing the plaza.
		const FVector Face = (Plaza - At).GetSafeNormal2D();
		const FRotator Yaw(0.f, FMath::RadiansToDegrees(FMath::Atan2(Face.Y, Face.X)) - 90.f, 0.f);
		const FVector GlyphAt = At + FVector(0.f, 0.f, 1350.f) + Face * 215.f;
		SpawnPiece(Sphere, GlyphAt, FVector(360.f, 40.f, 300.f), Gold, Yaw, false);
		SpawnPiece(Cube, GlyphAt + Face * 6.f, FVector(46.f, 26.f, 460.f), Gold, Yaw, false);
	}
}

// ---------------------------------------------------------------------------
// THE MARKET. Tents on the terraces: a cone canopy over four posts, a counter
// slab, a lamp-bright cloth. Sixteen on the golden angle, radius stepping by phi,
// which is what makes a market read as grown rather than gridded.
// ---------------------------------------------------------------------------
void AVEILConceptWorld::BuildMarket(UStaticMesh* Cube, UStaticMesh* Cone, UStaticMesh* Cylinder,
	UMaterialInterface* Gold, UMaterialInterface* Stone)
{
	for (int32 i = 0; i < 16; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * ConceptGoldenDeg);
		const float Ring = 1500.f * FMath::Pow(ConceptPhi, (float)(i % 3) * 0.5f);
		const FVector At(Plaza.X + FMath::Cos(Angle) * Ring, Plaza.Y + FMath::Sin(Angle) * Ring, 0.f);

		SpawnPiece(Cube, At + FVector(0.f, 0.f, 55.f), FVector(620.f, 620.f, 110.f), Stone, FRotator::ZeroRotator, true);
		SpawnPiece(Cone, At + FVector(0.f, 0.f, 470.f), FVector(760.f, 760.f, 380.f), Gold, FRotator::ZeroRotator, false);

		for (int32 Post = 0; Post < 4; ++Post)
		{
			const float PA = FMath::DegreesToRadians(45.f + Post * 90.f);
			SpawnPiece(Cylinder, At + FVector(FMath::Cos(PA) * 260.f, FMath::Sin(PA) * 260.f, 230.f),
				FVector(60.f, 60.f, 460.f), Stone, FRotator::ZeroRotator, true);
		}
	}
}

// ---------------------------------------------------------------------------
// THE FALLS AND THE BRIDGES. Four falls off the shelves, on the golden angle, and
// three bridges that span them. The falls are emissive sheets - no particle system
// exists in this project, so the water is geometry that reads as water.
// ---------------------------------------------------------------------------
void AVEILConceptWorld::BuildWaterAndBridges(UStaticMesh* Cube, UStaticMesh* Cylinder,
	UMaterialInterface* Water, UMaterialInterface* Stone, UMaterialInterface* Gold)
{
	for (int32 i = 0; i < 4; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * ConceptGoldenDeg + 20.f);
		const float Radius = 2300.f * FMath::Pow(ConceptPhi, (float)i);
		const FVector At(Plaza.X + FMath::Cos(Angle) * Radius, Plaza.Y + FMath::Sin(Angle) * Radius, 0.f);

		// The shelf, then the fall down its face.
		SpawnPiece(Cube, At + FVector(0.f, 0.f, 320.f), FVector(1400.f, 1400.f, 640.f), Stone, FRotator::ZeroRotator, true);
		SpawnPiece(Cube, At + FVector(0.f, 0.f, 520.f), FVector(900.f, 120.f, 1500.f), Water,
			FRotator(0.f, FMath::RadiansToDegrees(Angle), 0.f), false);
	}

	for (int32 i = 0; i < 3; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * ConceptGoldenDeg + 75.f);
		const FVector Mid(Plaza.X + FMath::Cos(Angle) * 2000.f, Plaza.Y + FMath::Sin(Angle) * 2000.f, 700.f);
		const FRotator Yaw(0.f, FMath::RadiansToDegrees(Angle), 0.f);

		SpawnPiece(Cube, Mid, FVector(1500.f, 340.f, 40.f), Stone, Yaw, true);
		SpawnPiece(Cube, Mid + FVector(0.f, 0.f, 90.f), FVector(1500.f, 20.f, 140.f), Gold, Yaw, false);
	}
}

// ---------------------------------------------------------------------------
// THE CRYSTAL RANGE. Blue peaks on the far flank: twelve clusters on the golden
// angle, each a cluster of cones at phi-scaled heights. This is the image's right
// side - the deep blue that is not stone.
// ---------------------------------------------------------------------------
void AVEILConceptWorld::BuildCrystals(UStaticMesh* Cone, UStaticMesh* Cylinder,
	UMaterialInterface* Water, UMaterialInterface* Glow)
{
	for (int32 i = 0; i < 12; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * ConceptGoldenDeg + 200.f);
		const float Radius = 5200.f * FMath::Pow(ConceptPhi, (float)(i % 4) * 0.25f);
		const FVector At(Plaza.X + FMath::Cos(Angle) * Radius, Plaza.Y + FMath::Sin(Angle) * Radius, 0.f);
		const float Height = 1200.f * FMath::Pow(ConceptPhi, (float)(i % 5) * 0.5f);

		SpawnPiece(Cone, At + FVector(0.f, 0.f, Height * 0.5f), FVector(900.f, 900.f, Height), Water, FRotator::ZeroRotator, true);
		SpawnPiece(Cone, At + FVector(320.f, -180.f, Height * 0.32f), FVector(520.f, 520.f, Height * 0.64f), Glow,
			FRotator(0.f, 22.f, 8.f), false);
	}
}

// ---------------------------------------------------------------------------
// THE DEEP. The image's lower right: a sunken structure under the district, blue
// and lit. Eight stepped decks descending on the golden angle, with glow columns.
// Toggling floors by data layer is a different job; this is the structure itself.
// ---------------------------------------------------------------------------
void AVEILConceptWorld::BuildDeep(UStaticMesh* Cube, UStaticMesh* Cylinder,
	UMaterialInterface* Water, UMaterialInterface* Glow)
{
	for (int32 i = 0; i < 8; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * ConceptGoldenDeg);
		const float Radius = 1300.f * FMath::Pow(ConceptPhi, (float)i * 0.35f);
		const FVector At(DeepAt.X + FMath::Cos(Angle) * Radius, DeepAt.Y + FMath::Sin(Angle) * Radius,
			-120.f - i * 140.f);

		SpawnPiece(Cube, At, FVector(1100.f, 1100.f, 60.f), Water, FRotator(0.f, i * 17.0324f, 0.f), true);
		SpawnPiece(Cylinder, At + FVector(0.f, 0.f, 420.f), FVector(160.f, 160.f, 840.f), Glow, FRotator::ZeroRotator, false);
	}
}

// ---------------------------------------------------------------------------
// THE WINGS. Two gold blades at the runner's back, beating on the golden pitch.
// The MetaHuman body cannot animate, so the wings are not an animation: they are
// two pieces of geometry moved every tick, which is the part that can be built.
// ---------------------------------------------------------------------------
void AVEILConceptWorld::BuildWings(UStaticMesh* Cube, UMaterialInterface* Gold)
{
	UWorld* W = GetWorld();
	if (!W) { return; }

	for (int32 Side = 0; Side < 2; ++Side)
	{
		AStaticMeshActor* WingActor = W->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
			FVector(0.f, 0.f, -10000.f), FRotator::ZeroRotator);
		if (!WingActor) { continue; }
		AActor* Wing = WingActor;
		Wing->Tags.Add(FName(TEXT("VEIL_CONCEPT")));

		UStaticMeshComponent* Comp = WingActor->GetStaticMeshComponent();
		Comp->SetStaticMesh(Cube);
		Comp->SetMobility(EComponentMobility::Movable);
		Comp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (Gold) { Comp->SetMaterial(0, Gold); }
		// A blade: long, thin, swept - the silhouette, not a feather.
		Wing->SetActorScale3D(FVector(1.35f, 0.06f, 0.34f));
		Wings.Add(Wing);
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI CONCEPT: wings bound to the runner (%d blades)."), Wings.Num());
}

void AVEILConceptWorld::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bBuilt || Wings.Num() == 0) { return; }

	const APawn* Runner = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Runner) { return; }

	WingPhase += DeltaSeconds * 2.6f;                       // the beat
	const float Flap = FMath::Sin(WingPhase) * 26.f;        // degrees of sweep
	const FVector Back = -Runner->GetActorForwardVector() * 22.f;
	const FVector Base = Runner->GetActorLocation() + FVector(0.f, 0.f, 55.f) + Back;
	const float Yaw = Runner->GetActorRotation().Yaw;

	for (int32 Side = 0; Side < Wings.Num(); ++Side)
	{
		AActor* Wing = Wings[Side].Get();
		if (!Wing) { continue; }
		const float Sign = (Side == 0) ? -1.f : 1.f;
		const FVector Offset = Runner->GetActorRightVector() * (95.f * Sign);
		Wing->SetActorLocation(Base + Offset);
		Wing->SetActorRotation(FRotator(0.f, Yaw + Sign * (90.f + Flap), Sign * 18.f));
	}
}
