#include "World/VEILCrowd.h"

#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "VEIL.h"

namespace
{
	constexpr float CrowdGoldenDeg = 137.50776405f;
	const TCHAR* CrowdStonePath = TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone");
	const TCHAR* CrowdGoldPath = TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic");
}

AVEILCrowd::AVEILCrowd()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	Bodies = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Bodies"));
	Bodies->SetupAttachment(RootComponent);
	Bodies->SetMobility(EComponentMobility::Movable);
	Bodies->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Bodies->SetCullDistances(0, 14000);

	Heads = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Heads"));
	Heads->SetupAttachment(RootComponent);
	Heads->SetMobility(EComponentMobility::Movable);
	Heads->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Heads->SetCullDistances(0, 14000);
}

void AVEILCrowd::Populate(int32 Count)
{
	UWorld* W = GetWorld();
	if (!W || bReady) { return; }

	UStaticMesh* BodyMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* HeadMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (!BodyMesh || !HeadMesh || !Bodies || !Heads) { return; }

	if (UMaterialInterface* Stone = LoadObject<UMaterialInterface>(nullptr, CrowdStonePath)) { Bodies->SetMaterial(0, Stone); }
	if (UMaterialInterface* Gold = LoadObject<UMaterialInterface>(nullptr, CrowdGoldPath)) { Heads->SetMaterial(0, Gold); }
	Bodies->SetStaticMesh(BodyMesh);
	Heads->SetStaticMesh(HeadMesh);

	const int32 N = FMath::Clamp(Count, 1, 64);
	Walkers.Reserve(N);

	for (int32 i = 0; i < N; ++i)
	{
		FVEILWalker Walker;
		Walker.Angle = FMath::DegreesToRadians(i * CrowdGoldenDeg);
		// Radius grows with the square root of the index: the sunflower rule, so the
		// crowd spreads across the market instead of drawing one bright circle.
		Walker.Radius = 900.f + 240.f * FMath::Sqrt((float)i);
		Walker.Speed = 38.f + 14.f * (float)(i % 5);
		Walker.Phase = i * 0.6180339887f;
		Walker.Scale = 0.9f + 0.25f * (float)(i % 3);
		Walker.GroundZ = 98.f;

		// Stand on the ground it will walk over, once, at build time.
		FHitResult Hit;
		const FVector At(FMath::Cos(Walker.Angle) * Walker.Radius, FMath::Sin(Walker.Angle) * Walker.Radius, 30000.f);
		if (W->LineTraceSingleByChannel(Hit, At, FVector(At.X, At.Y, -4000.f), ECC_Visibility, FCollisionQueryParams::DefaultQueryParam))
		{
			Walker.GroundZ = Hit.ImpactPoint.Z;
		}

		const FTransform BodyT(FRotator::ZeroRotator,
			FVector(FMath::Cos(Walker.Angle) * Walker.Radius, FMath::Sin(Walker.Angle) * Walker.Radius, Walker.GroundZ + 90.f * Walker.Scale),
			FVector(0.34f, 0.34f, 1.8f * Walker.Scale));
		const FTransform HeadT(FRotator::ZeroRotator,
			FVector(BodyT.GetLocation().X, BodyT.GetLocation().Y, Walker.GroundZ + 190.f * Walker.Scale),
			FVector(0.30f, 0.30f, 0.30f));

		Bodies->AddInstance(BodyT, true);
		Heads->AddInstance(HeadT, true);
		Walkers.Add(Walker);
	}

	bReady = true;
	UE_LOG(LogVEIL, Warning, TEXT("PHI CROWD: %d walkers pacing the market on golden orbits, two draw calls."), Walkers.Num());
}

void AVEILCrowd::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bReady || !Bodies || !Heads) { return; }

	Clock += DeltaSeconds;

	for (int32 i = 0; i < Walkers.Num(); ++i)
	{
		FVEILWalker& Walker = Walkers[i];

		// Arc rate is constant, so bodies turn slower on the wide orbits: the same
		// walking speed at every radius instead of a carousel.
		Walker.Angle += (Walker.Speed / FMath::Max(Walker.Radius, 1.f)) * DeltaSeconds;

		const float Breath = FMath::Sin(Clock * 1.1f + Walker.Phase) * 70.f;
		const float R = Walker.Radius + Breath;
		const float X = FMath::Cos(Walker.Angle) * R;
		const float Y = FMath::Sin(Walker.Angle) * R;
		const float Bob = FMath::Abs(FMath::Sin(Clock * 3.2f + Walker.Phase)) * 6.f;

		const FRotator Facing(0.f, FMath::RadiansToDegrees(Walker.Angle) + 90.f, 0.f);
		Bodies->UpdateInstanceTransform(i,
			FTransform(Facing, FVector(X, Y, Walker.GroundZ + 90.f * Walker.Scale + Bob),
				FVector(0.34f, 0.34f, 1.8f * Walker.Scale)), false, false, true);
		Heads->UpdateInstanceTransform(i,
			FTransform(Facing, FVector(X, Y, Walker.GroundZ + 190.f * Walker.Scale + Bob),
				FVector(0.30f, 0.30f, 0.30f)), false, false, true);
	}

	Bodies->MarkRenderStateDirty();
	Heads->MarkRenderStateDirty();
}
