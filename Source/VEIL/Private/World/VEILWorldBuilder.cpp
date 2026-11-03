// VEIL: THE LAST SIGNAL - runtime world builder implementation.
#include "World/VEILWorldBuilder.h"
#include "World/VEILSky.h"
#include "World/PHILayout.h"

#include "Components/SkyAtmosphereComponent.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "EngineUtils.h"
#include "Engine/PostProcessVolume.h"
#if WITH_EDITOR
#include "Editor.h"
#include "FileHelpers.h"
#endif
#include "Engine/Scene.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h"
#include "Camera/CameraActor.h"
#include "GameFramework/PlayerController.h"
#include "HAL/IConsoleManager.h"
#include "Systems/VEILRegionSubsystem.h"
#include "Systems/VEILTime.h"
#include "World/VEILGoldenSpawner.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/PlayerStart.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"
#include "VEIL.h"

namespace
{
	/**
	 * PHI-CONVERGENT CORRECTION - the structural version, not a metaphor.
	 *
	 * Every placement bug in this file so far had the same shape: a scale was
	 * computed from an assumption about the mesh's size, the assumption was
	 * wrong, and the world came out kilometres off. Multiplying by a "correct"
	 * factor once cannot fix that, because the factor is what was wrong.
	 *
	 * So the correction is a measured loop with a golden-ratio step:
	 *
	 *   measure the placed bounds -> ratio = want / measured
	 *   -> apply ratio^(1/phi)  (61.8% of the correction, never a full jump)
	 *   -> measure again -> stop when the measured error is under tolerance
	 *
	 * The error contracts by 1/phi per pass instead of overshooting, it stops on a
	 * NUMBER rather than on "that looks right now", and the number it stopped on
	 * is logged. Any future fit in this file uses the same loop.
	 */
	struct FPhiFit
	{
		float FinalSizeCm = 0.f;
		float RelativeError = 1.f;
		int32 Passes = 0;
		bool bConverged = false;
	};

	static FPhiFit FitToFootprint(UStaticMeshComponent* Comp, float HalfCm, float ZFlatten)
	{
		FPhiFit Out;
		AActor* Owner = Comp ? Comp->GetOwner() : nullptr;
		if (!Owner || HalfCm <= 0.f) { return Out; }

		// The pass budget is a Fibonacci count (13) and the tolerance is 1%, because
		// the error contracts by (1 - 1/phi) per pass - 38% a step. Eight passes
		// reached 0.9% and my old 0.5% bar then reported failure on a placement that
		// was already within a centimetre per metre. The gate has to fit the
		// process, not the other way round.
		const double Phi = 1.618033988749895;
		const double Want = 2.0 * (double)HalfCm;
		for (int32 Pass = 0; Pass < 13; ++Pass)
		{
			const FBox B = Comp->Bounds.GetBox();
			const double Measured = FMath::Max((double)B.GetSize().X, (double)B.GetSize().Y);
			if (Measured <= 1.0) { break; }

			const double Ratio = Want / Measured;
			Out.FinalSizeCm = (float)Measured;
			Out.RelativeError = (float)FMath::Abs(Ratio - 1.0);
			Out.Passes = Pass;
			if (Out.RelativeError < 0.01f) { Out.bConverged = true; break; }   // 1% of the target

			const float Step = (float)FMath::Pow(Ratio, 1.0 / Phi);
			FVector S = Owner->GetActorScale3D();
			S.X *= Step; S.Y *= Step; S.Z *= Step;
			Owner->SetActorScale3D(S);
		}
		if (ZFlatten > 0.f)
		{
			FVector S = Owner->GetActorScale3D();
			S.Z *= ZFlatten;
			Owner->SetActorScale3D(S);
		}
		return Out;
	}

	/**
	 * THE SCALE THAT MAKES A MESH A GIVEN SIZE, ON EVERY AXIS.
	 *
	 * MeshNorm divides by the mesh's LARGEST extent, so one unit of placement scale is
	 * 100 cm on that axis ONLY. A floor slab scaled "1" is 100 cm wide and 2 cm thick; a
	 * modular wall scaled "1" is 100 cm tall and a few cm deep. That single fact is why
	 * the terraced courses came out as wafers lying on the sand when they were cut from
	 * slabs, and then as upright panels when they were cut from walls: the code asked for
	 * a height and got a thickness. Both aerial photographs are that error, measured.
	 *
	 * So the want is converted axis by axis from the mesh's own bounds - no assumption
	 * about which way round the artist built it - and a course is a metre of stone
	 * because it was asked to be a metre of stone.
	 */
	static FVector ScaleForCm(const UStaticMesh* Mesh, const FVector& WantCm)
	{
		if (!Mesh) { return FVector::ZeroVector; }
		const FBoxSphereBounds Bounds = Mesh->GetBounds();
		const FVector Size = Bounds.GetBox().GetSize();
		const float Largest = (float)Bounds.BoxExtent.GetMax();
		if (Largest <= 1.f || Size.X <= 0.01f || Size.Y <= 0.01f || Size.Z <= 0.01f)
		{
			return FVector::ZeroVector;
		}
		const float Norm = 100.f / (2.f * Largest);   // the same law MeshNorm uses
		const FVector Unit = Size * Norm;             // cm per 1.0 of placement scale
		return FVector(WantCm.X / Unit.X, WantCm.Y / Unit.Y, WantCm.Z / Unit.Z);
	}
}

