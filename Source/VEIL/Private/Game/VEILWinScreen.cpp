// VEIL: THE LAST SIGNAL - win screen implementation.
#include "Game/VEILWinScreen.h"
#include "Styling/CoreStyle.h"
#include "Systems/PHI_Sfx.h"
#include "Systems/PHI_Sfx.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Game/VEILGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "VEIL.h"

static const TCHAR* PhiCycleFile()
{
	static FString P = FPaths::ProjectSavedDir() / TEXT("phi_ngplus.txt");
	return *P;
}

int32 UVEILWinScreen::ReadCycle()
{
	FString S;
	if (FFileHelper::LoadFileToString(S, PhiCycleFile()))
	{
		return FMath::Max(0, FCString::Atoi(*S));
	}
	return 0;
}

void UVEILWinScreen::WriteCycle(int32 Cycle)
{
	FFileHelper::SaveStringToFile(FString::FromInt(FMath::Max(0, Cycle)) + TEXT("\n"), PhiCycleFile());
}

void UVEILWinScreen::SetStats(int32 InGold, int32 InLevel, float InPlaySeconds, int32 InCycle)
{
	WinGold = InGold;
	WinLevel = InLevel;
	WinSeconds = InPlaySeconds;
	WinCycle = InCycle;
	if (StatsText)
	{
		StatsText->SetText(FText::FromString(FString::Printf(
			TEXT("Runner level %d - %d gold - %.0f seconds of signal - cycle %d"),
			WinLevel, WinGold, WinSeconds, WinCycle)));
	}
}

void UVEILWinScreen::NativeConstruct()
{
	Super::NativeConstruct();

	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Canvas;
	UBorder* Dim = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Dim"));
	Dim->SetBrushColor(FLinearColor(0.f, 0.02f, 0.06f, 0.85f));
	UCanvasPanelSlot* DimSlot = Canvas->AddChildToCanvas(Dim);
	DimSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	DimSlot->SetOffsets(FMargin(0.f));

	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("WinBox"));
	UCanvasPanelSlot* BoxSlot = Canvas->AddChildToCanvas(Box);
	BoxSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
	BoxSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	BoxSlot->SetAutoSize(true);

	auto AddText = [this, Box](const FString& Content, int32 Size) -> UTextBlock*
	{
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetText(FText::FromString(Content));
		// C4996: the .ttf-path constructor is deprecated. The style sheet's bold face
		// is the supported route and renders identically.
		Text->SetFont(FCoreStyle::GetDefaultFontStyle("Bold", Size));
		UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Text);
		Slot->SetPadding(FMargin(0.f, 6.f, 0.f, 6.f));
		Slot->SetHorizontalAlignment(HAlign_Center);
		return Text;
	};

	auto AddButton = [this, Box](const FString& Label) -> UButton*
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		Button->SetBackgroundColor(FLinearColor(0.10f, 0.08f, 0.0f, 0.95f));
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetText(FText::FromString(Label));
		Text->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 22));
		Button->AddChild(Text);
		UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Button);
		Slot->SetPadding(FMargin(0.f, 4.f, 0.f, 4.f));
		return Button;
	};

	AddText(TEXT("SIGNAL IS SEALED"), 44);
	AddText(TEXT("WARDEN PRIME HAS FALLEN - THE PATH OF LIGHT IS CLEAR"), 16);
	StatsText = AddText(TEXT(""), 16);
	SetStats(WinGold, WinLevel, WinSeconds, WinCycle);

	NewGameButton = AddButton(TEXT("NEW GAME+  (A)"));
	KeepButton = AddButton(TEXT("KEEP WALKING  (B)"));
	NewGameButton->OnClicked.AddDynamic(this, &UVEILWinScreen::OnNewGameClicked);
	KeepButton->OnClicked.AddDynamic(this, &UVEILWinScreen::OnKeepWalkingClicked);
	NewGameButton->SetKeyboardFocus();
}

void UVEILWinScreen::OnNewGameClicked()
{
	UPHI_Sfx::Click(GetWorld());
	const int32 Next = ReadCycle() + 1;
	WriteCycle(Next);
	UE_LOG(LogVEIL, Log, TEXT("PHI WIN: NEW GAME+ cycle %d sealed at level %d, %d gold."), Next, WinLevel, WinGold);
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}

void UVEILWinScreen::OnKeepWalkingClicked()
{
	UPHI_Sfx::Click(GetWorld());
	if (AVEILPlayerController* PC = Cast<AVEILPlayerController>(GetOwningPlayer()))
	{
		PC->CloseWinScreen();
	}
}

