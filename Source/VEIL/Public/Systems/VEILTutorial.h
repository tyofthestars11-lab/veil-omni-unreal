// VEIL field manual: the first ten minutes, taught by doing.
//
// A tutorial that prints a wall of keys is a manual nobody reads. This watches
// what the player actually does - the runner's speed, the lens sweeping, a jump
// leaving the ground, a conversation opening - and only then moves on. Each step
// names the action and shows the glyph for the device in the player's hands.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VEILTutorial.generated.h"

struct FVEILTutorialStep
{
	FString Action;
	FString Key;
	FString Pad;
	FString Hint;
};

UCLASS()
class VEIL_API UVEILTutorial : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	bool IsComplete() const { return bComplete; }
	bool IsHidden() const { return bHidden; }
	void Hide() { bHidden = true; }
	void Show() { bHidden = false; }

	int32 GetStepIndex() const { return StepIndex; }
	int32 NumSteps() const { return 8; }

	/** The step the player is on (or the last one when complete). */
	const FVEILTutorialStep& GetStep() const;
	/** 0..1 - how far through the current step's gesture. */
	float GetStepProgress() const { return StepProgress; }
	/** The glyph for the device currently in hand. */
	FString GetGlyph() const;

private:
	void Build();
	void Advance();
	const class AVEILCharacter* GetRunner() const;
	class AVEILPlayerController* GetPC() const;

	TArray<FVEILTutorialStep> Steps;
	int32 StepIndex = 0;
	bool bComplete = false;
	bool bHidden = false;
	float StepProgress = 0.f;

	// Per-step accumulators - the gesture, not a timer.
	float Held = 0.f;
	float Turned = 0.f;
	float EnterTime = 0.f;
	float LastYaw = 0.f;
	bool bCameraAtStepStart = false;
};
