#include "World/VEILGoldenSpawner.h"
#include "VEILGolden.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AVEILGoldenSpawner::AVEILGoldenSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("GoldenRoot"));
	SetRootComponent(Root);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeFinder.Succeeded()) { CubeMesh = CubeFinder.Object; }

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GoldFinder(
		TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold"));
	if (GoldFinder.Succeeded()) { CubeMaterial = GoldFinder.Object; }
}

void AVEILGoldenSpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildSpiral();
}

void AVEILGoldenSpawner::BeginPlay()
{
	Super::BeginPlay();
	// OnConstruction usually covers this; if the world did not construct the
	// spiral (runtime-spawned owner), raise it here instead.
	if (Cubes.Num() == 0)
	{
		BuildSpiral();
	}
}

void AVEILGoldenSpawner::BuildSpiral()
{
	UWorld* World = GetWorld();
	if (!World || !CubeMesh) { return; }

	// Idempotent: OnConstruction can run more than once, and a doubled spiral
	// would look like a bug in the formula.
	for (UStaticMeshComponent* Old : Cubes)
	{
		if (Old) { Old->DestroyComponent(); }
	}
	Cubes.Reset();

	const FVector Origin = GetActorLocation();
	const float HalfCube = 50.f * CubeScale;   // engine Cube is 100 cm across

	for (int32 Index = 1; Index <= Count; ++Index)
	{
		const FVector2D Local = VEILGolden::GoldenLoc(Index, Spacing);
		FVector Spot(Origin.X + Local.X, Origin.Y + Local.Y, Origin.Z);

		// Snap to the floor: trace down from well above the spot.
		FHitResult Ground;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(VEILGoldenSpawner), false, this);
		const FVector From = Spot + FVector(0.f, 0.f, 500.f);
		const FVector To = Spot - FVector(0.f, 0.f, 2000.f);
		if (World->LineTraceSingleByChannel(Ground, From, To, ECC_Visibility, Params))
		{
			Spot.Z = Ground.ImpactPoint.Z + HalfCube;
		}

		UStaticMeshComponent* Cube = NewObject<UStaticMeshComponent>(this);
		if (!Cube) { continue; }
		Cube->SetupAttachment(GetRootComponent());
		Cube->SetStaticMesh(CubeMesh);
		Cube->SetRelativeScale3D(FVector(CubeScale));
		Cube->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (CubeMaterial) { Cube->SetMaterial(0, CubeMaterial); }
		Cube->RegisterComponent();
		Cube->SetWorldLocation(Spot);

		Cubes.Add(Cube);

		// Yellow proof lines on the three indices Hark called out.
		if (bDrawProofLines && (Index == 1 || Index == 2 || Index == 5))
		{
			DrawDebugLine(World, Spot, Spot + FVector(0.f, 0.f, 300.f),
				FColor::Yellow, false, 30.f, 0, 3.f);
			DrawDebugString(World, Spot + FVector(0.f, 0.f, 320.f),
				FString::Printf(TEXT("i=%d (%.4f, %.4f)"), Index, Local.X, Local.Y),
				nullptr, FColor::Yellow, 30.f);
		}
	}

	UE_LOG(LogTemp, Display,
		TEXT("PHI GOLDEN: %d cubes in a sunflower spiral (spacing %.1f, scale %.2f) at %s"),
		Cubes.Num(), Spacing, CubeScale, *Origin.ToCompactString());
}
