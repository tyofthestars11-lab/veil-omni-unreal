#pragma once
#include "CoreMinimal.h"
#include "AIController.h"
#include "VeilMind.generated.h"

/**
 * One mind per body, and it is real AI: it reads the Runner, decides a state
 * from distance and people, and drives its pawn. It also owns the navigation
 * request, so a body of the Deep can be told to move to a place rather than
 * being animated along a line.
 */
UCLASS()
class VEILOMNI_API AVeilMind : public AAIController
{
	GENERATED_BODY()
public:
	AVeilMind();
	virtual void OnPossess(APawn* InPawn) override;

private:
	void Think();
	FTimerHandle ThinkTimer;
	int32 LastState = -1;
	int32 Ticks = 0;
};
