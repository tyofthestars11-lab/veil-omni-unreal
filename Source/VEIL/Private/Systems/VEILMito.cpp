#include "Systems/VEILMito.h"
#include "Systems/PHIMito.h"
#include "GameFramework/DamageType.h"
#include "Character/VEILCharacter.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "VEIL.h"

namespace
{
	// The pull home: the gradient always returns to equilibrium, never snaps.
	constexpr float EquilibriumTau = 6.f;   // seconds
	// One rung of integration per second of held coherence.
	constexpr float IntegrationRungsPerSecond = 1.f;
}

void UVEILMito::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	RungATP = EquilibriumRung;
	UE_LOG(LogVEIL, Log, TEXT("PHI MITO ONLINE: gradient rests at %s."), *Status());
}

float UVEILMito::GradientValue() const
{
	return UPHIMito::PhiPow(RungATP);
}

float UVEILMito::PhaseOfRung() const
{
	return UPHIMito::PhaseDeg(RungATP) / 360.f;
}

float UVEILMito::Phase01() const
{
	const float Span = EchoGapRungs - DissolveRung;      // 25.293 - (-62.160)
	if (Span <= 0.f) { return 0.f; }
	return FMath::Clamp((RungATP - DissolveRung) / Span, 0.f, 1.f);
}

FString UVEILMito::Status() const
{
	const float Value = GradientValue();
	// Round-trip: rung -> value -> rung must return the same address.
	const float Back = UPHIMito::Rung(Value);
	return FString::Printf(
		TEXT("rung %.4f -> value %.6f -> rung %.4f | nodes %d | collapses %d%s"),
		RungATP, Value, Back, NodesEngulfed, Collapses,
		bIntegrating ? *FString::Printf(TEXT(" | integrating %.4f/%.4f rungs"),
			IntegratedRungs, EchoGapRungs) : TEXT(""));
}

void UVEILMito::PumpProtons(float Rungs)
{
	if (Rungs <= 0.f) { return; }
	const float Before = RungATP;
	RungATP += Rungs;
	UE_LOG(LogVEIL, Log, TEXT("PHI MITO PUMP: +%.4f rungs (%.4f -> %.4f)."), Rungs, Before, RungATP);
}

void UVEILMito::SpendProtons(float Rungs)
{
	if (Rungs <= 0.f) { return; }
	const float Before = RungATP;
	RungATP -= Rungs;
	UE_LOG(LogVEIL, Log, TEXT("PHI MITO SPEND: -%.4f rungs (%.4f -> %.4f)."), Rungs, Before, RungATP);
	if (RungATP <= DissolveRung)
	{
		// Coherence broke: the phase collapsed, the node in the gap is lost.
		++Collapses;
		const bool bLostNode = bIntegrating;
		bIntegrating = false;
		IntegratedRungs = 0.f;
		UE_LOG(LogVEIL, Warning, TEXT("PHI MITO COLLAPSE: coherence dissolved at rung %.4f%s."),
			RungATP, bLostNode ? TEXT(" - the engulfed node was lost in the gap") : TEXT(""));
		RungATP = EquilibriumRung;
		if (APawn* Pawn = UGameplayStatics::GetPlayerPawn(this, 0))
		{
			// Dying is not a health bar event here: it is the phase failing.
			UGameplayStatics::ApplyDamage(Pawn, 40.f, nullptr, nullptr, UDamageType::StaticClass());
		}
	}
}

bool UVEILMito::EngulfNode()
{
	if (bIntegrating) { return false; }
	bIntegrating = true;
	IntegratedRungs = 0.f;
	UE_LOG(LogVEIL, Log, TEXT("PHI MITO ENGULF: node taken in - hold coherence for %.3f rungs."),
		EchoGapRungs);
	return true;
}

void UVEILMito::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CollapseCooldown > 0.f) { CollapseCooldown -= DeltaTime; }

	// Equilibrium: the gradient is pulled home, never snapped.
	const float Pull = 1.f - FMath::Exp(-DeltaTime / EquilibriumTau);
	RungATP += (EquilibriumRung - RungATP) * Pull;

	if (!bIntegrating) { return; }

	// Integration only advances while coherence holds above equilibrium.
	if (RungATP < EquilibriumRung)
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI MITO ECHO GAP BROKEN at rung %.4f - integration resets."), RungATP);
		bIntegrating = false;
		IntegratedRungs = 0.f;
		return;
	}
	IntegratedRungs += IntegrationRungsPerSecond * DeltaTime;
	if (IntegratedRungs >= EchoGapRungs)
	{
		bIntegrating = false;
		IntegratedRungs = 0.f;
		++NodesEngulfed;
		UE_LOG(LogVEIL, Log, TEXT("PHI MITO INTEGRATED: node %d whole - the upgrade is the mitochondria's, not loot."),
			NodesEngulfed);
	}
}
