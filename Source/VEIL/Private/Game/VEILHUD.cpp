// VEIL: THE LAST SIGNAL - gameplay-loop HUD implementation.
#include "Game/VEILHUD.h"
#include "Blueprint/WidgetTree.h"
#include "Game/VEILGameMode.h"
#include "Character/VEILCharacter.h"
#include "Systems/OMNIQuestSystem.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/CoreStyle.h"

static UTextBlock* PhiMakeLabel(UUserWidget* Owner, UVerticalBox* Box, int32 Size, const FLinearColor& Color)
{
	UTextBlock* T = Owner->WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	T->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), Size));
	T->SetColorAndOpacity(FSlateColor(Color));
	T->SetShadowOffset(FVector2D(2.f, 2.f));
	Box->AddChildToVerticalBox(T);
	return T;
}

void UVEILHUD::NativeConstruct()
{
	Super::NativeConstruct();
	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass());
	WidgetTree->RootWidget = Root;
	UCanvasPanelSlot* RootSlot = Cast<UCanvasPanelSlot>(Root->Slot);
	if (RootSlot) RootSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));

	UVerticalBox* Top = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(Root->AddChild(Top)))
	{
		S->SetAnchors(FAnchors(0.f, 0.f, 0.f, 0.f));
		S->SetOffsets(FMargin(24.f, 20.f, 640.f, 220.f));
	}
	ObjectiveText = PhiMakeLabel(this, Top, 24, FLinearColor(1.f, 0.85f, 0.3f));
	HealthText = PhiMakeLabel(this, Top, 20, FLinearColor(0.4f, 1.f, 0.4f));
	ProgressText = PhiMakeLabel(this, Top, 20, FLinearColor(0.7f, 0.9f, 1.f));
	PhaseText = PhiMakeLabel(this, Top, 18, FLinearColor(0.8f, 0.8f, 0.8f));

	UVerticalBox* Center = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(Root->AddChild(Center)))
	{
		S->SetAnchors(FAnchors(0.5f, 0.4f, 0.5f, 0.4f));
		S->SetOffsets(FMargin(-300.f, -80.f, 600.f, 160.f));
		S->SetAlignment(FVector2D(0.5f, 0.5f));
	}
	WinText = PhiMakeLabel(this, Center, 44, FLinearColor(1.f, 0.85f, 0.2f));
	WinText->SetText(FText::FromString(TEXT("SIGNAL RESTORED - YOU WIN")));
	WinText->SetVisibility(ESlateVisibility::Collapsed);
	LoseText = PhiMakeLabel(this, Center, 40, FLinearColor(1.f, 0.25f, 0.2f));
	LoseText->SetText(FText::FromString(TEXT("SIGNAL LOST")));
	LoseText->SetVisibility(ESlateVisibility::Collapsed);
	RespawnButton = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
	UTextBlock* BL = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
	BL->SetText(FText::FromString(TEXT("RESPAWN AT THE VAULT")));
	BL->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), 22));
	RespawnButton->AddChild(BL);
	RespawnButton->OnClicked.AddDynamic(this, &UVEILHUD::OnRespawnClicked);
	RespawnButton->SetVisibility(ESlateVisibility::Collapsed);
	Center->AddChildToVerticalBox(RespawnButton);
	UE_LOG(LogTemp, Display, TEXT("PHI HUD BUILT: root=%s kids=%d"), *GetName(), Root->GetChildrenCount());
}

const TCHAR* UVEILHUD::ObjectiveForStage(int32 StageIdx)
{
	switch (StageIdx)
	{
	case 0: return TEXT("Find the KEEPER statue (gold) and walk to it");
	case 1: return TEXT("Gather the 3 signal fragments (walk into them)");
	case 2: return TEXT("Find the SEER statue and walk to it");
	case 3: return TEXT("Reach the green CACHE at the south wall");
	case 4: return TEXT("WARDEN PRIME wakes - bring it down");
	default: return TEXT("Signal restored - district breathes");
	}
}