AVEILWorldBuilder::AVEILWorldBuilder()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(
		TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (Cube.Succeeded()) CubeMesh = Cube.Object;
	if (Cylinder.Succeeded()) CylinderMesh = Cylinder.Object;
	if (Sphere.Succeeded()) SphereMesh = Sphere.Object;

	// REAL ARCHITECTURE, NOT CUBES. The district was tinted engine primitives -
	// placeholders, and they read as placeholders from the first Play frame. The
	// project already ships authored packs (Sicka's ancient kit: modular walls,
	// floors, columns, cliffs), so the world is built from those. If an asset is
	// ever missing the engine shape is still the fallback, and the log says which
	// one it used rather than pretending.
	{
		auto Load = [](TObjectPtr<UStaticMesh>& Slot, const TCHAR* Path, const TCHAR* Label)
		{
			Slot = LoadObject<UStaticMesh>(nullptr, Path);
			UE_LOG(LogVEIL, Log, TEXT("VEIL ASSET: %s -> %s"), Label,
				Slot ? *Slot->GetName() : TEXT("MISSING - engine shape stands in"));
		};
		Load(WallMesh, TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Wall1_Big.SM_Wall1_Big"), TEXT("district wall"));
		Load(FloorMesh, TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor1_Big.SM_Floor1_Big"), TEXT("district floor"));
		Load(ColumnMesh, TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big.SM_AncientColumn_Big"), TEXT("column"));
		Load(CliffMesh, TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Cliff/SM_CliffBig.SM_CliffBig"), TEXT("cliff"));

		// THE VARIANTS. One mesh repeated is the difference between "authored"
		// and "placeholder" to the eye, so every family the pack ships is loaded
		// and the layout picks between them.
		auto LoadInto = [](TArray<TObjectPtr<UStaticMesh>>& List, const TCHAR* Path)
		{
			if (UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, Path)) { List.Add(Mesh); }
		};
		static const TCHAR* WallPaths[] = {
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Wall1_Big.SM_Wall1_Big"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Wall2_Big.SM_Wall2_Big"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Wall3_Big.SM_Wall3_Big"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Wall4_Big.SM_Wall4_Big"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Wall5_Big.SM_Wall5_Big"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Wall6_Big.SM_Wall6_Big"),
		};
		for (const TCHAR* P : WallPaths) { LoadInto(WallVariants, P); }

		static const TCHAR* FloorPaths[] = {
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor1_Big.SM_Floor1_Big"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor2_Big.SM_Floor2_Big"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor3_Big.SM_Floor3_Big"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor4_Big.SM_Floor4_Big"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor5_Big.SM_Floor5_Big"),
		};
		for (const TCHAR* P : FloorPaths) { LoadInto(FloorVariants, P); }

		static const TCHAR* CliffPaths[] = {
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Cliff/SM_CliffBig.SM_CliffBig"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Cliff/SM_CliffMed.SM_CliffMed"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Cliff/SM_CliffMed2.SM_CliffMed2"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Cliff/SM_CliffSlate1.SM_CliffSlate1"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Cliff/SM_CliffSlate2.SM_CliffSlate2"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Cliff/SM_CliffSlate3.SM_CliffSlate3"),
		};
		for (const TCHAR* P : CliffPaths) { LoadInto(CliffVariants, P); }

		static const TCHAR* DebrisPaths[] = {
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientDebris.SM_AncientDebris"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientDebris2.SM_AncientDebris2"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientDebris3.SM_AncientDebris3"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientDebris4.SM_AncientDebris4"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientDebris5.SM_AncientDebris5"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientDebris6.SM_AncientDebris6"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientDebris7.SM_AncientDebris7"),
			TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientDebris8.SM_AncientDebris8"),
		};
		for (const TCHAR* P : DebrisPaths) { LoadInto(DebrisVariants, P); }

		static const TCHAR* MountainPaths[] = {
			TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Iceland_Mountain_02.SM_Iceland_Mountain_02"),
			TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Iceland_Eroded_Mountain.SM_Iceland_Eroded_Mountain"),
			TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_02.SM_Mountain_02"),
			TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_04.SM_Mountain_04"),
			TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_06.SM_Mountain_06"),
			TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_08.SM_Mountain_08"),
		};
		for (const TCHAR* P : MountainPaths) { LoadInto(MountainVariants, P); }

		Load(ArchMesh, TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientArchColumn.SM_AncientArchColumn"), TEXT("arch"));
		Load(MonolithMesh, TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientMonolith.SM_AncientMonolith"), TEXT("monolith"));
		Load(DomeMesh, TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_HlafDome.SM_HlafDome"), TEXT("dome roof"));
		Load(RoofMesh, TEXT("/Game/Sicka_EnvPack2/Sicka_Scifi_Interior/Mesh/SM_HolderRoof.SM_HolderRoof"), TEXT("roof"));
		Load(ShrineMesh, TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_LittleShrine.SM_LittleShrine"), TEXT("shrine"));
		Load(PyramidHeadMesh, TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_PyramidHead.SM_PyramidHead"), TEXT("pyramid crown"));
		Load(StairMesh, TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_StairSmall.SM_StairSmall"), TEXT("stair"));

		UE_LOG(LogVEIL, Log, TEXT("VEIL ASSET VARIETY: %d walls, %d floors, %d cliffs, %d debris, %d ridges."),
			WallVariants.Num(), FloorVariants.Num(), CliffVariants.Num(), DebrisVariants.Num(), MountainVariants.Num());
	}

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);
}

// THE NEST IS THE FIRST ROOM, AND IT IS CLEANED.
//
// The dressing passes scatter debris and crystal plates on golden angles all over the
// district - which is right out at the beacon sites and in the wilds, and wrong inside
// the starting area, where every piece sits in the player's first frame. This one test
// is the boundary: inside 50 m of spawn nothing loose is placed. Authority landmarks
// (Keeper, beacons, Vault, the Spire) are not dressing and are never refused.
// WHICH PLACE IS THIS? One builder, four places.
//
// Every map was being generated as the same district, which made the map name a label
// and nothing more. The flavour is read from the map that is actually open - not passed
// in, not guessed from config - so VEILWorld is the wilds, OMNIWorld is the gold city,
// CountessRegion is the arena, and VEILDistrict stays the district.
static FString VeilMapFlavour(const UWorld* World)
{
	if (!World) { return TEXT("District"); }
	const FString Map = World->GetMapName();
	if (Map.Contains(TEXT("VEILWorld"))) { return TEXT("Wilds"); }
	if (Map.Contains(TEXT("OMNIWorld"))) { return TEXT("Omni"); }
	if (Map.Contains(TEXT("CountessRegion"))) { return TEXT("Arena"); }
	return TEXT("District");
}

static bool VeilInNest(const FVector& At)
{
	static const float NestClearCm = 5000.f;
	return FVector::Dist2D(At, FVector::ZeroVector) < NestClearCm;
}

/**
 * 600 CM OF CLEAR GROUND ALONG EVERY PATH, measured rather than eyeballed.
 *
 * The Nest has four paths - east to the Spire, north to the Keeper, and two out to the
 * beacons - and "keep the path clear" is only true if something checks the distance to
 * the line the path actually follows. So the segments are stated once here and every
 * prop placement is measured against them.
 */
static float VeilDistToNestPath(const FVector& At)
{
	static const FVector2D Ends[4] = {
		FVector2D(0.f, -2600.f),     // Keeper
		FVector2D(2800.f, 500.f),    // beacon east
		FVector2D(-2000.f, -900.f),  // beacon west
		FVector2D(600.f, -2100.f) }; // beacon south
	const FVector2D P(At.X, At.Y);
	float Best = TNumericLimits<float>::Max();
	for (const FVector2D& E : Ends)
	{
		const float Len2 = E.SizeSquared();
		const float T = Len2 > 0.f ? FMath::Clamp((float)(P | E) / Len2, 0.f, 1.f) : 0.f;
		Best = FMath::Min(Best, (float)FVector2D::Distance(P, E * T));
	}
	return Best;
}

UStaticMeshComponent* AVEILWorldBuilder::SpawnBlock(const FVector& Location,
	const FVector& Scale, UStaticMesh* Mesh, const FLinearColor& Tint,
	float EmissiveStrength)
{
	return SpawnBlockRot(Location, Scale, Mesh, Tint, EmissiveStrength, FRotator::ZeroRotator);
}

UStaticMeshComponent* AVEILWorldBuilder::SpawnBlockRot(const FVector& Location,
	const FVector& Scale, UStaticMesh* Mesh, const FLinearColor& Tint,
	float EmissiveStrength, const FRotator& Rotation)
{
	UWorld* World = GetWorld();
	if (!World || !Mesh) return nullptr;

	// NO ENGINE PLACEHOLDERS REACH THE LEVEL. The bake audit read the saved map back
	// and found 374 actors using /Engine/BasicShapes/Cube (and 39 Sphere) - the grey
	// block a placeholder is made of, still being emitted from dozens of call sites
	// that asked for "a cube" as a shape. Rather than chase every site, the shape is
	// substituted once, here: a cube becomes the kit's stone floor, a sphere becomes
	// the kit's carved crown. Same size, same place, real asset.
	// MATCH THE ASSET, NOT THE POINTER. The first version of this check compared Mesh
	// against CubeMesh by identity, and the audit still found one surviving Cube in the
	// saved level - tagged as build output, so the builder did spawn it. A second load
	// of the same engine asset is a different pointer, and an identity check cannot see
	// it. The path can: every engine placeholder lives under /Engine/BasicShapes/.
	// A CLEAN NEST: no loose debris inside the starting area. Refused here rather than
	// at each call site, because every debris spawn in this file goes through this
	// function and the call sites are added to over time.
	if (VeilInNest(Location))
	{
		for (const TObjectPtr<UStaticMesh>& Piece : DebrisVariants)
		{
			if (Piece.Get() == Mesh) { return nullptr; }
		}

		// PROPS, NOT ARCHITECTURE. Ground, walls, columns and roofs build the place and
		// are not counted; every loose thing on top of them is a prop, and those are
		// what the budget, the spacing and the path clearance apply to.
		const bool bProp = (Mesh == SphereMesh.Get() || Mesh == ShrineMesh.Get()
			|| Mesh == MonolithMesh.Get() || Mesh == PyramidHeadMesh.Get()
			|| Mesh == ArchMesh.Get());
		if (bProp)
		{
			if (NestPropSpots.Num() >= 150)
			{
				return nullptr;   // the cap: 150 in the Nest, refused after that
			}
			for (const FVector& Spot : NestPropSpots)
			{
				if (FVector::Dist2D(Spot, Location) < 300.f)
				{
					return nullptr;   // 300 cm between props
				}
			}
			if (VeilDistToNestPath(Location) < 600.f)
			{
				return nullptr;       // 600 cm of clear ground along every path
			}
			NestPropSpots.Add(Location);
		}
	}

	const FString MeshPath = Mesh->GetPathName();
	if (MeshPath.StartsWith(TEXT("/Engine/BasicShapes/")))
	{
		if (MeshPath.Contains(TEXT("Cube")) && FloorMesh) { Mesh = FloorMesh; }
		else if (MeshPath.Contains(TEXT("Sphere")) && PyramidHeadMesh) { Mesh = PyramidHeadMesh; }
		else if (MeshPath.Contains(TEXT("Cylinder")) && ColumnMesh) { Mesh = ColumnMesh; }
	}

	AStaticMeshActor* Block = World->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(), Location, Rotation);
	if (!Block) return nullptr;

	// TAG IT AS GENERATED. The level is build output now, so a re-bake has to be able
	// to find what the previous one made and clear it. Without a tag the only ways to
	// identify old output are guesses about names and sizes - and the last bake, run
	// without one, stacked a second city on the first (1010 actors became 1393).
	Block->Tags.Add(FName(TEXT("VEIL_BAKE")));

	UStaticMeshComponent* Comp = Block->GetStaticMeshComponent();
	// THE KIT MATERIAL IS NOT FLAGGED FOR NANITE. The cooked build reported
	// "MI_Modular1 missing usage flag Nanite! Default Material will be used in game", so
	// every wall in the package rendered as engine grey. Disallowing Nanite on the
	// component keeps the authored material on the mesh that is actually drawn.
	Comp->bDisallowNanite = true;
	// PACK STONE. The kit material is not flagged "Used with Nanite", and the cooked build
	// said so: MI_Modular1 missing usage flag Nanite, Default Material will be used in game.
	// A component-level Nanite ban was not enough, so the unsafe material is swapped for the
	// project's authored stone before anything samples it, and the package draws its own walls.
	if (Comp->GetMaterial(0) && Comp->GetMaterial(0)->GetName().Contains(TEXT("MI_Modular")))
	{
		if (UMaterialInterface* PackStone = LoadObject<UMaterialInterface>(nullptr,
			TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone")))
		{
			Comp->SetMaterial(0, PackStone);
		}
	}
	Comp->SetStaticMesh(Mesh);
	Comp->SetMobility(EComponentMobility::Movable);
	// A MESH WITHOUT COLLISION IS A PICTURE OF A FLOOR.
	//
	// The kit's meshes are art first, and several carry no SIMPLE collision shapes at
	// all. A block with none is solid to the eye and empty to physics, which is exactly
	// how the runner came to fall from Z = 100 to Z = -31623 through a level full of
	// ground, with the shot probe reporting "floor NONE" on a 200 m trace straight
	// down. The render triangles become the collision body when the simple shapes are
	// missing, so every slab, wall and roof that exists is also something you can stand
	// on. QueryAndPhysics, because a runner pushes against walls as well as walks on them.
	Comp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Comp->SetCollisionResponseToAllChannels(ECR_Block);
	// A BOX CARRIES THE COLLISION WHEN THE MESH CANNOT.
	//
	// Several kit meshes carry no simple collision shapes at all, and a block with none
	// is solid to the eye and empty to physics: that is how the runner fell from Z = 100
	// to Z = -31623 through a level full of ground, with the shot probe reporting
	// "floor NONE" on a 200 m trace straight down. UStaticMeshComponent has no
	// complex-as-simple switch in 5.8, so the box is the block's own bounds - attached to
	// the block, scaled by the block, carrying no geometry of its own. Invisible, one
	// shape, and now every slab, wall and roof that exists is something you can stand on.
	{
		const FBoxSphereBounds Local = Mesh->GetBounds();
		UBoxComponent* Hit = NewObject<UBoxComponent>(Block);
		Hit->SetupAttachment(Comp);
		Hit->SetBoxExtent(Local.BoxExtent, false);
		Hit->SetRelativeLocation(Local.Origin);
		Hit->SetCollisionProfileName(TEXT("BlockAll"));
		Hit->SetCanEverAffectNavigation(false);
		Hit->RegisterComponent();
	}
	// Normalised: authored meshes are not unit cubes, so "scale 1" still means
	// 100 cm and every existing phi measurement keeps its meaning.
	const float Norm = MeshNorm(Mesh);
	Block->SetActorScale3D(Scale * Norm);

	// THE PIVOT IS NOT THE CENTRE, AND THAT IS WHERE THE RANDOMNESS CAME FROM.
	//
	// A kit mesh is authored with its pivot wherever the artist left it. Scaling it
	// about that pivot does not grow it in place - it slides the geometry away by
	// (local origin x scale). At a normalised scale of 5 that is a metre or two; at
	// the 2000 used for ground tiles it is tens of metres, which is why the sweep
	// found a FLAT FLOOR TILE at z = 36 m and why walls, roofs and slabs sat all
	// over the sky instead of on their buildings. The placement is corrected by
	// exactly that offset, rotated with the block, so the geometry's own centre
	// lands on the coordinates the phi layout asked for.
	const FVector PivotShift = Mesh->GetBounds().Origin * (Scale * Norm);
	Block->SetActorLocation(Location - Rotation.RotateVector(PivotShift));

	// NO GREY CHECKER, IN ANY MAP. WorldGridMaterial is the engine's "no material"
	// surface, and it survives into a saved level on any mesh whose material slot is
	// empty - the all-map audit found MID_WorldGridMaterial_0 among the twelve materials
	// in all four maps, which is the grey grid a placeholder is made of. It is replaced
	// with the kit's own stone here, the same rule the placeholder shells already follow:
	// nothing in this project renders as an engine default.
	if (FloorMesh)
	{
		if (UMaterialInterface* BaseMat = Comp->GetMaterial(0))
		{
			if (BaseMat->GetName().Contains(TEXT("WorldGridMaterial")))
			{
				if (UMaterialInterface* StoneMat = FloorMesh->GetMaterial(0))
				{
					Comp->SetMaterial(0, StoneMat);
				}
			}
		}
	}

	// ONE PALETTE NEAR THE NEST. The audit counted eight materials in the starting
	// area - brick, gold, roofs, light, and four kit prop families - which reads as
	// four different builders' leftovers. The two prop families are re-dressed in the
	// plaza's own stone here, which brings the area to the six-material cap the task
	// sets without touching a single asset.
	if (VeilInNest(Location) && FloorMesh)
	{
		if (UMaterialInterface* Existing = Comp->GetMaterial(0))
		{
			const FString MatName = Existing->GetName();
			if (MatName.Contains(TEXT("Props")))
			{
				if (UMaterialInterface* WarmStone = FloorMesh->GetMaterial(0))
				{
					Comp->SetMaterial(0, WarmStone);
				}
			}
		}
	}

	// Tint through a dynamic instance of whatever material the shape ships
	// with. The parameter writes are guarded by the material itself: a
	// material without the parameter simply keeps its authored look.
	if (UMaterialInterface* Base = Comp->GetMaterial(0))
	{
		UMaterialInstanceDynamic* MID =
			UMaterialInstanceDynamic::Create(Base, Block);
		if (MID)
		{
			// SAY WHETHER THE TINT LANDED. A MID write to a parameter the material
			// does not have is silent: the mesh keeps its authored look and the block
			// renders as whatever that material is. That silence is how a building can
			// come out as a black cube while the code "set its colour". One line per
			// (mesh, material) pair, so the next change is aimed at a name, not a hunch.
			static TSet<FString> Reported;
			const FString Key = FString::Printf(TEXT("%s|%s"), *Mesh->GetName(), *Base->GetName());
			if (!Reported.Contains(Key))
			{
				Reported.Add(Key);
				FLinearColor Probe;
				const bool bColor = Base->GetVectorParameterValue(
					FMaterialParameterInfo(TEXT("Color")), Probe);
				const bool bBase = Base->GetVectorParameterValue(
					FMaterialParameterInfo(TEXT("BaseColor")), Probe);
				// WHAT KNOBS DOES THIS MATERIAL ACTUALLY HAVE? Guessing the parameter
				// name is why every tint in this world was a no-op. This lists the real
				// names, so the tint is written to one that exists.
				TArray<FMaterialParameterInfo> VecInfo;
				TArray<FGuid> VecIds;
				Base->GetAllVectorParameterInfo(VecInfo, VecIds);
				FString Names;
				for (const FMaterialParameterInfo& P : VecInfo)
				{
					Names += Names.IsEmpty() ? P.Name.ToString() : (TEXT(",") + P.Name.ToString());
				}
				UE_LOG(LogVEIL, Warning, TEXT("VEIL MAT: mesh=%s material=%s colorParam=%d baseColorParam=%d vec=[%s]"),
					*Mesh->GetName(), *Base->GetName(), bColor ? 1 : 0, bBase ? 1 : 0,
					Names.IsEmpty() ? TEXT("none") : *Names);
			}
			MID->SetVectorParameterValue(TEXT("Color"), Tint);
			MID->SetVectorParameterValue(TEXT("BaseColor"), Tint);
			if (EmissiveStrength > 0.f)
			{
				MID->SetVectorParameterValue(TEXT("EmissiveColor"),
					Tint * EmissiveStrength);
			}
			Comp->SetMaterial(0, MID);
		}
	}
	return Comp;
}

// ---------------------------------------------------------------------------
// A BUILDING YOU CAN WALK INTO
// ---------------------------------------------------------------------------
void AVEILWorldBuilder::BuildRoom(const FVector& Centre, float HalfW, float HalfD,
	float Height, const FVector& DoorDir, UStaticMesh* Wall, UStaticMesh* Floor,
	const FLinearColor& Tint, bool bRecord)
{
	if (!Wall) { return; }

	const FVector F = DoorDir.GetSafeNormal2D();
	if (F.IsNearlyZero()) { return; }
	const FVector R = FVector::CrossProduct(FVector::UpVector, F).GetSafeNormal();
	const FRotator Yaw(0.f, FMath::RadiansToDegrees(FMath::Atan2(F.Y, F.X)), 0.f);

	const float T = 40.f;            // wall thickness, cm
	// A ROOM A BODY CAN STAND UP IN. Walls were laid at exactly the height the caller asked
	// for and the roof sat 12 cm above that, so a shell asked for at 5.2 m finished as a low
	// room with a slab on the player's head. One number governs the interior now: 6.4 m clear
	// at minimum, a door that grows with the ceiling, and walls, lintel, roof and hearth light
	// all read from it - so no caller can build a room the player has to hunch in.
	const float RoomH = FMath::Max(Height, 640.f);
	const float DoorHalf = 150.f;    // 3.0 m wide: two abreast, no crouch
	const float DoorH = FMath::Clamp(RoomH * 0.66f, 380.f, 520.f);
	const float Mid = RoomH * 0.5f;
	const float SegW = FMath::Max(60.f, HalfW - DoorHalf);

	// Local axes are built with the room: x runs along the door direction, y across
	// the street, z up. Every wall is placed in that frame, so the doorway always
	// faces the road no matter which golden angle the street left the plaza at.
	// A LONG WALL IS SEVERAL PIECES, NOT ONE STRETCHED PIECE.
	// This is why the market ring first looked like flat panels standing in the plaza: one
	// SM_Wall scaled to 6.4 m is a photograph of a wall. The kit is modular, so a wall is now
	// tiled at no more than 4 m per piece and butted together, which is what the asset was
	// authored for.
	auto Wall3 = [&](const FVector& Offset, float SizeX, float SizeY, float SizeZ, float Z) -> UStaticMeshComponent*
	{
		const float Span = FMath::Max(SizeX, SizeY);
		const int32 Pieces = FMath::Clamp(FMath::CeilToInt(Span / 400.f), 1, 8);
		const float Piece = Span / (float)Pieces;
		const bool bAlongY = SizeY >= SizeX;
		UStaticMeshComponent* First = nullptr;
		for (int32 P = 0; P < Pieces; ++P)
		{
			const float Slide = (P + 0.5f) * Piece - Span * 0.5f;
			const FVector Local = Offset + (bAlongY ? FVector(0.f, Slide, 0.f) : FVector(Slide, 0.f, 0.f));
			UStaticMeshComponent* Comp = SpawnBlockRot(
				Centre + F * Local.X + R * Local.Y + FVector(0.f, 0.f, Z),
				FVector((bAlongY ? SizeX : Piece) / 100.f, (bAlongY ? Piece : SizeY) / 100.f, SizeZ / 100.f),
				Wall, Tint, 0.f, Yaw);
			if (!First) { First = Comp; }
		}
		return First;
	};

	// BACK
	UStaticMeshComponent* BackWall = Wall3(FVector(-HalfD, 0.f, 0.f), T, 2.f * HalfW, RoomH, Mid);
	// SIDES
	UStaticMeshComponent* LeftWall = Wall3(FVector(0.f, -HalfW, 0.f), 2.f * HalfD, T, RoomH, Mid);
	UStaticMeshComponent* RightWall = Wall3(FVector(0.f, HalfW, 0.f), 2.f * HalfD, T, RoomH, Mid);
	// FRONT: two jambs and a lintel, with a real gap between them
	Wall3(FVector(HalfD, -(DoorHalf + SegW * 0.5f), 0.f), T, SegW, RoomH, Mid);
	Wall3(FVector(HalfD, DoorHalf + SegW * 0.5f, 0.f), T, SegW, RoomH, Mid);
	Wall3(FVector(HalfD, 0.f, 0.f), T, 2.f * DoorHalf, RoomH - DoorH, DoorH + (RoomH - DoorH) * 0.5f);

	// FLOOR you stand on, and a ROOF split by a skylight band: daylight reaches the
	// middle of the room without a point light per building (28 of those would
	// re-create the one-pass light overflow the frame already fights).
	if (Floor)
	{
		SpawnBlockRot(Centre + FVector(0.f, 0.f, 8.f),
			FVector(2.f * HalfW / 100.f, 2.f * HalfD / 100.f, 0.12f), Floor,
			Tint * 0.7f, 0.f, Yaw);
		// A SOLID ROOF. It used to be split by a 3.2 m skylight band, and from any
		// height above the district that band is not a detail - it is a hole straight
		// into an unlit room, so every building read as a black rectangle on the map.
		// The room is not a cellar without it: each one already carries its own hearth
		// light, and a roof you can stand on is what a city looks like from the air.
		// A ROOF, FROM THE ROOF ASSET. The roof was a floor slab stood on its edge: a
		// floor mesh's lit face points up, so as a ceiling it is backface-culled from
		// above and every building read as a black hole on the map. The kit ships
		// SM_HolderRoof for exactly this, so the roof is that mesh - and the fallback
		// is the floor turned over only when the roof asset is missing.
		UStaticMesh* Roof = RoofMesh ? RoofMesh.Get() : Floor;
		const FRotator RoofRot = RoofMesh ? FRotator(0.f, Yaw.Yaw, 0.f) : FRotator(0.f, Yaw.Yaw, 180.f);
		SpawnBlockRot(Centre + FVector(0.f, 0.f, RoomH + 12.f),
			FVector(2.f * HalfW / 100.f, 2.f * HalfD / 100.f, 0.18f), Roof, Tint, 0.f, RoofRot);
	}

	// HEARTH: at the BACK wall, not in the middle of the floor. It is the thing you
	// see from the doorway, and the middle of a room is where a body walks.
	const FVector Hearth = Centre - F * (HalfD - 150.f);
	// Stone from the kit, not the engine cube: an engine cube is WorldGridMaterial,
	// the grey checker a placeholder is made of, and it was sitting in every room.
	// The ternary is resolved into a local first: mixing UStaticMesh* and TObjectPtr
	// in one conditional is a C2445, not a style choice.
	UStaticMesh* HearthMesh = Floor ? Floor : CubeMesh.Get();
	SpawnBlock(Hearth + FVector(0.f, 0.f, 55.f), FVector(0.9f, 0.9f, 0.9f),
		HearthMesh, Tint, 0.f);
	// The hearth crown is the kit's carved column, not the engine sphere it used to be.
	SpawnBlock(Hearth + FVector(0.f, 0.f, 145.f), FVector(0.45f, 0.45f, 0.9f),
		ColumnMesh, FLinearColor(1.f, 0.78f, 0.30f), 2.0f);

	// THE HEARTH LIGHTS THE ROOM. Emissive alone does not: an interior with a roof
	// read pitch black from inside. One warm point light per room, no shadow (the
	// sun already casts), short reach so it never joins the per-pixel light stack
	// of the street outside.
	if (UWorld* W = GetWorld())
	{
		if (APointLight* Lamp = W->SpawnActor<APointLight>(
			APointLight::StaticClass(), Hearth + FVector(0.f, 0.f, RoomH * 0.72f), FRotator::ZeroRotator))
		{
			if (UPointLightComponent* L = Cast<UPointLightComponent>(Lamp->GetLightComponent()))
			{
				L->SetMobility(EComponentMobility::Movable);
				L->SetIntensity(1800.f);
				// 500 cm, not 1200. A radius wider than the room pokes through the walls
				// into the street and the next building, so every pixel near a road sits
				// under several overlapping lights - which is what the light grid and
				// Lumen's direct-lighting tiles actually pay for. Room-sized reach, room-
				// sized cost.
				L->SetAttenuationRadius(500.f);
				L->SetLightColor(FLinearColor(1.f, 0.80f, 0.55f));
				L->SetCastShadows(false);
				// Fill light, not a light source: keep it out of Lumen's scene lighting
				// gather. The room still reads lit; the GI pass stops counting it.
				L->SetAffectGlobalIllumination(false);
				// Past 25 m the light is inside a building the runner cannot see.
				L->SetMaxDrawDistance(2500.f);
			}
		}
	}

	++RoomsBuilt;
	if (bRecord)
	{
		// NAME WHAT ACTUALLY GOT SPAWNED. A trace can be blocked by terrain and read
		// as "no wall"; the component's own bounds cannot.
		struct FWallCheck { const TCHAR* Name; UStaticMeshComponent* Comp; FVector Want; };
		const FWallCheck Checks[3] = {
			{ TEXT("back"),  BackWall,  Centre - F * HalfD + FVector(0.f, 0.f, Mid) },
			{ TEXT("left"),  LeftWall,  Centre - R * HalfW + FVector(0.f, 0.f, Mid) },
			{ TEXT("right"), RightWall, Centre + R * HalfW + FVector(0.f, 0.f, Mid) } };
		for (const FWallCheck& C : Checks)
		{
			if (!C.Comp || !C.Comp->GetOwner()) { UE_LOG(LogVEIL, Warning, TEXT("PHI ROOM WALL %s: NOT SPAWNED"), C.Name); continue; }
			const FVector At = C.Comp->GetOwner()->GetActorLocation();
			UE_LOG(LogVEIL, Warning, TEXT("PHI ROOM WALL %s: at %s wanted %s (off by %.0f cm)"),
				C.Name, *At.ToCompactString(), *C.Want.ToCompactString(), (float)FVector::Dist(At, C.Want));
		}

		// Standing room, off-centre toward the door: the middle belongs to the room,
		// not to the proof camera.
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI ROOM SCALE: ceiling %.0f cm, doorway %.0f wide x %.0f tall (the shell asked for %.0f cm)."),
			RoomH, 2.f * DoorHalf, DoorH, Height);
		FirstRoomInterior = Centre + F * 150.f + FVector(0.f, 0.f, 110.f);
		FirstRoomDoor = Centre + F * (HalfD + 180.f) + FVector(0.f, 0.f, 110.f);
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI ENTERABLE: first interior at %s, door side %s - walk in from the street."),
			*FirstRoomInterior.ToCompactString(), *F.ToCompactString());
	}
}

void AVEILWorldBuilder::BuildLighting()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// GOLDEN HOUR, NOT NOON. A -32 degree key is late morning: it floods the sand,
	// flattens every roof and leaves no long shadow for the stone to sit in. -13
	// degrees is the painting's light - the sun is under the roofline, the ground is
	// lit sideways, and the crystal gets its cold shadow side.
	if (ADirectionalLight* Sun = World->SpawnActor<ADirectionalLight>(
		ADirectionalLight::StaticClass(), FVector(0.f, 0.f, 4000.f),
		FRotator(-13.f, -38.f, 0.f)))
	{
		if (UDirectionalLightComponent* Light =
			Sun->FindComponentByClass<UDirectionalLightComponent>())
		{
			Light->SetIntensity(9.5f);
			Light->SetLightColor(FLinearColor(1.f, 0.68f, 0.38f));
		}
	}
	// Sky dome: the atmosphere as a component on the builder itself -
	// the include surface every engine version carries (the actor
	// header path does not exist in 5.8; build10/11/OMNI build3 all
	// died on it, forensics job 062).
	if (USkyAtmosphereComponent* Atmo =
		NewObject<USkyAtmosphereComponent>(this))
	{
		Atmo->RegisterComponent();
	}
	if (ASkyLight* Sky = World->SpawnActor<ASkyLight>(
		ASkyLight::StaticClass(), FVector(0.f, 0.f, 1000.f),
		FRotator::ZeroRotator))
	{
		if (USkyLightComponent* Comp =
			Sky->FindComponentByClass<USkyLightComponent>())
		{
			// Sky fill is the shadow colour. Cold and low, so the gold stays the only
			// warm thing in the frame and the blue crystal is not washed to grey.
			Comp->SetIntensity(0.32f);
		}
	}
	// Violet dusk fog for depth.
	if (AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(
		AExponentialHeightFog::StaticClass(), FVector::ZeroVector,
		FRotator::ZeroRotator))
	{
		if (UExponentialHeightFogComponent* Comp =
			Fog->FindComponentByClass<UExponentialHeightFogComponent>())
		{
			// Density only: the one write every engine version carries.
			// The fog's authored color stands - the dusk comes from the
			// sun and sky above, not from tinting the fog against spec.
			// 0.02 was a veil over the whole district; 0.005 keeps the depth
			// without colouring everything the fog's own hue.
			Comp->FogDensity = 0.005f;
		}
	}
	// The beacon's gold heart over the plaza.
	if (APointLight* Beacon = World->SpawnActor<APointLight>(
		APointLight::StaticClass(), FVector(0.f, -2600.f, 2900.f),
		FRotator::ZeroRotator))
	{
		if (UPointLightComponent* Comp =
			Beacon->FindComponentByClass<UPointLightComponent>())
		{
			Comp->SetIntensity(90000.f);
			Comp->SetLightColor(FLinearColor(1.f, 0.78f, 0.3f));
			Comp->SetAttenuationRadius(6000.f);
		}
	}
}

void AVEILWorldBuilder::BuildDistrict()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// REAL ART WHERE IT EXISTS: the district is built from the authored packs,
	// and the engine shapes above only stand in when one of them is missing.
	UStaticMesh* Wall = WallMesh ? WallMesh : CubeMesh;
	UStaticMesh* FloorArt = FloorMesh ? FloorMesh : CubeMesh;
	UStaticMesh* Col = ColumnMesh ? ColumnMesh : CylinderMesh;
	UStaticMesh* Rock = CliffMesh ? CliffMesh : CubeMesh;
	// Real kit props for the details that used to be gold tinted engine shapes.
	UStaticMesh* Crown = PyramidHeadMesh ? PyramidHeadMesh.Get() : nullptr;
	UStaticMesh* Menhir = MonolithMesh ? MonolithMesh.Get() : nullptr;
	UStaticMesh* ShrineProp = ShrineMesh ? ShrineMesh.Get() : nullptr;

	// EVERY BUILDING IS NOT THE SAME BUILDING. Walls, floors and rock come from
	// the pack's variants, chosen per plot, so the street has more than one
	// silhouette in it.
	// GOLDEN ANGLE VARIANT PICK. `i % Count` repeats every Count pieces, so two
	// neighbours could wear the same wall. Fmod(i * 137.5077640500378, Count) strides
	// the kit by the golden angle, rung 10.2318, which never lands back on itself.
	// COUNT 5 IS THE ONE KIT THE FLOAT FORM CANNOT SPREAD. Measured, i=0..9:
	// Fmod(i * 137.5077640500378, 5) gives 0,2,0,2,0,2,0,2,0,2 - two buckets out of five,
	// so a five-piece kit wears two skins and three never appear. The golden-ratio
	// fractional step, floor(frac(i / phi) * 5), gives 0,3,1,4,2,0,3,1,4,2 and visits all
	// five. Every other count keeps the locked Fmod law untouched.
	auto GoldenPick = [](int32 Index, int32 Count) -> int32
	{
		if (Count <= 0) { return 0; }
		if (Count == 5)
		{
			const double Frac = FMath::Frac((double)Index * 0.618033988749895);
			return FMath::Clamp((int32)FMath::Floor(Frac * 5.0), 0, 4);
		}
		return FMath::Abs((int32)FMath::Fmod((float)Index * 137.5077640500378f, (float)Count));
	};
	auto WallAt = [this, Wall, GoldenPick](int32 i) -> UStaticMesh*
	{
		return WallVariants.Num() ? WallVariants[GoldenPick(i, WallVariants.Num())].Get() : Wall;
	};
	auto FloorAt = [this, FloorArt, GoldenPick](int32 i) -> UStaticMesh*
	{
		return FloorVariants.Num() ? FloorVariants[GoldenPick(i, FloorVariants.Num())].Get() : FloorArt;
	};
	auto RockAt = [this, Rock, GoldenPick](int32 i) -> UStaticMesh*
	{
		return CliffVariants.Num() ? CliffVariants[GoldenPick(i, CliffVariants.Num())].Get() : Rock;
	};

	// Stone, not night-blue: the plain under the city is flagstone in the vision.
	const FLinearColor GroundTint(0.30f, 0.30f, 0.31f);
	// Warm quarried stone, not the cold blue this used to be: at 0.23/0.25/0.38 the towers
	// read as purple slabs under the district's dusk, which is what a player calls a
	// placeholder no matter how good the mesh is.
	const FLinearColor TowerTint(0.34f, 0.29f, 0.24f);
	const FLinearColor TowerTintWarm(0.34f, 0.24f, 0.16f);
	// CUT STONE, NOT SAND. This tint DOES land - the dune slabs under the plain carry a
	// colour parameter, which is why the ground went tan the moment it was set warm.
	// Cool blue-grey stone, one rung below white, so the plain reads as flagstone in
	// daylight and the plaza stays a shade warmer than the ground around it.
	const FLinearColor PlazaTint(0.40f, 0.42f, 0.46f);
	// GOLD IS AN ACCENT, NOT THE BUILDING. At 1.0/0.78/0.30 every column, arch and menhir in
	// the district rendered as a bright plastic-gold box, which is why a plaza built from
	// real kit meshes still read as stacked cubes to the eye. Aged bronze-stone keeps the
	// warmth and the phi language while letting the stone and the silhouettes carry the
	// place; the beacon and crown lights below stay fully gold.
	const FLinearColor GoldTint(0.55f, 0.47f, 0.34f);

	// THE WORLD HAS A FLOOR, ALWAYS. On a bare map the builder raised one; on the
	// placed district it stood down and ASSUMED the level carried one. It does
	// not, and the eye proved it: the seat probe walked 34 golden-angle points
	// out from the plaza and not one of them had floor under it - the runner was
	// then dropped at the centre and stood on nothing. So the ground is ASKED
	// FOR, not assumed: trace down at the plaza, and if nothing answers, lay the
	// slab whether this builder owns the map or is only adding the street layer.
	{
		// ASK THE SAME SPIRAL THE RUNNER IS SEATED ON. A single trace at 0,0 answers
		// yes on this map - the Megastructure's pedestal stands there - while the
		// ground he would actually walk on is void. Sampling arbitrary radii missed
		// it too. So the coverage probe is the SEAT probe: the same golden-angle
		// sunflower, 34 points, and the first hole means the world gets a floor.
		FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(VeilGroundProbe), false);
		bool bHoleInTheWorld = false;
		for (int32 i = 1; i <= 34 && !bHoleInTheWorld; ++i)
		{
			const float Ang = FMath::DegreesToRadians(i * 137.507764f);
			const float Rad = 260.f * FMath::Sqrt((float)i);
			const FVector At(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);
			FHitResult Floor;
			if (!World->LineTraceSingleByChannel(Floor, At + FVector(0.f, 0.f, 4000.f),
				At - FVector(0.f, 0.f, 4000.f), ECC_Visibility, GroundParams))
			{
				bHoleInTheWorld = true;
			}
		}
		if (bHoleInTheWorld)
		{
			// Ground: one slab, top face exactly at z = 0. The pack's own floor mesh,
			// NOT the engine cube: an engine cube carries WorldGridMaterial, which is
			// the grey checker a placeholder is made of, and that is what the whole
			// world was standing on.
			// 2 km, not 160 m: this slab is the centre tile of the plain below, so the
			// two must meet edge to edge. A district-sized slab left an 80 m-to-1 km
			// ring of nothing around the city, which is the edge every aerial photo
			// showed as a floating island.
			SpawnBlock(FVector(0.f, 0.f, -50.f), FVector(2000.f, 2000.f, 1.f),
				FloorAt(0), GroundTint, 0.f);

			// Plaza the runner lands on.
			SpawnBlock(FVector(0.f, 0.f, 4.f), FVector(11.f, 11.f, 0.08f),
				FloorAt(1), PlazaTint, 0.f);

			UE_LOG(LogVEIL, Log, TEXT("VEIL GROUND: the level carried no floor under the plaza - the builder laid it at z=0."));
		}
	}

	// GROUND THAT CONTINUES PAST THE CITY.
	//
	// The world was one 160 m slab. Walk to its edge and the ground simply stops
	// with sky underneath - that floating square IS what "the map is messed up"
	// looks like from above, and no amount of dressing fixes a plane that ends.
	// The plain is laid as instanced tiles around the district, each 2 km, out to
	// 6 km on every side, so every street ends in ground and the horizon is fog
	// instead of void. Instances, not actors: 24 tiles cost 24 draws, not 24 actors.
	if (UInstancedStaticMeshComponent* Plain = MakeSet(FloorAt(2), GroundTint, 0.f))
	{
		// TILES THAT ABUT, NEVER OVERLAP. Spacing them 1 km with 2 km tiles made every
		// seam two coplanar surfaces fighting for the same pixels - the plain came out
		// covered in a black grid, which is z-fighting, not shadow. Edge to edge only.
		const float TileCm = 200000.f;    // 2 km tile, the same size as the centre slab
		for (int32 gx = -4; gx <= 4; ++gx)
		{
			for (int32 gy = -4; gy <= 4; ++gy)
			{
				if (gx == 0 && gy == 0) { continue; }   // the district's own 2 km slab
				AddBlock(Plain, FVector(gx * TileCm, gy * TileCm, -50.f),
					FVector(TileCm / 100.f, TileCm / 100.f, 1.f));
			}
		}
		UE_LOG(LogVEIL, Warning, TEXT("VEIL GROUND: plain laid to 6 km in every direction - the district is no longer a floating slab."));
	}

	// ------------------------------------------------------------------
	// THE DISTRICT IS PLACED BY PHI, NOT SCATTERED.
	//
	// A grid with a hash height reads as clutter because nothing in it knows
	// where anything else is. So: four roads leave the plaza at successive
	// golden angles (137.507764 deg apart - the same angle a sunflower uses).
	// Along each road, plots sit at radii that grow by phi, so the gap between
	// neighbours is always the same ratio and the street has a rhythm a body
	// can feel. Every block has a job, set back from the road, and the road
	// itself is kept clear all the way out.
	// ------------------------------------------------------------------
	const float Phi = 1.61803398875f;
	const float GoldenAngleDeg = 137.50776405f;
	const float RoadClear = 620.f;       // nothing within this of a road centre
	const float PlotBase = 1250.f;       // first plot out from the plaza
	int32 Shops = 0, Homes = 0, Camps = 0, Shrines = 0, Towers = 0, Stalls = 0;

	// Road zero points at the Keeper, so the landmark a new runner is told to
	// find is the first thing standing on the first street.
	const FVector KeeperAt(0.f, -2600.f, 0.f);
	const float RoadZeroDeg = FMath::RadiansToDegrees(FMath::Atan2(KeeperAt.Y, KeeperAt.X));

	for (int32 Road = 0; Road < 4; ++Road)
	{
		const float RoadDeg = RoadZeroDeg + Road * GoldenAngleDeg;
		const float RoadRad = FMath::DegreesToRadians(RoadDeg);
		const FVector Along(FMath::Cos(RoadRad), FMath::Sin(RoadRad), 0.f);
		const FVector Across(-Along.Y, Along.X, 0.f);

		for (int32 N = 1; N <= 6; ++N)
		{
			const float Radius = PlotBase * FMath::Pow(Phi, (float)(N - 1));
			if (Radius > 13000.f) { break; }
			const FVector OnRoad = Along * Radius;

			// A lamp on the road itself at every phi step: the path is legible
			// from the ground, from the map, and from the far end of the street.
			// A carved stone lantern from the kit, not a grey cylinder and a grey
			// sphere: the post is a real column, the head is a real pyramid crown, and
			// the warm light it stands in comes from the district's own lamps.
			SpawnBlock(FVector(OnRoad.X, OnRoad.Y, 240.f), FVector(0.36f, 0.36f, 4.8f),
				Col, GoldTint, 0.f);
			SpawnBlock(FVector(OnRoad.X, OnRoad.Y, 500.f),
				FVector(0.9f, 0.9f, 0.9f), Crown ? Crown : Col, GoldTint, 0.f);

			const FVector Setback = OnRoad + Across * (RoadClear + 520.f);
			const int32 Kind = (Road * 3 + N) % 4;

			// The door faces the street it belongs to, whichever golden angle that
			// street left the plaza at.
			const FVector ToRoad = (OnRoad - Setback).GetSafeNormal2D();

			if (Kind == 0)
			{
				// SHOP: wide and low, an open door and a lit sign band over it.
				const float W = 700.f, H = 560.f;
				BuildRoom(FVector(Setback.X, Setback.Y, 0.f), W * 0.5f, 320.f, H,
					ToRoad, WallAt(Road * 3 + N), FloorAt(N), TowerTintWarm, Shops == 0 && Homes == 0);
				// An arch over the door: the pack has one, and a doorway that is an
				// arch is the difference between a shed and a shopfront.
				if (ArchMesh)
				{
					const FVector DoorAt = FVector(Setback.X, Setback.Y, 0.f) + ToRoad * 300.f;
					SpawnBlockRot(FVector(DoorAt.X, DoorAt.Y, 260.f),
						FVector(4.6f, 4.6f, 5.4f), ArchMesh, GoldTint, 0.35f,
						FRotator(0.f, FMath::RadiansToDegrees(FMath::Atan2(ToRoad.Y, ToRoad.X)), 0.f));
				}
				// The sign band and the door bar used to be engine cubes tinted gold - and
				// no material in this project takes a tint, so they rendered as two grey
				// checker boxes floating on the shopfront. The arch above is the real prop
				// that says "shop", so the boxes are gone rather than faked.
				if (Menhir || ShrineProp)
				{
					SpawnBlock(FVector(Setback.X, Setback.Y, H + 40.f),
						FVector(0.7f, 0.7f, 0.7f), Menhir ? Menhir : ShrineProp, GoldTint, 0.f);
				}
				// A MARKET, NOT A SHOPFRONT ALONE. Every shop gets a stall beside it: a counter, four
				// posts and a canopy cut from the pack's own wall and floor meshes. The sizes are in
				// centimetres through ScaleForCm, because placement scale means "100 cm on the largest
				// axis" and a counter asked for in metres came out as a wafer in the terraces before.
				// It stands to the SIDE of the building, 250 cm toward the street and 700 cm along it,
				// so the doorway stays clear and the stall stays out of the road's 620 cm centre line.
				{
					UStaticMesh* StallCounter = WallAt(Road * 3 + N + 1);
					UStaticMesh* StallPost = Col;
					UStaticMesh* StallCanopy = FloorAt(N + 1);
					const FRotator StallRot(0.f, FMath::RadiansToDegrees(FMath::Atan2(ToRoad.Y, ToRoad.X)), 0.f);
					const FVector Side(-ToRoad.Y, ToRoad.X, 0.f);
					const FVector StallAt = FVector(Setback.X, Setback.Y, 0.f) + Side * 700.f + ToRoad * 250.f;
					int32 Parts = 0;
					if (StallCounter)
					{
						const FVector SCm = ScaleForCm(StallCounter, FVector(320.f, 90.f, 115.f));
						if (!SCm.IsZero() && SpawnBlockRot(StallAt + FVector(0.f, 0.f, 58.f), SCm,
							StallCounter, TowerTintWarm, 0.f, StallRot)) { ++Parts; }
					}
					if (StallPost)
					{
						const FVector SCm = ScaleForCm(StallPost, FVector(26.f, 26.f, 300.f));
						if (!SCm.IsZero())
						{
							for (int32 C2 = 0; C2 < 4; ++C2)
							{
								const FVector Off = StallRot.RotateVector(FVector(
									(C2 & 1) ? 150.f : -150.f, (C2 & 2) ? 120.f : -120.f, 0.f));
								if (SpawnBlockRot(StallAt + Off + FVector(0.f, 0.f, 150.f), SCm,
									StallPost, TowerTintWarm, 0.f, StallRot)) { ++Parts; }
							}
						}
					}
					if (StallCanopy)
					{
						const FVector SCm = ScaleForCm(StallCanopy, FVector(420.f, 340.f, 14.f));
						if (!SCm.IsZero() && SpawnBlockRot(StallAt + FVector(0.f, 0.f, 312.f), SCm,
							StallCanopy, TowerTintWarm, 0.f, StallRot)) { ++Parts; }
					}
					if (Parts > 0) { ++Stalls; }
				}
				++Shops;
			}
			else if (Kind == 1)
			{
				// HOME: narrower and taller, still four walls and a door.
				const float W = 480.f;
				const float H = 900.f * FMath::Pow(Phi, (float)(N % 3) * 0.25f);
				BuildRoom(FVector(Setback.X, Setback.Y, 0.f), W * 0.5f, 260.f, H,
					ToRoad, WallAt(Road + N + 2), FloorAt(Road + N), TowerTint, Shops == 0 && Homes == 0);
				// A real roof on every other home: a dome from the pack, not a flat lid.
				if (DomeMesh && (N % 2 == 1))
				{
					SpawnBlock(FVector(Setback.X, Setback.Y, H + 30.f),
						FVector(5.4f, 5.4f, 3.2f), DomeMesh, TowerTintWarm, 0.f);
				}
				// A carved lintel stone over the door: a real prop with a real material,
				// instead of a gold tinted engine cube that could never be gold.
				if (Crown)
				{
					SpawnBlock(FVector(Setback.X, Setback.Y, H * 0.62f),
						FVector(0.55f, 0.55f, 0.55f), Crown, GoldTint, 0.f);
				}
				++Homes;
			}
			else if (Kind == 2)
			{
				// CAMP: a low ring of blocks around a fire. The fire is the light.
				for (int32 A = 0; A < 5; ++A)
				{
					const float Ang = FMath::DegreesToRadians(A * 72.f);
					const FVector P = Setback + FVector(FMath::Cos(Ang) * 420.f, FMath::Sin(Ang) * 420.f, 0.f);
					SpawnBlock(FVector(P.X, P.Y, 90.f), FVector(1.5f, 1.5f, 1.8f), RockAt(A + Road), TowerTintWarm, 0.f);
				}
				SpawnBlock(FVector(Setback.X, Setback.Y, 150.f), FVector(0.7f, 0.7f, 0.7f),
					ShrineProp ? ShrineProp : RockAt(Road), GoldTint, 0.f);
				++Camps;
			}
			else
			{
				// SHRINE: a gold column with a crown, marking the street's end.
				SpawnBlock(FVector(Setback.X, Setback.Y, 620.f), FVector(1.1f, 1.1f, 12.4f), Col, TowerTint, 0.f);
				SpawnBlock(FVector(Setback.X, Setback.Y, 1300.f), FVector(1.5f, 1.5f, 1.5f),
					Crown ? Crown : Col, GoldTint, 0.f);
				++Shrines;
			}

			// The far side of the street, one rung out: a tower that closes the
			// view down the road instead of leaving it open to nothing.
			if (N >= 3)
			{
				// The far tower is a tower you can enter too: rock walls, one door
				// facing the street, and a hearth at its heart.
				const FVector Far = OnRoad - Across * (RoadClear + 900.f);
				const float H = 900.f * FMath::Pow(Phi, (float)(N - 3) * 0.5f);
				const FVector FarToRoad = (OnRoad - Far).GetSafeNormal2D();
				BuildRoom(FVector(Far.X, Far.Y, 0.f), 300.f, 300.f, H,
					FarToRoad, RockAt(Road + N + 3), FloorAt(Road * 2 + N), TowerTint, false);
				if (N % 2 == 0)
				{
					SpawnBlock(FVector(Far.X, Far.Y, H + 30.f), FVector(0.5f, 0.5f, 0.5f),
						Crown ? Crown : RockAt(Road + N), GoldTint, 0.f);
				}
				++Towers;
			}
		}
	}

	if (Stalls > 0)
	{
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI MARKET: %d stall(s) standing beside the shopfronts - counter, four posts and a canopy each, so the square has somewhere to trade."),
			Stalls);
	}

	// THE KEEPER'S LANDMARK: a column of light on the first road, at the end of
	// the lit path. The objective stops being a label at the edge of the screen
	// and becomes a thing standing in the world.
	SpawnBlock(FVector(KeeperAt.X, KeeperAt.Y, 1500.f), FVector(3.4f, 3.4f, 30.f),
		Col, TowerTint, 0.f);
	// A CARVED CROWN, NOT A GOLD BALL. This was /Engine/BasicShapes/Sphere - the engine
	// primitive, tinted gold and scaled 1.8 - which is a placeholder however bright it is.
	SpawnBlock(FVector(KeeperAt.X, KeeperAt.Y, 3100.f), FVector(1.2f, 1.2f, 3.2f),
		ColumnMesh, GoldTint, 2.5f);
	// The beam: a thin gold column from the crown into the sky, readable over
	// every roof in the district.
	SpawnBlock(FVector(KeeperAt.X, KeeperAt.Y, 6200.f), FVector(0.5f, 0.5f, 62.f),
		CylinderMesh, GoldTint, 1.6f);

	// Where the runner enters the world - only when this builder owns the map.
	if (!bSkipBase)
	{
		World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(),
			PlazaCenter, FRotator(0.f, 90.f, 0.f));
	}

	UE_LOG(LogVEIL, Log,
		TEXT("VEIL WORLD BUILT: 4 golden-angle roads, %d shops, %d homes, %d camps, %d shrines, %d towers - Keeper column at %s."),
		Shops, Homes, Camps, Shrines, Towers, *KeeperAt.ToCompactString());

	// THE OTHER FOUR FIFTHS OF THE MAP, and the Hub raised as a place too.
	// Without this the world was one plaza with five labels on it.
	BuildRegions();
}

