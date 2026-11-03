#include "World/VEILMount.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "VEIL.h"

namespace
{
	UStaticMesh* MountCube = nullptr;
	UStaticMesh* MountCyl = nullptr;
	UStaticMesh* MountCone = nullptr;
	UMaterialInterface* MountGold = nullptr;
	UMaterialInterface* MountBlue = nullptr;
	UMaterialInterface* MountGlow = nullptr;
	UMaterialInterface* MountStone = nullptr;

	void LoadMountParts()
	{
		if (MountCube) { return; }
		MountCube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		MountCyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		MountCone = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
		MountGold = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic"));
		MountBlue = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue"));
		MountGlow = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive"));
		MountStone = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
	}
}

AVEILMount::AVEILMount()
{
	PrimaryActorTick.bCanEverTick = false;
	MountRoot = CreateDefaultSubobject<USceneComponent>(TEXT("MountRoot"));
	SetRootComponent(MountRoot);
}

const TCHAR* AVEILMount::KindName() const
{
	switch (Kind)
	{
	case EVEILMountKind::GoldWing: return TEXT("GOLD WING MOUNT");
	case EVEILMountKind::Crystal:  return TEXT("CRYSTAL MOUNT");
	case EVEILMountKind::Market:   return TEXT("MARKET MOUNT");
	case EVEILMountKind::Deep:     return TEXT("DEEP MOUNT");
	}
	return TEXT("MOUNT");
}

void AVEILMount::Configure(EVEILMountKind InKind, const FVector& Post)
{
	Kind = InKind;
	HomePost = Post;
	BuildBody();
}

void AVEILMount::BuildBody()
{
	LoadMountParts();
	if (!MountCube) { return; }

	auto Part = [this](UStaticMesh* Mesh, const FVector& Loc, const FRotator& Rot,
		const FVector& Scale, UMaterialInterface* Mat)
	{
		UStaticMeshComponent* C = NewObject<UStaticMeshComponent>(this);
		if (!C) { return; }
		C->SetupAttachment(MountRoot);
		C->SetStaticMesh(Mesh);
		C->SetRelativeLocation(Loc);
		C->SetRelativeRotation(Rot);
		C->SetRelativeScale3D(Scale);
		C->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (Mat) { C->SetMaterial(0, Mat); }
		C->RegisterComponent();
		Parts.Add(C);
	};

	// BODY, NECK, FOUR LEGS: a beast that stands, in engine shapes, no Fab asset.
	Part(MountCube, FVector(0.f, 0.f, 105.f), FRotator::ZeroRotator, FVector(1.30f, 0.55f, 0.50f), MountStone);
	Part(MountCube, FVector(88.f, 0.f, 145.f), FRotator(0.f, 0.f, -18.f), FVector(0.42f, 0.30f, 0.34f), MountStone);
	for (int32 Side = -1; Side <= 1; Side += 2)
	{
		for (int32 Fore = 0; Fore < 2; ++Fore)
		{
			const float X = Fore ? 62.f : -62.f;
			Part(MountCyl, FVector(X, 24.f * (float)Side, 40.f), FRotator::ZeroRotator,
				FVector(0.09f, 0.09f, 0.42f), MountStone);
		}
	}

	switch (Kind)
	{
	case EVEILMountKind::GoldWing:
		// THE GOLD WING: two swept gold blades over the shoulders, the runner's own motif.
		Part(MountCone, FVector(-30.f, 34.f, 155.f), FRotator(24.f, 0.f, -28.f), FVector(0.30f, 0.06f, 0.90f), MountGold);
		Part(MountCone, FVector(-30.f, -34.f, 155.f), FRotator(24.f, 0.f, 28.f), FVector(0.30f, 0.06f, 0.90f), MountGold);
		break;
	case EVEILMountKind::Crystal:
		// CRYSTAL: a lit blue crest along the spine.
		Part(MountCone, FVector(10.f, 0.f, 175.f), FRotator(0.f, 20.f, 0.f), FVector(0.16f, 0.16f, 0.62f), MountBlue);
		Part(MountCone, FVector(-34.f, 0.f, 168.f), FRotator(0.f, 45.f, 0.f), FVector(0.12f, 0.12f, 0.46f), MountGlow);
		break;
	case EVEILMountKind::Market:
		// MARKET: a blue canvas saddle-cloth and a gold rail, a trade beast.
		Part(MountCube, FVector(0.f, 0.f, 138.f), FRotator::ZeroRotator, FVector(0.52f, 0.62f, 0.05f), MountBlue);
		Part(MountCube, FVector(0.f, 0.f, 160.f), FRotator(0.f, 0.f, 12.f), FVector(0.10f, 0.66f, 0.05f), MountGold);
		break;
	case EVEILMountKind::Deep:
		// DEEP: a dark fin and a cold lamp, the chasm's own animal.
		Part(MountCone, FVector(-26.f, 0.f, 190.f), FRotator(12.f, 0.f, 0.f), FVector(0.08f, 0.42f, 0.84f), MountBlue);
		Part(MountCube, FVector(96.f, 0.f, 158.f), FRotator::ZeroRotator, FVector(0.10f, 0.10f, 0.10f), MountGlow);
		break;
	}
}

void AVEILMount::BeginPlay()
{
	Super::BeginPlay();

	// GROUNDED, MEASURED. The post is where the blueprint puts the mount; the height it
	// stands at is whatever floor is actually under that post, found by trace, never assumed.
	FHitResult Floor;
	FCollisionQueryParams Q(SCENE_QUERY_STAT(MountFloor), false, this);
	const FVector From = HomePost + FVector(0.f, 0.f, 1200.f);
	const FVector To = HomePost - FVector(0.f, 0.f, 1200.f);
	if (GetWorld() && GetWorld()->LineTraceSingleByChannel(Floor, From, To, ECC_Visibility, Q))
	{
		SetActorLocation(FVector(HomePost.X, HomePost.Y, Floor.ImpactPoint.Z));
		UE_LOG(LogVEIL, Warning, TEXT("PHI MOUNT: %s stands on %s at %s, speed %.0f, no float."),
			KindName(), *Floor.GetActor()->GetName(), *GetActorLocation().ToCompactString(), MountSpeed);
	}
	else
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI MOUNT: %s has no floor under its post and is not floated."), KindName());
	}
}
