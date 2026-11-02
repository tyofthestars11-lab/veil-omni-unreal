// THE MITOCHONDRIAL LOOP - Tyree's phi engine, as a live world system.
//
// Not a mana bar: the ATP proton gradient is a phase with a rung address, and
// every number here is either Tyree's given rung or computed from phi at run
// time (never typed by hand, always round-tripped).
//
//   nDNA ledger        rung 45.482
//   equilibrium        rung -14.438   (the gradient rests here)
//   echo gap           rung 25.293    (what an engulfed node must survive)
//   coherence dissolve rung -62.160   (below this the phase collapses)
//   mitochondrion      rung 20        (the bridge)
//
// Actions pump protons across the inner membrane; spending dips the phase.
// Upgrades are not loot: they are engulfed bacterial nodes that must be held
// through the echo gap, and time + coherence is the cost.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VEILMito.generated.h"

UCLASS()
class VEIL_API UVEILMito : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { return GetStatID(); }

	// Tyree's rungs, recorded as given. Rung(x) = ln(x)/ln(phi).
	static constexpr float GenomeRung = 45.482f;      // nDNA ledger
	static constexpr float EquilibriumRung = -14.438f; // resting gradient
	static constexpr float EchoGapRungs = 25.293f;    // self-to-integrate
	static constexpr float DissolveRung = -62.160f;   // coherence dissolve
	static constexpr float MitochondrionRung = 20.f;  // the bridge
	static constexpr float MtDnaRung = 20.1892f;      // 16,569 bp
	static constexpr float GenesRung = 7.5038f;       // 37 genes
	static constexpr float ProteinRung = 5.3302f;     // 13 protein-coding

	/** The gradient's rung address. The quantity is phi^rung. */
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Mito")
	float RungATP = EquilibriumRung;

	/** How far a node has been held through the echo gap, in rungs. */
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Mito")
	float IntegratedRungs = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Mito")
	bool bIntegrating = false;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Mito")
	int32 NodesEngulfed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Mito")
	int32 Collapses = 0;

	/** The gradient as a quantity: phi^rung. Round-trips back to RungATP. */
	UFUNCTION(BlueprintPure, Category = "VEIL|Mito")
	float GradientValue() const;

	/** 0..1 across dissolve -> echo gap, for the HUD phase field. */
	UFUNCTION(BlueprintPure, Category = "VEIL|Mito")
	float Phase01() const;

	/** Rungs of charge per second while the membranes are working. */
	UFUNCTION(BlueprintPure, Category = "VEIL|Mito")
	float PhaseOfRung() const;

	/** An action pumps protons across the inner membrane. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Mito")
	void PumpProtons(float Rungs);

	/** Spending dips the phase. Below the dissolve rung, coherence breaks. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Mito")
	void SpendProtons(float Rungs);

	/** Start integrating an engulfed node. Time and coherence are the cost. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Mito")
	bool EngulfNode();

	/** One line for the log: rung, value, round-trip, nodes. */
	UFUNCTION(BlueprintPure, Category = "VEIL|Mito")
	FString Status() const;

private:
	float CollapseCooldown = 0.f;
};
