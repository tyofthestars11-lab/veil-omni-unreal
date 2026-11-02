#pragma once
#include "CoreMinimal.h"
#include "PHI_GodmodeSafe.generated.h"
class APawn;
UCLASS()
class VEIL_API UPHI_GodmodeSafe : public UObject
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="PHI|Godmode")
    static void SetGodmode(APawn* Pawn, bool Enable);
    UFUNCTION(BlueprintCallable, Category="PHI|Godmode")
    static bool IsGodmode(APawn* Pawn);
};