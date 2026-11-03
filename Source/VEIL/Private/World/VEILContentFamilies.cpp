#include "World/VEILContentFamilies.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "VEIL.h"

namespace
{
	constexpr float ContentGoldenDeg = 137.50776405f;
	constexpr float ContentPhi = 1.6180339887f;
	const TCHAR* GoldPath = TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic");
	const TCHAR* BluePath = TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue");
}

AVEILContentPass::AVEILContentPass()
{
	PrimaryActorTick.bCanEverTick = false;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("ContentRoot"));
	SetRootComponent(Root);
}

UStaticMeshComponent* AVEILContentPass::Part(UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
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
	C->SetCollisionEnabled(bSolid ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
	if (Mat) { C->SetMaterial(0, Mat); }
	C->RegisterComponent();
	Parts.Add(C);
	return C;
}

bool AVEILContentPass::FindSpot(const FVector& Anchor, float Radius, float AngleDeg,
	float MinSpacing, FVector& Out)
{
	UWorld* W = GetWorld();
	if (!W) { return false; }

	const float Ang = FMath::DegreesToRadians(AngleDeg);
	const FVector2D P2(Anchor.X + FMath::Cos(Ang) * Radius, Anchor.Y + FMath::Sin(Ang) * Radius);

	// THE FLOOR THE RUNNER IS STANDING ON, not the top of the skyline. A window of
	// 300 cm up and 300 cm down keeps every piece on his level: a crystal on a
	// roof ten storeys above him is not world, it is debris.
	FCollisionQueryParams Params(SCENE_QUERY_STAT(VEILContentSpot), false, this);
	FHitResult Ground;
	if (!W->LineTraceSingleByChannel(Ground,
		FVector(P2.X, P2.Y, Anchor.Z + 300.f), FVector(P2.X, P2.Y, Anchor.Z - 300.f),
		ECC_Visibility, Params))
	{
		return false;
	}
	const float Z = Ground.ImpactPoint.Z;
	if (FMath::Abs(Z - Anchor.Z) > 120.f) { return false; }

	// ROOM TO EXIST: a 220 cm sphere whose bottom clears the pavement must be
	// empty. This is what refuses the inside of a wall or a parked machine.
	if (W->OverlapAnyTestByChannel(FVector(P2.X, P2.Y, Z + 300.f), FQuat::Identity,
		ECC_WorldStatic, FCollisionShape::MakeSphere(220.f), Params))
	{
		return false;
	}

	// OPEN SKY: anything overhead within six metres means we are under a building,
	// a bridge deck or an awning, where a tree has no business growing.
	FHitResult Above;
	if (W->LineTraceSingleByChannel(Above, FVector(P2.X, P2.Y, Z + 100.f),
		FVector(P2.X, P2.Y, Z + 600.f), ECC_Visibility, Params))
	{
		return false;
	}

	// SPACING: the ring has to read as a district, not a heap.
	const FVector Here(P2.X, P2.Y, Z);
	for (const FVector& T : Taken)
	{
		if (FVector::Dist2D(T, Here) < MinSpacing) { return false; }
	}

	Out = Here;
	Taken.Add(Here);
	return true;
}

void AVEILContentPass::BuildCrystal(const FVector& At)
{
	// FOUR FACETED SHARDS, the tallest leaning on the golden angle. The Deep's own
	// material is emissive blue, so the cluster reads as signal crystal at night.
	Part(Cone, At + FVector(0.f, 0.f, 320.f), FVector(1.5f, 1.5f, 6.4f),
		FRotator::ZeroRotator, DeepBlue, true);
	for (int32 i = 1; i <= 3; ++i)
	{
		const float A = FMath::DegreesToRadians(i * ContentGoldenDeg);
		const float H = 3.6f / FMath::Pow(ContentPhi, (float)i * 0.5f);
		Part(Cone, At + FVector(FMath::Cos(A) * 90.f, FMath::Sin(A) * 90.f, H * 50.f),
			FVector(0.9f, 0.9f, H), FRotator(14.f, i * 41.f, 9.f), DeepBlue, true);
	}
	++Crystals;
}

void AVEILContentPass::BuildTree(const FVector& At)
{
	// TRUNK AND THREE CROWNS. Gold wood, blue leaves: the world is warm, the signal
	// is blue, and a tree in this district shows both rather than pretending.
	Part(Cylinder, At + FVector(0.f, 0.f, 340.f), FVector(0.42f, 0.42f, 6.8f),
		FRotator::ZeroRotator, Gold, true);
	float Z = 620.f;
	float R = 2.5f;
	for (int32 i = 0; i < 3; ++i)
	{
		Part(Cone, At + FVector(0.f, 0.f, Z), FVector(R, R, R * 2.1f),
			FRotator(0.f, i * 23.f, 0.f), DeepBlue, false);
		Z += R * 90.f;
		R /= ContentPhi;
	}
	++Trees;
}

void AVEILContentPass::BuildStall(const FVector& At, float YawDeg)
{
	// A COUNTER, FOUR POSTS, AN AWNING. A market stall is a silhouette, not an
	// asset: this one is walk-around and can be stood behind.
	const FRotator Yaw(0.f, YawDeg, 0.f);
	const FVector Fwd = Yaw.Vector();
	const FVector Right = FVector::CrossProduct(FVector::UpVector, Fwd);

	Part(Cube, At + FVector(0.f, 0.f, 48.f), FVector(2.3f, 1.0f, 0.55f), Yaw, Gold, true);
	for (int32 i = 0; i < 4; ++i)
	{
		const float SX = (i & 1) ? 1.f : -1.f;
		const float SY = (i & 2) ? 1.f : -1.f;
		const FVector Corner = Right * (SX * 115.f) + Fwd * (SY * 55.f);
		Part(Cylinder, At + Corner + FVector(0.f, 0.f, 135.f), FVector(0.12f, 0.12f, 2.7f),
			FRotator::ZeroRotator, Gold, true);
	}
	Part(Cube, At + FVector(0.f, 0.f, 268.f), FVector(2.7f, 1.5f, 0.10f),
		FRotator(-12.f, YawDeg, 0.f), DeepBlue, false);
	++Stalls;
}

void AVEILContentPass::BuildTent(const FVector& At, float YawDeg)
{
	// A MARKET TENT IS A CANOPY ON FOUR POLES. Two cloth planes cross under the peak
	// so the roof reads as canvas instead of a floating slab, and the poles are wood.
	const FRotator Yaw(0.f, YawDeg, 0.f);
	const FVector Fwd = Yaw.Vector();
	const FVector Right = FVector::CrossProduct(FVector::UpVector, Fwd);
	for (int32 i = 0; i < 4; ++i)
	{
		const float SX = (i & 1) ? 1.f : -1.f;
		const float SY = (i & 2) ? 1.f : -1.f;
		const FVector Corner = Right * (SX * 130.f) + Fwd * (SY * 130.f);
		Part(Cylinder, At + Corner + FVector(0.f, 0.f, 130.f), FVector(0.10f, 0.10f, 2.6f),
			FRotator::ZeroRotator, Gold, true);
	}
	Part(Cube, At + FVector(0.f, 0.f, 268.f), FVector(3.0f, 3.0f, 0.10f), Yaw, DeepBlue, false);
	Part(Cube, At + FVector(0.f, 0.f, 232.f), FVector(3.1f, 3.1f, 0.06f),
		FRotator(0.f, YawDeg + 45.f, 0.f), Gold, false);
	++Tents;
}

void AVEILContentPass::BuildBridge(const FVector& A, const FVector& B)
{
	// PLANKS AND TWO ROPES, laid between two spots that both have real ground. Each
	// plank sits on the sag line, so the deck dips instead of running dead flat.
	const FVector Delta = B - A;
	const float Len = Delta.Size2D();
	if (Len < 200.f) { return; }
	const FRotator Facing = Delta.Rotation();
	const int32 Planks = FMath::Max(6, FMath::RoundToInt(Len / 90.f));
	const float Sag = Len * 0.05f;

	for (int32 i = 0; i <= Planks; ++i)
	{
		const float T = (float)i / (float)Planks;
		const FVector P = FMath::Lerp(A, B, T) - FVector(0.f, 0.f, FMath::Sin(T * PI) * Sag);
		Part(Cube, P + FVector(0.f, 0.f, 6.f), FVector(0.95f, 0.22f, 0.05f), Facing, Gold, true);
	}

	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		const FVector Off = Facing.RotateVector(FVector(0.f, Side * 95.f, 0.f));
		const FVector Mid = (A + B) * 0.5f + Off - FVector(0.f, 0.f, Sag * 0.5f);
		const FRotator RopeRot = FRotationMatrix::MakeFromZ(Delta + FVector(0.f, 0.f, -Sag)).Rotator();
		Part(Cylinder, Mid + FVector(0.f, 0.f, 92.f), FVector(0.05f, 0.05f, Len / 100.f), RopeRot, Gold, false);
	}
	++Bridges;
}

