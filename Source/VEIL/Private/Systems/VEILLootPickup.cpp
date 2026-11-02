#include "Systems/VEILLootPickup.h"
#include "Character/VEILCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/VEILProgression.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Systems/PHIMito.h"
#include "VEIL.h"

namespace
{
	// Tier colour, lifted by phi so each rung is visibly hotter than the last.
	FLinearColor TierColour(int32 Tier)
	{
		static const FLinearColor Base[] = {
			FLinearColor(0.62f, 0.78f, 1.00f), // SHARD  - cool signal blue
			FLinearColor(0.45f, 1.00f, 0.72f), // CORE   - green phase
			FLinearColor(1.00f, 0.72f, 0.24f), // RELIC  - gold
			FLinearColor(1.00f, 0.36f, 0.62f)  // SIGNET - hot magenta
		};
		return Base[FMath::Clamp(Tier, 0, 3)];
	}
}

AVEILLootPickup::AVEILLootPickup()
{
	PrimaryActorTick.bCanEverTick = true;

	Touch = CreateDefaultSubobject<USphereComponent>(TEXT("Touch"));
	RootComponent = Touch;
	Touch->InitSphereRadius(150.f);
	Touch->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Touch->SetCollisionResponseToAllChannels(ECR_Ignore);
	Touch->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Touch->SetGenerateOverlapEvents(true);

	Core = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Core"));
	Core->SetupAttachment(RootComponent);
	Core->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Core->SetRelativeScale3D(FVector(0.16f, 0.16f, 0.16f));

	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(RootComponent);
	Light->SetIntensity(6000.f);
	Light->SetAttenuationRadius(500.f);
	Light->SetCastShadows(false);
}

void AVEILLootPickup::BeginPlay()
{
	Super::BeginPlay();

	// The drop sits on the floor it fell on, lit in its own tier's colour.
	if (UStaticMesh* Ball = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		Core->SetStaticMesh(Ball);
	}
	const FLinearColor Colour = TierColour(Tier);
	Light->SetLightColor(Colour);
	if (UMaterialInterface* Base = Core->GetMaterial(0))
	{
		if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this))
		{
			MID->SetVectorParameterValue(TEXT("Color"), Colour);
			MID->SetVectorParameterValue(TEXT("EmissiveColor"), Colour * (6.f + Tier * UPHIMito::PhiPow(1.f)));
			Core->SetMaterial(0, MID);
		}
	}

	Touch->OnComponentBeginOverlap.AddDynamic(this, &AVEILLootPickup::OnTouch);
	UE_LOG(LogVEIL, Log, TEXT("PHI DROP: %s (tier %d) lies at %s - walk over it to take it."),
		*ItemName.ToString(), Tier, *GetActorLocation().ToCompactString());
}

void AVEILLootPickup::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	// A slow turn on the golden section: a drop in the world, not a UI badge.
	Spin += DeltaSeconds * (UPHIMito::GoldenAngleDeg / UPHIMito::GoldenAngleDeg);
	Core->SetRelativeRotation(FRotator(Spin * 40.f, Spin * UPHIMito::GoldenAngleDeg, 0.f));
}

void AVEILLootPickup::OnTouch(UPrimitiveComponent*, AActor* OtherActor, UPrimitiveComponent*,
	int32, bool, const FHitResult&)
{
	if (bTaken || !OtherActor) return;
	AVEILCharacter* Runner = Cast<AVEILCharacter>(OtherActor);
	if (!Runner) return; // only the runner takes a drop
	bTaken = true;

	// A drop is power: it closes wounds and raises his damage, tier by tier.
	Runner->ApplyDrop(Tier);

	UWorld* World = GetWorld();
	if (UVEILProgression* Ledger = World ? World->GetSubsystem<UVEILProgression>() : nullptr)
	{
		Ledger->AddLoot(ItemName, GoldValue); // banks it, and writes the one save
	}

	UE_LOG(LogVEIL, Log, TEXT("PHI PICKUP: the runner takes %s (+%d gold) at %s."),
		*ItemName.ToString(), GoldValue, *GetActorLocation().ToCompactString());
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.5f, TierColour(Tier).ToFColor(true),
			FString::Printf(TEXT("%s  +%d GOLD"), *ItemName.ToString(), GoldValue));
	}

	Destroy();
}
