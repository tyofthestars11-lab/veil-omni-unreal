// VEIL: THE LAST SIGNAL - pause menu implementation.
#include "Game/VEILPauseMenu.h"
#include "Styling/CoreStyle.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Game/VEILGameMode.h"
#include "Systems/VEILOmniAll.h"
#include "Character/VEILCharacter.h"
#include "GameFramework/Pawn.h"
#include "Components/Border.h"
#include "Styling/CoreStyle.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "VEIL.h"

void UVEILPauseMenu::NativeConstruct()
{
	Super::NativeConstruct();

	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Canvas;

	// PAUSED IS A PLACE, NOT A GREY POPUP. Navy field under gold rubrics - the same
	// page the title screen is, so opening the menu never flips the screen to a dark
	// random panel over the world.
	UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Backdrop"));
	Backdrop->SetBrushColor(FLinearColor(0.043f, 0.118f, 0.227f, 0.93f));
	if (UCanvasPanelSlot* BgSlot = Canvas->AddChildToCanvas(Backdrop))
	{
		BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
		BgSlot->SetOffsets(FMargin(0.f));
		BgSlot->SetZOrder(-10);
	}

	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("MenuBox"));
	UCanvasPanelSlot* BoxSlot = Canvas->AddChildToCanvas(Box);
	BoxSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
	BoxSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	BoxSlot->SetAutoSize(true);

	auto AddText = [this, Box](const FString& Content, int32 Size) -> UTextBlock*
	{
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetText(FText::FromString(Content));
		Text->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", Size));
		Text->SetColorAndOpacity(FSlateColor(Size >= 40
			? FLinearColor(0.788f, 0.635f, 0.290f, 1.f)
			: FLinearColor(0.945f, 0.902f, 0.800f, 1.f)));
		UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Text);
		Slot->SetPadding(FMargin(0.f, 6.f, 0.f, 6.f));
		Slot->SetHorizontalAlignment(HAlign_Center);
		return Text;
	};

	auto AddButton = [this, Box](const FString& Label) -> UButton*
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetText(FText::FromString(Label));
		// FSlateFontInfo from a .ttf path is deprecated (C4996 - it cannot carry a
		// composite font). The style sheet's own font is the supported constructor and
		// is the same Roboto at the same size.
		Text->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 22));
		// Gold on navy, not the engine's grey box.
		FButtonStyle Style = FCoreStyle::Get().GetWidgetStyle<FButtonStyle>("Button");
		Style.Normal.TintColor         = FSlateColor(FLinearColor(0.055f, 0.086f, 0.157f, 0.96f));
		Style.Hovered.TintColor        = FSlateColor(FLinearColor(0.196f, 0.153f, 0.067f, 1.f));
		Style.Pressed.TintColor        = FSlateColor(FLinearColor(0.788f, 0.635f, 0.290f, 0.90f));
		Button->SetStyle(Style);
		Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.945f, 0.902f, 0.800f, 1.f)));
		Button->AddChild(Text);
		UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Button);
		Slot->SetPadding(FMargin(0.f, 4.f, 0.f, 4.f));
		return Button;
	};

	AddText(TEXT("PAUSED"), 40);
	AddText(TEXT("THE LAST SIGNAL HOLDS"), 16);

	GodButton = AddButton(TEXT("GODMODE"));
	GodLabel = AddText(TEXT(""), 16);
	ResumeButton = AddButton(TEXT("Resume"));
	QualityButton = AddButton(TEXT("Graphics"));
	QualityLabel = AddText(TEXT(""), 16);
	TitleButton = AddButton(TEXT("Quit to Title"));
	SaveButton = AddButton(TEXT("SAVE GAME"));
	LoadButton = AddButton(TEXT("LOAD GAME"));
	SaveLabel = AddText(TEXT(""), 14);
	QuitButton = AddButton(TEXT("Quit Game"));

	ResumeButton->OnClicked.AddDynamic(this, &UVEILPauseMenu::OnResumeClicked);
	GodButton->OnClicked.AddDynamic(this, &UVEILPauseMenu::OnGodClicked);
	QualityButton->OnClicked.AddDynamic(this, &UVEILPauseMenu::OnQualityClicked);
	TitleButton->OnClicked.AddDynamic(this, &UVEILPauseMenu::OnTitleClicked);
	QuitButton->OnClicked.AddDynamic(this, &UVEILPauseMenu::OnQuitClicked);
	SaveButton->OnClicked.AddDynamic(this, &UVEILPauseMenu::OnSaveClicked);
	LoadButton->OnClicked.AddDynamic(this, &UVEILPauseMenu::OnLoadClicked);

	ResumeButton->SetKeyboardFocus();
	RefreshQualityLabel();
	RefreshGodLabel();
	RefreshSaveLabel();
	// BUILT IS NOT THE SAME AS WORKS, SO IT SAYS WHICH ITEMS EXIST AND WHAT THE SLOT IS.
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI SAVE MENU: built - SAVE GAME %s, LOAD GAME %s, slot VEILOMNI_Omni0 %s."),
		SaveButton ? TEXT("present") : TEXT("MISSING"),
		LoadButton ? TEXT("present") : TEXT("MISSING"),
		UGameplayStatics::DoesSaveGameExist(TEXT("VEILOMNI_Omni0"), 0) ? TEXT("present") : TEXT("empty"));
}

