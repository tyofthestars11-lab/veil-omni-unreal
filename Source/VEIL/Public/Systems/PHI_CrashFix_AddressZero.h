#pragma once
#include "CoreMinimal.h"
class APawn;
class UWorld;
#include "PHI_CrashFix_AddressZero.generated.h"
UCLASS()
class VEIL_API UPHI_CrashFix_AddressZero : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PHI|Crash")
    static bool IsValidPawnForGodmode(APawn* Pawn);
    UFUNCTION(BlueprintCallable, Category="PHI|Crash")
    static bool IsValidWorldForProgression(UWorld* World);
    UFUNCTION(BlueprintCallable, Category="PHI|Crash")
    static void GuardedAdvanceProgression(UObject* WorldContext, int32 Stage, float Percent);
};