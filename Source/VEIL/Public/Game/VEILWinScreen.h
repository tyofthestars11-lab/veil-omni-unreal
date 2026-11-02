// VEIL: THE LAST SIGNAL - win screen, built in C++ (no widget assets needed).
// SIGNAL IS SEALED on Warden Prime's fall. A = NEW GAME+, B = keep walking.
// The world stays living behind it; pause is never set here.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VEILWinScreen.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class VEIL_API UVEILWinScreen : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetStats(int32 InGold, int32 InLevel, float InPlaySeconds, int32 InCycle);

	/** NEW GAME+ cycle counter, kept in Saved/phi_ngplus.txt. */
	static int32 ReadCycle();
	static void WriteCycle(int32 Cycle);

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnNewGameClicked();

	UFUNCTION()
	void OnKeepWalkingClicked();

	TObjectPtr<UButton> NewGameButton;
	TObjectPtr<UButton> KeepButton;
	TObjectPtr<UTextBlock> StatsText;

	int32 WinGold = 0;
	int32 WinLevel = 1;
	float WinSeconds = 0.f;
	int32 WinCycle = 0;
};