void UVEILPauseMenu::RefreshGodLabel()
{
	bool bOn = false;
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AVEILCharacter* Runner = Cast<AVEILCharacter>(PC->GetPawn()))
		{
			bOn = Runner->IsGodMode();
		}
	}
	if (GodLabel)
	{
		GodLabel->SetText(FText::FromString(bOn ? TEXT("GODMODE: ON") : TEXT("GODMODE: OFF")));
	}
}

void UVEILPauseMenu::OnGodClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (AVEILCharacter* Runner = Cast<AVEILCharacter>(PC->GetPawn()))
		{
			Runner->ToggleGodMode();
		}
	}
	RefreshGodLabel();
}

void UVEILPauseMenu::RefreshQualityLabel()
{
	static const TCHAR* Names[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
	if (QualityLabel)
	{
		QualityLabel->SetText(FText::FromString(FString::Printf(TEXT("Graphics: %s"), Names[QualityLevel])));
	}
}

void UVEILPauseMenu::OnResumeClicked()
{
	if (AVEILPlayerController* PC = Cast<AVEILPlayerController>(GetOwningPlayer()))
	{
		PC->TogglePauseMenu();
	}
}

void UVEILPauseMenu::OnQualityClicked()
{
	QualityLevel = (QualityLevel + 1) % 4;
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		Settings->SetOverallScalabilityLevel(QualityLevel);
		Settings->ApplySettings(false);
	}
	RefreshQualityLabel();
}

void UVEILPauseMenu::OnTitleClicked()
{
	// Back to the title: reload the level; boot state is the main menu
	// over a paused world, so the game lands on its title screen.
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetPause(false);
	}
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
	UE_LOG(LogVEIL, Log, TEXT("Pause menu: Quit to Title."));
}

void UVEILPauseMenu::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

// THE SAVE MENU IS NOT A SEPARATE SAVE. Both buttons call the one system that already owns the
// slot, and the label afterwards asks the DISK whether the slot exists rather than reporting what
// the button hoped for.
void UVEILPauseMenu::OnSaveClicked()
{
	UWorld* W = GetWorld();
	UVEILOmniAll* Omni = W ? W->GetSubsystem<UVEILOmniAll>() : nullptr;
	const bool bWrote = Omni ? Omni->SaveGame() : false;
	UE_LOG(LogVEIL, Warning, TEXT("PHI SAVE MENU: manual save %s, slot %s."),
		bWrote ? TEXT("written") : TEXT("refused"), Omni ? *Omni->LastSaveSlot : TEXT("none"));
	RefreshSaveLabel();
}

void UVEILPauseMenu::OnLoadClicked()
{
	UWorld* W = GetWorld();
	UVEILOmniAll* Omni = W ? W->GetSubsystem<UVEILOmniAll>() : nullptr;
	const bool bRead = Omni ? Omni->LoadGame() : false;
	UE_LOG(LogVEIL, Warning, TEXT("PHI SAVE MENU: manual load %s, slot %s."),
		bRead ? TEXT("read") : TEXT("refused"), Omni ? *Omni->LastSaveSlot : TEXT("none"));
	RefreshSaveLabel();
}

void UVEILPauseMenu::RefreshSaveLabel()
{
	// Ask the disk. A label that says SAVED because a button was pressed is a label that can lie.
	const bool bOnDisk = UGameplayStatics::DoesSaveGameExist(TEXT("VEILOMNI_Omni0"), 0);
	if (SaveLabel)
	{
		SaveLabel->SetText(FText::FromString(bOnDisk
			? TEXT("SAVE VEILOMNI_Omni0 - present on disk")
			: TEXT("SAVE VEILOMNI_Omni0 - EMPTY, press SAVE GAME")));
	}
}
