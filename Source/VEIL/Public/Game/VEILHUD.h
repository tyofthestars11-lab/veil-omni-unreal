// VEIL: THE LAST SIGNAL - gameplay-loop HUD (pure C++, zero assets).
// Objective tracker for the OMNI six-stage chain, health/XP readout,
// WIN screen on Resolution, LOSE screen on death with respawn.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VEILHUD.generated.h"

class UTextBlock;
class UButton;
class UVerticalBox;

UCLASS()
class VEIL_API UVEILHUD : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	void OnRespawnClicked();

private:
	void RefreshFromWorld();
	static const TCHAR* ObjectiveForStage(int32 StageIdx);

	TObjectPtr<UTextBlock> ObjectiveText = nullptr;
	TObjectPtr<UTextBlock> HealthText = nullptr;
	TObjectPtr<UTextBlock> ProgressText = nullptr;
	TObjectPtr<UTextBlock> PhaseText = nullptr;
	TObjectPtr<UTextBlock> WinText = nullptr;
	TObjectPtr<UTextBlock> LoseText = nullptr;
	TObjectPtr<UButton> RespawnButton = nullptr;

	int32 LastStage = -1;
	int32 LastPhase = -1;
	bool bWinShown = false;
	bool bLoseShown = false;
};
