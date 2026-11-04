// PHI ocean implementation.
#include "Systems/PHIOcean.h"

#include "Character/VEILCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInterface.h"
#include "VEIL.h"
#include "Systems/VEILPhi.h"

APhiOcean::APhiOcean()
{
	PrimaryActorTick.bCanEverTick = true;
	Water = CreateDefaultSubobject<UBoxComponent>(TEXT("Water"));
	Water->SetBoxExtent(FVector(1500.f, 1500.f, 200.f));
	Water->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(Water);
	Skin = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Skin"));
	Skin->SetupAttachment(RootComponent);
}

void APhiOcean::BeginPlay()
{
	Super::BeginPlay();

	// THE SEA WAS OVER THE CITY. The water surface sat above the plaza, so every frame
	// from the market was shot along the underside of the sea - a flat purple wash with
	// glowing shapes in it, which reads as "floating geometry" from inside. Four metres
	// down puts the surface below the plaza the runner stands on.
	SetActorLocation(GetActorLocation() - FVector(0.f, 0.f, 400.f));
	UE_LOG(LogTemp, Warning, TEXT("PHI SEA: ocean dropped 400 cm, surface now below the plaza."));
	Water->SetBoxExtent(FVector(WaterSize.X / 2, WaterSize.Y / 2, WaterSize.Z / 2));
	// A SURFACE IS A PLANE, NOT A BOX. This skin used to be an engine Cube stretched to
	// WaterSize/100 - 3000 x 3000 x 400 cm - carrying an emissive material and sitting at eye
	// height, so every frame from the market was shot across a flat white sheet. The sheet cull
	// in VEILGameMode never caught it: that pass walks AStaticMeshActor, and this is a component
	// on APhiOcean. A plane has no thickness to look through, and the sea sits under the plaza.
	if (UStaticMesh* Plane = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Plane.Plane")))
	{
		Skin->SetStaticMesh(Plane);
		Skin->SetWorldScale3D(FVector(WaterSize.X / 100.f, WaterSize.Y / 100.f, 1.f));
		Skin->SetRelativeLocation(FVector(0.f, 0.f, -WaterSize.Z * 0.5f));
		Skin->SetCastShadow(false);
	}
	// Real water first. The white emissive was the glare: a 160 m emissive sheet reads as a
	// light panel, not a sea. Only fall back to it when no water material resolves.
	UMaterialInterface* Surf = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/ModularS30eeb54ea3e4V1/data/Content/StarterContent/Materials/M_Water_Ocean.M_Water_Ocean"));
	if (!Surf)
	{
		Surf = LoadObject<UMaterialInterface>(nullptr,
			TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive"));
	}
	if (Surf)
	{
		Skin->SetMaterial(0, Surf);
	}
	Water->OnComponentBeginOverlap.AddDynamic(this, &APhiOcean::OnSplash);
	GrowFish(6);
	GrowFlora(10);
	UE_LOG(LogVEIL, Log, TEXT("PHI OCEAN: water open, fish 6, flora 10."));
}

void APhiOcean::OnSplash(UPrimitiveComponent*, AActor* Other,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	AVEILCharacter* Runner = Cast<AVEILCharacter>(Other);
	if (!Runner || !Runner->IsAlive()) return;
	if (UCharacterMovementComponent* Move = Runner->GetCharacterMovement())
	{
		if (!Move->IsSwimming())
		{
			Move->SetMovementMode(MOVE_Swimming);
			UE_LOG(LogVEIL, Log, TEXT("PHI OCEAN: the Runner swims."));
		}
	}
}

void APhiOcean::GrowFish(int32 Count)
{
	UWorld* W = GetWorld();
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UMaterialInterface* Gold = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold"));
	if (!W || !Cube) return;
	for (int32 i = 0; i < Count; ++i)
	{
		// GOLDEN ANGLE, NOT A DICE ROLL. The school was scattered with FRandRange, so the
		// same water looked different on every run and nothing could be checked twice. Index
		// on the golden angle at a phi-spaced radius: the same fish are in the same places,
		// every session, and a screenshot can be compared with the last one.
		const float Ang = FMath::DegreesToRadians(i * VeilPhi::GoldenAngleF);
		const float Rad = FMath::Sqrt((float)i + 0.5f) * (WaterSize.X * 0.18f);
		const FVector At = GetActorLocation() + FVector(
			FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad,
			-100.f + 60.f * FMath::Frac(i * 0.6180339887f));
		if (AActor* One = W->SpawnActor<AActor>(AActor::StaticClass(), At, FRotator::ZeroRotator))
		{
			UStaticMeshComponent* Body = NewObject<UStaticMeshComponent>(One);
			Body->SetStaticMesh(Cube);
			Body->SetWorldScale3D(FVector(0.5f, 0.2f, 0.2f));
			if (Gold) { Body->SetMaterial(0, Gold); }
			Body->RegisterComponent();
			One->SetRootComponent(Body);
			Fish.Add(One);
		}
	}
}

void APhiOcean::GrowFlora(int32 Count)
{
	UWorld* W = GetWorld();
	UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UMaterialInterface* Blue = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue"));
	if (!W || !Cube) return;
	FVector Base = GetActorLocation();
	Base.Z -= WaterSize.Z / 2;
	for (int32 i = 0; i < Count; ++i)
	{
		// THE BED IS PLANTED, NOT SOWN AT RANDOM. Same golden angle as the fish, so the
		// fronds follow one law instead of two different dice.
		const float Ang = FMath::DegreesToRadians(i * VeilPhi::GoldenAngleF);
		const float Rad = FMath::Sqrt((float)i + 0.5f) * (WaterSize.X * 0.12f);
		const FVector At = Base + FVector(
			FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad,
			120.f * FMath::Frac(i * 0.3819660113f));
		if (AActor* One = W->SpawnActor<AActor>(AActor::StaticClass(), At, FRotator::ZeroRotator))
		{
			UStaticMeshComponent* Frond = NewObject<UStaticMeshComponent>(One);
			Frond->SetStaticMesh(Cube);
			// Height is a phi walk, not a dice roll: 1.0, 1.618, 1.0, 1.618 across the bed.
			Frond->SetWorldScale3D(FVector(0.3f, 0.3f,
				1.f + 2.f * FMath::Frac(i * 0.6180339887f)));
			if (Blue) { Frond->SetMaterial(0, Blue); }
			Frond->RegisterComponent();
			One->SetRootComponent(Frond);
		}
	}
}

void APhiOcean::DriftFish(float DT)
{
	FVector C = GetActorLocation();
	float T = GetWorld()->GetTimeSeconds();
	for (int32 i = 0; i < Fish.Num(); ++i)
	{
		if (!IsValid(Fish[i])) continue;
		float A = T * 0.3f + i * (6.2831f / FMath::Max(1, Fish.Num()));
		FVector At = C + FVector(FMath::Cos(A) * WaterSize.X / 3, FMath::Sin(A) * WaterSize.Y / 3, -40.f);
		Fish[i]->SetActorLocation(At);
		Fish[i]->SetActorRotation(FRotator(0.f, FMath::RadiansToDegrees(A) + 90.f, 0.f));
	}
}

void APhiOcean::Tick(float DT)
{
	Super::Tick(DT);
	if (Fish.Num() > 0) { DriftFish(DT); }
}

