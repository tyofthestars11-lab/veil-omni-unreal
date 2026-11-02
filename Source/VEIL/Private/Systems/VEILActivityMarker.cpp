#include "Systems/VEILActivityMarker.h"

#include "Components/PointLightComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Systems/VEILCampaign.h"
#include "UObject/ConstructorHelpers.h"
#include "VEIL.h"

AVEILActivityMarker::AVEILActivityMarker()
{
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	if (Sphere.Succeeded()) { Mesh->SetStaticMesh(Sphere.Object); }
	Mesh->SetWorldScale3D(FVector(1.15f));
	// A marker is a beacon, not a wall: you walk through it.
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Lamp = CreateDefaultSubobject<UPointLightComponent>(TEXT("Lamp"));
	Lamp->SetupAttachment(RootComponent);
	Lamp->SetLightColor(FLinearColor(1.f, 0.82f, 0.30f));
	Lamp->SetIntensity(2600.f);
	Lamp->SetAttenuationRadius(1400.f);
	Lamp->SetCastShadows(false);
	Lamp->SetAffectGlobalIllumination(false);

	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->SetupAttachment(RootComponent);
	Trigger->InitSphereRadius(240.f);
	Trigger->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Trigger->SetCollisionObjectType(ECC_WorldDynamic);
	Trigger->SetCollisionResponseToAllChannels(ECR_Ignore);
	Trigger->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	Trigger->SetGenerateOverlapEvents(true);
}

void AVEILActivityMarker::Bind(UVEILCampaignSubsystem* InCampaign, int32 InIndex, const FString& Title)
{
	Campaign = InCampaign;
	Index = InIndex;
	if (Trigger && !Trigger->OnComponentBeginOverlap.IsAlreadyBound(this, &AVEILActivityMarker::OnEnter))
	{
		Trigger->OnComponentBeginOverlap.AddDynamic(this, &AVEILActivityMarker::OnEnter);
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI ACTIVITY MARKER: %d at %s - %s"),
		Index, *GetActorLocation().ToCompactString(), *Title);
}

void AVEILActivityMarker::OnEnter(UPrimitiveComponent* OverlappedComp, AActor* Other,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Sweep)
{
	if (!Campaign.IsValid() || Index == INDEX_NONE) { return; }
	Campaign->NoteVisited(Index);
	// Reached is not finished: the counter decides. The lamp dims so a site that
	// has been stood at stops advertising itself at full strength.
	if (Lamp) { Lamp->SetIntensity(600.f); }
}
