// PHI portal travel: walk in glowing, wake up standing somewhere bigger.
#include "Systems/PHIPortal.h"

#include "Character/VEILCharacter.h"
#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "Systems/PHI_Sfx.h"
#include "Engine/Engine.h"
#include "VEIL.h"

APhiPortal::APhiPortal()
{
	PrimaryActorTick.bCanEverTick = false;
	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->InitSphereRadius(140.f); // PHI LAW: doors are chosen, never clipped mid-sprint
	Trigger->SetCollisionProfileName(TEXT("Trigger"));
	SetRootComponent(Trigger);
	Tower = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Tower"));
	Tower->SetupAttachment(RootComponent);
	Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
	Glow->SetupAttachment(RootComponent);
	Glow->AttenuationRadius = 1600.f;
	Glow->Intensity = 3000.f;
	Glow->SetLightColor(FLinearColor(1.f, 0.75f, 0.3f));
}

void APhiPortal::BeginPlay()
{
	Super::BeginPlay();
	if (UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
	{
		Tower->SetStaticMesh(Cube);
		Tower->SetWorldScale3D(FVector(0.5f, 3.4f, 4.6f));
	}
	if (UMaterialInterface* M = LoadObject<UMaterialInterface>(nullptr,
		TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold")))
	{
		Tower->SetMaterial(0, M);
	}
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &APhiPortal::OnPortalTouch);
	UE_LOG(LogVEIL, Log, TEXT("PHI PORTAL: door to %s stands at %s."), *TargetMap.ToString(), *GetActorLocation().ToString());
}

void APhiPortal::OnPortalTouch(UPrimitiveComponent*, AActor* OtherActor,
	UPrimitiveComponent*, int32, bool, const FHitResult&)
{
	AVEILCharacter* Runner = Cast<AVEILCharacter>(OtherActor);
	if (!Runner || !Runner->IsAlive()) return;
	UE_LOG(LogVEIL, Log, TEXT("PHI PORTAL TOUCH: %s at %s controlled by %s."), *Runner->GetName(), *Runner->GetActorLocation().ToString(), Runner->GetController() ? *Runner->GetController()->GetClass()->GetName() : TEXT("NONE"));
	if (!Cast<APlayerController>(Runner->GetController())) return; // companions ride, only the played pawn steers
	UWorld* PW = GetWorld();
	if (!PW) return;
	const float Now = PW->GetTimeSeconds();
	if (Now - LastUseTime < 3.f) return; // arrival performs once, never bounces
	if (TargetMap.IsNone()) return;
	LastUseTime = Now;
	UE_LOG(LogVEIL, Log, TEXT("PHI PORTAL: %s steps through to %s."), *GetName(), *TargetMap.ToString());
	UPHI_Sfx::Chime(PW, GetActorLocation());
	if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("REGION %s"), *TargetMap.ToString())); }
	UGameplayStatics::OpenLevel(this, TargetMap);
}