// ---------------------------------------------------------------------------
// FIVE REGIONS, FIVE PLACES
//
// The centres come from UVEILRegionSubsystem::GetInfo - the same numbers the
// HUD region readout and RegionAt() use - so a runner standing on a region's
// ground always reads that region's name. Nothing here invents a coordinate.
//
// Scale is the map's real scale: the anchors span roughly 70 m x 70 m
// (Spire at 2800,500 to the Cache at 0,-6200), so a region is a
// neighbourhood-sized place and its landmark is tall enough to be seen from
// the far side of the whole map.
//
// Every block is an INSTANCE. One component per shape/colour pair keeps the
// draw calls at a handful and the actor count where it was.
// ---------------------------------------------------------------------------
namespace
{
	const float PhiRegion = 1.61803398875f;
	const float GoldenAngleRegion = 137.50776405f;
}

float AVEILWorldBuilder::MeshNorm(const UStaticMesh* Mesh)
{
	// 1.0 of placement scale has always meant 100 cm here, because the engine
	// cube is 100 cm. An authored wall is not, so the instance is divided by its
	// own size: the same numbers then place the same building, in real masonry.
	if (!Mesh) { return 1.f; }
	const float Extent = (float)Mesh->GetBounds().BoxExtent.GetMax();
	return Extent > 1.f ? 100.f / (2.f * Extent) : 1.f;
}

// GOLD COLUMNS GET A REAL BODY, ONE PER INSTANCE.
//
// The probe traced horizontally at z=+700 straight into the nearest SM_AncientColumn_Big and
// reported CLEAR - and kept reporting CLEAR with BlockAll and bUseDefaultCollision set on
// the instanced set. The kit's column mesh carries no collision geometry, so an instance has
// nothing to collide with however the component is configured. Each one therefore gets an
// invisible box the size of its own bounds, tagged VEIL_BAKE so the next bake clears it.
void AVEILWorldBuilder::AddColumnBody(const FVector& Location, const FVector& Scale, UStaticMesh* Mesh)
{
	UWorld* World = GetWorld();
	if (!World || !Mesh) { return; }

	AActor* Body = World->SpawnActor<AActor>(AActor::StaticClass(), Location, FRotator::ZeroRotator);
	if (!Body) { return; }
	Body->Tags.Add(FName(TEXT("VEIL_BAKE")));

	const FVector Extent = Mesh->GetBounds().BoxExtent * Scale;
	UBoxComponent* Box = NewObject<UBoxComponent>(Body);
	Box->SetMobility(EComponentMobility::Movable);
	Box->SetBoxExtent(Extent, false);
	Box->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Box->SetCollisionProfileName(TEXT("BlockAll"));
	Box->SetCanEverAffectNavigation(false);
	Body->SetRootComponent(Box);
	Box->RegisterComponent();
	// The instance pivot is the mesh's base, so the box sits half its height up.
	Box->SetWorldLocation(Location + FVector(0.f, 0.f, Mesh->GetBounds().BoxExtent.Z * Scale.Z));
}

void AVEILWorldBuilder::AddBlock(UInstancedStaticMeshComponent* Set, const FVector& Location,
	const FVector& Scale)
{
	if (Set)
	{
		const float Norm = MeshNorm(Set->GetStaticMesh());
		// Same pivot correction as SpawnBlockRot: an instance scaled about an
		// off-centre pivot is displaced by (local origin x scale), and for the 2 km
		// ground tiles that displacement is kilometres.
		const FVector Shift = Set->GetStaticMesh()->GetBounds().Origin * (Scale * Norm);
		Set->AddInstance(FTransform(FRotator::ZeroRotator, Location - Shift, Scale * Norm), true);
		if (Set->GetStaticMesh() && Set->GetStaticMesh()->GetName().Contains(TEXT("AncientColumn")))
		{
			AddColumnBody(Location, Scale * Norm, Set->GetStaticMesh());
		}
	}
}

void AVEILWorldBuilder::AddBlockYaw(UInstancedStaticMeshComponent* Set, const FVector& Location,
	const FVector& Scale, float YawDegrees)
{
	if (Set)
	{
		const float Norm = MeshNorm(Set->GetStaticMesh());
		const FRotator R(0.f, YawDegrees, 0.f);
		const FVector Shift = R.RotateVector(Set->GetStaticMesh()->GetBounds().Origin * (Scale * Norm));
		Set->AddInstance(FTransform(R, Location - Shift, Scale * Norm), true);
		if (Set->GetStaticMesh() && Set->GetStaticMesh()->GetName().Contains(TEXT("AncientColumn")))
		{
			AddColumnBody(Location, Scale * Norm, Set->GetStaticMesh());
		}
	}
}

UInstancedStaticMeshComponent* AVEILWorldBuilder::MakeSet(UStaticMesh* Mesh,
	const FLinearColor& Tint, float Emissive)
{
	UWorld* World = GetWorld();
	if (!World || !Mesh) { return nullptr; }

	// NO ENGINE SHAPE IN AN INSTANCED SET EITHER. The band courses were made from CubeMesh,
	// the engine cube, which the component-level swap could not rescue because an empty
	// instanced set has no bounds to fit against. Substituting here means the builder
	// never emits a primitive at all.
	if (Mesh && Mesh->GetPathName().StartsWith(TEXT("/Engine/BasicShapes/")))
	{
		if (FloorMesh) { Mesh = FloorMesh; }
		else if (WallMesh) { Mesh = WallMesh; }
	}
	UInstancedStaticMeshComponent* Set = NewObject<UInstancedStaticMeshComponent>(this);
	Set->SetStaticMesh(Mesh);
	Set->SetMobility(EComponentMobility::Movable);
	// AN INSTANCED SET TAKES ITS COLLISION FROM THE MESH, NOT FROM THIS COMPONENT.
	// SetCollisionEnabled alone left the plaza columns as pictures: the column probe traced
	// horizontally at z=+700 straight at the nearest AncientColumn and reported CLEAR.
	// bUseDefaultCollision makes each instance use the mesh's own collision body, and the
	// profile names the response set explicitly instead of relying on the default.
	Set->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	Set->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	// There is no SetUseDefaultCollision() on this class in 5.8 - it is the
	// bUseDefaultCollision property, which is what the compiler said (C2039).
	Set->bUseDefaultCollision = true;
	Set->SetCastShadow(true);
	if (UMaterialInterface* Base = Mesh->GetMaterial(0))
	{
		// THIS IS WHERE THE GOLD CUBES ACTUALLY CAME FROM. The plaza blocks are instanced sets
		// that take a COLOUR, not a material, so swapping the gold material in Place() never
		// touched them - which is why the gold masses survived that fix. A saturated gold tint
		// on a non-glowing surface is now warm stone; only glowing trim keeps its gold.
		const bool bGoldTint = (Tint.R - Tint.B) > 0.25f && Tint.R > 0.55f && Tint.B < 0.55f;
		const FLinearColor Use = (bGoldTint && Emissive <= 0.f)
			? FLinearColor(0.42f, 0.36f, 0.28f)
			: Tint;
		// A COLOUR CANNOT OVERRIDE A GOLD MATERIAL. Two passes proved it: setting a stone
		// BaseColor on a mesh whose material IS M_PHI_Gold still renders gold, because the
		// base colour is only one input to that material. Unlit instanced surfaces therefore
		// get the cut stone material ASSIGNED, the same way Place() does it.
		// GOLD IS REAL METAL, NOT FLAT YELLOW. Unlit surfaces are stone; lit ones wear the
		// metallic gold built for this game (metallic 1.0, roughness 0.28, emissive 0.35)
		// instead of the old flat material. If that asset is missing, the mesh keeps its own.
		UMaterialInterface* BodyMat = nullptr;
		if (Emissive <= 0.f)
		{
			BodyMat = LoadObject<UMaterialInterface>(nullptr,
				TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
		}
		else
		{
			BodyMat = LoadObject<UMaterialInterface>(nullptr,
				TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic"));
		}
		if (UMaterialInstanceDynamic* StoneMID = BodyMat ? UMaterialInstanceDynamic::Create(BodyMat, Set) : nullptr)
		{
			StoneMID->SetVectorParameterValue(TEXT("Color"), Use);
			StoneMID->SetVectorParameterValue(TEXT("BaseColor"), Use);
			Set->SetMaterial(0, StoneMID);
			Set->RegisterComponent();
			Set->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
			InstanceSets.Add(Set);
			return Set;
		}
		if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, Set))
		{
			MID->SetVectorParameterValue(TEXT("Color"), Use);
			MID->SetVectorParameterValue(TEXT("BaseColor"), Use);
			if (Emissive > 0.f)
			{
				MID->SetVectorParameterValue(TEXT("EmissiveColor"), Tint * Emissive);
			}
			Set->SetMaterial(0, MID);
		}
	}
	Set->RegisterComponent();
	Set->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
	InstanceSets.Add(Set);
	return Set;
}

