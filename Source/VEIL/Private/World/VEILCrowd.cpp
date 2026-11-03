#include "World/VEILCrowd.h"

#include "Animation/AnimInstance.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "VEIL.h"

namespace
{
	constexpr float CrowdGoldenDeg = 137.50776405f;
	const TCHAR* CrowdStonePath = TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone");
	const TCHAR* CrowdGoldPath = TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic");
	const TCHAR* CrowdQuinnPath = TEXT("/Game/Polyart/SharedResources/Characters/Meshes/SkeletalMeshes/SK_Quinn_Simple.SK_Quinn_Simple");
	const TCHAR* CrowdMetaHumanPath = TEXT("/Game/MetaHumans/MH_Runner_Built/MH_Runner/Body/SKM_MH_Runner_BodyMesh.SKM_MH_Runner_BodyMesh");
	const TCHAR* CrowdAnimPath = TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/AnimationBlueprints/ABP_MaleCharacter.ABP_MaleCharacter_C");
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
	Bodies->SetVisibility(false);

	Heads = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("Heads"));
	Heads->SetupAttachment(RootComponent);
	Heads->SetMobility(EComponentMobility::Movable);
	Heads->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Heads->SetCullDistances(0, 14000);
	Heads->SetVisibility(false);
}

void AVEILCrowd::Populate(int32 Count)
{
	UWorld* W = GetWorld();
	if (!W || bReady) { return; }
	if (Bodies) { Bodies->SetVisibility(false); }
	if (Heads) { Heads->SetVisibility(false); }

	// PEOPLE, NOT PRIMITIVES. The crowd used to be a cylinder and a sphere per walker: two
	// draw calls, and a market full of architecture wearing stone. The bodies are the Polyart
	// character this project owns, already driven by the Polyart locomotion graph, with the
	// MetaHuman as the second face so a crowd is not one man repeated.
	USkeletalMesh* Quinn = LoadObject<USkeletalMesh>(nullptr, CrowdQuinnPath);
	USkeletalMesh* MetaHuman = LoadObject<USkeletalMesh>(nullptr, CrowdMetaHumanPath);
	UClass* WalkAnim = LoadClass<UAnimInstance>(nullptr, CrowdAnimPath);

	if (!Quinn && !MetaHuman)
	{
		// Last stand: the old instanced bodies, so the plaza is never empty, and the log says why.
		UStaticMesh* BodyMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		UStaticMesh* HeadMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		if (BodyMesh && HeadMesh && Bodies && Heads)
		{
			if (UMaterialInterface* Stone = LoadObject<UMaterialInterface>(nullptr, CrowdStonePath)) { Bodies->SetMaterial(0, Stone); }
			if (UMaterialInterface* Gold = LoadObject<UMaterialInterface>(nullptr, CrowdGoldPath)) { Heads->SetMaterial(0, Gold); }
			Bodies->SetStaticMesh(BodyMesh);
			Heads->SetStaticMesh(HeadMesh);
			Bodies->SetVisibility(true);
			Heads->SetVisibility(true);
		}
		UE_LOG(LogVEIL, Warning, TEXT("PHI CROWD FALLBACK: no character mesh resolved - instanced bodies stand."));
		return;
	}

	const int32 N = FMath::Clamp(Count, 4, 18);
	Walkers.Reserve(N);
	People.Reserve(N);

	for (int32 i = 0; i < N; ++i)
	{
		FVEILWalker Walker;
		Walker.Angle = FMath::DegreesToRadians(i * CrowdGoldenDeg);
		// The sunflower rule: radius grows with sqrt(i), so people spread across the market
		// instead of drawing one bright circle.
		Walker.Radius = 900.f + 240.f * FMath::Sqrt((float)i);
		Walker.Speed = 22.f + 8.f * (float)(i % 5);
		Walker.Phase = i * 0.6180339887f;
		Walker.Scale = 0.96f + 0.06f * (float)(i % 3);
		Walker.GroundZ = 98.f;

		FHitResult Hit;
		const FVector At(FMath::Cos(Walker.Angle) * Walker.Radius, FMath::Sin(Walker.Angle) * Walker.Radius, 30000.f);
		if (W->LineTraceSingleByChannel(Hit, At, FVector(At.X, At.Y, -4000.f), ECC_Visibility, FCollisionQueryParams::DefaultQueryParam))
		{
			Walker.GroundZ = Hit.ImpactPoint.Z;
		}

		USkeletalMeshComponent* Person = NewObject<USkeletalMeshComponent>(this,
			*FString::Printf(TEXT("Person%d"), i));
		if (!Person) { continue; }
		Person->SetupAttachment(RootComponent);
		Person->SetSkeletalMeshAsset((MetaHuman && (i % 4) == 3) ? MetaHuman : Quinn);
		// Bone-driven body: keep it out of FMorphVertexBufferPool (assert :336).
		Person->bDisableMorphTarget = true;
		Person->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Person->SetMobility(EComponentMobility::Movable);
		if (WalkAnim)
		{
			Person->SetAnimationMode(EAnimationMode::AnimationBlueprint);
			Person->SetAnimInstanceClass(WalkAnim);
		}
		Person->RegisterComponent();

		People.Add(Person);
		Walkers.Add(Walker);
	}

	bReady = true;
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI CROWD: %d person(s) standing in the market on golden orbits - %d skeletal bod(ies), anim %s."),
		Walkers.Num(), People.Num(), WalkAnim ? TEXT("driven") : TEXT("bind pose"));
}

void AVEILCrowd::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bReady || Walkers.Num() == 0) { return; }

	Clock += DeltaSeconds;

	for (int32 i = 0; i < Walkers.Num() && i < People.Num(); ++i)
	{
		FVEILWalker& Walker = Walkers[i];
		USkeletalMeshComponent* Person = People[i];
		if (!Person) { continue; }

		// Arc rate is constant, so bodies turn slower on the wide orbits: the same
		// walking speed at every radius instead of a carousel.
		Walker.Angle += (Walker.Speed / FMath::Max(Walker.Radius, 1.f)) * DeltaSeconds;

		const float Breath = FMath::Sin(Clock * 1.1f + Walker.Phase) * 60.f;
		const float R = Walker.Radius + Breath;
		const float X = FMath::Cos(Walker.Angle) * R;
		const float Y = FMath::Sin(Walker.Angle) * R;
		const float Bob = FMath::Abs(FMath::Sin(Clock * 2.4f + Walker.Phase)) * 3.f;

		// Facing is the tangent of the orbit, so people look where they walk.
		const float FacingYaw = FMath::RadiansToDegrees(Walker.Angle) + 90.f;
		Person->SetWorldLocation(FVector(X, Y, Walker.GroundZ + Bob));
		Person->SetWorldRotation(FRotator(0.f, FacingYaw, 0.f));
		Person->SetWorldScale3D(FVector(Walker.Scale));
	}
}
