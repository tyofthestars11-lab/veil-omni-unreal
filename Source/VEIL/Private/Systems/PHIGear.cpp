// PHI gear pickup implementation: walk over, feel it forever.
#include "Systems/PHIGear.h"

#include "Character/VEILCharacter.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Materials/MaterialInterface.h"
#include "Systems/PHI_Sfx.h"
#include "VEIL.h"

APhiGear::APhiGear()
{
	PrimaryActorTick.bCanEverTick = false;
	Touch = CreateDefaultSubobject<USphereComponent>(TEXT("GearTouch"));
	Touch->InitSphereRadius(220.f);
	Touch->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(Touch);

	Relic = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GearRelic"));
	Relic->SetupAttachment(RootComponent);
	Shine = CreateDefaultSubobject<UPointLightComponent>(TEXT("GearShine"));
	Shine->SetupAttachment(RootComponent);
	Shine->AttenuationRadius = 900.f;
	Shine->Intensity = 1500.f;
}

void APhiGear::BeginPlay()
{
	Super::BeginPlay();
	Tier = FMath::Clamp(Tier, 1, 3); // economy guard: gear math assumes T1-T3
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		Relic->SetStaticMesh(Cube);
		Relic->SetWorldScale3D(FVector(0.6f, 0.6f, 0.6f));
	}
	const TCHAR* MatPath = TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold");
	FLinearColor LightCol(1.f, 0.8f, 0.3f);
	if (Stat == EPHIGearStat::Speed)
	{
		MatPath = TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive");
		LightCol = FLinearColor(0.8f, 0.9f, 1.f);
	}
	else if (Stat == EPHIGearStat::Health)
	{
		MatPath = TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue");
		LightCol = FLinearColor(0.3f, 0.5f, 1.f);
	}
	if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr, MatPath))
	{
		Relic->SetMaterial(0, M);
	}
	Shine->SetLightColor(LightCol);
	Touch->OnComponentBeginOverlap.AddDynamic(this, &APhiGear::OnGearTouch);
}

void APhiGear::OnGearTouch(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!IsValid(OtherActor)) return;
	AVEILCharacter* Runner = Cast<AVEILCharacter>(OtherActor);
	if (!Runner || !Runner->IsAlive()) return;
	UWorld* W = GetWorld();
	if (!W) return;
	if (Touch && Touch->GetCollisionEnabled() == ECollisionEnabled::NoCollision) return; // already claimed
	FString What;
	switch (Stat)
	{
	case EPHIGearStat::Damage:
		Runner->WeaponDamage *= (1.f + 0.1f * Tier);
		What = FString::Printf(TEXT("EDGE T%d: bolt damage now %.0f"), Tier, Runner->WeaponDamage);
		break;
	case EPHIGearStat::Speed:
		Runner->WalkSpeed += 40.f * Tier;
		Runner->SprintSpeed += 40.f * Tier;
		if (Runner->GetCharacterMovement()) { Runner->GetCharacterMovement()->MaxWalkSpeed += 40.f * Tier; }
		What = FString::Printf(TEXT("STRIDE T%d: pace now %.0f"), Tier, Runner->WalkSpeed);
		break;
	case EPHIGearStat::Rapid:
		Runner->FireCooldown = FMath::Max(0.08f, Runner->FireCooldown * FMath::Pow(0.92f, Tier));
		What = FString::Printf(TEXT("TEMPO T%d: cooldown now %.2fs"), Tier, Runner->FireCooldown);
		break;
	default:
		Runner->MaxHealth += 25.f * Tier;
		Runner->Health = FMath::Min(Runner->MaxHealth, Runner->Health + 25.f * Tier);
		What = FString::Printf(TEXT("HEART T%d: ceiling now %.0f"), Tier, Runner->MaxHealth);
		break;
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI GEAR %s: %s takes it."), *GearName.ToString(), *What);
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Yellow, What); }
	if (W) { UPHI_Sfx::Chime(W, GetActorLocation()); }
	Touch->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(0.2f);
}