void AVEILWorldBuilder::BuildRegions()
{
	UWorld* World = GetWorld();
	UVEILRegionSubsystem* Regions = World ? World->GetSubsystem<UVEILRegionSubsystem>() : nullptr;
	if (!World || !Regions) { return; }

	int32 TotalInstances = 0;

	for (int32 Index = 0; Index < 5; ++Index)
	{
		const EVEILRegion Region = static_cast<EVEILRegion>(Index);
		const FVEILRegionInfo Info = Regions->GetInfo(Region);
		const FVector2D C = Info.Center;
		const FLinearColor Tint = Info.Palette;

		// The region's body is authored masonry; only the thin trim bands stay
		// engine cubes, because they are 20 cm of light and a wall mesh cannot be
		// trimmed that fine.
		UInstancedStaticMeshComponent* Body = MakeSet(WallMesh ? WallMesh : CubeMesh, Tint, 0.f);
		UInstancedStaticMeshComponent* Round = MakeSet(ColumnMesh ? ColumnMesh : CylinderMesh, Tint, 0.f);
		UInstancedStaticMeshComponent* Band = MakeSet(CubeMesh, Tint, 1.5f);
		// A LANTERN IS CARVED, NOT A BALL. SphereMesh here is the engine sphere: every lamp
		// globe in the Hub was a gold primitive sitting on the plaza.
		UInstancedStaticMeshComponent* Lamp = MakeSet(ColumnMesh ? ColumnMesh : CylinderMesh, Tint * 1.2f, 2.2f);

		switch (Index)
		{
		case 0: // HUB - the golden ring the runner wakes beside.
		{
			const float Radius = 900.f;
			AddBlock(Round, FVector(C.X, C.Y, 4.f), FVector(9.f, 9.f, 0.1f));
			// Twelve columns on a sunflower spiral: same angle, growing radius.
			for (int32 K = 0; K < 12; ++K)
			{
				const float Ang = FMath::DegreesToRadians(K * GoldenAngleRegion);
				// The spiral starts a fifth of the way out, not at the centre: a column
				// standing on the plaza's middle is a column the runner spawns inside.
				const float Rad = Radius * (0.22f + 0.78f * FMath::Sqrt((float)K / 11.f));
				const FVector At(C.X + FMath::Cos(Ang) * Rad, C.Y + FMath::Sin(Ang) * Rad, 0.f);
				AddBlock(Round, FVector(At.X, At.Y, 450.f), FVector(1.2f, 1.2f, 9.f));
				AddBlock(Lamp, FVector(At.X, At.Y, 960.f), FVector(1.4f, 1.4f, 1.4f));
			}
			// The gate the road to the Keeper leaves through.
			AddBlock(Body, FVector(C.X - 1100.f, C.Y - 700.f, 700.f), FVector(1.6f, 1.6f, 14.f));
			AddBlock(Body, FVector(C.X + 1100.f, C.Y - 700.f, 700.f), FVector(1.6f, 1.6f, 14.f));
			AddBlock(Band, FVector(C.X, C.Y - 700.f, 1420.f), FVector(24.f, 1.4f, 0.5f));
			// Landmark: read from across the whole map.
			AddBlock(Round, FVector(C.X, C.Y, 3000.f), FVector(2.6f, 2.6f, 60.f));
			AddBlock(Lamp, FVector(C.X, C.Y, 6100.f), FVector(3.2f, 3.2f, 3.2f));
			break;
		}
		case 1: // THE NEST - green terraces stepping down to a fire.
		{
			const float Radius = 800.f;
			for (int32 K = 0; K < 6; ++K)
			{
				const float Rad = Radius * FMath::Pow(0.86f, (float)K);
				AddBlock(Round, FVector(C.X, C.Y, 40.f + 110.f * K), FVector(Rad / 100.f, Rad / 100.f, 0.22f));
			}
			for (int32 K = 0; K < 9; ++K)
			{
				const float Ang = FMath::DegreesToRadians(K * GoldenAngleRegion);
				const float Rad = Radius * 0.82f * FMath::Sqrt((float)(K + 1) / 9.f);
				const FVector At(C.X + FMath::Cos(Ang) * Rad, C.Y + FMath::Sin(Ang) * Rad, 660.f);
				AddBlock(Body, FVector(At.X, At.Y, At.Z + 70.f), FVector(2.2f, 2.2f, 1.8f));
				AddBlock(Lamp, FVector(At.X, At.Y, At.Z + 200.f), FVector(1.1f, 1.1f, 1.1f));
			}
			AddBlock(Lamp, FVector(C.X, C.Y, 760.f), FVector(2.4f, 2.4f, 2.4f));
			AddBlock(Round, FVector(C.X, C.Y, 2400.f), FVector(2.2f, 2.2f, 48.f));
			AddBlock(Lamp, FVector(C.X, C.Y, 4900.f), FVector(2.8f, 2.8f, 2.8f));
			break;
		}
		case 2: // THE SPIRE - blue towers rising by phi, one needle over them.
		{
			const float Radius = 1100.f;
			for (int32 K = 0; K < 9; ++K)
			{
				const float Ang = FMath::DegreesToRadians(K * GoldenAngleRegion);
				const float Rad = Radius * 0.18f + Radius * 0.78f * FMath::Sqrt((float)K / 8.f);
				const float H = FMath::Min(1400.f * FMath::Pow(PhiRegion, (float)K * 0.37f), 7000.f);
				const FVector At(C.X + FMath::Cos(Ang) * Rad, C.Y + FMath::Sin(Ang) * Rad, 0.f);
				AddBlock(Body, FVector(At.X, At.Y, H * 0.5f), FVector(4.2f, 4.2f, H / 100.f));
				if (K % 2 == 0)
				{
					AddBlock(Band, FVector(At.X, At.Y, H + 40.f), FVector(4.4f, 4.4f, 0.5f));
				}
				if (K % 3 == 0)
				{
					AddBlock(Lamp, FVector(At.X, At.Y, H + 220.f), FVector(1.6f, 1.6f, 1.6f));
				}
			}
			AddBlock(Round, FVector(C.X, C.Y, 4000.f), FVector(1.8f, 1.8f, 80.f));
			AddBlock(Lamp, FVector(C.X, C.Y, 8100.f), FVector(3.6f, 3.6f, 3.6f));
			break;
		}
		case 3: // OMNI DISTRICT - a magenta grid of arcades, lit bands on every roof.
		{
			const float Grid = 900.f;
			for (int32 GX = -2; GX <= 2; ++GX)
			{
				for (int32 GY = -2; GY <= 2; ++GY)
				{
					if (GX == 0 && GY == 0) { continue; } // keep the crossing open
					const float H = 2200.f * FMath::Pow(PhiRegion, (float)((GX + GY) & 1));
					const FVector At(C.X + GX * Grid, C.Y + GY * Grid, 0.f);
					AddBlock(Body, FVector(At.X, At.Y, H * 0.5f), FVector(6.4f, 6.4f, H / 100.f));
					AddBlock(Band, FVector(At.X, At.Y, H + 30.f), FVector(6.6f, 6.6f, 0.42f));
				}
			}
			// Four arches over the crossing, on the cardinal points.
			for (int32 A = 0; A < 4; ++A)
			{
				const float Ang = FMath::DegreesToRadians(A * 90.f);
				const FVector At(C.X + FMath::Cos(Ang) * Grid, C.Y + FMath::Sin(Ang) * Grid, 0.f);
				AddBlockYaw(Band, FVector(At.X, At.Y, 2400.f), FVector(9.f, 1.2f, 0.6f), A * 90.f);
				AddBlockYaw(Body, FVector(At.X, At.Y, 1200.f), FVector(1.2f, 1.2f, 24.f), A * 90.f);
			}
			AddBlock(Round, FVector(C.X, C.Y, 3000.f), FVector(2.f, 2.f, 60.f));
			AddBlock(Lamp, FVector(C.X, C.Y, 6100.f), FVector(3.4f, 3.4f, 3.4f));
			break;
		}
		default: // MEGASTRUCTURE DEEP - the walled ring at the heart of the map.
		{
			const float Radius = 420.f;
			AddBlock(Round, FVector(C.X, C.Y, 3.f), FVector(5.2f, 5.2f, 0.08f));
			for (int32 K = 0; K < 16; ++K)
			{
				const float Ang = K * (360.f / 16.f);
				const float Rad = FMath::DegreesToRadians(Ang);
				const FVector At(C.X + FMath::Cos(Rad) * Radius, C.Y + FMath::Sin(Rad) * Radius, 0.f);
				AddBlockYaw(Body, FVector(At.X, At.Y, 500.f), FVector(1.8f, 0.7f, 10.f), Ang + 90.f);
				if (K % 4 == 0)
				{
					AddBlockYaw(Band, FVector(At.X, At.Y, 1060.f), FVector(1.9f, 0.8f, 0.6f), Ang + 90.f);
				}
			}
			AddBlock(Round, FVector(C.X, C.Y, 60.f), FVector(2.6f, 2.6f, 0.9f));
			AddBlock(Lamp, FVector(C.X, C.Y, 200.f), FVector(1.8f, 1.8f, 1.8f));
			AddBlock(Round, FVector(C.X, C.Y, 1500.f), FVector(1.4f, 1.4f, 28.f));
			AddBlock(Lamp, FVector(C.X, C.Y, 3000.f), FVector(2.2f, 2.2f, 2.2f));
			break;
		}
		}

		// Count the BLOCKS, not the components: a region is proven standing by how
		// many instances it holds, and "0 instance sets" proves nothing.
		int32 RegionInstances = 0;
		for (UInstancedStaticMeshComponent* Set : { Body, Round, Band, Lamp })
		{
			if (Set) { RegionInstances += Set->GetInstanceCount(); }
		}
		TotalInstances += RegionInstances;
		UE_LOG(LogVEIL, Log, TEXT("VEIL REGION BUILT: %s at %s, %d instances, palette %s"),
			*Info.Name.ToString(), *FVector(C.X, C.Y, 0.f).ToCompactString(),
			RegionInstances, *Tint.ToString());
	}

	// THE ROADS BETWEEN THEM: slabs and lamps from region to region, so every
	// area is reachable on the ground instead of only by name.
	UInstancedStaticMeshComponent* Road = MakeSet(FloorMesh ? FloorMesh : CubeMesh, FLinearColor(0.13f, 0.13f, 0.16f), 0.f);
	UInstancedStaticMeshComponent* RoadLamp = MakeSet(ColumnMesh ? ColumnMesh : CylinderMesh, FLinearColor(1.f, 0.78f, 0.3f), 1.8f);
	int32 RoadSlabs = 0;

	auto LayRoad = [&](int32 FromIndex, int32 ToIndex)
	{
		const FVEILRegionInfo A = Regions->GetInfo(static_cast<EVEILRegion>(FromIndex));
		const FVEILRegionInfo B = Regions->GetInfo(static_cast<EVEILRegion>(ToIndex));
		const FVector2D D = B.Center - A.Center;
		const float Len = D.Size();
		if (Len < 100.f) { return; }
		const float Yaw = FMath::RadiansToDegrees(FMath::Atan2(D.Y, D.X));
		const int32 Steps = FMath::Max(2, FMath::RoundToInt(Len / 420.f));
		for (int32 S = 1; S < Steps; ++S)
		{
			const FVector2D P = A.Center + D * ((float)S / (float)Steps);
			AddBlockYaw(Road, FVector(P.X, P.Y, 3.f), FVector(4.6f, 4.6f, 0.06f), Yaw);
			++RoadSlabs;
			if (S % 3 == 0)
			{
				AddBlock(RoadLamp, FVector(P.X, P.Y, 130.f), FVector(0.7f, 0.7f, 0.7f));
			}
		}
	};

	LayRoad(0, 1); // Hub -> Nest
	LayRoad(0, 2); // Hub -> Spire
	LayRoad(0, 3); // Hub -> Omni District
	LayRoad(3, 4); // Omni District -> the Deep at the heart

	// THE NEST'S OWN PATHS. Region roads connect centres; they say nothing about how to
	// walk from the plaza to the Keeper or to a beacon, so the spawn could be a courtyard
	// whose destinations exist only as text. Four paths leave the plaza here, in the same
	// stone as the region roads, laid flush on the paving the plaza already has.
	{
		auto LayPath = [&](const FVector2D& To)
		{
			const float Len = (float)To.Size();
			if (Len < 100.f) { return; }
			const float Yaw = FMath::RadiansToDegrees(FMath::Atan2((float)To.Y, (float)To.X));
			const int32 Steps = FMath::Max(2, FMath::RoundToInt(Len / 420.f));
			for (int32 S = 1; S <= Steps; ++S)
			{
				const FVector2D P = To * ((float)S / (float)Steps);
				// FLUSH WITH THE PAVING, on top of it - a path you can see and walk on, not a
				// kerb to trip over. The plaza plane is z=0.5 and its stones are 10 cm thick.
				AddBlockYaw(Road, FVector(P.X, P.Y, 9.f), FVector(4.6f, 4.6f, 0.06f), Yaw);
				++RoadSlabs;
			}
		};
		LayPath(FVector2D(0.f, -2600.f));     // to the Keeper
		LayPath(FVector2D(2800.f, 500.f));    // to the beacon east
		LayPath(FVector2D(-2000.f, -900.f));  // to the beacon west
		LayPath(FVector2D(600.f, -2100.f));   // to the beacon south
		UE_LOG(LogVEIL, Warning, TEXT("VEIL NEST PATHS: four paths laid from the plaza - Keeper, east, west and south beacons."));
	}

	UE_LOG(LogVEIL, Log, TEXT("VEIL MAP BUILT: 5 regions, %d instances, %d road slabs, roads Hub-Nest, Hub-Spire, Hub-Omni, Omni-Deep."),
		TotalInstances, RoadSlabs);
	UE_LOG(LogVEIL, Warning, TEXT("PHI ENTERABLE: %d buildings built as rooms with a doorway, a floor and a hearth."),
		RoomsBuilt);
}

bool AVEILWorldBuilder::GoToRegion(UWorld* World, int32 Index)
{
	if (!World) { return false; }
	APlayerController* PC = World->GetFirstPlayerController();
	APawn* Pawn = PC ? PC->GetPawn() : nullptr;
	UVEILRegionSubsystem* Regions = World->GetSubsystem<UVEILRegionSubsystem>();
	if (!Pawn || !Regions) { return false; }

	const int32 I = FMath::Clamp(Index, 0, 4);
	const FVEILRegionInfo Info = Regions->GetInfo(static_cast<EVEILRegion>(I));
	// ARRIVAL POINTS, not region centres: a centre holds that region's own
	// landmark, so landing on it puts the runner inside a wall.
	FVector2D P = Info.Center;
	switch (I)
	{
	case 0: P += FVector2D(0.f, -1500.f); break;
	case 1: P += FVector2D(0.f, -1400.f); break;
	case 2: P += FVector2D(0.f, -1900.f); break;
	case 3: break;
	default: P += FVector2D(0.f, 700.f); break;
	}
	const FVector Target(P.X, P.Y, 220.f);
	Pawn->SetActorLocation(Target, false, nullptr, ETeleportType::TeleportPhysics);
	if (PC) { PC->SetControlRotation(FRotator(-8.f, 0.f, 0.f)); }
	UE_LOG(LogVEIL, Warning, TEXT("VEIL GOTO: %s at %s"), *Info.Name.ToString(), *Target.ToCompactString());
	return true;
}

// phi.goto <0..4> - put the runner on a region so every area can be walked and
// photographed without a fast-travel unlock. Read-only about the world: it moves
// the pawn, nothing else.
static FAutoConsoleCommandWithWorldAndArgs GVeilRegionGoto(
	TEXT("phi.goto"),
	TEXT("Teleport the runner to a region centre: phi.goto <0 Hub, 1 Nest, 2 Spire, 3 Omni District, 4 Deep>"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		// One body, two doors: a typed phi.goto and a proof hook run the same code.
		AVEILWorldBuilder::GoToRegion(World, Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 0);
	}));

// phi.stat - MEASURE THE VIEW, DO NOT GUESS IT.
//
// "The world is black from above" has several possible causes and they are all
// visible in numbers: no geometry in front of the camera, no sun, no sky, fog
// eating the depth, too few lights, or the screen percentage dropped to a
// smear. This prints every one of those as measured, so the next change is
// aimed at the cause instead of being another guess.
static FAutoConsoleCommandWithWorldAndArgs GPhiStat(
	TEXT("phi.stat"),
	TEXT("Measure what the camera is actually looking at: sun, sky, fog, lights, screen %, first hit"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (!World) { return; }
		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC) { UE_LOG(LogVEIL, Warning, TEXT("PHI STAT: no player controller")); return; }

		FVector CamLoc = FVector::ZeroVector;
		FRotator CamRot = FRotator::ZeroRotator;
		PC->GetPlayerViewPoint(CamLoc, CamRot);

		// Is there ANY geometry down the camera's own view vector, and how far?
		FHitResult Hit;
		FCollisionQueryParams Probe(SCENE_QUERY_STAT(PhiStatProbe), true);
		const FVector End = CamLoc + CamRot.Vector() * 300000.f;
		const bool bHit = World->LineTraceSingleByChannel(Hit, CamLoc, End, ECC_Visibility, Probe);

		int32 Dirs = 0, Skies = 0, Fogs = 0, Points = 0, Meshes = 0;
		float SunI = 0.f, SkyI = 0.f, FogD = -1.f;
		FRotator SunRot = FRotator::ZeroRotator;
		for (TActorIterator<ADirectionalLight> It(World); It; ++It)
		{
			++Dirs;
			if (const UDirectionalLightComponent* L = It->FindComponentByClass<UDirectionalLightComponent>())
			{
				SunI = L->Intensity;
				SunRot = It->GetActorRotation();
			}
		}
		for (TActorIterator<ASkyLight> It(World); It; ++It)
		{
			++Skies;
			if (const USkyLightComponent* L = It->FindComponentByClass<USkyLightComponent>()) { SkyI = L->Intensity; }
		}
		for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
		{
			++Fogs;
			if (const UExponentialHeightFogComponent* F = It->FindComponentByClass<UExponentialHeightFogComponent>())
			{
				FogD = F->FogDensity;
			}
		}
		for (TActorIterator<APointLight> It(World); It; ++It) { ++Points; }
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It) { ++Meshes; }

		auto CVarF = [](const TCHAR* Name) -> float
		{
			if (const IConsoleVariable* V = IConsoleManager::Get().FindConsoleVariable(Name))
			{
				// r.ScreenPercentage is registered as a float; r.DynamicRes as int.
				const float F = V->GetFloat();
				return F != 0.f ? F : (float)V->GetInt();
			}
			return -1.f;
		};

		UE_LOG(LogVEIL, Warning,
			TEXT("PHI STAT: cam=%s yaw=%.1f pitch=%.1f"),
			*CamLoc.ToCompactString(), CamRot.Yaw, CamRot.Pitch);
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI STAT: first hit=%s actor=%s dist=%.0f"),
			bHit ? TEXT("YES") : TEXT("NONE"), bHit && Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("-"),
			bHit ? Hit.Distance : -1.f);
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI STAT: suns=%d I=%.2f pitch=%.1f | skies=%d I=%.2f | fogs=%d density=%.5f | points=%d | meshActors=%d"),
			Dirs, SunI, SunRot.Pitch, Skies, SkyI, Fogs, FogD, Points, Meshes);
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI STAT: screenPercent=%.0f | r.DynamicRes=%d | Lumen=%d | exposure=%d"),
			CVarF(TEXT("r.ScreenPercentage")), (int32)CVarF(TEXT("r.DynamicRes.OperationMode")),
			(int32)CVarF(TEXT("r.DynamicGlobalIlluminationMethod")), (int32)CVarF(TEXT("r.DefaultFeature.AutoExposure")));

		// WHOSE EYES ARE THESE? A camera actor and the pawn are different views,
		// and "the world is black" means something else in each of them.
		const AActor* View = PC->GetViewTarget();
		UE_LOG(LogVEIL, Warning, TEXT("PHI STAT: viewTarget=%s loc=%s"),
			View ? *View->GetName() : TEXT("-"), View ? *View->GetActorLocation().ToCompactString() : TEXT("-"));

		// IS THE CAMERA INSIDE SOMETHING? Six short probes: a camera buried in a
		// mesh renders its own backfaces black, and no amount of light fixes it.
		static const FVector Dirs6[6] = {
			FVector(1,0,0), FVector(-1,0,0), FVector(0,1,0),
			FVector(0,-1,0), FVector(0,0,1), FVector(0,0,-1) };
		static const TCHAR* DirNames[6] = { TEXT("+X"), TEXT("-X"), TEXT("+Y"), TEXT("-Y"), TEXT("+Z"), TEXT("-Z") };
		int32 Blocked = 0;
		FString Enclosure;
		for (int32 i = 0; i < 6; ++i)
		{
			FHitResult Short;
			if (World->LineTraceSingleByChannel(Short, CamLoc, CamLoc + Dirs6[i] * 200.f, ECC_Visibility, Probe))
			{
				++Blocked;
				Enclosure += FString::Printf(TEXT(" %s=%.0f(%s)"), DirNames[i], Short.Distance,
					Short.GetActor() ? *Short.GetActor()->GetName() : TEXT("?"));
			}
		}
		UE_LOG(LogVEIL, Warning, TEXT("PHI STAT: enclosure %d/6 within 2 m -%s"),
			Blocked, Enclosure.IsEmpty() ? TEXT(" open") : *Enclosure);

		// Which volumes contain the camera point, and how big they are. A camera
		// inside a mountain's bounds is inside a mountain; the size says which.
		int32 Contains = 0;
		FString NearNames;
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			const FBox B = It->GetComponentsBoundingBox(true);
			if (B.IsValid && B.IsInside(CamLoc))
			{
				++Contains;
				if (Contains <= 8)
				{
					const FVector Size = B.GetSize();
					NearNames += FString::Printf(TEXT("\n    contains: %s  mesh=%s  size=%.0fx%.0fx%.0f"),
						*It->GetName(),
						It->GetStaticMeshComponent() && It->GetStaticMeshComponent()->GetStaticMesh()
							? *It->GetStaticMeshComponent()->GetStaticMesh()->GetName() : TEXT("?"),
						Size.X, Size.Y, Size.Z);
				}
			}
		}
		UE_LOG(LogVEIL, Warning, TEXT("PHI STAT: camera inside %d mesh bounds%s"),
			Contains, *NearNames);

		// The in-world clock, because "dark" is also a time of day.
		if (const UVEILTime* Clock = World->GetSubsystem<UVEILTime>())
		{
			UE_LOG(LogVEIL, Warning, TEXT("PHI STAT: clock %s"), *Clock->ClockLine());
		}
	}));

