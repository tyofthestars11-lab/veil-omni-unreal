#pragma once
#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "PHI_GameState_Fixed.generated.h"
UCLASS()
class VEIL_API APHI_GameState_Fixed : public AGameStateBase
{
    GENERATED_BODY()
public:
    APHI_GameState_Fixed();
    UPROPERTY(BlueprintReadOnly)
    float ProgressionPercent;
    UPROPERTY(BlueprintReadOnly)
    int32 CurrentStage;
    UFUNCTION(BlueprintCallable)
    void AdvanceProgression(int32 Stage, float Percent);
    UFUNCTION(BlueprintCallable)
    void AddGameplayNeed(FString Need);
};