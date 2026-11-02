#include "Systems/VEILBurstFX.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "Systems/PHIMito.h"
#include "VEIL.h"

namespace
{
	FLinearColor BurstColour(int32 Tier)
	{
		static const FLinearColor Base[] = {
			FLinearColor(0.62f, 0.78f, 1.00f),
			FLinearColor(0.45f, 1.00f, 0.72f),
			FLinearColor(1.00f, 0.72f, 0.24f),
			FLinearColor(1.00f, 0.36f, 0.62f)
		};
		return Base[FMath::Clamp(Tier, 0, 3)];
	}
}

AVEILBurstFX::AVEILBurstFX()
{
	PrimaryActorTick.bCanEverTick = true;

	Shell = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Shell"));
	RootComponent = Shell;
	Shell->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Shell->SetRelativeScale3D(FVector(0.2f));

	Ring = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ring"));
	Ring->SetupAttachment(RootComponent);
	Ring->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Ring->SetRelativeScale3D(FVector(0.3f, 0.3f, 0.02f));

	Flash = CreateDefaultSubobject<UPointLightComponent>(TEXT("Flash"));
	Flash->SetupAttachment(RootComponent);
	Flash->SetCastShadows(false);
	Flash->SetAttenuationRadius(900.f);
}

AVEILBurstFX* AVEILBurstFX::Fire(UWorld* World, const FVector& At, int32 InTier)
{
	if (!World) return nullptr;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AVEILBurstFX* Burst = World->SpawnActorDeferred<AVEILBurstFX>(
		AVEILBurstFX::StaticClass(), FTransform(FRotator::ZeroRotator, At),
		nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!Burst) return nullptr;
	Burst->Tier = FMath::Clamp(InTier, 0, 3);
	Burst->FinishSpawning(FTransform(FRotator::ZeroRotator, At));
	UE_LOG(LogVEIL, Verbose, TEXT("PHI FX: burst tier %d at %s"), Burst->Tier, *At.ToCompactString());
	return Burst;
}

void AVEILBurstFX::BeginPlay()
{
	Super::BeginPlay();

	Colour = BurstColour(Tier);
	if (UStaticMesh* Sphere = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
	{
		Shell->SetStaticMesh(Sphere);
	}
	if (UStaticMesh* Cylinder = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder")))
	{
		Ring->SetStaticMesh(Cylinder);
	}

	auto Glow = [&](UStaticMeshComponent* Mesh, float Strength)
	{
		if (UMaterialInterface* Base = Mesh->GetMaterial(0))
		{
			if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, this))
			{
				MID->SetVectorParameterValue(TEXT("Color"), Colour);
				MID->SetVectorParameterValue(TEXT("EmissiveColor"), Colour * Strength);
				Mesh->SetMaterial(0, MID);
			}
		}
	};
	Glow(Shell, 8.f + Tier * 4.f);
	Glow(Ring, 3.f);

	Flash->SetLightColor(Colour);
	Flash->SetIntensity(9000.f + Tier * 4000.f);
}

void AVEILBurstFX::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	Age += DeltaSeconds;

	// One phi-weighted expansion: fast out of the gate, slowing as it dies -
	// a burst that spends most of its growth early and then lets go.
	const float T = FMath::Clamp(Age / FMath::Max(0.01f, LifeSeconds), 0.f, 1.f);
	const float Ease = 1.f - FMath::Pow(1.f - T, UPHIMito::Phi);
	const float Size = 0.2f + Ease * (0.9f + Tier * UPHIMito::PhiPow(1.f));

	Shell->SetRelativeScale3D(FVector(Size));
	Ring->SetRelativeScale3D(FVector(Size * UPHIMito::PhiPow(1.f), Size * UPHIMito::PhiPow(1.f), 0.02f));
	Ring->SetRelativeRotation(FRotator(0.f, UPHIMito::GoldenAngleDeg * Ease, 0.f));

	Flash->SetIntensity((9000.f + Tier * 4000.f) * (1.f - Ease));
	Shell->SetVisibility(T < 0.92f);
	Ring->SetVisibility(T < 0.92f);

	if (T >= 1.f)
	{
		Destroy();
	}
}
