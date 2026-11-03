#include "World/VEILContentFamilies.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
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

bool AVEILContentPass::GroundAt(float X, float Y, float& OutZ) const
{
	UWorld* W = GetWorld();
	if (!W) { return false; }
	FHitResult Hit;
	FCollisionQueryParams P(SCENE_QUERY_STAT(VEILContentGround), false, this);
	if (!W->LineTraceSingleByChannel(Hit, FVector(X, Y, 60000.f), FVector(X, Y, -20000.f), ECC_Visibility, P))
	{
		return false;
	}
	OutZ = Hit.ImpactPoint.Z;
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
	// A COUNTER, FOUR POSTS, AN AWNING AND A LAMP. A market stall is a silhouette,
	// not an asset: this one is walk-around and can be stood behind.
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
	// PLANKS AND TWO ROPES, laid between two pieces of ground that exist. Each plank
	// sits on the sag line, so the deck dips instead of running dead flat.
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

void AVEILContentPass::Populate()
{
	UWorld* W = GetWorld();
	if (!W) { return; }

	Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	Cone = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
	Gold = LoadObject<UMaterialInterface>(nullptr, GoldPath);
	DeepBlue = LoadObject<UMaterialInterface>(nullptr, BluePath);
	if (!Cube || !Cylinder || !Cone) { return; }

	// CRYSTALS: fourteen clusters on the golden spiral, 900 cm out and climbing.
	for (int32 c = 0; c < 14; ++c)
	{
		const float Ang = FMath::DegreesToRadians(c * ContentGoldenDeg + 37.f);
		const float R = 900.f + 120.f * c;
		float Z = 0.f;
		if (!GroundAt(FMath::Cos(Ang) * R, FMath::Sin(Ang) * R, Z)) { continue; }
		BuildCrystal(FVector(FMath::Cos(Ang) * R, FMath::Sin(Ang) * R, Z));
	}

	// TREES: eighteen, further out, on the same law.
	for (int32 i = 0; i < 18; ++i)
	{
		const float Ang = FMath::DegreesToRadians(i * ContentGoldenDeg + 101.f);
		const float R = 1400.f + 160.f * i;
		float Z = 0.f;
		if (!GroundAt(FMath::Cos(Ang) * R, FMath::Sin(Ang) * R, Z)) { continue; }
		BuildTree(FVector(FMath::Cos(Ang) * R, FMath::Sin(Ang) * R, Z));
	}

	// STALLS: eight round the plaza, each facing the centre it sells to.
	for (int32 s = 0; s < 8; ++s)
	{
		const float Ang = FMath::DegreesToRadians(s * ContentGoldenDeg);
		const float R = 760.f;
		float Z = 0.f;
		if (!GroundAt(FMath::Cos(Ang) * R, FMath::Sin(Ang) * R, Z)) { continue; }
		BuildStall(FVector(FMath::Cos(Ang) * R, FMath::Sin(Ang) * R, Z),
			FMath::RadiansToDegrees(Ang) + 180.f);
	}

	// TENTS: eight market canopies on the ring between the stalls and the trees.
	for (int32 t = 0; t < 8; ++t)
	{
		const float Ang = FMath::DegreesToRadians(t * ContentGoldenDeg + 137.f);
		const float R = 620.f;
		float Z = 0.f;
		if (!GroundAt(FMath::Cos(Ang) * R, FMath::Sin(Ang) * R, Z)) { continue; }
		BuildTent(FVector(FMath::Cos(Ang) * R, FMath::Sin(Ang) * R, Z),
			FMath::RadiansToDegrees(Ang) + 180.f);
	}

	// BRIDGES: three, between two points that both have real ground under them.
	for (int32 b = 0; b < 3; ++b)
	{
		const float Ang = FMath::DegreesToRadians(b * ContentGoldenDeg + 240.f);
		const FVector2D A2(FMath::Cos(Ang) * 2000.f, FMath::Sin(Ang) * 2000.f);
		const FVector2D B2(FMath::Cos(Ang) * 3200.f, FMath::Sin(Ang) * 3200.f);
		float ZA = 0.f, ZB = 0.f;
		if (!GroundAt(A2.X, A2.Y, ZA) || !GroundAt(B2.X, B2.Y, ZB)) { continue; }
		BuildBridge(FVector(A2.X, A2.Y, ZA), FVector(B2.X, B2.Y, ZB));
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI CONTENT: crystals=%d trees=%d stalls=%d tents=%d bridges=%d built by phi rule, %d parts total."),
		Crystals, Trees, Stalls, Tents, Bridges, Parts.Num());
}