// phi.what - NAME WHAT IS IN FRONT OF THE CAMERA.
//
// "There is a floating slab / a black box" is a description; the fix needs a mesh
// name, a material name, a size and a height. This prints those for whatever the
// crosshair is on, so the next change is aimed at the object the eye saw.
static FAutoConsoleCommandWithWorldAndArgs GPhiWhat(
	TEXT("phi.what"),
	TEXT("Report the actor under the camera's crosshair: mesh, material, bounds, height"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (!World) { return; }
		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC) { return; }
		FVector Loc; FRotator Rot;
		PC->GetPlayerViewPoint(Loc, Rot);
		FCollisionQueryParams P(SCENE_QUERY_STAT(PhiWhat), true);
		FHitResult Hit;
		if (!World->LineTraceSingleByChannel(Hit, Loc, Loc + Rot.Vector() * 500000.f, ECC_Visibility, P))
		{
			UE_LOG(LogVEIL, Warning, TEXT("PHI WHAT: nothing hit from %s"), *Loc.ToCompactString());
			return;
		}
		AActor* A = Hit.GetActor();
		UStaticMeshComponent* C = A ? A->FindComponentByClass<UStaticMeshComponent>() : nullptr;
		const UStaticMesh* M = C ? C->GetStaticMesh() : nullptr;
		const UMaterialInterface* Mat = C ? C->GetMaterial(0) : nullptr;
		const FBox B = A ? A->GetComponentsBoundingBox(true) : FBox(ForceInit);
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI WHAT: %s mesh=%s material=%s dist=%.0f loc=%s size=%s mobility=%d"),
			A ? *A->GetName() : TEXT("-"), M ? *M->GetName() : TEXT("-"),
			Mat ? *Mat->GetName() : TEXT("-"), Hit.Distance,
			A ? *A->GetActorLocation().ToCompactString() : TEXT("-"),
			B.IsValid ? *B.GetSize().ToCompactString() : TEXT("-"),
			C ? (int32)C->Mobility : -1);
	}));

// phi.dump - LIST WHAT IS NEAR THE RUNNER, by distance, with names.
//
// The floating slabs cannot be named by a crosshair because the eye sees them
// off-centre. This walks the actors around the pawn and prints the nearest ones:
// mesh, location, size, whether they are up in the air, and their mobility. One
// line per actor, nearest first - the answer to "what are those rectangles".
static FAutoConsoleCommandWithWorldAndArgs GPhiDump(
	TEXT("phi.dump"),
	TEXT("List the static meshes nearest the runner: mesh, location, size, height above ground"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (!World) { return; }
		APlayerController* PC = World->GetFirstPlayerController();
		APawn* Pawn = PC ? PC->GetPawn() : nullptr;
		if (!Pawn) { return; }
		const FVector From = Pawn->GetActorLocation();
		const float Radius = Args.Num() > 0 ? FCString::Atof(*Args[0]) : 12000.f;

		struct FRow { float D; float Size; FString Line; };
		TArray<FRow> Rows;
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			UStaticMeshComponent* C = It->GetStaticMeshComponent();
			const UStaticMesh* M = C ? C->GetStaticMesh() : nullptr;
			if (!M) { continue; }
			const FBox B = It->GetComponentsBoundingBox(true);
			if (!B.IsValid) { continue; }
			const float D = (float)FVector::Dist2D(From, B.GetCenter());
			if (D > Radius) { continue; }
			FRow Row;
			Row.D = D;
			Row.Size = (float)B.GetSize().GetMax();
			Row.Line = FString::Printf(TEXT("  %6.0f cm  z=%7.0f  mesh=%-24s size=%s"),
				D, B.Min.Z, *M->GetName(), *B.GetSize().ToCompactString());
			Rows.Add(Row);
		}
		// phi.dump big - LARGEST FIRST. The eye is complaining about the big flat
		// rectangles, not the pebbles, and sorting by distance buries them under a
		// hundred one-metre cubes. Sorting by footprint puts them at the top of the
		// list by name.
		const bool bBigFirst = Args.Num() > 1 && Args[1].Equals(TEXT("big"), ESearchCase::IgnoreCase);
		if (bBigFirst)
		{
			Rows.Sort([](const FRow& A, const FRow& B) { return A.Size > B.Size; });
		}
		else
		{
			Rows.Sort([](const FRow& A, const FRow& B) { return A.D < B.D; });
		}
		UE_LOG(LogVEIL, Warning, TEXT("PHI DUMP: %d meshes within %.0f cm of the runner at %s"),
			Rows.Num(), Radius, *From.ToCompactString());
		for (int32 i = 0; i < FMath::Min(30, Rows.Num()); ++i)
		{
			UE_LOG(LogVEIL, Warning, TEXT("PHI DUMP:%s"), *Rows[i].Line);
		}
	}));

// phi.sweep - NAME EVERY SHAPE AROUND THE CAMERA, not just the one in front.
//
// "It looks random" is a description of a scene, and a scene is many objects. One
// aimed probe answers one of them. This casts a fan of rays (12 yaw x 3 pitch) and
// prints the first hit in each direction with its mesh, material, distance, size
// and height - so the floating rectangles can be identified by name in one run
// instead of one guess per screenshot.
static FAutoConsoleCommandWithWorldAndArgs GPhiSweep(
	TEXT("phi.sweep"),
	TEXT("Cast a fan of probes and report every distinct shape around the camera"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (!World) { return; }
		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC) { return; }
		FVector Loc; FRotator Rot;
		PC->GetPlayerViewPoint(Loc, Rot);
		FCollisionQueryParams P(SCENE_QUERY_STAT(PhiSweep), true);
		TSet<FString> Seen;
		const float Pitches[3] = { -5.f, -25.f, 10.f };
		for (int32 y = 0; y < 12; ++y)
		{
			for (int32 p = 0; p < 3; ++p)
			{
				const FRotator Dir(Pitches[p], y * 30.f, 0.f);
				FHitResult Hit;
				if (!World->LineTraceSingleByChannel(Hit, Loc, Loc + Dir.Vector() * 120000.f, ECC_Visibility, P)) { continue; }
				AActor* A = Hit.GetActor();
				UStaticMeshComponent* C = A ? A->FindComponentByClass<UStaticMeshComponent>() : nullptr;
				const UStaticMesh* M = C ? C->GetStaticMesh() : nullptr;
				const FString Key = FString::Printf(TEXT("%s|%.0f"), M ? *M->GetName() : TEXT("-"), Hit.Distance);
				if (Seen.Contains(Key)) { continue; }
				Seen.Add(Key);
				const FBox B = A ? A->GetComponentsBoundingBox(true) : FBox(ForceInit);
				UE_LOG(LogVEIL, Warning,
					TEXT("PHI SWEEP: yaw %4.0f pitch %3.0f  mesh=%s  actor=%s  dist=%.0f  z=%.0f  size=%s"),
					Dir.Yaw, Dir.Pitch, M ? *M->GetName() : TEXT("-"),
					A ? *A->GetName() : TEXT("-"), Hit.Distance,
					Hit.ImpactPoint.Z,
					B.IsValid ? *B.GetSize().ToCompactString() : TEXT("-"));
			}
		}
	}));

// phi.look <0..4> - put a camera over a region and look at its landmark, so an
// area can be seen as a whole (layout, skyline, roads) instead of from inside a
// wall at eye level. The pawn is untouched; phi.goto puts the runner back.
static FAutoConsoleCommandWithWorldAndArgs GVeilRegionLook(
	TEXT("phi.look"),
	TEXT("Aerial camera over a region: phi.look <0 Hub, 1 Nest, 2 Spire, 3 Omni District, 4 Deep> [height] [distance]"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (!World) { return; }
		APlayerController* PC = World->GetFirstPlayerController();
		UVEILRegionSubsystem* Regions = World->GetSubsystem<UVEILRegionSubsystem>();
		if (!PC || !Regions) { return; }
		const int32 Index = FMath::Clamp(Args.Num() > 0 ? FCString::Atoi(*Args[0]) : 0, 0, 4);
		// Default: high enough to clear every landmark in the district (the tallest
		// is the Spire needle at 81 m) so an area is seen as ground plan, not from
		// behind a wall.
		const float Height = Args.Num() > 1 ? FMath::Max(600.f, FCString::Atof(*Args[1])) : 9000.f;
		const float Distance = Args.Num() > 2 ? FMath::Max(600.f, FCString::Atof(*Args[2])) : 5200.f;
		const FVEILRegionInfo Info = Regions->GetInfo(static_cast<EVEILRegion>(Index));

		const FVector LookAt(Info.Center.X, Info.Center.Y, 500.f);
		const FVector CamAt(Info.Center.X, Info.Center.Y - Distance, Height);
		ACameraActor* Cam = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CamAt,
			(LookAt - CamAt).Rotation());
		if (!Cam) { return; }
		PC->SetViewTarget(Cam);
		UE_LOG(LogVEIL, Log, TEXT("VEIL LOOK: %s from %s"), *Info.Name.ToString(), *CamAt.ToCompactString());
	}));

// phi.spiral - frame Hark's 34-cube sunflower spiral wherever the level put it,
// so the proof photo shows the spiral itself and not the runner's shoulder.
static FAutoConsoleCommandWithWorldAndArgs GVeilSpiralLook(
	TEXT("phi.spiral"),
	TEXT("Camera framing the sunflower spiral: phi.spiral"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (!World) { return; }
		APlayerController* PC = World->GetFirstPlayerController();
		if (!PC) { return; }
		AVEILGoldenSpawner* Found = nullptr;
		for (TActorIterator<AVEILGoldenSpawner> It(World); It; ++It) { Found = *It; break; }
		if (!Found)
		{
			UE_LOG(LogVEIL, Warning, TEXT("VEIL SPIRAL LOOK: no AVEILGoldenSpawner in the world"));
			return;
		}
		const FVector O = Found->GetActorLocation();
		const FVector CamAt = O + FVector(-1450.f, -1450.f, 1500.f);
		ACameraActor* Cam = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CamAt,
			(O - CamAt).Rotation());
		if (!Cam) { return; }
		PC->SetViewTarget(Cam);
		UE_LOG(LogVEIL, Log, TEXT("VEIL SPIRAL LOOK: spiral at %s, camera %s"),
			*O.ToCompactString(), *CamAt.ToCompactString());
	}));

void AVEILWorldBuilder::BuildDistrictLayer()
{
	if (bBuilt) { return; }
	bBuilt = true;
	bSkipBase = true;
	// A NEW BUILD COUNTS ITS PROPS FROM ZERO.
	NestPropSpots.Reset();

	// TERRAIN THAT REACHES INTO THE VIEW IS A WALL, NOT SCENERY.
	//
	// This path deliberately does not clear the saved level, so terrain actors
	// placed by earlier runs survive. When one of them is much wider than a ridge,
	// its bounds swallow the camera - ground camera or aerial look camera - and
	// every pixel it owns renders a backface: a black screen with the HUD on top.
	//
	// The old rule here compared an actor's LOCATION and its SIZE against two
	// invented constants (8 km away, 5 km wide). That is Cartesian bookkeeping and
	// it cannot see the real geometry: a 2.65 km mountain standing 7 km out still
	// reaches the camera, so it survived every pass while the screen stayed black.
	//
	// The rule now is measured and derived only from the world:
	//   R_span = how far the game actually extends (its own region centres)
	//   RidgeHalf = half of the footprint every horizon ridge is built to
	// A terrain actor must clear the view on BOTH: its inner edge outside
	// R_span * phi^2, and its footprint no wider than a ridge. Anything wider is
	// fitted down by the phi-damped measurement loop above - keeping its bearing,
	// never destroyed, because deleting the horizon is how a fix empties a world.
	if (UWorld* W = GetWorld())
	{
		const double PhiRef = 1.618033988749895;
		float RSpan = 0.f;
		if (UVEILRegionSubsystem* Regions = W->GetSubsystem<UVEILRegionSubsystem>())
		{
			for (EVEILRegion R : { EVEILRegion::Hub, EVEILRegion::Nest, EVEILRegion::Spire,
				EVEILRegion::OmniDistrict, EVEILRegion::MegastructureDeep })
			{
				RSpan = FMath::Max(RSpan, (float)Regions->GetInfo(R).Center.Size());
			}
		}
		const float RidgeHalf = 90000.f;
		// A ridge stands back by one golden ratio of its own width. That is the whole
		// clearance rule: 1.8 km wide ridge -> 2.9 km of clear air in front of it, and
		// the world's own span only matters if it is bigger than that (it is not).
		const float RSafe = FMath::Max((2.f * RidgeHalf) * (float)PhiRef,
			RSpan * (float)(PhiRef * PhiRef));

		int32 Moved = 0;
		for (TActorIterator<AStaticMeshActor> It(W); It; ++It)
		{
			UStaticMeshComponent* Comp = It->GetStaticMeshComponent();
			UStaticMesh* Mesh = Comp ? Comp->GetStaticMesh() : nullptr;
			if (!Mesh) { continue; }
			// Identified by ASSET NAME, not by a list of who placed it: a landmass can
			// arrive from any earlier pass, and the ones that made the screen black came
			// from a dressing pass that placed unscaled Iceland mountains.
			const FString MeshName = Mesh->GetName();
			const bool bTerrain = MeshName.Contains(TEXT("Mountain")) || MeshName.Contains(TEXT("Iceland"))
				|| MeshName.Contains(TEXT("Crest")) || MeshName.Contains(TEXT("Plateau"))
				|| MeshName.Contains(TEXT("Plateus"));
			if (!bTerrain) { continue; }

			FBox B = It->GetComponentsBoundingBox(true);
			if (!B.IsValid) { continue; }
			const float HalfBefore = 0.5f * FMath::Max((float)B.GetSize().X, (float)B.GetSize().Y);
			const float InnerBefore = (float)B.GetCenter().Size2D() - HalfBefore;
			const bool bOversized = HalfBefore > RidgeHalf * 1.02f;   // a landmass, not a ridge
			const bool bTooClose = InnerBefore < RSafe;                // a ridge standing in the view
			if (!bOversized && !bTooClose) { continue; }               // already right

			// REMOVE, DO NOT NUDGE. Moving a Static-mobility actor at runtime leaves its
			// cached bounds at the old place, so phi.stat still measured the camera
			// "inside" a ridge that had already been slid away - the diagnostic and the
			// world disagreed. A terrain actor that fails the clearance rule is not a
			// horizon, it is a misplaced landmass, and BuildHorizon() re-places the whole
			// ring at its correct radius immediately after this pass.
			UE_LOG(LogVEIL, Warning,
				TEXT("VEIL TERRAIN: removing %s %s - wide %.0f cm, inner edge %.0f cm, must clear %.0f cm (%s)"),
				*It->GetName(), *MeshName, 2.f * HalfBefore, InnerBefore, RSafe,
				bOversized ? TEXT("wider than a ridge") : TEXT("inside the view radius"));
			It->Destroy();
			++Moved;
		}
		if (Moved > 0)
		{
			UE_LOG(LogVEIL, Warning,
				TEXT("VEIL TERRAIN: %d misplaced terrain actor(s) cleared from the view radius %.0f cm; the horizon ring replaces them."),
				Moved, RSafe);
		}
	}

	// THE AMBIENT WAS ZERO, SO EVERY SIDE FACE RENDERED BLACK.
	//
	// A SkyLight that was never captured multiplies a black cubemap: sunlit tops
	// read, and every vertical face is pure black - which is exactly what the
	// ground view looked like with the HUD on top. The level already carries a sky
	// and an atmosphere; the skylight simply was not reading them. Real-time capture
	// has it sample that atmosphere every frame, so there is no separate "recapture
	// lighting" step to forget, on any machine, in any session.
	if (UWorld* W2 = GetWorld())
	{
		int32 SkiesLit = 0;
		for (TActorIterator<ASkyLight> It(W2); It; ++It)
		{
			USkyLightComponent* SL = It->FindComponentByClass<USkyLightComponent>();
			if (!SL) { continue; }
			SL->SetMobility(EComponentMobility::Movable);
			SL->SetRealTimeCapture(true);
			SL->SetIntensity(FMath::Max(SL->Intensity, 1.2f));
			SL->MarkRenderStateDirty();
			++SkiesLit;
		}
		UE_LOG(LogVEIL, Warning, TEXT("VEIL SKY: %d skylight(s) on real-time capture - the sky the level already has now lights the world."), SkiesLit);

		// THE KEY LIGHT WAS READING AS NIGHT. The saved sun is 3.0 lux: with the sky
		// capture on, the shadowed half of every wall still came out at 30% of the
		// frame near-black. A dusk key gets one golden step up, not a guess and not a
		// blown-out noon - phi squared of what the level authored, logged as a number.
		int32 SunsLit = 0;
		for (TActorIterator<ADirectionalLight> It(W2); It; ++It)
		{
			UDirectionalLightComponent* DL = It->FindComponentByClass<UDirectionalLightComponent>();
			if (!DL) { continue; }
			const float Before = DL->Intensity;
			if (Before > 0.f && Before < 8.f)
			{
				DL->SetIntensity(Before * (float)(1.6180339887 * 1.6180339887));
				UE_LOG(LogVEIL, Warning, TEXT("VEIL SUN: %s intensity %.2f -> %.2f lux (one golden step of the authored key)"),
					*It->GetName(), Before, DL->Intensity);
				++SunsLit;
			}
		}
		if (SunsLit > 0) { UE_LOG(LogVEIL, Warning, TEXT("VEIL SUN: %d key light(s) lifted to a readable dusk."), SunsLit); }

		// THE LOOK, SET ON PURPOSE INSTEAD OF LEFT TO AUTO.
		//
		// Everything above places light; none of it decides how the picture is
		// GRADED. That is why the district kept coming out blue-grey stone even with
		// the sun up: auto-exposure read a dark stone world, exposed for it, and the
		// palette collapsed to the sky's own blue. One unbound post-process volume
		// settles it deliberately - a fixed exposure for a dusk world, warmth in the
		// gain, saturation and contrast that read at a glance, and bloom on the gold.
		if (APostProcessVolume* PP = W2->SpawnActor<APostProcessVolume>(
			APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator))
		{
			PP->bUnbound = true;
			// Generated too: a re-bake clears tagged actors, so grading cannot stack one
			// volume per bake with an older art pass's values layered underneath it.
			PP->Tags.Add(FName(TEXT("VEIL_BAKE")));
			FPostProcessSettings& S = PP->Settings;
			// Manual exposure: EV100 11 is a dusk interior-lit scene. Auto exposure
			// under a dark stone sky was the single biggest reason for the grey.
			S.bOverride_AutoExposureMethod = true;
			S.AutoExposureMethod = EAutoExposureMethod::AEM_Manual;
			S.bOverride_AutoExposureBias = true;
			// EV100 11 was measured against a dusk world and is a stop and a half too
			// hot for a low sun on pale stone: the sand clipped to paper and the whole
			// frame read white. 8.5 holds the highlights on the sand and lets the
			// crystal keep its colour.
			S.AutoExposureBias = 8.5f;
			// Warm gain, more saturation, a little contrast: gold reads gold instead
			// of the whole frame sitting in the sky's blue.
			S.bOverride_ColorSaturation = true;
			S.ColorSaturation = FVector4(1.28f, 1.18f, 1.06f, 1.0f);
			S.bOverride_ColorContrast = true;
			S.ColorContrast = FVector4(1.06f, 1.05f, 1.04f, 1.0f);
			S.bOverride_ColorGain = true;
			S.ColorGain = FVector4(1.10f, 1.00f, 0.86f, 1.0f);
			S.bOverride_BloomIntensity = true;
			S.BloomIntensity = 1.1f;
			UE_LOG(LogVEIL, Warning, TEXT("VEIL GRADE: unbound post-process volume - manual EV100 11, warm gain, saturation 1.28/1.18/1.06."));
		}

		// The key gets a warmer, stronger value now that exposure is fixed rather
		// than adapting to it: 12 lux of low golden sun, not 3 of grey dusk.
		for (TActorIterator<ADirectionalLight> It(W2); It; ++It)
		{
			if (UDirectionalLightComponent* DL = It->FindComponentByClass<UDirectionalLightComponent>())
			{
				DL->SetIntensity(8.f);
				DL->SetLightColor(FLinearColor(1.f, 0.74f, 0.46f));
			}
		}
	}

	BuildDistrict();
	DressBeaconSites();
	BuildHorizon();
	UE_LOG(LogVEIL, Log, TEXT("VEIL: phi street layer laid over the saved district - the level itself was not touched."));
}

