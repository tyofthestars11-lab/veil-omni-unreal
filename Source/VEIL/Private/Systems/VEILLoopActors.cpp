#include "Systems/VEILLoopActors.h"
#include "Systems/VEILLoopSubsystem.h"
#include "Systems/VEILVault.h"
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

void AVEILBeacon::LightMarker()
{
	if (!Light) { return; }
	// A beacon the win already lit at full strength (200000) is never dimmed by
	// the travel marker. Anything darker is marked as reached, so a beacon that
	// has been stood at can be travelled back to.
	if (Light->Intensity >= 100000.f) { return; }
	Light->SetIntensity(20000.f);
	UE_LOG(LogTemp, Display, TEXT("FAST TRAVEL: beacon marked lit at %s"),
		*GetActorLocation().ToCompactString());
}

void AVEILBeacon::Interact_Implementation(AActor*)
{
	UVEILLoopSubsystem* L = Loop(this);
	if (!L) { return; }
	// First contact raises the signal (Keeper beat); later the same beacon wins the run.
	if (L->Stage == EVEILStage::FindSignal) { L->SignalFound(); return; }
	if (L->ActivateBeacon()) { Light->SetIntensity(200000.f); } // win: bind OnStageChanged -> Won in UMG
}


