#include "Systems/VEILTutorial.h"

#include "Character/VEILCharacter.h"
#include "Game/VEILGameMode.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Systems/VEILDialogue.h"
#include "Systems/VEILInputHelp.h"
#include "VEIL.h"

DECLARE_CYCLE_STAT(TEXT("VEIL Tutorial"), STAT_VEILTutorialTick, STATGROUP_Tickables);

TStatId UVEILTutorial::GetStatId() const
{
	// The stat is declared at file scope above; hand its id back.
	return GET_STATID(STAT_VEILTutorialTick);
}

void UVEILTutorial::Build()
{
	if (Steps.Num() > 0)
	{
		return;
	}
	// Order is the order a person needs them: stand, look, move fast, leave the
	// ground, then the verbs that only make sense once you can move.
	Steps = {
		{ TEXT("MOVE"),   TEXT("W A S D"), TEXT("LEFT STICK"),
		  TEXT("Push forward. The Runner walks the ground, not a grid.") },
		{ TEXT("LOOK"),   TEXT("MOUSE"),   TEXT("RIGHT STICK"),
		  TEXT("Sweep the lens. The district is behind you as much as ahead.") },
		{ TEXT("SPRINT"), TEXT("SHIFT"),   TEXT("L3"),
		  TEXT("Hold to run. Speed is free; it is time you are spending.") },
		{ TEXT("JUMP"),   TEXT("SPACE"),   TEXT("A"),
		  TEXT("Jump. There is a coyote beat and a buffer - it forgives.") },
		{ TEXT("CAMERA"), TEXT("V"),       TEXT("R3"),
		  TEXT("Switch the rig: third person, then the Runner's own eyes.") },
		{ TEXT("SPEAK"),  TEXT("E"),       TEXT("X"),
		  TEXT("Walk to a statue or a machine and speak. They all have names.") },
		{ TEXT("CHART"),  TEXT("TAB"),     TEXT("VIEW"),
		  TEXT("Open the chart. Every beacon you light is drawn on it.") },
		{ TEXT("STRIKE"), TEXT("F"),       TEXT("Y"),
		  TEXT("Swing the lance. The machines answer back - that is the point.") },
	};
}

const AVEILCharacter* UVEILTutorial::GetRunner() const
{
	const UWorld* W = GetWorld();
	const APlayerController* PC = W ? W->GetFirstPlayerController() : nullptr;
	return PC ? Cast<AVEILCharacter>(PC->GetPawn()) : nullptr;
}

AVEILPlayerController* UVEILTutorial::GetPC() const
{
	const UWorld* W = GetWorld();
	return W ? Cast<AVEILPlayerController>(W->GetFirstPlayerController()) : nullptr;
}

const FVEILTutorialStep& UVEILTutorial::GetStep() const
{
	static const FVEILTutorialStep Done = { TEXT("FIELD MANUAL COMPLETE"), TEXT(""), TEXT(""),
		TEXT("The rest is yours. [H] / D-PAD LEFT opens this page any time.") };
	const int32 I = FMath::Clamp(StepIndex, 0, Steps.Num() - 1);
	if (bComplete || Steps.Num() == 0)
	{
		return Done;
	}
	return Steps[I];
}

FString UVEILTutorial::GetGlyph() const
{
	if (bComplete)
	{
		return VEILInput::IsGamepad() ? FString(TEXT("D-PAD LEFT")) : FString(TEXT("F1"));
	}
	const FVEILTutorialStep& S = GetStep();
	const bool bPad = VEILInput::IsGamepad();
	if (bPad)
	{
		return S.Pad.IsEmpty() ? FString(TEXT("KEYBOARD")) : S.Pad;
	}
	return S.Key;
}

void UVEILTutorial::Advance()
{
	if (bComplete)
	{
		return;
	}
	const FVEILTutorialStep& Done = Steps[StepIndex];
	UE_LOG(LogVEIL, Log, TEXT("PHI FIELD MANUAL: step %d/%d '%s' done."),
		StepIndex + 1, Steps.Num(), *Done.Action);

	++StepIndex;
	StepProgress = 0.f;
	Held = 0.f;
	Turned = 0.f;
	EnterTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	if (const AVEILCharacter* R = GetRunner())
	{
		bCameraAtStepStart = R->IsFirstPersonNow();
	}
	if (StepIndex >= Steps.Num())
	{
		bComplete = true;
		StepIndex = Steps.Num() - 1;
		UE_LOG(LogVEIL, Log, TEXT("PHI FIELD MANUAL: complete. All 8 gestures taught."));
	}
}

void UVEILTutorial::Tick(float DeltaTime)
{
	Build();
	UWorld* W = GetWorld();
	if (!W || bHidden || bComplete || Steps.Num() == 0)
	{
		return;
	}
	AVEILPlayerController* PC = GetPC();
	const AVEILCharacter* R = GetRunner();
	if (!PC || !R)
	{
		return;
	}

	if (EnterTime <= 0.f)
	{
		EnterTime = W->GetTimeSeconds();
		bCameraAtStepStart = R->IsFirstPersonNow();
		LastYaw = PC->GetControlRotation().Yaw;
	}

	const FVector Vel = R->GetVelocity();
	const float Speed = FVector(Vel.X, Vel.Y, 0.f).Size();
	// The lens: how far the view has swept since this step opened.
	const float YawNow = PC->GetControlRotation().Yaw;
	Turned += FMath::Abs(FRotator::NormalizeAxis(YawNow - LastYaw));
	LastYaw = YawNow;
	const UCharacterMovementComponent* Move = R->GetCharacterMovement();

	switch (StepIndex)
	{
	case 0: // MOVE
		if (Speed > 220.f && Move && Move->IsMovingOnGround())
		{
			Held += DeltaTime;
			StepProgress = FMath::Clamp(Held / 0.6f, 0.f, 1.f);
			if (Held >= 0.6f) { Advance(); }
		}
		else
		{
			Held = FMath::Max(0.f, Held - DeltaTime * 0.5f);
			StepProgress = FMath::Clamp(Held / 0.6f, 0.f, 1.f);
		}
		break;

	case 1: // LOOK
		StepProgress = FMath::Clamp(Turned / 70.f, 0.f, 1.f);
		if (Turned >= 70.f) { Advance(); }
		break;

	case 2: // SPRINT
		if (R->IsSprintingNow() && Speed > 300.f)
		{
			Held += DeltaTime;
			StepProgress = FMath::Clamp(Held / 0.4f, 0.f, 1.f);
			if (Held >= 0.4f) { Advance(); }
		}
		break;

	case 3: // JUMP
		if (Move && Move->IsFalling())
		{
			StepProgress = 1.f;
			Held += DeltaTime;
			if (Held > 0.15f) { Advance(); }
		}
		break;

	case 4: // CAMERA
		if (R->IsFirstPersonNow() != bCameraAtStepStart)
		{
			StepProgress = 1.f;
			Advance();
		}
		break;

	case 5: // SPEAK
		if (UVEILDialogue* Dlg = W->GetSubsystem<UVEILDialogue>(); Dlg && Dlg->IsActive())
		{
			StepProgress = 1.f;
			Advance();
		}
		break;

	case 6: // CHART
		if (PC->bShowChart)
		{
			StepProgress = 1.f;
			Advance();
		}
		break;

	case 7: // STRIKE
		if (R->GetLastSwingTime() > EnterTime || R->GetLastFireTime() > EnterTime)
		{
			StepProgress = 1.f;
			Advance();
		}
		break;

	default:
		break;
	}
}
