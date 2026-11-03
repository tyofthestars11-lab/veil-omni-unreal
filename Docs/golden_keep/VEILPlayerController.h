// GOLDEN_PITCH 17.0324 rung 5.8916
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VEILPlayerController.generated.h"
UCLASS()
class VEIL_API AVEILPlayerController : public APlayerController
{
    GENERATED_BODY()
public:
    virtual void BeginPlay() override;
};

