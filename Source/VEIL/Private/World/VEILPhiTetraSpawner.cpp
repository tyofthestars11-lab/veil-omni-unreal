#include "World/VEILPhiTetraSpawner.h"
#include "VEILGolden.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "ProceduralMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

AVEILPhiTetraSpawner::AVEILPhiTetraSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("PhiTetraRoot"));
	SetRootComponent(Root);

	// NO MESH ASSET. There is no tetrahedron in this project and there never was -
	// the old link pointed at a Paragon shape that was deleted with the pack, and a
	// constructor that names a missing package fails at CDO time on every cook. The
	// solid is built from its own four corners in BuildChain instead: a regular
	// tetrahedron is four points and four triangles, and it cannot go missing.

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
	if (!World) { return; }

	// Idempotent: OnConstruction may run more than once and a doubled chain
	// would look like a bug in the growth.
	for (UProceduralMeshComponent* Old : Tetras)
	{
		if (Old) { Old->DestroyComponent(); }
	}
	Tetras.Reset();
	SavedPositions.Reset();
	SavedSizes.Reset();

	// Native height of the tetra, so "size" means its real height in cm instead of a
	// guessed scale factor. The solid below is authored exactly one unit tall, so
	// the native height is 1 and the drawn size IS the scale.
	const double NativeHeight = 1.0;

	// THE SOLID, MADE HERE. Three base corners on a circle of radius 1/sqrt(2) and
	// an apex one unit above the base plane, centred on the centroid so the actor
	// can stand it on the ground at Spot.Z + height/2. Each face gets its own three
	// vertices and its own normal, so the tetrahedron is faceted, not shaded like a
	// ball. Winding is fixed against the centroid instead of trusted.
	TArray<FVector> TetraVerts;
	TArray<FVector> TetraNormals;
	TArray<FVector2D> TetraUVs;
	TArray<int32> TetraTris;
	{
		const double R = 0.70710678118654752;   // 1/sqrt(2): edge 1.224745, height 1
		const double BaseZ = -0.25;
		const double ApexZ = 0.75;
		const FVector Corner[4] = {
			FVector(0.0, 0.0, ApexZ),
			FVector(R, 0.0, BaseZ),
			FVector(-0.35355339059327376, 0.61237243569579452, BaseZ),
			FVector(-0.35355339059327376, -0.61237243569579452, BaseZ)
		};
		const int32 Faces[4][3] = { {0,1,2}, {0,2,3}, {0,3,1}, {1,3,2} };
		for (int32 f = 0; f < 4; ++f)
		{
			int32 I0 = Faces[f][0];
			int32 I1 = Faces[f][1];
			int32 I2 = Faces[f][2];
			FVector N = FVector::CrossProduct(Corner[I1] - Corner[I0], Corner[I2] - Corner[I0]);
			const FVector Mid = (Corner[I0] + Corner[I1] + Corner[I2]) / 3.0;
			if (FVector::DotProduct(N, Mid) < 0.0)
			{
				const int32 Tmp = I1; I1 = I2; I2 = Tmp;
				N = -N;
			}
			N.Normalize();
			const int32 Start = TetraVerts.Num();
			TetraVerts.Add(Corner[I0]); TetraVerts.Add(Corner[I1]); TetraVerts.Add(Corner[I2]);
			TetraNormals.Add(N); TetraNormals.Add(N); TetraNormals.Add(N);
			TetraUVs.Add(FVector2D(0.5, 1.0)); TetraUVs.Add(FVector2D(0.0, 0.0)); TetraUVs.Add(FVector2D(1.0, 0.0));
			TetraTris.Add(Start); TetraTris.Add(Start + 1); TetraTris.Add(Start + 2);
		}
	}

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

		UProceduralMeshComponent* Tetra = NewObject<UProceduralMeshComponent>(this);
		if (!Tetra) { continue; }
		Tetra->SetupAttachment(GetRootComponent());
		Tetra->bUseAsyncCooking = false;
		Tetra->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Tetra->RegisterComponent();
		Tetra->CreateMeshSection_LinearColor(0, TetraVerts, TetraTris, TetraNormals, TetraUVs,
			TArray<FLinearColor>(), TArray<FProcMeshTangent>(), false);
		// THE CHAIN IS GOLD, NOT CHALK. With no material the procedural solid reads
		// as a white pyramid, which is why the chain was switched off in the district.
		if (!TetraMaterial)
		{
			TetraMaterial = LoadObject<UMaterialInterface>(nullptr,
				TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic"));
		}
		if (TetraMaterial) { Tetra->SetMaterial(0, TetraMaterial); }

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