void AVEILContentPass::Populate(bool bCompositionExists)
{
	UWorld* W = GetWorld();
	if (!W) { return; }

	Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	Cone = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
	Gold = LoadObject<UMaterialInterface>(nullptr, GoldPath);
	DeepBlue = LoadObject<UMaterialInterface>(nullptr, BluePath);
	if (!Cube || !Cylinder || !Cone) { return; }

	// ANCHOR IS THE RUNNER, NOT THE WORLD ORIGIN. This is the correction that
	// matters: origin is 2,200 cm from where he wakes, inside the city.
	FVector Anchor(0.f, 0.f, 100.f);
	if (const APawn* P = UGameplayStatics::GetPlayerPawn(W, 0)) { Anchor = P->GetActorLocation(); }

	// His floor: the first ground within 3 m up and 15 m down of his feet. The seat
	// code has already moved him off roofs, so this is the pavement he may walk on.
	{
		FCollisionQueryParams Params(SCENE_QUERY_STAT(VEILContentFloor), false, this);
		FHitResult Floor;
		if (W->LineTraceSingleByChannel(Floor, Anchor + FVector(0.f, 0.f, 300.f),
			Anchor - FVector(0.f, 0.f, 1500.f), ECC_Visibility, Params))
		{
			Anchor.Z = Floor.ImpactPoint.Z;
		}
	}

	Taken.Reset();
	int32 Tried = 0;
	int32 Refused = 0;

	// The sweep itself: golden angle per candidate, a bounded ring per family, and
	// every refusal counted rather than papered over.
	auto Sweep = [&](const TCHAR* Family, int32 Wanted, float BaseR, float StepR,
		float MinSpacing, float SeedDeg, TFunctionRef<void(const FVector&, float)> Place)
	{
		int32 Placed = 0;
		for (int32 i = 0; i < Wanted * 4 && Placed < Wanted; ++i)
		{
			const float Ang = SeedDeg + i * ContentGoldenDeg;
			const float R = BaseR + StepR * (float)(i % 6);
			FVector Spot;
			++Tried;
			if (!FindSpot(Anchor, R, Ang, MinSpacing, Spot)) { ++Refused; continue; }
			Place(Spot, Ang);
			++Placed;
			UE_LOG(LogVEIL, Display,
				TEXT("PHI CONTENT PLACE: %s %d at %s, %.0f cm from the runner."),
				Family, Placed, *Spot.ToCompactString(), FVector::Dist2D(Spot, Anchor));
		}
		if (Placed < Wanted)
		{
			UE_LOG(LogVEIL, Warning, TEXT("PHI CONTENT SHORT: %s placed %d of %d - no more open ground on that ring."),
				Family, Placed, Wanted);
		}
	};

	// ONE AUTHOR PER FAMILY. The concept composition already lays this district's
	// market, bridges and crystal peaks - 236 pieces - so when it is present this
	// pass adds only what nothing else places: the trees. Two systems laying
	// crystals on the same plaza is exactly the clutter this pass was meant to cure.
	if (!bCompositionExists)
	{
	// STALLS ring the runner at 9-12 m, facing him.
	Sweep(TEXT("stall"), 8, 900.f, 90.f, 620.f, 0.f, [this](const FVector& Spot, float Ang)
	{
		BuildStall(Spot, FMath::RadiansToDegrees(Ang) + 180.f);
	});
	// TENTS one ring further out, between the stalls and the trees.
	Sweep(TEXT("tent"), 8, 1500.f, 90.f, 620.f, 137.f, [this](const FVector& Spot, float Ang)
	{
		BuildTent(Spot, FMath::RadiansToDegrees(Ang) + 180.f);
	});
	// CRYSTALS on the golden sweep at 23-29 m.
	Sweep(TEXT("crystal"), 14, 2300.f, 110.f, 700.f, 37.f, [this](const FVector& Spot, float)
	{
		BuildCrystal(Spot);
	});
	// TREES at 32-40 m, the edge of the open ground.
	// BRIDGES: pairs of open spots on the outer ring, joined across what the market
	// does not already fill.
	// A BRIDGE NEEDS TWO OPEN SPOTS, so it walks outward instead of demanding the
	// one ring the city may have already filled: the first radius where both ends
	// answer is the radius the deck is built at.
	for (int32 b = 0; b < 3; ++b)
	{
		const float A1 = 240.f + b * ContentGoldenDeg;
		const float A2 = A1 + 46.f;
		for (float R = 1800.f; R <= 5200.f; R += 350.f)
		{
			FVector PA, PB;
			if (!FindSpot(Anchor, R, A1, 300.f, PA)) { ++Refused; continue; }
			if (!FindSpot(Anchor, R, A2, 300.f, PB)) { ++Refused; continue; }
			BuildBridge(PA, PB);
			UE_LOG(LogVEIL, Display, TEXT("PHI CONTENT PLACE: bridge %d from %s to %s over %.0f cm."),
				Bridges, *PA.ToCompactString(), *PB.ToCompactString(), FVector::Dist2D(PA, PB));
			break;
		}
	}
	}

	// TREES SHIP IN EVERY DISTRICT. Nothing else in the game lays them, so this is
	// the one sweep that runs whether or not the composition is already present.
	Sweep(TEXT("tree"), 18, 3200.f, 130.f, 700.f, 101.f, [this](const FVector& Spot, float)
	{
		BuildTree(Spot);
	});

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI CONTENT: crystals=%d trees=%d stalls=%d tents=%d bridges=%d built by phi rule around the runner at %s, %d parts total (tried=%d, refused=%d, composition=%d)."),
		Crystals, Trees, Stalls, Tents, Bridges, *Anchor.ToCompactString(), Parts.Num(), Tried, Refused,
		bCompositionExists ? 1 : 0);
}
