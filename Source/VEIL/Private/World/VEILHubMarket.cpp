#include "World/VEILHubMarket.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "Engine/PointLight.h"
#include "Components/PointLightComponent.h"
#include "VEIL.h"
#include "Systems/VEILPhi.h"

namespace
{
	// The golden angle, to the digit the vision states. Placement only - no random seed,
	// no jitter, so the same district is the same district on every boot.
	constexpr float HubGoldenDeg = VeilPhi::GoldenAngleF;
	const TCHAR* HubGoldPath = TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic");
	const TCHAR* HubBluePath = TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue");
	const TCHAR* HubStonePath = TEXT("/Game/PHI/Palette/M_PHI_CreamStone.M_PHI_CreamStone");
	const TCHAR* HubColumnPath = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big.SM_AncientColumn_Big");
	const TCHAR* HubArchPath = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientArchColumn.SM_AncientArchColumn");
	const TCHAR* HubPlatformPath = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_Platform.SM_Platform");
	const TCHAR* HubStandPath = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_StandBox.SM_StandBox");
	const TCHAR* HubMonolithPath = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientMonolith.SM_AncientMonolith");
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
	// ENABLED IS NOT SOLID. With only SetCollisionEnabled the stone is a picture: every
	// trace passes through it, so the camera probe cannot see the wall the lens is buried
	// in and the frame fills with its inside. VEILWorldBuilder already learned this; the
	// market had the same hole. Object type and responses are what make it an obstacle.
	if (bSolid)
	{
		C->SetCollisionObjectType(ECC_WorldStatic);
		C->SetCollisionResponseToAllChannels(ECR_Block);
	}
	if (Mat) { C->SetMaterial(0, Mat); }
	C->RegisterComponent();
	Parts.Add(C);
	return C;
}

UStaticMeshComponent* AVEILHubMarket::PartAtGround(UStaticMesh* Mesh, const FVector& Ground,
	const FVector& Size, const FRotator& Rot, UMaterialInterface* Mat, bool bSolid)
{
	if (!Mesh) { return nullptr; }
	const FBox Bounds = Mesh->GetBoundingBox();
	const FVector MeshSize = Bounds.GetSize();
	if (MeshSize.X <= KINDA_SMALL_NUMBER || MeshSize.Y <= KINDA_SMALL_NUMBER
		|| MeshSize.Z <= KINDA_SMALL_NUMBER)
	{
		return nullptr;
	}

	const FVector Scale(Size.X / MeshSize.X, Size.Y / MeshSize.Y, Size.Z / MeshSize.Z);
	const FVector CenterOffset = Rot.RotateVector(Bounds.GetCenter() * Scale);
	const FVector ComponentLocation = Ground + FVector(0.f, 0.f, Size.Z * 0.5f) - CenterOffset;
	return Part(Mesh, ComponentLocation, Scale, Rot, Mat, bSolid);
}

bool AVEILHubMarket::GroundAt(float X, float Y, FVector& Out) const
{
	const UWorld* W = GetWorld();
	if (!W) { return false; }
	FCollisionQueryParams Params(SCENE_QUERY_STAT(VEILHubGround), false, this);
	FHitResult Hit;

	// TWO CHANNELS, BECAUSE ONE WAS NOT ENOUGH. The bake reported "25 of 25 sites had no
	// ground" and built an empty market: a single Visibility trace, and in a level loaded
	// headless the paving answered neither pass. Asking WorldStatic as well costs one trace
	// and removes the single point of failure.
	for (const ECollisionChannel Channel : { ECC_Visibility, ECC_WorldStatic })
	{
		if (W->LineTraceSingleByChannel(Hit, FVector(X, Y, 3000.f), FVector(X, Y, -3000.f), Channel, Params))
		{
			Out = FVector(X, Y, (float)Hit.ImpactPoint.Z);
			return true;
		}
	}

	// THE PLAZA IS THE FLOOR. The hub lays its paving at the authority point (0,0) and its top
	// face is z = 0; the market's own sites are inside r = 2,200 cm of that, so zero IS the
	// ground there. When no query answers - a bake with no physics step, or a kit mesh with no
	// simple collision - the market stands on that floor instead of vanishing, and says so.
	static int32 Fallbacks = 0;
	if (++Fallbacks <= 3)
	{
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI HUB MARKET: no trace answered at (%.0f, %.0f); standing the site on the plaza floor z=0."),
			X, Y);
	}
	Out = FVector(X, Y, 0.f);
	return true;
}

