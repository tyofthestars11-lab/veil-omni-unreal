#include "World/VEILHubMarket.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "VEIL.h"

namespace
{
	// The golden angle, to the digit the vision states. Placement only - no random seed,
	// no jitter, so the same district is the same district on every boot.
	constexpr float HubGoldenDeg = 137.50776405f;
	const TCHAR* HubGoldPath = TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic");
	const TCHAR* HubBluePath = TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue");
	const TCHAR* HubGlowPath = TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive");
	const TCHAR* HubStonePath = TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone");
}

AVEILHubMarket::AVEILHubMarket()
{
	PrimaryActorTick.bCanEverTick = false;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("HubMarketRoot"));
	SetRootComponent(Root);
}

UStaticMeshComponent* AVEILHubMarket::Part(UStaticMesh* Mesh, const FVector& Loc, const FVector& Scale,
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

bool AVEILHubMarket::GroundAt(float X, float Y, FVector& Out) const
{
	const UWorld* W = GetWorld();
	if (!W) { return false; }
	FCollisionQueryParams Params(SCENE_QUERY_STAT(VEILHubGround), false, this);
	FHitResult Hit;
	if (!W->LineTraceSingleByChannel(Hit, FVector(X, Y, 3000.f), FVector(X, Y, -3000.f), ECC_Visibility, Params))
	{
		return false;
	}
	Out = FVector(X, Y, (float)Hit.ImpactPoint.Z);
	return true;
}

void AVEILHubMarket::BuildPillar(const FVector& At, float YawDeg)
{
	// PLINTH, SHAFT, CAPITAL, LANTERN, BANNER. The reference pillar is a lit column with a
	// phi banner down its face and a lamp on top, not a bare cylinder.
	const FRotator Yaw(0.f, YawDeg, 0.f);
	const FVector Fwd = Yaw.Vector();

	Part(Cube, At + FVector(0.f, 0.f, 25.f), FVector(1.7f, 1.7f, 0.5f), Yaw, Gold, true);
	Part(Cylinder, At + FVector(0.f, 0.f, 470.f), FVector(0.55f, 0.55f, 8.8f), Yaw, Gold, true);
	Part(Cube, At + FVector(0.f, 0.f, 915.f), FVector(1.6f, 1.6f, 0.35f), Yaw, Gold, true);
	// The lamp: an emissive body between two gold caps, so it reads as lit at distance.
	Part(Cube, At + FVector(0.f, 0.f, 990.f), FVector(0.85f, 0.85f, 0.85f), Yaw, Glow, false);
	Part(Cube, At + FVector(0.f, 0.f, 1045.f), FVector(1.1f, 1.1f, 0.16f), Yaw, Gold, false);
	// REAL LIGHT, NOT JUST AN EMISSIVE CUBE. The reference market is lit by its lanterns;
	// an emissive surface without a light source leaves the plaza black at night.
	if (UWorld* W = GetWorld())
	{
		if (APointLight* Lamp = W->SpawnActor<APointLight>(APointLight::StaticClass(),
			At + FVector(0.f, 0.f, 1010.f), FRotator::ZeroRotator))
		{
			if (UPointLightComponent* L = Cast<UPointLightComponent>(Lamp->GetLightComponent()))
			{
				L->SetIntensity(3200.f);
				L->SetAttenuationRadius(1600.f);
				L->SetLightColor(FLinearColor(1.f, 0.70f, 0.36f));
			}
		}
	}

	// Banner down the face, blue cloth with gold rails top and bottom, and the phi mark.
	const FVector Face = At + Fwd * 44.f;
	Part(Cube, Face + FVector(0.f, 0.f, 560.f), FVector(0.05f, 1.05f, 3.1f), Yaw, Blue, false);
	Part(Cube, Face + FVector(0.f, 0.f, 715.f), FVector(0.07f, 1.15f, 0.10f), Yaw, Gold, false);
	Part(Cube, Face + FVector(0.f, 0.f, 405.f), FVector(0.07f, 1.15f, 0.10f), Yaw, Gold, false);
	// A thin gold square turned 45 degrees on its face: the phi lozenge the reference paints.
	Part(Cube, Face + Fwd * 4.f + FVector(0.f, 0.f, 560.f), FVector(0.06f, 0.62f, 0.62f),
		FRotator(0.f, YawDeg, 45.f), Gold, false);

	++Pillars;
	++Banners;
}

void AVEILHubMarket::BuildStall(const FVector& At, float YawDeg)
{
	// COUNTER, FOUR POSTS, BLUE CANVAS. The awning is DeepBlue cloth with a gold valance,
	// which is the one change that makes the market read as the reference instead of a
	// row of gold boxes.
	const FRotator Yaw(0.f, YawDeg, 0.f);
	const FVector Fwd = Yaw.Vector();
	const FVector Right = FVector::CrossProduct(FVector::UpVector, Fwd);

	Part(Cube, At + FVector(0.f, 0.f, 48.f), FVector(2.7f, 1.15f, 0.95f), Yaw, Stone, true);
	for (int32 i = 0; i < 4; ++i)
	{
		const float SX = (i & 1) ? 1.f : -1.f;
		const float SY = (i & 2) ? 1.f : -1.f;
		const FVector Corner = Right * (SX * 125.f) + Fwd * (SY * 58.f);
		Part(Cylinder, At + Corner + FVector(0.f, 0.f, 132.f), FVector(0.10f, 0.10f, 2.65f), Yaw, Gold, true);
	}
	Part(Cube, At + FVector(0.f, 0.f, 282.f), FVector(3.1f, 1.8f, 0.08f), FRotator(-12.f, YawDeg, 0.f), Blue, false);
	Part(Cube, At + FVector(0.f, 0.f, 247.f), FVector(3.1f, 0.06f, 0.42f), Yaw, Blue, false);
	Part(Cube, At + FVector(0.f, 0.f, 269.f), FVector(3.1f, 0.06f, 0.06f), Yaw, Gold, false);

	++Stalls;
}

void AVEILHubMarket::BuildCrystal(const FVector& At)
{
	// FOUR FACETED SHARDS. Faceted means tilted cones of different heights on the golden
	// angle, not one flat slab - the vision's crystal peaks catch the sky that way.
	Part(Cone, At + FVector(0.f, 0.f, 460.f), FVector(1.7f, 1.7f, 9.2f), FRotator(4.f, 12.f, 3.f), Blue, true);
	for (int32 i = 1; i <= 3; ++i)
	{
		const float A = FMath::DegreesToRadians(i * HubGoldenDeg);
		const float H = 5.4f / FMath::Pow(1.618034f, (float)i * 0.5f);
		Part(Cone, At + FVector(FMath::Cos(A) * 130.f, FMath::Sin(A) * 130.f, H * 50.f),
			FVector(1.0f, 1.0f, H), FRotator(16.f, i * 47.f, 10.f), Blue, true);
	}
	++Crystals;
}

void AVEILHubMarket::BuildHouse(const FVector& At, float YawDeg)
{
	// THE BLUE-ROOFED HOUSE. Stone below, blue pyramid above, warm windows: the
	// reference's town is blue-roofed and lamplit, not a row of gold boxes.
	const FRotator Yaw(0.f, YawDeg, 0.f);
	Part(Cube, At + FVector(0.f, 0.f, 210.f), FVector(4.4f, 4.4f, 4.2f), Yaw, Stone, true);
	Part(Cone, At + FVector(0.f, 0.f, 500.f), FVector(4.9f, 4.9f, 2.4f), Yaw, Blue, true);
	Part(Cube, At + FVector(0.f, 0.f, 432.f), FVector(4.7f, 4.7f, 0.16f), Yaw, Gold, false);
	for (int32 W = 0; W < 4; ++W)
	{
		const float A = FMath::DegreesToRadians((float)W * 90.f);
		const FVector Out(FMath::Cos(A), FMath::Sin(A), 0.f);
		Part(Cube, At + Out * 224.f + FVector(0.f, 0.f, 250.f), FVector(0.9f, 0.9f, 0.9f), Yaw, Glow, false);
	}
	++Houses;
}

void AVEILHubMarket::Build()
{
	UWorld* W = GetWorld();
	if (!W) { return; }

	Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	Cone = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
	Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane"));
	Gold = LoadObject<UMaterialInterface>(nullptr, HubGoldPath);
	Blue = LoadObject<UMaterialInterface>(nullptr, HubBluePath);
	Glow = LoadObject<UMaterialInterface>(nullptr, HubGlowPath);
	Stone = LoadObject<UMaterialInterface>(nullptr, HubStonePath);
	if (!Cube || !Cylinder || !Cone) { return; }

	// The Hub's own authority point: 0,0. Ground is asked for, never assumed - a pillar that
	// cannot find pavement is skipped and counted, not floated.
	int32 Missing = 0;
	auto Place = [&](float Radius, float Index, float SeedDeg, TFunctionRef<void(const FVector&, float)> Build)
	{
		const float Ang = FMath::DegreesToRadians(SeedDeg + Index * HubGoldenDeg);
		const float X = FMath::Cos(Ang) * Radius, Y = FMath::Sin(Ang) * Radius;
		FVector At;
		if (!GroundAt(X, Y, At)) { ++Missing; return; }
		Build(At, FMath::RadiansToDegrees(Ang) + 90.f);
	};

	for (int32 i = 0; i < 12; ++i)
	{
		Place(980.f + 60.f * (float)(i % 3), (float)i, 8.f,
			[this](const FVector& At, float Yaw) { BuildPillar(At, Yaw); });
	}
	for (int32 i = 0; i < 8; ++i)
	{
		Place(1560.f + 70.f * (float)(i % 2), (float)i, 63.f,
			[this](const FVector& At, float Yaw) { BuildStall(At, Yaw); });
	}
	for (int32 i = 0; i < 5; ++i)
	{
		Place(2140.f + 90.f * (float)i, (float)i, 29.f,
			[this](const FVector& At, float) { BuildCrystal(At); });
	}
	// THE EIGHT MARKET HOUSES, on their own ring between the stalls and the peaks.
	for (int32 i = 0; i < 8; ++i)
	{
		Place(1260.f + 70.f * (float)(i % 3), (float)i, 47.f,
			[this](const FVector& At, float Yaw) { BuildHouse(At, Yaw); });
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI HUB MARKET: %d phi pillar(s) with lamps and banners, %d blue-canvas stall(s), %d blue-roofed house(s), %d crystal peak(s); %d spot(s) had no ground and were not floated."),
		Pillars, Stalls, Houses, Crystals, Missing);
}