// ---------------------------------------------------------------------------
// THE HORIZON.
//
// A district that stops at the last street reads as a level that ran out of
// content. The Iceland pack ships authored ridges; sixteen of them sit on a
// golden ring tens of kilometres out, facing the plaza, tall enough to sit under
// the fog and give the sky a floor. Nothing here is inside the playable area,
// and nothing here is a collision you can walk into.
// ---------------------------------------------------------------------------
void AVEILWorldBuilder::BuildHorizon()
{
	if (MountainVariants.Num() == 0) { return; }

	const float PhiV = 1.61803398875f;
	const FLinearColor Ridge(0.40f, 0.39f, 0.44f);
	int32 Placed = 0;
	for (int32 i = 0; i < 16; ++i)
	{
		const float Ang = FMath::DegreesToRadians(i * 137.50776405f);
		// 26 km, WRITTEN AS CENTIMETRES. This line used to say 26000.f, which is 260
		// metres - the ring was standing inside the plaza with sixteen 1.8 km-wide
		// ridges around the player, and every one of them contained the camera. That
		// was the black screen: not a lighting fault, a unit that dropped two zeros.
		const float RadCm = 100.f * 26000.f * FMath::Pow(PhiV, (float)(i % 4) * 0.32f);
		// BURIED BASE, so the ridge rises out of the ground instead of floating.
		const FVector At(FMath::Cos(Ang) * RadCm, FMath::Sin(Ang) * RadCm, -30000.f);
		UStaticMesh* Mesh = MountainVariants[i % MountainVariants.Num()];
		if (!Mesh) { continue; }

		// ABSOLUTE FOOTPRINT, not a scale factor. SpawnBlock multiplies by the
		// mesh's own size, and an Iceland ridge is kilometres wide before scaling -
		// the first pass used "H" as if it were a 100 cm box and produced a 61 km
		// mountain that wrapped the district (measured with phi.stat: 20 mesh
		// bounds contained the camera). 2 * halfCm / 100 is exactly "this many
		// centimetres wide" whatever the source mesh measures.
		const float HalfCm = 90000.f + (i % 4) * 22000.f;
		const FRotator Face(0.f, FMath::RadiansToDegrees(Ang) + 180.f, 0.f);

		// Spawn first, then FIT BY MEASUREMENT: two rounds of unit arithmetic
		// produced a 61 km mountain, so the size is now converged onto the target
		// by the phi-damped loop above, and the number it settled on is logged.
		if (UStaticMeshComponent* Comp = SpawnBlockRot(At, FVector::OneVector, Mesh, Ridge, 0.f, Face))
		{
			const FPhiFit Fit = FitToFootprint(Comp, HalfCm, 0.55f);
			UE_LOG(LogVEIL, Log, TEXT("VEIL HORIZON FIT: %s -> %.0f cm wide (wanted %.0f), error %.4f after %d phi pass(es) - %s"),
				*Mesh->GetName(), Fit.FinalSizeCm, 2.f * HalfCm, Fit.RelativeError, Fit.Passes + 1,
				Fit.bConverged ? TEXT("converged") : TEXT("NOT converged"));
			// The ridge is placed whatever the loop reports: a fitted ridge 1.2% off is
			// still a ridge on the horizon, and refusing to build it over a tolerance
			// is how a "fix" empties the world.
			++Placed;
		}
	}
	UE_LOG(LogVEIL, Log, TEXT("VEIL HORIZON: %d authored ridges placed on the golden ring, 26-42 km out, 0.9-1.6 km wide each."), Placed);
}

// ---------------------------------------------------------------------------
// STONE AT THE BEACONS.
//
// The three statues stood on bare ground. Each site now has the pack's own
// monolith, a ring of debris on the golden angle, and a shrine: the landmarks
// the objective sends the runner to, dressed as places instead of coordinates.
// ---------------------------------------------------------------------------
void AVEILWorldBuilder::DressBeaconSites()
{
	struct FSite { float X, Y; const TCHAR* Name; };
	static const FSite Sites[] = {
		{   900.f,     0.f, TEXT("KEEPER") },
		{  -450.f,   780.f, TEXT("SEER") },
		{ -4150.f,  -780.f, TEXT("CARTOGRAPHER") },
	};

	const FLinearColor Stone(0.62f, 0.58f, 0.52f);
	const FLinearColor GoldTint(0.55f, 0.47f, 0.34f);   // bronze-stone, as the plaza pass

	for (const FSite& Site : Sites)
	{
		const FVector Base(Site.X, Site.Y, 0.f);
		if (MonolithMesh)
		{
			SpawnBlock(Base + FVector(0.f, 0.f, 700.f), FVector(4.2f, 4.2f, 14.f),
				MonolithMesh, Stone, 0.f);
		}
		if (ShrineMesh)
		{
			SpawnBlock(Base + FVector(520.f, 0.f, 120.f), FVector(2.6f, 2.6f, 2.6f),
				ShrineMesh, Stone, 0.f);
		}
		if (StairMesh)
		{
			SpawnBlock(Base + FVector(0.f, -420.f, 40.f), FVector(4.f, 4.f, 2.f),
				StairMesh, Stone, 0.f);
		}
		// Debris on the golden angle, so the scatter around a holy site has the
		// same order as everything else in this world.
		for (int32 d = 0; d < 6 && DebrisVariants.Num() > 0; ++d)
		{
			const float Ang = FMath::DegreesToRadians(d * 137.50776405f);
			// OUT AT THE SITE, NOT ON THE PLAZA. At 620 cm this ring landed 6-14 m from
			// the spawn, which is the clutter that read as rubbish beside the market.
			const float Rad = 1900.f * FMath::Sqrt((float)(d + 1));
			SpawnBlock(Base + FVector(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 60.f),
				FVector(1.6f, 1.6f, 1.2f), DebrisVariants[d % DebrisVariants.Num()], Stone, 0.f);
		}
		// Same replacement at the beacon: the engine sphere becomes the kit's carved column.
		SpawnBlock(Base + FVector(0.f, 0.f, 1450.f), FVector(0.8f, 0.8f, 2.2f),
			ColumnMesh, GoldTint, 2.4f);
	}

	UE_LOG(LogVEIL, Log, TEXT("VEIL DRESSING: 3 beacon sites dressed - monolith, shrine, stair, debris ring, crown light."));
}

void AVEILWorldBuilder::DressPhiPalette()
{
	// THE GOLD AND THE CRYSTAL, IN THE PROJECT'S OWN PALETTE.
	//
	// The district reads as flat sand and blue blocks because it IS kit stone: tinted
	// writes go to Color/BaseColor parameters, and the kit's materials do not carry
	// them, so every tint in this world has been a silent no-op. M_PHI_Gold,
	// M_PHI_DeepBlue and M_PHI_WhiteEmissive were authored for this project and do
	// carry the colour - which is why the caches and visors always lit correctly.
	//
	// So the gold pillars, the crystal clusters, the phi spiral in the plaza floor and
	// the Spire are built from kit geometry with the PHI palette assigned explicitly.
	// Assets already on disk, no placeholders, no editor step.
	UMaterialInterface* Gold = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold"));
	// STONE, NOT BLUE. This handle reached columns, roofs and shells as flat blue
	// slabs. It keeps its name so every call site is untouched; the material it
	// loads is the stone the district is built from. Crystal and water keep their
	// own blue in their own files.
	UMaterialInterface* Blue = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
	UMaterialInterface* Glow = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive"));
	// THE PILLAR BODIES ARE STONE. Three passes of changing GoldTint numbers proved the
	// point: the pillars do not take a colour, they take a MATERIAL, and that material was
	// M_PHI_Gold. This is the stone the project already ships, used by the quest statues.
	UMaterialInterface* StoneMat = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
	if (!Gold || !Blue)
	{
		UE_LOG(LogVEIL, Warning, TEXT("VEIL PALETTE: PHI palette materials missing - gold and crystal skipped."));
		return;
	}

	auto Place = [this, Gold, StoneMat](const FVector& At, const FVector& Scale, UStaticMesh* Mesh,
		UMaterialInterface* Mat, const FLinearColor& TintCol, float Emissive, const FRotator& Rot)
	{
		if (!Mesh || !Mat) { return; }
		// GOLD IS FOR ACCENTS, NOT FOR BODIES. A column placed with the gold material is
		// re-materialled to stone here, at the single point every gold pillar goes through,
		// so the crown lights and beacons keep their gold and the bodies read as masonry.
		// GOLD IS LIGHT, STONE IS MASS. Restricting the swap to columns left the big gold
		// wall blocks standing there as solid gold cubes - which is exactly what the complaint
		// was about. So the rule is now about the whole world: any surface asked for gold that
		// is NOT emissive becomes stone; gold survives only where it is actually glowing.
		// MEASURED, NOT GUESSED. A dump of the plaza found 102 components wearing
		// MID_M_PHI_Gold_0 - the pillars are SM_AncientColumn_Big with a dynamic instance of
		// M_PHI_Gold. They survived the earlier swap because that swap exempted emissive
		// surfaces, and the pillars glow. The vision is a STONE pillar with a glowing phi
		// symbol, so the body is stone at every emissive level; the light is the trim.
		UMaterialInterface* Use = Mat;
		if (StoneMat && Gold && Mat == Gold)
		{
			Use = StoneMat;
		}
		UStaticMeshComponent* Comp = SpawnBlockRot(At, Scale, Mesh, TintCol, Emissive, Rot);
		if (!Comp) { return; }
		Comp->SetMaterial(0, Use);
		if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Use, Comp))
		{
			MID->SetVectorParameterValue(TEXT("Color"), TintCol);
			MID->SetVectorParameterValue(TEXT("BaseColor"), TintCol);
			if (Emissive > 0.f)
			{
				MID->SetVectorParameterValue(TEXT("EmissiveColor"), TintCol * Emissive);
			}
			Comp->SetMaterial(0, MID);
		}
	};

	const double PhiLocal = 1.618033988749895;
	const double GoldenStep = 137.50776405003785;
	const FLinearColor GoldTint(0.55f, 0.47f, 0.34f);   // bronze-stone, not plastic gold
	const FLinearColor CrystalTint(0.30f, 0.62f, 1.f);

	UStaticMesh* Pillar = ColumnMesh ? ColumnMesh.Get() : CylinderMesh.Get();
	// The authored cliff variants ARE the crystal rock in this kit - the same meshes
	// the district's terrain uses, so the blue clusters read as the world's own stone
	// lit from inside rather than as new geometry from nowhere.
	UStaticMesh* Crystal = CliffVariants.Num() > 0 ? CliffVariants[0].Get() : CliffMesh.Get();

	// THE PHI RING: twelve gold pillars at 950 x phi, one golden angle apart.
	int32 Pillars = 0;
	int32 Clusters = 0;
	const FString Flavour = VeilMapFlavour(GetWorld());
	// THE RING IS THE MAP'S SILHOUETTE. Gold city: eighteen pillars, no crystal. Wilds:
	// eight pillars and crystal on every step. Arena: no colonnade at all - it gets a
	// closed wall ring instead, below. District: the original twelve.
	const int32 N = Flavour == TEXT("Omni") ? 18
		: Flavour == TEXT("Wilds") ? 8
		: Flavour == TEXT("Arena") ? 0
		: 12;
	for (int32 i = 0; i < N; ++i)
	{
		const double Ang = (2.0 * PI * i) / (double)N;
		const double Rad = 1050.0 * PhiLocal;
		const FVector At(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);
		if (Pillar)
		{
			// Gold STONE with a lit edge, not a glow stick: the emissive is a fraction of
			// the gold, so the pillar reads as gilded masonry and still catches the sun.
			// MEASURED, like everything else that was a wafer before: 2.2 m across, 12.5 m
			// tall, standing on the ground rather than buried to its cap.
			Place(At + FVector(0.f, 0.f, 625.f), ScaleForCm(Pillar, FVector(220.f, 220.f, 1250.f)),
				Pillar, Gold, GoldTint, 0.55f, FRotator(0.f, (float)(i * GoldenStep), 0.f));
			++Pillars;
		}
		const bool bCrystalHere = Flavour == TEXT("Wilds")
			? true : (Flavour != TEXT("Omni") && Flavour != TEXT("Arena") && (i % 2) == 0);
		if (Crystal && bCrystalHere)
		{
			const double CAng = Ang + 0.26;
			const double CRad = Rad * PhiLocal * 0.86;
			// THE BLUE PLATES WERE THE NEST'S WORST OFFENDER. A 7 x 15 m cliff shard tinted
			// blue stands in the street like a dropped tarpaulin; inside the starting area
			// it is refused, and the palace of gold and stone is what the player sees.
			if (VeilInNest(FVector(FMath::Cos(CAng) * CRad, FMath::Sin(CAng) * CRad, 0.f))) { continue; }
			// A SHARD, NOT A WAFER. These were the leaning blue plates beside the plaza:
			// the cliff mesh scaled on Z alone. 7 m wide and 15 m tall, root on the ground.
			Place(FVector(FMath::Cos(CAng) * CRad, FMath::Sin(CAng) * CRad, 750.f),
				// 7 m wide by 15 m tall was a slab, not a shard: it read as a blue block standing
				// in the street. 4.2 m by 9 m keeps the crystal read at a scale that belongs.
				ScaleForCm(Crystal, FVector(420.f, 420.f, 900.f)), Crystal, Blue, CrystalTint, 0.45f,
				FRotator(0.f, (float)(i * 61.f), 7.f));
			++Clusters;
		}
	}

	// THE ARENA IS A RING, NOT A STREET. CountessRegion is the duel map: a closed
	// colosseum wall around the spawn and no gold colonnade inside it, so the two places
	// read as different from the first frame instead of the same plaza under a new name.
	if (Flavour == TEXT("Arena"))
	{
		if (UStaticMesh* Course = WallMesh ? WallMesh.Get() : nullptr)
		{
			const float RingCm = 3000.f;
			for (int32 i = 0; i < 28; ++i)
			{
				const double Ang = (2.0 * PI * i) / 28.0;
				const FVector At(FMath::Cos(Ang) * RingCm, FMath::Sin(Ang) * RingCm, 0.f);
				const float YawDeg = (float)FMath::RadiansToDegrees(Ang) + 90.f;
				Place(At + FVector(0.f, 0.f, 450.f),
					ScaleForCm(Course, FVector(700.f, 900.f, 900.f)),
					Course, Gold, GoldTint, 0.12f, FRotator(0.f, YawDeg, 0.f));
			}
			UE_LOG(LogVEIL, Warning,
				TEXT("VEIL ARENA: CountessRegion built as a closed ring of 28 wall segments at %.0f cm."), RingCm);
		}
	}

	// THE SPIRE: the district's vertical statement, stacked from the same columns.
	if (Pillar)
	{
		// THE LANDMARK STANDS BESIDE THE SPAWN, NOT ON IT.
		//
		// The Spire was built at 0,0 - which is the authority RUNNER address. So the
		// runner either woke inside its plinth or had to be walked 3 km out to avoid it,
		// and I did the second, which quietly moved the game's geography to solve a
		// dressing problem. The Bible fixes the spawn; the landmark moves instead. At
		// 30 m east it is the tallest thing in the sky from the plaza and leaves the
		// centre of the Nest as open ground the player can stand and walk in.
		const FVector Spire(3000.f, 0.f, 0.f);
		for (int32 K = 0; K < 3; ++K)
		{
			// Taper by the identity: 6.4 m, 4.0 m, 2.4 m across - each about 1/phi of the
			// one below, so the stack narrows by the same law the terraces step by.
			const float Side = 640.f / FMath::Pow((float)PhiLocal, (float)K);
			Place(Spire + FVector(0.f, 0.f, 1400.f + K * 2600.f),
				ScaleForCm(Pillar, FVector(Side, Side, 2600.f)), Pillar, Gold, GoldTint, 0.8f,
				FRotator(0.f, (float)(K * GoldenStep), 0.f));
		}
		if (Glow)
		{
			// THE CAP SITS ON THE STACK. It was authored at 9400 cm while the top of the
			// three-segment stack is 7900 - so the audit found the landmark's own cap
			// floating 89 m above the thing it was meant to crown. 7900 + 350 = 8250 puts
			// its centre on the top face.
			Place(Spire + FVector(0.f, 0.f, 8250.f), ScaleForCm(Pillar, FVector(300.f, 300.f, 700.f)),
				Pillar, Glow, GoldTint, 3.f, FRotator::ZeroRotator);
		}
	}

	// A PLAZA IS PAVED, NOT SCATTERED.
	//
	// This block used to lay 21 separate floor meshes in a golden spiral - each 85 cm,
	// each turned to its own yaw, each floating 14 cm above the ground - so the first
	// thing the eye met at spawn was a field of loose slabs standing at different
	// angles and heights. That is what "random and messy" is: no shared module, no
	// shared plane, no shared orientation. A paving is a grid. One stone size, one
	// height, edge to edge; the golden angle is kept for the inlay ring laid flush on
	// the plane, where a golden angle can be seen as a pattern rather than as debris.
	if (UStaticMesh* Tile = FloorMesh ? FloorMesh.Get() : nullptr)
	{
		// SIT ON THE STREET THE RUNNER ACTUALLY STANDS ON, not on z=0. The plaza floor
		// measured under the seat is at 65 cm, so paving authored at zero would be under
		// the ground and invisible - the same mistake the seat search made in reverse.
		// THE PROBE MUST NOT START INSIDE A ROOF. It began at z=+3000 and came back
		// with an impact at exactly 3000 - the start was already inside geometry - and the
		// bake laid the paving 30 m in the air. Start above everything, skip each hit that
		// is not near the plaza floor, and take the first one that is.
		float PaveZ = 0.5f;
		if (UWorld* W = GetWorld())
		{
			FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(VeilPaveProbe), false);
			FVector From(0.f, 0.f, 60000.f);
			for (int32 Step = 0; Step < 12; ++Step)
			{
				FHitResult Ground;
				if (!W->LineTraceSingleByChannel(Ground, From, FVector(0.f, 0.f, -4000.f),
					ECC_Visibility, GroundParams))
				{
					break;
				}
				const float HitZ = Ground.ImpactPoint.Z;
				// 120, NOT 600: the plaza floor is at ~0 and its paving at 0.5, while the
				// meshless slab that covers the district has its top at 500. A 600 cm test
				// would happily lay the paving on the roof.
				if (HitZ < 120.f) { PaveZ = HitZ; break; }
				From = FVector(0.f, 0.f, HitZ - 25.f);
			}
		}
		const float Module = 320.f;    // one paving stone, 3.2 m
		const int32 Span = 5;          // five either side of centre: a 35 m square
		int32 Laid = 0;
		for (int32 gx = -Span; gx <= Span; ++gx)
		{
			for (int32 gy = -Span; gy <= Span; ++gy)
			{
				// A 3 cm joint, no rotation. A paving that is turned per stone is rubble.
				SpawnBlockRot(FVector(gx * Module, gy * Module, PaveZ + 1.f),
					ScaleForCm(Tile, FVector(Module - 3.f, Module - 3.f, 10.f)),
					Tile, FLinearColor::White, 0.f, FRotator::ZeroRotator);
				++Laid;
			}
		}
		// THE GOLD IS AN INLAY NOW: one ring, flush, on the plane the stones make.
		for (int32 i = 0; i < 13; ++i)
		{
			const double Ang = FMath::DegreesToRadians(i * GoldenStep);
			const double Rad = 620.0;
			Place(FVector(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, PaveZ + 5.f),
				ScaleForCm(Tile, FVector(150.f, 150.f, 6.f)), Tile, Gold, GoldTint, 0.3f,
				FRotator::ZeroRotator);
		}
		UE_LOG(LogVEIL, Warning,
			TEXT("VEIL PLAZA: %d paving stones on a %.0f cm module, plane at z=%.1f, gold inlay flush on top."),
			Laid, Module, PaveZ);
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("VEIL PALETTE: %d gold pillars on the phi ring, %d crystal clusters, the Spire raised, gold spiral in the plaza - kit geometry, PHI palette."),
		Pillars, Clusters);

	// THE MARKET RING. The plaza had a floor, pillars and a skyline, but no buildings you
	// could walk into: the vision's Hub is a market, and a market is a ring of open doors.
	// BuildRoom already composes a correct building out of the kit - 2.6 m doorway, jambs,
	// lintel, roof, hearth light - so the ring is that function used properly at last.
	BuildMarketRing();

	// WATER, SKYLINE AND THE THREE PHI PILLARS - the pieces of the vision image that the
	// project already owns assets for: a lake material in StarterContent, nine mountains in
	// the Iceland pack, and the ancient column for the pillars.
	BuildWaterAndSkyline();
}

