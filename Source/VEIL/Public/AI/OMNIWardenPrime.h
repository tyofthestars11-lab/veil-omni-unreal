// OMNI RPG: PATH OF LIGHT - WARDEN PRIME, the plaza duel.
#pragma once

#include "CoreMinimal.h"
#include "AI/VEILFactionAI.h"
#include "OMNIWardenPrime.generated.h"

UCLASS()
class VEIL_API AOMNIWardenPrime : public AVEILFactionAgent
{
	GENERATED_BODY()

public:
	AOMNIWardenPrime();

	virtual void BeginPlay() override;
};

