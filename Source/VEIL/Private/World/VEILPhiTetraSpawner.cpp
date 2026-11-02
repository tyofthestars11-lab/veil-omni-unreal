#include "World/VEILPhiTetraSpawner.h"
#include "VEILGolden.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AVEILPhiTetraSpawner::AVEILPhiTetraSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("PhiTetraRoot"));
	SetRootComponent(Root);

	// A REAL tetrahedron already in the project (Paragon travel-buff shape),
	// never a cone standing in for one.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TetraFinder(
		TEXT("/Game/ParagonMinions/FX/Meshes/Shapes/SM_Travel_Tetra.SM_Travel_Tetra"));
	if (TetraFinder.Succeeded()) { TetraMesh = TetraFinder.Object; }

	static ConstructorHelpers::FObjectFinder<UMaterialInterface> GoldFinder(
		TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold"));
	if (GoldFinder.Succeeded()) { TetraMaterial = GoldFinder.Object; }
}

void AVEILPhiTetraSpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	BuildChain();
}

void AVEILPhiTetraSpawner::BeginPlay()
{
	Super::BeginPlay();
	if (Tetras.Num() == 0)
	{
		BuildChain();
	}
}

void AVEILPhiTetraSpawner::BuildChain()
{
	UWorld* World = GetWorld();
	if (!World || !TetraMesh) { return; }

	// Idempotent: OnConstruction may run more than once and a doubled chain
	// would look like a bug in the growth.
	for (UStaticMeshComponent* Old : Tetras)
	{
		if (Old) { Old->DestroyComponent(); }
	}
	Tetras.Reset();
	SavedPositions.Reset();
	SavedSizes.Reset();

	// Native height of the tetra, so "size" means its real height in cm
	// instead of a guessed scale factor.
	const FBoxSphereBounds Bounds = TetraMesh->GetBounds();
	const double NativeHeight = FMath::Max(1.0, (double)Bounds.BoxExtent.Z * 2.0);

	const FVector Origin = GetActorLocation();

	FVector Previous = FVector::ZeroVector;

	// Hark's chain, built once and read step by step: each spot steps forward by
	// the size the step before it had, 60 degrees around, and grows by phi.
	TArray<FVector2D> ChainSpots;
	TArray<double> ChainSizes;
	if (Rule == EPhiTetraRule::Chain60)
	{
		VEILTetra::BuildChain(Count, (double)BaseSize, ChainSpots, ChainSizes);
	}

	// THE BAND, NOT A CAP. phi^RungOffset is the rung the chain is read in: the
	// geometry and the drawn sizes come down by it, so a 34-step chain stands in
	// the same world span a 6-step chain does. SavedSizes and the log keep the
	// TRUE size of every step - only the band moves.
	const double Band = FMath::Pow(VEILGolden::Phi, (double)RungOffset);
	if (RungOffset != 0.f)
	{
		UE_LOG(LogTemp, Display,
			TEXT("PHI TETRA: reading the chain in rung band %.2f (phi^%.2f = %.6f) - sizes stay true, the world stays whole."),
			(double)RungOffset, (double)RungOffset, Band);
	}

	for (int32 Index = 1; Index <= Count; ++Index)
	{
		const int32 Step = Index - 1;

		// 1. Where this tetra stands, by the rule that was chosen.
		const FVector2D LocalTrue = (Rule == EPhiTetraRule::Chain60 && ChainSpots.IsValidIndex(Step))
			? ChainSpots[Step]
			: VEILGolden::GoldenLoc(Index, Spacing);
		const FVector2D Local = LocalTrue / Band;   // same law, read in this band

		// 2. How big it is: the chain's own sizes, or BaseSize * phi^(Index-1).
		const double Size = (Rule == EPhiTetraRule::Chain60 && ChainSizes.IsValidIndex(Step))
			? ChainSizes[Step]
			: (double)BaseSize * FMath::Pow((double)Growth, (double)(Index - 1));
		const double Drawn = Size / Band;          // what the world draws
		const double Scale = Drawn / NativeHeight;

		FVector Spot(Origin.X + Local.X, Origin.Y + Local.Y, Origin.Z);

		// Snap to the FLOOR, not to the first thing under the column. A single trace
		// caught a ledge and stood step 4 at Z=421 while its neighbours sat at 66-167.
		// LineTraceMulti does NOT help: it also stops at the first blocking hit, so
		// this walks DOWN instead: each trace resumes just below the previous surface
		// and the DEEPEST surface wins. A retry that reports no hit means the probe
		// start sits inside a solid (a line inside a mesh finds nothing), so the walk
		// steps down and carries on - otherwise a tetra stopped mid-forge at Z=407
		// while the plaza floor under it was at Z=52.
		FCollisionQueryParams Params(SCENE_QUERY_STAT(VEILPhiTetraSpawner), false, this);
		const float BottomZ = Spot.Z - 2500.f;
		float ProbeZ = Spot.Z + 600.f;
		float Lowest = -MAX_flt;
		for (int32 Guard = 0; Guard < 64 && ProbeZ > BottomZ; ++Guard)
		{
			FHitResult Ground;
			const FVector From(Spot.X, Spot.Y, ProbeZ);
			const FVector To(Spot.X, Spot.Y, BottomZ);
			if (World->LineTraceSingleByChannel(Ground, From, To, ECC_Visibility, Params))
			{
				const float HitZ = (float)Ground.ImpactPoint.Z;
				if (HitZ < Lowest) { Lowest = HitZ; }
				ProbeZ = HitZ - 25.f;
			}
			else
			{
				ProbeZ -= 100.f; // inside a solid: keep descending to the real floor
			}
		}
		if (Lowest > -MAX_flt)
		{
			Spot.Z = Lowest + (Drawn * 0.5);
		}
		else
		{
			Spot.Z += Drawn * 0.5;
		}

		UStaticMeshComponent* Tetra = NewObject<UStaticMeshComponent>(this);
		if (!Tetra) { continue; }
		Tetra->SetupAttachment(GetRootComponent());
		Tetra->SetStaticMesh(TetraMesh);
		Tetra->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (TetraMaterial) { Tetra->SetMaterial(0, TetraMaterial); }
		Tetra->RegisterComponent();

		// 3. 60 degrees forward per step (the chain counts from its anchor).
		const float YawDeg = YawStepDegrees * (float)((Rule == EPhiTetraRule::Chain60) ? Step : Index);
		Tetra->SetWorldLocationAndRotation(Spot, FRotator(0.f, YawDeg, 0.f));
		Tetra->SetWorldScale3D(FVector(Scale));

		Tetras.Add(Tetra);

		// 4. Saved pos/size each step.
		SavedPositions.Add(Spot);
		SavedSizes.Add((float)Size);

		UE_LOG(LogTemp, Display,
			TEXT("PHI TETRA %d/%d: pos=%s size=%.2fcm yaw=%.0fdeg scale=%.4f rule=%s"),
			Index, Count, *Spot.ToCompactString(), Size, YawDeg, Scale,
			(Rule == EPhiTetraRule::Chain60) ? TEXT("chain60") : TEXT("spiral"));

		if (bDrawChain && Index > 1)
		{
			DrawDebugLine(World, Previous, Spot, FColor::Cyan, false, 60.f, 0, 2.f);
		}
		Previous = Spot;
	}

	UE_LOG(LogTemp, Display,
		TEXT("PHI TETRA CHAIN: %d tetrahedra, sizes %.2f -> %.2f cm, origin %s"),
		Tetras.Num(),
		SavedSizes.Num() > 0 ? SavedSizes[0] : 0.f,
		SavedSizes.Num() > 0 ? SavedSizes.Last() : 0.f,
		*Origin.ToCompactString());
}
