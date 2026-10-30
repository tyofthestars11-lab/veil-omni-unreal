#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "VEILInteractable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UVEILInteractable : public UInterface { GENERATED_BODY() };

// Anything the Countess can interact with (anchors, fragments, altar, beacon).
class VEIL_API IVEILInteractable
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "VEIL")
	void Interact(AActor* Instigator);
};