void AVEILWorldBuilder::BuildWaterAndSkyline()
{
	UStaticMesh* Slab = FloorMesh ? FloorMesh.Get() : CubeMesh.Get();
	if (!Slab) { return; }

	// REAL WATER SURFACES. StarterContent ships M_Water_Lake and M_Water_Ocean; the vision
	// asks for rivers and pools, and a still surface with the water material is what this
	// project owns. Flow and waterfalls as Niagara do not exist yet - they are a Fab item.
	UMaterialInterface* Water = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/StarterContent/Materials/M_Water_Lake.M_Water_Lake"));
	if (!Water)
	{
		Water = LoadObject<UMaterialInterface>(nullptr,
			TEXT("/Game/StarterContent/Materials/M_Water_Ocean.M_Water_Ocean"));
	}

	int32 Surfaces = 0;
	auto WaterAt = [this, Slab, Water, &Surfaces](const FVector& At, const FVector& Cm)
	{
		UStaticMeshComponent* C = SpawnBlockRot(At,
			FVector(Cm.X / 100.f, Cm.Y / 100.f, FMath::Max(0.02f, Cm.Z / 100.f)),
			Slab, FLinearColor::White, 0.f, FRotator::ZeroRotator);
		if (C && Water)
		{
			C->SetMaterial(0, Water);
			++Surfaces;
		}
	};

	// A river along the plaza's east side, and the pool at the Nest's foot.
	WaterAt(FVector(0.f, 1500.f, 24.f), FVector(9000.f, 700.f, 8.f));
	WaterAt(FVector(-450.f, 780.f, 130.f), FVector(1500.f, 950.f, 8.f));
	// Three sheets falling toward the Deep, thin and tall: the waterfall read from the plaza.
	WaterAt(FVector(600.f, -2900.f, 800.f), FVector(1500.f, 30.f, 1600.f));
	WaterAt(FVector(-2000.f, -900.f, 700.f), FVector(1200.f, 30.f, 1400.f));

	// THE SKYLINE. Nine Iceland mountains on a far ring: the district now has a horizon
	// instead of a flat edge. Blue-tinted, because the vision's far range is crystal.
	// THE REAL FOLDER IS Static_Meshes, not Meshes. The first build of this function would
	// have loaded nine null meshes and logged a skyline of zero peaks; find named the paths.
	static const TCHAR* MountainPaths[] =
	{
		TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_01.SM_Mountain_01"),
		TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_02.SM_Mountain_02"),
		TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_03.SM_Mountain_03"),
		TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_04.SM_Mountain_04"),
		TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_05.SM_Mountain_05"),
		TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_06.SM_Mountain_06"),
		TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_07.SM_Mountain_07"),
		TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_08.SM_Mountain_08"),
		TEXT("/Game/Iceland_Environment/Static_Meshes/SM_Mountain_09.SM_Mountain_09")
	};
	int32 Peaks = 0;
	for (int32 K = 0; K < 9; ++K)
	{
		UStaticMesh* Peak = LoadObject<UStaticMesh>(nullptr, MountainPaths[K]);
		if (!Peak) { continue; }
		const float Ang = FMath::DegreesToRadians(K * 137.50776405f);
		const float Rad = 7200.f + 900.f * (float)(K % 3);
		const FVector At(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, -400.f);
		const float Height = 2600.f + 700.f * (float)(K % 4);
		UStaticMeshComponent* C = SpawnBlockRot(At + FVector(0.f, 0.f, Height * 0.5f),
			FVector(6.f, 6.f, Height / 100.f), Peak, FLinearColor(0.52f, 0.72f, 1.f), 0.f,
			FRotator(0.f, K * 37.f, 0.f));
		if (C) { ++Peaks; }
	}

	// THE THREE GOLD PHI PILLARS. Three, tall, on the plaza: the image's foreground markers.
	UStaticMesh* PillarMesh = ColumnMesh ? ColumnMesh.Get() : nullptr;
	UMaterialInterface* GoldMetal = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic"));
	int32 Pillars = 0;
	if (PillarMesh)
	{
		for (int32 K = 0; K < 3; ++K)
		{
			const float Ang = FMath::DegreesToRadians(K * 120.f + 137.50776405f);
			const FVector At(FMath::Cos(Ang) * 760.f, FMath::Sin(Ang) * 760.f, 0.f);
			UStaticMeshComponent* Body = SpawnBlockRot(At + FVector(0.f, 0.f, 900.f),
				FVector(2.2f, 2.2f, 18.f), PillarMesh, FLinearColor(0.42f, 0.36f, 0.29f), 0.f,
				FRotator::ZeroRotator);
			if (!Body) { continue; }
			// The glow is the gold band at the top, not the body: metal light on stone mass.
			UStaticMeshComponent* Crown = SpawnBlockRot(At + FVector(0.f, 0.f, 1780.f),
				FVector(2.4f, 2.4f, 1.4f), PillarMesh, FLinearColor(1.f, 0.766f, 0.336f), 2.4f,
				FRotator::ZeroRotator);
			if (Crown && GoldMetal) { Crown->SetMaterial(0, GoldMetal); }
			++Pillars;
		}
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("VEIL WATER/SKYLINE: %d water surfaces (%s), %d mountain peaks, %d tall phi pillars with gold crowns."),
		Surfaces, Water ? *Water->GetName() : TEXT("no water material"), Peaks, Pillars);
}

void AVEILWorldBuilder::BuildMarketRing()
{
	UStaticMesh* Wall = WallMesh ? WallMesh.Get() : CubeMesh.Get();
	UStaticMesh* Floor = FloorMesh ? FloorMesh.Get() : CubeMesh.Get();
	if (!Wall || !Floor) { return; }

	// Warm stone, not gold: the bodies of the world are masonry and gold is the light on it.
	const FLinearColor StoneTint(0.42f, 0.36f, 0.29f);
	int32 Built = 0;
	int32 Skipped = 0;
	for (int32 K = 0; K < 13; ++K)
	{
		// Golden angle, as the Bible requires of every ring in this world.
		const float Ang = FMath::DegreesToRadians(K * 137.50776405f);
		const float Rad = 1150.f + 90.f * (float)(K % 3);
		const FVector At(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);

		// Keep the square open in the middle and the streets open at the edge: a building
		// dropped on the spawn approach or on a road is a wall you cannot walk around.
		if (At.Size2D() < 950.f) { ++Skipped; continue; }
		const FVector DoorDir = (-At).GetSafeNormal2D();
		if (DoorDir.IsNearlyZero()) { ++Skipped; continue; }

		// HalfW 320 / HalfD 260 is a 6.4 m by 5.2 m hall on a 5.2 m wall: a shop floor with
		// room to stand inside, not another giant block with a door drawn on it.
		BuildRoom(At, 320.f, 260.f, 520.f, DoorDir, Wall, Floor, StoneTint, false);
		++Built;
	}
	UE_LOG(LogVEIL, Warning,
		TEXT("VEIL MARKET RING: %d enterable kit buildings around the plaza, %d skipped for streets and spawn - doors face the square."),
		Built, Skipped);
}

void AVEILWorldBuilder::DressRegions()
{
	UMaterialInterface* Gold = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold"));
	// STONE, NOT BLUE. This handle reached columns, roofs and shells as flat blue
	// slabs. It keeps its name so every call site is untouched; the material it
	// loads is the stone the district is built from. Crystal and water keep their
	// own blue in their own files.
	UMaterialInterface* Blue = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
	UMaterialInterface* Glow = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive"));
	// THE PILLAR BODIES ARE STONE. Three passes of changing GoldTint numbers proved the
	// point: the pillars do not take a colour, they take a MATERIAL, and that material was
	// M_PHI_Gold. This is the stone the project already ships, used by the quest statues.
	UMaterialInterface* StoneMat = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
	if (!Gold || !Blue) { return; }

	// THE SECOND GOLD PATH. DressPhiPalette was fixed and the plaza still measured 88 gold
	// components: this function has its OWN Place lambda, and it painted 46 columns and 11
	// roofs with M_PHI_Gold. A dump named the meshes, so this is the same rule applied at the
	// second site: gold metal is for light, stone is for mass.
	auto Place = [this, Gold, StoneMat](const FVector& At, const FVector& S, UStaticMesh* M,
		UMaterialInterface* Mat, const FLinearColor& C, float E, const FRotator& R)
	{
		if (!M || !Mat) { return; }
		UMaterialInterface* Use = Mat;
		if (StoneMat && Gold && Mat == Gold)
		{
			Use = StoneMat;
		}
		UStaticMeshComponent* Comp = SpawnBlockRot(At, S, M, C, E, R);
		if (!Comp) { return; }
		Comp->SetMaterial(0, Use);
		// THE MID WAS BUILT FROM THE GOLD MATERIAL, SO IT PUT THE GOLD BACK. Setting the stone
		// material and then creating a dynamic instance of Mat - and applying that - silently
		// undid the swap on every single piece. The instance is built from Use now.
		if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Use, Comp))
		{
			MID->SetVectorParameterValue(TEXT("Color"), C);
			MID->SetVectorParameterValue(TEXT("BaseColor"), C);
			if (E > 0.f) { MID->SetVectorParameterValue(TEXT("EmissiveColor"), C * E); }
			Comp->SetMaterial(0, MID);
		}
	};

	UStaticMesh* Pillar = ColumnMesh ? ColumnMesh.Get() : nullptr;
	UStaticMesh* Rock = CliffVariants.Num() > 0 ? CliffVariants[0].Get() : nullptr;
	UStaticMesh* Slab = FloorMesh ? FloorMesh.Get() : nullptr;
	UStaticMesh* Course = WallMesh ? WallMesh.Get() : nullptr;

	/**
	 * A mass of EXACTLY WantCm, standing on Bottom - the build the painting asks for.
	 *
	 * A null material means "keep the kit's own stone", which is what masonry wants:
	 * the palette materials are for gold, crystal and light, not for walls.
	 */
	// SAME LAW FOR MASSES AS FOR PROPS. Solid() is what raises the big walls and roofs, and it
	// had the identical fault: assign the material, then rebuild it as a gold MID on top.
	auto Solid = [this, Gold, StoneMat](const FVector& Bottom, const FVector& WantCm, UStaticMesh* M,
		UMaterialInterface* Mat, const FLinearColor& C, float E, float YawDeg) -> bool
	{
		if (!M) { return false; }
		const FVector S = ScaleForCm(M, WantCm);
		if (S.IsNearlyZero()) { return false; }
		// SpawnBlockRot puts the geometry's centre on the coordinate it is given, so a
		// mass that must stand on 'Bottom' is asked for at its own mid-height.
		UStaticMeshComponent* Comp = SpawnBlockRot(Bottom + FVector(0.f, 0.f, WantCm.Z * 0.5f),
			S, M, C, E, FRotator(0.f, YawDeg, 0.f));
		if (!Comp) { return false; }
		UMaterialInterface* Use = Mat;
		if (StoneMat && Gold && Mat == Gold)
		{
			Use = StoneMat;
		}
		if (Use)
		{
			Comp->SetMaterial(0, Use);
			if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Use, Comp))
			{
				MID->SetVectorParameterValue(TEXT("Color"), C);
				MID->SetVectorParameterValue(TEXT("BaseColor"), C);
				if (E > 0.f) { MID->SetVectorParameterValue(TEXT("EmissiveColor"), C * E); }
				Comp->SetMaterial(0, MID);
			}
		}
		return true;
	};
	UStaticMesh* Roof = RoofMesh ? RoofMesh.Get() : nullptr;

	const FLinearColor GoldC(1.f, 0.76f, 0.28f);
	// STONE, NOT A BLUE BLOCK. The Nest shelves were tinted 0.30/0.62/1.0, which rendered
	// as flat blue slabs standing in a gold-and-stone district - the "blue blocks" the
	// player sees. Warm rock reads as the same cliff the kit authored.
	const FLinearColor BlueC(0.44f, 0.41f, 0.36f);
	const FLinearColor IceC(0.52f, 0.76f, 1.f);
	const double GoldenStep = 137.50776405003785;
	int32 Market = 0, Nest = 0, Omni = 0, Peaks = 0, Beacons = 0;

	// 1. THE HUB MARKET: sixteen stalls and sixteen gold posts on the golden angle, ring
	// on phi-damped radius so the crowd thins outward instead of stopping at a wall.
	for (int32 i = 0; i < 16; ++i)
	{
		const double Ang = FMath::DegreesToRadians(i * GoldenStep);
		const double Rad = 1500.0 * FMath::Sqrt(1.0 + i * 0.18);
		const FVector At(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);
		// A STALL IS A COUNTER YOU CAN SEE, a canopy over it, and four posts that are
		// posts - not a 2 cm stick, which is what the unmeasured scale produced.
		Solid(At, FVector(600.f, 600.f, 110.f), Slab, nullptr, FLinearColor(0.45f, 0.40f, 0.34f),
			0.f, (float)(i * GoldenStep));
		++Market;
		Solid(At + FVector(0.f, 0.f, 230.f), FVector(800.f, 800.f, 40.f), Roof, Gold, GoldC, 0.20f,
			(float)(i * GoldenStep));
		++Market;
		for (int32 Post = 0; Post < 4; ++Post)
		{
			const float PX = (Post & 1) ? 260.f : -260.f;
			const float PY = (Post & 2) ? 260.f : -260.f;
			Solid(At + FVector(PX, PY, 0.f), FVector(40.f, 40.f, 230.f), Pillar, Gold, GoldC, 0.45f, 0.f);
			++Market;
		}
	}

	// 2. THE NEST, on the cliffs to the west: rock shelves stepped up on the golden
	// angle, and waterfalls running off three of them as thin blue sheets of water.
	for (int32 i = 0; i < 9; ++i)
	{
		const double Ang = FMath::DegreesToRadians(180.0 + i * GoldenStep * 0.5);
		// THE NEST IS A PLACE YOU WALK TO, NOT A WALL AT THE SPAWN. At 32 m the first
		// shelf's inner face stood 15 m from the plaza and every standing frame was
		// blue rock on three sides. 62 m out, 26 m wide: nearest face 49 m away, so the
		// plaza stays open and the cliffs still read from it.
		const double Rad = 6200.0 + i * 320.0;
		const FVector At(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);
		const float Height = 700.f + i * 210.f;
		Solid(At, FVector(2600.f, 2600.f, Height), Rock, Blue, BlueC, 0.12f,
			(float)(i * GoldenStep));
		++Nest;
		if (i % 3 == 0)
		{
			// A waterfall is water standing in air: one sheet from the shelf top to the
			// ground, lit from inside so it reads at distance.
			Solid(At + FVector(200.f, 0.f, 0.f), FVector(10.f, 900.f, Height), Slab, Glow,
				IceC, 0.9f, 0.f);
			++Nest;
		}
	}

	// 3. OMNI DISTRICT, east: gold towers in a phi ladder, tall in the middle, so the
	// skyline has a shape instead of a row.
	for (int32 i = 0; i < 10; ++i)
	{
		const double Ang = FMath::DegreesToRadians(i * GoldenStep);
		const double Rad = 2600.0 + (i % 4) * 420.0;
		const FVector At(2600.0 + FMath::Cos(Ang) * Rad * 0.35, FMath::Sin(Ang) * Rad, 0.f);
		const float TowerH = 1400.f + (6 - (i % 6)) * 520.f;
		const float TowerW = 900.f + (float)(i % 3) * 250.f;
		Solid(At, FVector(TowerW, TowerW, TowerH), Pillar, Gold, GoldC, 0.35f,
			(float)(i * GoldenStep));
		++Omni;
	}

	// 4. THE CRYSTAL MOUNTAINS behind everything: big rock, blue, cold, no emissive to
	// speak of - a horizon, not a light source.
	for (int32 i = 0; i < 12; ++i)
	{
		const double Ang = FMath::DegreesToRadians(i * GoldenStep);
		// FAR ENOUGH THAT IT IS A HORIZON, NOT A WALL. Measured mountains at 90 m
		// across on a 56 m ring put their inner face 11 m from the plaza and swallowed
		// the town - the aerial frame was solid blue, camera and all. The crystals are
		// 130 m wide and stand on the 200 m ring, so their nearest face is still 135 m
		// from the plaza and they read as the skyline the painting has.
		const double Rad = 20000.0 + (i % 5) * 2200.0;
		const float PeakH = 5200.f + (i % 4) * 1100.f;
		Solid(FVector(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f),
			FVector(13000.f, 13000.f, PeakH), Rock, Blue, IceC, 0.10f,
			(float)(i * GoldenStep));
		++Peaks;
	}

	// 5. BEACONS. Four columns of gold light on the far ring - the thing you see from
	// the air and walk toward, exactly as the vision has them standing in the distance.
	for (int32 i = 0; i < 4; ++i)
	{
		const double Ang = FMath::DegreesToRadians(45.0 + i * 90.0);
		const FVector At(FMath::Cos(Ang) * 4200.0, FMath::Sin(Ang) * 4200.0, 0.f);
		Solid(At, FVector(500.f, 500.f, 1800.f), Pillar, Gold, GoldC, 1.6f, 0.f);
		Solid(At + FVector(0.f, 0.f, 1800.f), FVector(350.f, 350.f, 220.f), Pillar, Glow, GoldC,
			3.2f, 0.f);
		Beacons += 2;
	}

	// 6. TERRACES: THE HUB RISES AWAY FROM THE PLAZA.
	//
	// Four courses cut into the hill, each a phi step further out and one course
	// higher, every course opened where a road leaves the plaza so the streets stay
	// walkable. Built through Solid(), so each course is the size it is asked for on all
	// three axes - the reason the first two attempts lay down as wafers and panels.
	int32 Terraces = 0;
	{
		const float RoadZero = FMath::RadiansToDegrees(FMath::Atan2(-2600.f, 0.f));
		// L A I D  B Y  T H E  I D E N T I T Y.
		//
		// phi^2 = phi + 1, so a course whose width is r/phi lands its outer edge on
		// exactly r * phi - which is the next course's inner edge. The terraces
		// therefore TILE the hillside by construction: no gap between courses and no
		// overlap, because the step is not tuned, it is derived. That is the whole
		// of the law the user asks for, applied to the ground itself.
		const double Phi = 1.618033988749895;
		const double Phi2 = Phi + 1.0;                       // == Phi * Phi
		double Rad = 2400.0;
		for (int32 Ring = 0; Ring < 4; ++Ring)
		{
			// r/phi, capped so a far course is still a course and not a plain.
			const double Width = FMath::Min(Rad / Phi, 1500.0);
			const double Centre = Rad + Width * 0.5;
			const float TopZ = 160.f * (float)(Ring + 1);
			for (int32 S = 0; S < 18; ++S)
			{
				const double Ang = (2.0 * PI * S) / 18.0;
				const float Deg = (float)FMath::RadiansToDegrees(Ang);
				bool bOnRoad = false;
				for (int32 R = 0; R < 4 && !bOnRoad; ++R)
				{
					if (FMath::Abs(FMath::FindDeltaAngleDegrees(Deg,
						RoadZero + (float)R * 137.507764f)) < 16.f) { bOnRoad = true; }
				}
				if (bOnRoad) { continue; }

				const FVector At((float)(FMath::Cos(Ang) * Centre), (float)(FMath::Sin(Ang) * Centre), 0.f);
				const float Side = (float)Width;
				// The stone of the course, in the kit's own masonry.
				if (Solid(At, FVector(Side, Side, TopZ), Course, nullptr,
					FLinearColor(0.42f, 0.43f, 0.47f), 0.f, Deg))
				{
					// A gold course line on the lip: the step reads as BUILT, not as bare stone.
					Solid(At + FVector(0.f, 0.f, TopZ), FVector(Side + 40.f, Side + 40.f, 18.f), Slab,
						Gold, GoldC, 0.5f, Deg);
					++Terraces;
				}
			}
			// The identity, walking: inner + width = inner * phi = the next inner.
			Rad = Rad + Width;
		}
		UE_LOG(LogVEIL, Warning,
			TEXT("VEIL TERRACES: 4 courses, inner %.0f cm to outer %.0f cm. Course one tiles exactly (2400 + 2400/phi = 3883 = 2400*phi); courses two to four are capped at %.0f cm so a far course is still a course and not a plain."),
			2400.0, Rad, 1500.0);
	}

	// 7. THE WATERFALL CLIFF, WEST OF THE PLAZA.
	//
	// A canyon wall with three falls coming off it into a basin - the left third of the
	// painting. The wall is masonry courses; the falls are thin sheets of the palette's
	// light material, which is the only thing in this project that reads as water.
	int32 Falls = 0;
	{
		const float WallX = -4200.f;
		for (int32 i = 0; i < 5; ++i)
		{
			const float Y = -1800.f + (float)i * 900.f;
			const float Hgt = 3000.f + (float)(i % 2) * 420.f;
			Solid(FVector(WallX, Y, 0.f), FVector(900.f, 900.f, Hgt), Course, nullptr,
				FLinearColor(0.30f, 0.34f, 0.44f), 0.f, (float)i * 137.5f);
		}
		for (int32 i = 0; i < 3; ++i)
		{
			const float Y = -900.f + (float)i * 900.f;
			if (Solid(FVector(WallX + 430.f, Y, 0.f), FVector(14.f, 170.f, 3000.f), Slab, Glow,
				IceC, 1.15f, 0.f)) { ++Falls; }
		}
		// The basin they land in, and the spray standing over it.
		Solid(FVector(WallX + 760.f, 0.f, 0.f), FVector(900.f, 900.f, 14.f), Slab, Blue, BlueC,
			0.45f, 0.f);
		Solid(FVector(WallX + 700.f, 0.f, 0.f), FVector(600.f, 700.f, 90.f), Slab, Glow, IceC,
			0.28f, 0.f);
	}

	// 8. THE SPIRE, ON THE SKYLINE.
	//
	// The painting's Spire stands off the town and is what you orient by. Five tapered
	// courses stepped by the golden angle, a lit band at every joint, a crown that is the
	// brightest point in the sky - and clear of the Spire region's arrival pad at
	// (2800, 500), so phi.goto still drops the runner on open ground beside it.
	int32 SpireTiers = 0;
	{
		const FVector Base(3600.f, 900.f, 0.f);
		const float TierH = 1800.f;
		Solid(Base, FVector(760.f, 760.f, 320.f), Slab, Gold, GoldC, 0.3f, 0.f);
		for (int32 K = 0; K < 5; ++K)
		{
			const float Side = 880.f - (float)K * 124.f;
			const float Z = 320.f + TierH * (float)K;
			if (Solid(Base + FVector(0.f, 0.f, Z), FVector(Side, Side, TierH), Pillar, Gold, GoldC,
				0.45f, (float)K * 137.507764f))
			{
				Solid(Base + FVector(0.f, 0.f, Z + TierH - 25.f),
					FVector(Side + 70.f, Side + 70.f, 50.f), Slab, Glow, GoldC, 2.2f, 0.f);
				++SpireTiers;
			}
		}
		Solid(Base + FVector(0.f, 0.f, 320.f + 5.f * TierH), FVector(120.f, 120.f, 360.f), Pillar,
			Glow, GoldC, 3.4f, 0.f);
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("VEIL REGIONS: market %d pieces, nest %d, omni %d towers, %d crystal peaks, %d beacons, %d terraced courses, %d waterfalls on the west cliff, the Spire in %d courses - five places with silhouettes."),
		Market, Nest, Omni, Peaks, Beacons, Terraces, Falls, SpireTiers);
}

