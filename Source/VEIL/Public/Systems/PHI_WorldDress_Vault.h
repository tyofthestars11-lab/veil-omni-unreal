#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PHI_WorldDress_Vault.generated.h"
UCLASS()
class VEIL_API APHI_WorldDress_Vault : public AActor
{
    GENERATED_BODY()
public:
    APHI_WorldDress_Vault();
    virtual void BeginPlay() override;
};