void AVEILHubMarket::BuildPillar(const FVector& At, float YawDeg)
{
	// Use the carved ancient-kit column and platform, not stretched engine primitives.
	const FRotator Yaw(0.f, YawDeg, 0.f);
	PartAtGround(PlatformMesh, At, FVector(340.f, 340.f, 110.f), Yaw, Stone, true);
	PartAtGround(ColumnMesh, At + FVector(0.f, 0.f, 110.f),
		FVector(220.f, 220.f, 900.f), Yaw, Gold, true);

	if (UWorld* W = GetWorld())
	{
		if (APointLight* Lamp = W->SpawnActor<APointLight>(APointLight::StaticClass(),
			At + FVector(0.f, 0.f, 1060.f), FRotator::ZeroRotator))
		{
			if (UPointLightComponent* L = Cast<UPointLightComponent>(Lamp->GetLightComponent()))
			{
				L->SetIntensity(3200.f);
				L->SetAttenuationRadius(1600.f);
				L->SetLightColor(FLinearColor(1.f, 0.70f, 0.36f));
			}
		}
	}

	++Pillars;
}

void AVEILHubMarket::BuildStall(const FVector& At, float YawDeg)
{
	// The ancient-kit arch and display stand make an open stall from authored meshes.
	const FRotator Yaw(0.f, YawDeg, 0.f);
	const FVector Fwd = Yaw.Vector();
	PartAtGround(ArchMesh, At, FVector(520.f, 180.f, 360.f), Yaw, Stone, true);
	PartAtGround(StandMesh, At + Fwd * 80.f, FVector(260.f, 150.f, 120.f), Yaw, Gold, true);

	++Stalls;
}

void AVEILHubMarket::BuildPeak(const FVector& At, float YawDeg)
{
	PartAtGround(MonolithMesh, At, FVector(300.f, 300.f, 820.f),
		FRotator(0.f, YawDeg, 0.f), Blue, true);
	++Peaks;
}

void AVEILHubMarket::Build()
{
	UWorld* W = GetWorld();
	if (!W) { return; }

	ColumnMesh = LoadObject<UStaticMesh>(nullptr, HubColumnPath);
	ArchMesh = LoadObject<UStaticMesh>(nullptr, HubArchPath);
	PlatformMesh = LoadObject<UStaticMesh>(nullptr, HubPlatformPath);
	StandMesh = LoadObject<UStaticMesh>(nullptr, HubStandPath);
	MonolithMesh = LoadObject<UStaticMesh>(nullptr, HubMonolithPath);
	Gold = LoadObject<UMaterialInterface>(nullptr, HubGoldPath);
	Blue = LoadObject<UMaterialInterface>(nullptr, HubBluePath);
	Stone = LoadObject<UMaterialInterface>(nullptr, HubStonePath);
	if (!ColumnMesh || !ArchMesh || !PlatformMesh || !StandMesh || !MonolithMesh)
	{
		UE_LOG(LogVEIL, Error,
			TEXT("PHI HUB MARKET: required ancient-kit mesh missing (column, arch, platform, stand, or monolith); no primitive fallback was created."));
		return;
	}

	// The Hub's own authority point: 0,0. Ground is asked for, never assumed - a pillar that
	// cannot find pavement is skipped and counted, not floated.
	int32 Missing = 0;
	auto Place = [&](float Radius, float Index, float PhaseDeg, TFunctionRef<void(const FVector&, float)> Build)
	{
		const double SeededAngle = FMath::Fmod(
			(double)VeilPhi::VisionSeed * VeilPhi::GoldenAngleDeg + PhaseDeg + (double)Index * VeilPhi::GoldenAngleDeg,
			360.0);
		const float Ang = FMath::DegreesToRadians((float)SeededAngle);
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
			[this](const FVector& At, float Yaw) { BuildPeak(At, Yaw); });
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI HUB MARKET: seed %d, %d carved columns, %d arch stalls, %d blue monolith peaks; %d of 25 sites had no ground. Art gaps: no matching crystal mesh or market canopy mesh; no primitive substitutes were made."),
		VeilPhi::VisionSeed, Pillars, Stalls, Peaks, Missing);
}
