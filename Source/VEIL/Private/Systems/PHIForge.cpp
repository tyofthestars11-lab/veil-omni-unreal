// PHI Forge implementation.
#include "Systems/PHIForge.h"

#include "Character/VEILCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "Systems/OMNIQuestSystem.h"
#include "Systems/PHI_Sfx.h"
#include "VEIL.h"

APhiForge::APhiForge()
{
	PrimaryActorTick.bCanEverTick = false;
	Anvil = CreateDefaultSubobject<UBoxComponent>(TEXT("Anvil"));
	Anvil->InitBoxExtent(FVector(200.f, 200.f, 150.f));
	Anvil->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(Anvil);
	Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Body"));
	Body->SetupAttachment(RootComponent);
	Fire = CreateDefaultSubobject<UPointLightComponent>(TEXT("Fire"));
	Fire->SetupAttachment(RootComponent);
	Fire->AttenuationRadius = 1400.f;
	Fire->Intensity = 2500.f;
	Fire->SetLightColor(FLinearColor(1.f, 0.45f, 0.15f));
}

void APhiForge::BeginPlay()
{
	Super::BeginPlay();
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		Body->SetStaticMesh(Cube);
		Body->SetWorldScale3D(FVector(2.4f, 1.6f, 1.f));
	}
	if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold")))
	{
		Body->SetMaterial(0, M);
	}
	Anvil->OnComponentBeginOverlap.AddDynamic(this, &APhiForge::OnForgeTouch);
}

void APhiForge::OnForgeTouch(UPrimitiveComponent*, AActor* Other,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	if (!Cast<AVEILCharacter>(Other)) return;
	int32 Lvl = ForgeUp();
	if (GEngine && Lvl >= 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Orange,
			FString::Printf(TEXT("FORGE %d: the bolt burns hotter."), Lvl));
	}
}

int32 APhiForge::ForgeUp()
{
	UWorld* W = GetWorld();
	if (!W) return -1;
	UOMNIQuestSystem* Q = W->GetSubsystem<UOMNIQuestSystem>();
	AVEILCharacter* Runner = Cast<AVEILCharacter>(W->GetFirstPlayerController()
		? W->GetFirstPlayerController()->GetPawn() : nullptr);
	if (!Q || !Runner || !Runner->IsAlive()) return -1;
	if (Q->Shards < 2)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Yellow, TEXT("FORGE wants 2 shards. The field provides.")); }
		return -1;
	}
	if (Q->Gold < 50)
	{
		if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 7.f, FColor::Yellow, TEXT("FORGE wants 50 gold for the fire.")); }
		return -1;
	}
	Q->Shards -= 2;
	Q->Gold -= 50;
	++Q->ForgeLevel;
	Runner->WeaponDamage *= 1.1f;
	UE_LOG(LogVEIL, Log, TEXT("PHI FORGE %d: damage now %.0f, shards left %d."),
		Q->ForgeLevel, Runner->WeaponDamage, Q->Shards);
	UPHI_Sfx::Roar(W, GetActorLocation());
	return Q->ForgeLevel;
}

