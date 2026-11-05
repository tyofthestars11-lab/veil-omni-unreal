#include "VeilMind.h"
#include "VeilNpc.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "TimerManager.h"

AVeilMind::AVeilMind()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AVeilMind::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	if (GetWorld())
	{
		GetWorldTimerManager().SetTimer(ThinkTimer, this, &AVeilMind::Think, 0.5f, true);
	}
}

void AVeilMind::Think()
{
	AVeilNpc* Body = Cast<AVeilNpc>(GetPawn());
	if (!Body || !GetWorld())
	{
		return;
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	APawn* Runner = PC ? PC->GetPawn() : nullptr;
	if (!Runner)
	{
		return;
	}

	const double D = FVector::Dist2D(Body->GetActorLocation(), Runner->GetActorLocation());

	// 0 far: own business. 1 near: aware. 2 close: turns to the Runner.
	const int32 State = (D < 800.0) ? 2 : ((D < 2600.0) ? 1 : 0);

	if (State != LastState)
	{
		LastState = State;
		UE_LOG(LogTemp, Display, TEXT("VEIL MIND: %s (%s) reads the Runner at %.0f cm, state %d"),
			*Body->GetName(), Body->GetRaceName(), D, State);
	}

	++Ticks;
	Body->Perceive(Runner, D, State);
}