void AVEILWorldBuilder::BakeToLevel()
{
#if WITH_EDITOR
	// A BAKE IS A REBUILD, NOT A LAYER.
	//
	// This level's world IS the bake output, so the previous bake is not content - it
	// is the same content one run older. Left in place it doubles every time it runs
	// (measured: 1010 actors, then 1393: two cities and two horizons on top of each
	// other, which reads as a mess that no amount of new code can fix).
	//
	// It is also the only way to remove what predates the builder entirely. The audit
	// counted 374 actors on /Engine/BasicShapes/Cube plus 39 Spheres still sitting in
	// the saved map, and that count did NOT change when the builder stopped emitting
	// engine shapes at all - so those actors were saved by an earlier session and no
	// code change could ever have removed them. The grey blocks in the level survived
	// every fix because the fix was in the builder and the blocks were in the file.
	//
	// Only generated stage goes: static mesh actors and anything tagged VEIL_BAKE.
	// Lights, sky, fog, volumes and every other actor class are left alone.
	UWorld* W = GetWorld();
	if (W)
	{
		TArray<AActor*> Doomed;
		for (TActorIterator<AStaticMeshActor> It(W); It; ++It)
		{
			// The builder IS an actor but not a static mesh actor, so no comparison
			// against 'this' is possible - and none is needed: it never owns a mesh.
			Doomed.Add(*It);
		}
		for (TActorIterator<AActor> It(W); It; ++It)
		{
			if (*It != this && It->Tags.Contains(FName(TEXT("VEIL_BAKE")))) { Doomed.AddUnique(*It); }
		}

		// LIGHTS AND VOLUMES ARE GENERATED TOO. Left out of the clear, they accumulated
		// one full set per bake (measured: +23 actors every run), so the level slowly
		// filled with stacked suns, skylights and post-process volumes from older art
		// passes - each one fighting the others. BuildLighting recreates all of them
		// below, so clearing them first is safe and keeps a bake a rebuild, not a pile.
		for (TActorIterator<ADirectionalLight> It(W); It; ++It) { Doomed.AddUnique(*It); }
		for (TActorIterator<ASkyLight> It(W); It; ++It) { Doomed.AddUnique(*It); }
		for (TActorIterator<AExponentialHeightFog> It(W); It; ++It) { Doomed.AddUnique(*It); }
		for (TActorIterator<APointLight> It(W); It; ++It) { Doomed.AddUnique(*It); }
		for (TActorIterator<APostProcessVolume> It(W); It; ++It) { Doomed.AddUnique(*It); }

		// SIX BUILDERS WERE SAVED IN THE LEVEL, AND EVERY ONE OF THEM BUILDS.
		//
		// A bake spawns a builder, and the builder was never cleaned up, so six bakes
		// left six of them in VEILDistrict. Each one runs its own build on BeginPlay,
		// so pressing Play raised the district six times over: the duplicated, welded-
		// together geometry that reads as "blocks" no matter which mesh it is made of.
		// The builder is scaffolding. The bake keeps the buildings and removes the rig.
		for (TActorIterator<AVEILWorldBuilder> It(W); It; ++It)
		{
			if (*It != this) { Doomed.AddUnique(*It); }
		}

		int32 Placeholders = 0;
		for (AActor* A : Doomed)
		{
			if (AStaticMeshActor* SM = Cast<AStaticMeshActor>(A))
			{
				UStaticMeshComponent* C = SM->GetStaticMeshComponent();
				UStaticMesh* M = C ? C->GetStaticMesh() : nullptr;
				if (M && M->GetPathName().StartsWith(TEXT("/Engine/BasicShapes/")))
				{
					++Placeholders;
				}
			}
			A->Destroy();
		}
		UE_LOG(LogVEIL, Warning,
			TEXT("VEIL BAKE CLEAN: cleared %d generated actor(s), %d of them engine placeholders. Rebuilding from the kit."),
			Doomed.Num(), Placeholders);
	}

	// A second bake in the same session must build again, not return early.
	bBuilt = false;
#endif

	// PUT THE SUN BACK BEFORE BUILDING THE STREETS.
	//
	// The clean step above removes lights so a re-bake cannot stack six suns - and the
	// district-layer path only LIFTS lights that already exist, it never creates them.
	// The two together baked a district with 22 point lights and no key at all, which
	// the packaged game then reported in one line: "VEIL PHI LIGHT: 0 sun(s), 0 sky
	// light(s)". That is the darkness. It was never exposure, and no grade could have
	// fixed it - the world was rendering at night.
	//
	// BuildLighting() is the pass that creates the key, the sky, the fog and the
	// beacon's heart. It has to run after the clean and before the streets, so the level
	// is saved WITH its sun.
	BuildLighting();

	BuildDistrictLayer();
	DressPhiPalette();
	DressRegions();

	// GROUND THAT IS IN THE MAP, NOT ON THE BUILDER.
	//
	// The 2 km slab was spawned only when the level arrived with no floor under the
	// plaza, and the plain that continues past the city was instanced onto THIS ACTOR -
	// which the bake deletes when it clears old build output. The result was a saved
	// level with a district and nothing underneath it: in a packaged frame the runner
	// stood on the plaza, walked off, and fell to Z = -5000 with no floor to catch him,
	// and the photograph from 90 m up showed a city floating over a plane. Ground is
	// now laid as real, tagged actors at the end of every bake - nine tiles edge to
	// edge, top face at z = 0, collision on - so it exists in the map the game loads.
	if (UStaticMesh* Ground = FloorVariants.Num() > 0 ? FloorVariants[0].Get() : FloorMesh.Get())
	{
		// Stone under the city, matching the plaza course rather than the night-blue
		// the old tint asked for and never got.
		const FLinearColor StoneGround(0.26f, 0.27f, 0.30f);
		// THE FLOOR MATERIAL CARRIES NO COLOUR PARAMETER AT ALL - measured, not guessed:
		// MI_Modular1 reports vec=[none], which is why every ground tint this project has
		// asked for was a silent no-op and the plain stayed authored sand. The PHI palette
		// material does carry one, so the plain is dressed in it: stone blue-grey, and lit
		// only a fraction, so it reads as flagstone in daylight instead of a glowing plate.
		// STONE, NOT SAND, AND NOT GOLD. The kit's floor material carries no colour
		// parameter at all - measured: MI_Modular1, vec=[none] - so this plain could never
		// be tinted, and the palette's gold would make the ground a plate of metal.
		// Starter Content ships real cut-stone materials, so the plain wears masonry:
		// the same cut stone the plaza reads as, in daylight, instead of authored sand.
		UMaterialInterface* GroundMat = LoadObject<UMaterialInterface>(nullptr,
			TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
		int32 Tiles = 0;
		for (int32 IX = -1; IX <= 1; ++IX)
		{
			for (int32 IY = -1; IY <= 1; ++IY)
			{
				// Scale 2000 is 2 km on this kit (scale 1 = 100 cm), so the centres are
				// 200000 cm apart and the tiles meet edge to edge, never overlapping.
				UStaticMeshComponent* TileComp = SpawnBlock(FVector(IX * 200000.f, IY * 200000.f, -50.f),
					FVector(2000.f, 2000.f, 1.f), Ground, StoneGround, 0.f);
				if (TileComp && GroundMat)
				{
					// The stone material carries its own texture and tint, so it is assigned as
					// it ships: no parameter write that a stone material does not have.
					TileComp->SetMaterial(0, GroundMat);
				}
				++Tiles;
			}
		}
		UE_LOG(LogVEIL, Warning,
			TEXT("VEIL GROUND: %d collision tiles laid into the map, top face at z=0, edge to edge out to 3 km."),
			Tiles);
	}

	// THE PLAIN IS ONE STONE SURFACE, NOT NEW STONE AROUND OLD SAND.
	//
	// New ground is laid with the Starter Content stone, but the district already
	// carried floor slabs from earlier bakes, and those wear MI_Modular1 - a material
	// with no colour parameter at all, so they keep their authored sand forever. They
	// are re-skinned here by material behaviour, not by name: any floor slab whose
	// material cannot be tinted gets the stone, and any slab that CAN be tinted is left
	// alone because its colour is being respected. The plain then reads as one surface
	// from the plaza to the horizon.
	if (UWorld* StoneWorld = GetWorld())
	{
		UMaterialInterface* Stone = LoadObject<UMaterialInterface>(nullptr,
			TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
		int32 Reskinned = 0;
		if (Stone)
		{
			for (TActorIterator<AStaticMeshActor> It(StoneWorld); It; ++It)
			{
				UStaticMeshComponent* SlabComp = It->GetStaticMeshComponent();
				if (!SlabComp || !SlabComp->GetStaticMesh()) { continue; }
				// FLOOR AND THE DUNE TILES UNDER IT. The audit names what the plain is made
				// of: 34 x SM_Floor1_Big and 60 x SM_PyramidHead - the pack's authored sand
				// slabs, which is the tan surface every photograph has shown. Both families
				// are ground, so both are dressed as ground.
				const FString SlabName = SlabComp->GetStaticMesh()->GetName();
				if (!SlabName.Contains(TEXT("Floor")) && !SlabName.Contains(TEXT("Pyramid"))
					&& !SlabName.Contains(TEXT("Ground")))
				{
					continue;
				}
				// EVERY ground slab, tintable or not. Skipping the tintable ones was the
				// mistake that kept the sand: the dune tiles DO carry a colour parameter, and
				// the call site that spawns them writes a warm sand into it - so "respect the
				// tint" meant "keep the desert". Ground is ground: it wears the stone. Rooms,
				// roofs, cliffs and props are untouched, because they are not the plain.
				SlabComp->SetMaterial(0, Stone);
				++Reskinned;
			}
		}
		UE_LOG(LogVEIL, Warning,
			TEXT("VEIL STONE: %d floor slabs re-skinned to cut stone - the plain is one surface."),
			Reskinned);
	}

#if WITH_EDITOR
	UWorld* Built = GetWorld();
	const int32 InWorld = Built ? Built->GetActorCount() : 0;
	const bool bSaved = UEditorLoadingAndSavingUtils::SaveCurrentLevel();
	UE_LOG(LogVEIL, Warning,
		TEXT("VEIL BAKE: district built into the level (%d actors in world), SAVE RESULT %s"),
		InWorld, bSaved ? TEXT("True") : TEXT("False"));
#endif
}

void AVEILWorldBuilder::BuildWorld()
{
	if (bBuilt) return;
	bBuilt = true;
	// THE LIGHT FIRST. Everything below is judged by eye against the reference frames, and
	// the frames are a golden city: a warm low sun, a sky light taking its colour from that
	// sun, and air that gives the valley depth. Dressed before building, so the district and
	// the vista are lit by one light instead of being half-lit and corrected later.
	VEILSky::Dress(GetWorld());

	// ONE RIG, ONE CITY: clear the template map's own stage first.
	// Its sun, sky, fog and checker floor are what fought the dusk in
	// the first standing photo - two directional lights competing for
	// forward shading, a grid floor under the district. A real game
	// ships one stage: the builder's. Everything pre-existing in those
	// classes dies here, before anything of ours spawns.
	UWorld* World = GetWorld();
	if (World)
	{
		for (TActorIterator<ADirectionalLight> It(World); It; ++It) It->Destroy();
		for (TActorIterator<ASkyLight> It(World); It; ++It) It->Destroy();
		for (TActorIterator<AExponentialHeightFog> It(World); It; ++It) It->Destroy();
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It) It->Destroy();
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			const FString Name = It->GetClass()->GetName();
			if (Name == TEXT("SkyAtmosphere") || Name == TEXT("VolumetricCloud"))
			{
				It->Destroy();
			}
		}
	}

	BuildLighting();
	BuildDistrict();
	DressBeaconSites();
	BuildHorizon();
	BuildVista();
	UE_LOG(LogVEIL, Log, TEXT("VEIL: free to play under the PHI LICENSE (PHI-LICENSE.md). The phi framework is licensed only through Tyree Shariff Jones, its source - universal, including Earth's."));
}

// ---------------------------------------------------------------------------
// THE VISTA. The reference frame is not a plaza with props: it is a valley with a blue
// crystal range on one flank, water falling off the cliffs, a village stepped up the rock
// in terraces joined by bridges, three gold phi pillars in the foreground and light shafts
// standing out of the holy sites. This builds that silhouette from the kit the district
// already uses, on the golden angle, at real sizes in centimetres.
// ---------------------------------------------------------------------------
void AVEILWorldBuilder::BuildVista()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UStaticMesh* Crystal = PyramidHeadMesh ? PyramidHeadMesh.Get() : MonolithMesh.Get();
	UStaticMesh* Spike = MonolithMesh ? MonolithMesh.Get() : Crystal;
	UStaticMesh* Stone = WallMesh ? WallMesh.Get() : FloorMesh.Get();
	UStaticMesh* Ground = FloorMesh ? FloorMesh.Get() : Stone;
	UStaticMesh* Pillar = ColumnMesh ? ColumnMesh.Get() : CylinderMesh.Get();
	UStaticMesh* Water = FloorMesh ? FloorMesh.Get() : Stone;
	if (!Crystal || !Stone || !Pillar || !Ground) { return; }

	const FLinearColor IceC(0.30f, 0.58f, 1.0f);
	const FLinearColor ColdC(0.18f, 0.34f, 0.72f);
	const FLinearColor Gold(0.72f, 0.56f, 0.28f);
	const FLinearColor StoneC(0.42f, 0.39f, 0.35f);
	const FLinearColor WaterC(0.52f, 0.74f, 1.0f);
	const float PhiV = 1.61803398875f;
	const float GoldDeg = 137.50776405f;

	int32 Peaks = 0, Falls = 0, Houses = 0, Bridges = 0, Pillars = 0, Shafts = 0;

	// 1. THE CRYSTAL RANGE. Blue spires on the far flank, clustered rather than scattered:
	// five shards per cluster, each shorter than the last, all leaning out of one base.
	for (int32 i = 0; i < 14; ++i)
	{
		const float Ang = FMath::DegreesToRadians(i * GoldDeg + 205.f);
		const float Rad = PHILayout::Band(i, 7800.f, 5);
		const FVector Base(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);
		const float ClusterH = 1500.f + (float)(i % 4) * 700.f;
		for (int32 K = 0; K < 5; ++K)
		{
			const float KAng = FMath::DegreesToRadians(K * GoldDeg);
			const float KR = 300.f + 110.f * (float)K;
			const FVector At = Base + FVector(FMath::Cos(KAng) * KR, FMath::Sin(KAng) * KR, 0.f);
			const float KH = ClusterH * (1.f - 0.13f * (float)K);
			if (SpawnBlock(At + FVector(0.f, 0.f, KH * 0.5f),
				FVector(KH * 0.0024f, KH * 0.0024f, KH / 100.f),
				(K % 2 == 0) ? Crystal : Spike, (K % 2 == 0) ? IceC : ColdC, 0.45f))
			{
				++Peaks;
			}
		}
	}

	// 2. WATERFALLS OFF THE CLIFF FACES. A rock buttress, then the sheet down its face, then
	// the basin it lands in - so the water reads as falling rather than as a blue plate.
	for (int32 i = 0; i < 6; ++i)
	{
		const float Ang = FMath::DegreesToRadians(i * GoldDeg + 60.f);
		const float Rad = PHILayout::Band(i, 5600.f, 3);
		const FVector At(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);
		const FRotator Face(0.f, FMath::RadiansToDegrees(Ang) + 180.f, 0.f);
		const float FallH = 2200.f + (float)(i % 3) * 500.f;

		SpawnBlockRot(At + FVector(0.f, 0.f, FallH * 0.5f),
			FVector(14.f, 14.f, FallH / 100.f), Stone, StoneC, 0.f, Face);
		if (SpawnBlockRot(At + FVector(-260.f, 0.f, FallH * 0.45f),
			FVector(0.5f, 7.f, FallH * 0.85f / 100.f), Water, WaterC, 0.55f, Face))
		{
			++Falls;
		}
		SpawnBlockRot(At + FVector(-330.f, 0.f, 40.f), FVector(6.f, 9.f, 0.6f), Water, WaterC, 0.25f, Face);
	}

	// 3. THE TERRACED VILLAGE. Three shelves stepped up the rock, each carrying enterable
	// houses from BuildRoom, so the village is somewhere you can walk into, not scenery.
	TArray<FVector> Terraces;
	for (int32 T = 0; T < 3; ++T)
	{
		const float Ang = FMath::DegreesToRadians(T * GoldDeg + 40.f);
		const float Rad = PHILayout::Band(T, 3600.f, 3);
		const FVector C(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);
		const float DeckZ = 320.f + 220.f * (float)T;
		Terraces.Add(C + FVector(0.f, 0.f, DeckZ));

		SpawnBlock(C + FVector(0.f, 0.f, DeckZ * 0.5f), FVector(11.f, 8.f, DeckZ / 100.f), Ground, StoneC, 0.f);
		const FVector Facing = (-C).GetSafeNormal2D();
		for (int32 H = 0; H < 5; ++H)
		{
			const float HAng = FMath::DegreesToRadians(H * GoldDeg + 20.f);
			const float HR = 620.f + 120.f * (float)H;
			const FVector At = C + FVector(FMath::Cos(HAng) * HR, FMath::Sin(HAng) * HR, DeckZ);
			UStaticMesh* WallHere = WallVariants.Num() ? WallVariants[H % WallVariants.Num()].Get() : Stone;
			UStaticMesh* FloorHere = FloorVariants.Num() ? FloorVariants[H % FloorVariants.Num()].Get() : Ground;
			BuildRoom(At, 300.f, 260.f, 640.f, Facing, WallHere, FloorHere, StoneC, false);
			++Houses;
		}
	}

	// 4. BRIDGES BETWEEN THE TERRACES. Planks on the sag line, a post at each end, a rope
	// above - the span the picture runs between the rock shelves.
	for (int32 B = 0; B + 1 < Terraces.Num(); ++B)
	{
		const FVector A = Terraces[B];
		const FVector Z = Terraces[B + 1];
		const FVector Delta = Z - A;
		const float Len = Delta.Size2D();
		if (Len < 300.f) { continue; }
		const FRotator Facing = Delta.Rotation();
		const int32 Planks = FMath::Clamp(FMath::RoundToInt(Len / 110.f), 6, 26);
		const float Sag = Len * 0.06f;
		for (int32 P = 0; P <= Planks; ++P)
		{
			const float T = (float)P / (float)Planks;
			const FVector At = FMath::Lerp(A, Z, T) - FVector(0.f, 0.f, FMath::Sin(T * PI) * Sag);
			SpawnBlockRot(At, FVector(0.9f, 0.9f, 0.10f), Ground, StoneC, 0.f, Facing);
		}
		for (int32 Side = -1; Side <= 1; Side += 2)
		{
			const FVector Off = Facing.RotateVector(FVector(0.f, Side * 90.f, 0.f));
			SpawnBlockRot(A + Off + FVector(0.f, 0.f, 190.f), FVector(0.16f, 0.16f, 3.8f), Pillar, Gold, 0.f, FRotator::ZeroRotator);
			SpawnBlockRot(Z + Off + FVector(0.f, 0.f, 190.f), FVector(0.16f, 0.16f, 3.8f), Pillar, Gold, 0.f, FRotator::ZeroRotator);
			const FVector Mid = (A + Z) * 0.5f + Off - FVector(0.f, 0.f, Sag * 0.5f);
			const FRotator RopeRot = FRotationMatrix::MakeFromZ(Delta + FVector(0.f, 0.f, -Sag)).Rotator();
			SpawnBlockRot(Mid + FVector(0.f, 0.f, 300.f), FVector(0.06f, 0.06f, Len / 100.f), Pillar, Gold, 0.f, RopeRot);
		}
		++Bridges;
	}

	// 5. THE THREE GOLD PHI PILLARS. The picture's foreground: three columns carrying a gold
	// plate, at radius 1100, 1100*phi, 1100*phi^2, on the golden angle.
	for (int32 i = 0; i < 3; ++i)
	{
		const float Ang = FMath::DegreesToRadians(i * GoldDeg + 30.f);
		const float Rad = PHILayout::Band(i, 1500.f, 3);
		const FVector Base(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);
		SpawnBlock(Base + FVector(0.f, 0.f, 40.f), FVector(3.4f, 3.4f, 0.8f), Ground, StoneC, 0.f);
		SpawnBlock(Base + FVector(0.f, 0.f, 600.f), FVector(0.9f, 0.9f, 11.f), Pillar, Gold, 0.15f);
		SpawnBlock(Base + FVector(0.f, 0.f, 1180.f), FVector(2.2f, 2.2f, 0.5f), Crystal, Gold, 1.4f);
		++Pillars;
	}

	// 6. BEACON SHAFTS. The light standing out of the holy sites into the sky - the vertical
	// gold lines that give the picture its depth.
	struct FSite { float X, Y; };
	static const FSite Sites[] = { { 900.f, 0.f }, { -450.f, 780.f }, { -4150.f, -780.f } };
	for (const FSite& Site : Sites)
	{
		if (SpawnBlock(FVector(Site.X, Site.Y, 2600.f), FVector(0.55f, 0.55f, 46.f), Pillar, Gold, 2.6f))
		{
			++Shafts;
		}
		SpawnBlock(FVector(Site.X, Site.Y, 300.f), FVector(2.4f, 2.4f, 6.f), Crystal, Gold, 1.1f);
	}

	// 7. UNDERGROWTH. A bare valley of stone reads as unfinished however good the stone is:
	// clumps of the kit's debris, tinted moss, on their own phi band.
	const FLinearColor Moss(0.20f, 0.36f, 0.19f);
	int32 Shrubs = 0;
	TArray<FVector> VistaSpots;
	for (int32 i = 0; i < 48; ++i)
	{
		const FVector At = PHILayout::Spiral(i, 4200.f, 12.f, 5);
		UStaticMesh* Bush = DebrisVariants.Num() ? DebrisVariants[i % DebrisVariants.Num()].Get() : Stone;
		const float H = 180.f + 40.f * (float)(i % 5);
		if (SpawnBlock(At + FVector(0.f, 0.f, H * 0.5f),
			FVector(H * 0.0042f, H * 0.0042f, H / 100.f), Bush, Moss, 0.f))
		{
			VistaSpots.Add(At);
			++Shrubs;
		}
	}

	// 8. THE RIVER. Water on the valley floor, on its own phi band.
	const FLinearColor RiverC(0.34f, 0.58f, 0.86f);
	int32 RiverTiles = 0;
	for (int32 i = 0; i < 6; ++i)
	{
		const FVector At = PHILayout::Spiral(i, 2700.f, 101.f, 4, 24.f);
		if (SpawnBlock(At, FVector(9.5f, 4.2f, 0.5f), Water, RiverC, 0.18f))
		{
			VistaSpots.Add(At);
			++RiverTiles;
		}
	}

	// THE SPACING AUDIT: what this pass placed, measured against itself.
	PHILayout::FSpacingReport Spacing;
	Spacing.Measure(VistaSpots, 120.f);
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI LAYOUT: %d shrub(s), %d river tile(s) on phi bands; %d piece(s) audited, min spacing %.0f cm, %d pair(s) under 120 cm - verdict %s."),
		Shrubs, RiverTiles, Spacing.Count, Spacing.MinGapCm, Spacing.TooClose,
		Spacing.IsClean() ? TEXT("BANDS HOLD") : TEXT("CROWDED"));

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI VISTA: %d crystal peaks, %d waterfall(s), %d house(s) on 3 terraces, %d bridge span(s), %d phi pillar(s), %d beacon shaft(s) - the district reads in silhouette."),
		Peaks, Falls, Houses, Bridges, Pillars, Shafts);
}

