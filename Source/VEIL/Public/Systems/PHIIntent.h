// PHI INTENT: the world updates itself the way the universe does - by measuring
// its own chaos and folding it through the golden ratio.
//
// Four real sources are measured every cycle, never invented:
//   jitter    frame-time spread, (p95 - avg) / (avg * phi)  - how uneven the machine is
//   combat    hostiles near the runner / 8               - how hard the world pushes
//   motion    runner speed / sprint speed                - how much is in flight
//   temporal  the runner's phase drift + field            - how far out of time he stands
//
// They are folded with phi weights (1, 1/phi, 1/phi^2, 1/phi^3), so the sharpest
// signal leads and the rest fade like a golden spiral. That folded number is the
// chaos of the moment.
//
// The update is a golden contraction:
//
//   intent <- intent + (chaos - intent) / phi^2
//
// Because 1/phi^2 = 0.382 < 1, this map is a contraction: every cycle pulls the
// world's intent toward the measured chaos by 38%, so noise is absorbed instead
// of amplified and the state can never oscillate. That is the whole claim of
// "the golden ratio filters chaos", stated as arithmetic rather than poetry.
//
// The intent is then fed back into the world: above 0.618 the sky leans to storm
// and the temporal field opens wider (a tense world really has more futures);
// below 0.382 it clears. Nothing is scripted - the world is reading itself.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "PHIIntent.generated.h"

UCLASS()
class VEIL_API UPHIIntent : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	static constexpr float Phi = 1.618033988749895f;

	/** The world's measured chaos this cycle, 0..1. */
	float GetChaos() const { return Chaos; }
	/** The filtered state the world is acting on, 0..1. Never jumps to chaos. */
	float GetIntent() const { return Intent; }
	/** The intent as a rung: ln(intent)/ln(phi), computed fresh. */
	float GetRung() const;
	/** Cycles completed since play began. */
	int32 GetCycles() const { return Cycles; }
	/** One line for the HUD, always live. */
	FString IntentLine() const;

	/** Proof hook: run a cycle now instead of waiting for the phi^2 window. */
	void ForceCycle();

private:
	void Cycle();
	float MeasureJitter();
	float MeasureCombat();
	float MeasureMotion();
	float MeasureTemporal();

	TArray<float> Frames;
	int32 FrameCursor = 0;
	float AvgMs = 0.f;
	float P95Ms = 0.f;

	float Chaos = 0.618f;
	float Intent = 0.618f;
	int32 Cycles = 0;
	float SinceCycle = 0.f;

	// The last measured parts, kept so the log and the HUD quote what was used.
	float PartJitter = 0.f;
	float PartCombat = 0.f;
	float PartMotion = 0.f;
	float PartTemporal = 0.f;
	/** Which side of the golden band the world last leaned to. */
	int32 LastLean = 0;
};
