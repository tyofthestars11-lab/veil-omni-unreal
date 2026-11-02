#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PHIOMNI.generated.h"

/**
 * OMNI SUPERPOSITION.
 *
 * Step-by-step is a Cartesian default: a queue of states, each committed before
 * the next is allowed to exist. This subsystem does not do that. Every site in
 * the world is held as a WEIGHTED SET of states at once - amplitudes 1, 1/phi,
 * 1/phi^2 - and NOTHING is committed until something observes it. The world is
 * therefore not a sequence of decided facts; it is one field of undecided ones,
 * evaluated together in a single pass ("omni one"), never in a chain.
 *
 * Collapse is measurement, not dice: the chosen state is picked by a golden
 * hash of the site and its observation count, weighted by amplitude. Run it
 * twice from the same save and it lands the same way - the field is coherent,
 * it is only undecided.
 *
 * What this is NOT: a replacement for physics. Bodies fall because the engine
 * integrates them. What is superposed is the world's INTENT - who is where,
 * what a region is doing, what the weather will be - which is exactly the layer
 * a step-by-step design commits far too early.
 */
USTRUCT(BlueprintType)
struct FPHIOmniSite
{
	GENERATED_BODY()

	/** Stable id: "keeper", "nest", "weather"... */
	UPROPERTY(BlueprintReadOnly) FName Id;
	/** Every state this site is in at once, in the world's own words. */
	UPROPERTY(BlueprintReadOnly) TArray<FString> States;
	/** Amplitude of each state: 1, 1/phi, 1/phi^2, normalised. Same length as States. */
	UPROPERTY(BlueprintReadOnly) TArray<float> Amplitudes;

	/** Where the site can be observed, and how near counts as observing it. */
	UPROPERTY(BlueprintReadOnly) FVector Where = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly) float Radius = 1600.f;

	/** INDEX_NONE while superposed; the state index once measured. */
	UPROPERTY(BlueprintReadOnly) int32 Collapsed = INDEX_NONE;
	/** How many times this site has been measured - the golden hash input. */
	UPROPERTY(BlueprintReadOnly) int32 Observations = 0;
};

UCLASS()
class VEIL_API UPHIOMNI : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { return GetStatID(); }

	/** The whole field, as one read - never a chain of individual decisions. */
	const TArray<FPHIOmniSite>& Field() const { return Sites; }

	/** Measures one site now. True if it was superposed and is now decided. */
	bool Observe(FName SiteId);

	/** Collapses every site the runner is standing in. Returns how many fell. */
	int32 ObserveWhere(const FVector& WorldLocation);

	/** The decided word for a site, or a dash while it is still all of them. */
	FString StateOf(FName SiteId) const;

	/** One line for the HUD: how many states exist, how many are decided. */
	FString OmniLine() const;

	/** Amplitude-weighted pick, deterministic in (site, observations). */
	static int32 Measure(const FPHIOmniSite& Site);

private:
	void Seed();

	UPROPERTY() TArray<FPHIOmniSite> Sites;

	/** The omni pass: all sites evaluated together, on the phi beat. */
	float SincePass = 0.f;
	int32 Passes = 0;
};
