// VEIL: THE LAST SIGNAL - main menu, built in C++ (no widget assets needed).
// Title, Enter the Signal (start), Graphics (cycles scalability for the
// RTX 3050), Quit. Shown by AVEILPlayerController at boot; works with
// mouse, keyboard focus, and Xbox controller navigation.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VEILMainMenu.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class VEIL_API UVEILMainMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnContinueClicked();
	UFUNCTION()
	void OnNewGameClicked();
	UFUNCTION()
	void OnLoadClicked();
	UFUNCTION()
	void OnInventoryClicked();
	UFUNCTION()
	void OnMapClicked();
	UFUNCTION()
	void OnQuestsClicked();
	UFUNCTION()
	void OnCharactersClicked();
	UFUNCTION()
	void OnMountsClicked();
	UFUNCTION()
	void OnCraftClicked();
	UFUNCTION()
	void OnShopClicked();
	UFUNCTION()
	void OnGuildClicked();
	UFUNCTION()
	void OnArenaClicked();
	UFUNCTION()
	void OnSettingsClicked();
	UFUNCTION()
	void OnCreditsClicked();

	/** One line under the buttons. Every section reports something true, or says it has none. */
	void OpenSection(const FString& Name, const FString& Detail);

	UFUNCTION()
	void OnGodClicked();

	UFUNCTION()
	void OnQualityClicked();

	UFUNCTION()
	void OnQuitClicked();

	void RefreshQualityLabel();
	void RefreshGodLabel();

	TObjectPtr<UButton> StartButton;
	TObjectPtr<UButton> GodButton;
	TObjectPtr<UButton> QualityButton;
	TObjectPtr<UButton> QuitButton;
	TObjectPtr<UButton> ContinueButton;
	TObjectPtr<UButton> NewGameButton;
	TObjectPtr<UButton> LoadButton;
	TObjectPtr<UButton> InventoryButton;
	TObjectPtr<UButton> MapButton;
	TObjectPtr<UButton> QuestsButton;
	TObjectPtr<UButton> CharactersButton;
	TObjectPtr<UButton> MountsButton;
	TObjectPtr<UButton> CraftButton;
	TObjectPtr<UButton> ShopButton;
	TObjectPtr<UButton> GuildButton;
	TObjectPtr<UButton> ArenaButton;
	TObjectPtr<UButton> SettingsButton;
	TObjectPtr<UButton> CreditsButton;
	TObjectPtr<UTextBlock> QualityLabel;
	TObjectPtr<UTextBlock> GodLabel;
	TObjectPtr<UTextBlock> SectionLabel;
	TObjectPtr<UTextBlock> LedgerLabel;

	/** 0 Low, 1 Medium, 2 High, 3 Epic - prototype default is 1 (Medium). */
	int32 QualityLevel = 1;
};
