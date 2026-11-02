// VEIL text reader: large-font lore panel over a living world.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "VEILTextReader.generated.h"

class UCanvasPanel;
class UTextBlock;

UCLASS()
class VEIL_API UVEILTextReader : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	void OpenReader(const FString& Title, const TArray<FString>& Pages);
	void CloseReader();
	bool IsOpen() const { return bIsOpen; }
	void NextPage();
	void PrevPage();

private:
	void ShowPage();
	bool bIsOpen = false;
	FString ReaderTitle;
	TArray<FString> ReaderPages;
	int32 PageIx = 0;
	UPROPERTY() UCanvasPanel* Root = nullptr;
	UPROPERTY() UTextBlock* TitleText = nullptr;
	UPROPERTY() UTextBlock* BodyText = nullptr;
	UPROPERTY() UTextBlock* PageText = nullptr;
};
