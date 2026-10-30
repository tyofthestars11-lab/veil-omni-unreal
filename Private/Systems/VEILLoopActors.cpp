#include "VEILLoopActors.h"
#include "VEILLoopSubsystem.h"
#include "VEILVault.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"

static UVEILLoopSubsystem* Loop(const UObject* Ctx)
{
	return Ctx && Ctx->GetWorld() ? Ctx->GetWorld()->GetSubsystem<UVEILLoopSubsystem>() : nullptr;
}

// ---- Fragment ----
AVEILFragment::AVEILFragment()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
	Glow = CreateDefaultSubobject<UPointLightComponent>(TEXT("Glow"));
	Glow->SetupAttachment(RootComponent);
	Glow->SetLightColor(FLinearColor(1.f, 0.78f, 0.23f));
	Glow->SetIntensity(4000.f);
}

void AVEILFragment::Interact_Implementation(AActor*)
{
	if (UVEILLoopSubsystem* L = Loop(this)) { L->CollectFragment(); }
	Destroy();
}

// ---- Forge ----
AVEILForge::AVEILForge()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
}

void AVEILForge::Interact_Implementation(AActor*)
{
	UVEILLoopSubsystem* L = Loop(this);
	if (!L || !L->BuildArtifact()) { return; } // needs 10/10 fragments
	for (TActorIterator<AVEILVault> It(GetWorld()); It; ++It)
	{
		It->bArtifactBuilt = true;
		It->NotifyAnchorAligned(); // re-check: opens now if all 4 anchors are already aligned
	}
}

// ---- Beacon ----
AVEILBeacon::AVEILBeacon()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	Mesh->SetCollisionProfileName(TEXT("BlockAll"));
	Light = CreateDefaultSubobject<UPointLightComponent>(TEXT("Light"));
	Light->SetupAttachment(RootComponent);
	Light->SetIntensity(0.f);
	Light->SetLightColor(FLinearColor(1.f, 0.9f, 0.5f));
}

void AVEILBeacon::Interact_Implementation(AActor*)
{
	UVEILLoopSubsystem* L = Loop(this);
	if (L && L->ActivateBeacon()) { Light->SetIntensity(200000.f); } // win: bind OnStageChanged -> Won in UMG
}
