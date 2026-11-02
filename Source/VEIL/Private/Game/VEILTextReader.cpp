// VEIL text reader implementation: built purely in C++, no assets.
#include "Game/VEILTextReader.h"

#include "Styling/CoreStyle.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "Misc/Paths.h"

static FSlateFontInfo ReaderFont(int32 Size, bool bBold)
{
	const FString Face = bBold ? TEXT("Slate/Fonts/Roboto-Bold.ttf") : TEXT("Slate/Fonts/Roboto-Regular.ttf");
	return FCoreStyle::GetDefaultFontStyle(Face == TEXT("Slate/Fonts/Roboto-Bold.ttf") ? "Bold" : "Regular", Size);
}

void UVEILTextReader::NativeConstruct()
{
	Super::NativeConstruct();
	Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("ReaderRoot"));
	WidgetTree->RootWidget = Root;
	UVerticalBox* Col = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("ReaderCol"));
	if (UCanvasPanelSlot* RS = Root->AddChildToCanvas(Col))
	{
		RS->SetAnchors(FAnchors(0.18f, 0.16f, 0.82f, 0.84f));
		RS->SetOffsets(FMargin(0.f));
	}
	TitleText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReaderTitle"));
	TitleText->SetFont(ReaderFont(30, true));
	TitleText->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.45f)));
	BodyText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReaderBody"));
	BodyText->SetFont(ReaderFont(21, false));
	BodyText->SetColorAndOpacity(FSlateColor(FLinearColor(0.93f, 0.93f, 0.88f)));
	BodyText->SetAutoWrapText(true);
	PageText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), TEXT("ReaderPage"));
	PageText->SetFont(ReaderFont(16, false));
	PageText->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.65f, 0.7f)));
	if (UVerticalBoxSlot* S1 = Col->AddChildToVerticalBox(TitleText)) { S1->SetPadding(FMargin(24.f, 20.f, 24.f, 8.f)); }
	if (UVerticalBoxSlot* S2 = Col->AddChildToVerticalBox(BodyText)) { S2->SetPadding(FMargin(24.f, 8.f, 24.f, 8.f)); }
	if (UVerticalBoxSlot* S3 = Col->AddChildToVerticalBox(PageText)) { S3->SetPadding(FMargin(24.f, 8.f, 24.f, 20.f)); }
	SetVisibility(ESlateVisibility::Collapsed);
}

void UVEILTextReader::OpenReader(const FString& Title, const TArray<FString>& Pages)
{
	ReaderTitle = Title;
	ReaderPages = Pages;
	PageIx = 0;
	bIsOpen = true;
	ShowPage();
	SetVisibility(ESlateVisibility::Visible);
}

void UVEILTextReader::CloseReader()
{
	bIsOpen = false;
	SetVisibility(ESlateVisibility::Collapsed);
	RemoveFromParent();
}

void UVEILTextReader::NextPage()
{
	if (ReaderPages.Num() < 1) return;
	PageIx = FMath::Min(PageIx + 1, ReaderPages.Num() - 1);
	ShowPage();
}

void UVEILTextReader::PrevPage()
{
	if (ReaderPages.Num() < 1) return;
	PageIx = FMath::Max(PageIx - 1, 0);
	ShowPage();
}

void UVEILTextReader::ShowPage()
{
	if (TitleText) { TitleText->SetText(FText::FromString(ReaderTitle)); }
	if (BodyText) { BodyText->SetText(FText::FromString(ReaderPages.IsValidIndex(PageIx) ? ReaderPages[PageIx] : TEXT(""))); }
	if (PageText)
	{
		PageText->SetText(FText::FromString(FString::Printf(TEXT("â€” %d of %d â€”   wheel / arrows turn the page, E closes"), PageIx + 1, FMath::Max(1, ReaderPages.Num()))));
	}
}