void UVEILHUD::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	static bool bVisLogged = false;
	if (!bVisLogged && ObjectiveText) { bVisLogged = true;
		UE_LOG(LogTemp, Display, TEXT("PHI HUD VISIBLE: vis=%d viewport=%d text=%s"), (int32)GetVisibility(), IsInViewport() ? 1 : 0, *ObjectiveText->GetText().ToString()); }
	RefreshFromWorld();
}

void UVEILHUD::RefreshFromWorld()
{
	UWorld* W = GetWorld();
	if (!W) return;
	APlayerController* PC = GetOwningPlayer();
	AVEILCharacter* Pawn = PC ? Cast<AVEILCharacter>(PC->GetPawn()) : nullptr;
	UOMNIQuestSystem* Q = W->GetSubsystem<UOMNIQuestSystem>();
	AVEILGameMode* GM = Cast<AVEILGameMode>(UGameplayStatics::GetGameMode(this));

	const int32 Stage = Q ? (int32)Q->Stage : 0;
	if (Stage != LastStage)
	{
		LastStage = Stage;
		UE_LOG(LogTemp, Display, TEXT("PHI LOOP OBJECTIVE: stage=%d %s"), Stage, ObjectiveForStage(Stage));
	}
	if (ObjectiveText) ObjectiveText->SetText(FText::FromString(
		FString::Printf(TEXT("OBJECTIVE: %s"), ObjectiveForStage(Stage))));
	if (Pawn && HealthText) HealthText->SetText(FText::FromString(
		FString::Printf(TEXT("HEALTH: %.0f / %.0f"), Pawn->Health, Pawn->MaxHealth)));
	if (Q && ProgressText) ProgressText->SetText(FText::FromString(
		FString::Printf(TEXT("LEVEL %d   XP %d/%d   FRAGMENTS %d/3   GOLD %d"),
			Q->Level, Q->XP, UOMNIQuestSystem::XPNeedForLevel(Q->Level), Q->Fragments, Q->Gold)));
	const int32 Phase = GM ? (int32)GM->Phase : -1;
	if (Phase != LastPhase)
	{
		LastPhase = Phase;
		UE_LOG(LogTemp, Display, TEXT("PHI LOOP PHASE: %d"), Phase);
	}
	if (PhaseText && GM) PhaseText->SetText(FText::FromString(
		FString::Printf(TEXT("SIGNAL PHASE: %d"), Phase)));

	// WIN: the loop closed - mystery solved, warden down, cache read.
	if (GM && GM->Phase == EVEILGamePhase::Resolution && !bWinShown)
	{
		bWinShown = true;
		if (WinText) WinText->SetVisibility(ESlateVisibility::Visible);
		UE_LOG(LogTemp, Display, TEXT("PHI LOOP WIN: signal restored, Resolution reached"));
	}
	// LOSE: the runner fell - offer respawn at the Vault seat.
	const bool bDead = Pawn && !Pawn->IsAlive();
	if (bDead && !bLoseShown)
	{
		bLoseShown = true;
		if (LoseText) LoseText->SetVisibility(ESlateVisibility::Visible);
		if (RespawnButton) RespawnButton->SetVisibility(ESlateVisibility::Visible);
		if (PC) { PC->SetInputMode(FInputModeGameAndUI()); PC->bShowMouseCursor = true; }
		UE_LOG(LogTemp, Display, TEXT("PHI LOOP LOSE: runner down, respawn offered"));
	}
	if (!bDead && bLoseShown)
	{
		bLoseShown = false;
		if (LoseText) LoseText->SetVisibility(ESlateVisibility::Collapsed);
		if (RespawnButton) RespawnButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UVEILHUD::OnRespawnClicked()
{
	if (AVEILPlayerController* PC = Cast<AVEILPlayerController>(GetOwningPlayer()))
	{
		PC->RespawnRunner();
	}
}

