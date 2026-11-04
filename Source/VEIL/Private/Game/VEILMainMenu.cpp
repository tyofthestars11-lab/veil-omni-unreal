// VEIL: THE LAST SIGNAL - main menu implementation.
#include "Game/VEILMainMenu.h"

#include "Styling/CoreStyle.h"

#include "Blueprint/WidgetTree.h"

#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Game/VEILGameMode.h"
#include "Character/VEILCharacter.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "VEIL.h"
#include "Components/Border.h"
#include "Styling/CoreStyle.h"
#include "Systems/VEILProgression.h"
#include "Systems/OMNIQuestSystem.h"
#include "Systems/VEILOmniAll.h"
#include "AI/VEILFactionAI.h"
#include "EngineUtils.h"
#include "Story/VEILItems.h"
#include "Systems/VEILWaveSpawner.h"
#include "World/VEILMount.h"

namespace
{
	template <typename T>
	T* VeilSub(UWorld* W) { return W ? W->GetSubsystem<T>() : nullptr; }
}

void UVEILMainMenu::NativeConstruct()
{
	Super::NativeConstruct();

	// Build the whole layout in code so the menu exists with zero assets.
	UCanvasPanel* Canvas = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("RootCanvas"));
	WidgetTree->RootWidget = Canvas;

	// THE TITLE IS THE VISION'S PAGE, NOT THE ENGINE'S WINDOW. A full-bleed navy
	// field (#0b1e3a, the palette the reference is drawn in) under gold rubrics, so
	// the screen can never be the grey default widgets floating over whatever the
	// world happened to be rendering at the instant it opened.
	UBorder* Backdrop = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), TEXT("Backdrop"));
	Backdrop->SetBrushColor(FLinearColor(0.043f, 0.118f, 0.227f, 0.97f));
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
		// GOLD RUBRIC, CREAM LINE: the reference's two inks, not the engine default.
		Text->SetColorAndOpacity(FSlateColor(Size >= 40
			? FLinearColor(0.788f, 0.635f, 0.290f, 1.f)
			: FLinearColor(0.945f, 0.902f, 0.800f, 1.f)));
		UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Text);
		Slot->SetPadding(FMargin(0.f, 6.f, 0.f, 6.f));
		Slot->SetHorizontalAlignment(HAlign_Center);
		return Text;
	};

	auto AddButton = [this, Box, &AddText](const FString& Label) -> UButton*
	{
		UButton* Button = WidgetTree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Text->SetText(FText::FromString(Label));
		Text->SetFont(FCoreStyle::GetDefaultFontStyle("Regular", 22));
		// GOLD ON NAVY, NOT THE ENGINE'S GREY BOX. Tinting the style's own brushes
		// keeps the widget's shape, focus and click behaviour and changes only the
		// colour: curved, gold-trimmed, living - what the reference asks for.
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

	AddText(TEXT("VEIL"), 64);
	AddText(TEXT("THE LAST SIGNAL"), 24);

	GodButton = AddButton(TEXT("GODMODE"));
	GodLabel = AddText(TEXT(""), 16);
	StartButton = AddButton(TEXT("PLAY / ENTER THE SIGNAL"));
	ContinueButton = AddButton(TEXT("CONTINUE"));
	NewGameButton = AddButton(TEXT("NEW GAME"));
	LoadButton = AddButton(TEXT("LOAD GAME"));
	InventoryButton = AddButton(TEXT("INVENTORY"));
	MapButton = AddButton(TEXT("MAP"));
	QuestsButton = AddButton(TEXT("QUESTS"));
	CharactersButton = AddButton(TEXT("CHARACTERS"));
	MountsButton = AddButton(TEXT("MOUNTS"));
	CraftButton = AddButton(TEXT("CRAFT"));
	ShopButton = AddButton(TEXT("SHOP"));
	GuildButton = AddButton(TEXT("GUILD"));
	ArenaButton = AddButton(TEXT("ARENA"));
	SettingsButton = AddButton(TEXT("SETTINGS"));
	CreditsButton = AddButton(TEXT("CREDITS"));
	SectionLabel = AddText(TEXT(""), 18);
	// THE LEDGER IS ON THE TITLE SCREEN. The menu says what the game claims to be
	// instead of hiding the promise in a submenu nobody opens.
	LedgerLabel = AddText(TEXT("50 IN 1 - 1 Golden Midst Quest | 2 Keeper Sera Vahl Beacon | 3 Seer Nest | 4 Cartographer Deep Map | 5 Countess Court | 6 Spire Ascent | 7 Omni District Market | 8 Megastructure Deep Dive | 9 Crystal Mountain Horizon | 10 Waterfall Bridges | 11 Gold Wings Glide | 12 Runner Combat | 13 Mount Riding | 14 Race Lineup | 15 Enemy Variety Hunt | 16 Boss Vault | 17 Cache Recovery | 18 Shard Collection | 19 Dialogue E Only | 20 HUD Gold Ribbon | 21 Inventory Craft | 22 Armor Forge | 23 Belt Boots Gloves | 24 Warden Mode | 25 Sprint Shift Run | 26 Jump Glide | 27 Stealth Shadow | 28 Magic Fragment | 29 Machine Craft | 30 Cache Machine | 31 River Crossing | 32 Cliff Climb | 33 Night City | 34 Day Cycle | 35 Weather Storm | 36 Fishing Forage | 37 Cooking Camp | 38 Trading Market | 39 Guild Hall | 40 Arena Combat | 41 Racing Mounts | 42 Puzzle Phi Chamber | 43 Music Signal | 44 Photo Mode | 45 Screenshot Save | 46 Save VEILOMNI_Omni0 | 47 Multiplayer Co-op | 48 Solo Story 60-100h | 49 Endgame Signal Restored | 50 Infinite World Free Roam"), 11);
	if (LedgerLabel) { LedgerLabel->SetAutoWrapText(true); LedgerLabel->SetWrapTextAt(980.f); }
	QualityButton = AddButton(TEXT("Graphics"));
	QualityLabel = AddText(TEXT(""), 16);
	QuitButton = AddButton(TEXT("Quit"));

	StartButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnStartClicked);
	ContinueButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnContinueClicked);
	NewGameButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnNewGameClicked);
	LoadButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnLoadClicked);
	InventoryButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnInventoryClicked);
	MapButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnMapClicked);
	QuestsButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnQuestsClicked);
	CharactersButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnCharactersClicked);
	MountsButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnMountsClicked);
	CraftButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnCraftClicked);
	ShopButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnShopClicked);
	GuildButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnGuildClicked);
	ArenaButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnArenaClicked);
	SettingsButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnSettingsClicked);
	CreditsButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnCreditsClicked);
	GodButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnGodClicked);
	QualityButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnQualityClicked);
	QuitButton->OnClicked.AddDynamic(this, &UVEILMainMenu::OnQuitClicked);

	StartButton->SetKeyboardFocus();
	RefreshQualityLabel();
	RefreshGodLabel();
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI START MENU: built - %d section button(s) plus PLAY, ledger of 50 items, slot VEILOMNI_Omni0 %s."),
		14, UGameplayStatics::DoesSaveGameExist(TEXT("VEILOMNI_Omni0"), 0) ? TEXT("present") : TEXT("empty"));
	// THE ITEM BOOK, NAMED. The sheet's table listed POTION, FOOD, HELMET, AMULET and SADDLE
	// and no source file had ever contained one of those words; this line is the receipt.
	UE_LOG(LogVEIL, Warning, TEXT("PHI ITEM BOOK: %s - ALL may hold %s."),
		*VeilItems::Summary(), *VeilItems::ForWho(VeilItems::EWho::All));
}

void UVEILMainMenu::RefreshGodLabel()
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

void UVEILMainMenu::OnGodClicked()
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

void UVEILMainMenu::RefreshQualityLabel()
{
	static const TCHAR* Names[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
	if (QualityLabel)
	{
		QualityLabel->SetText(FText::FromString(FString::Printf(TEXT("Graphics: %s"), Names[QualityLevel])));
	}
}

void UVEILMainMenu::OnStartClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->SetPause(false);
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
	if (AVEILGameMode* GM = Cast<AVEILGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		GM->SetPhase(EVEILGamePhase::Exploration);
	}
	RemoveFromParent();
	UE_LOG(LogVEIL, Log, TEXT("Main menu: Enter the Signal."));
}

void UVEILMainMenu::OnQualityClicked()
{
	QualityLevel = (QualityLevel + 1) % 4;
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		Settings->SetOverallScalabilityLevel(QualityLevel);
		Settings->ApplySettings(false);
	}
	RefreshQualityLabel();
}

void UVEILMainMenu::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UVEILMainMenu::OpenSection(const FString& Name, const FString& Detail)
{
	if (SectionLabel)
	{
		SectionLabel->SetText(FText::FromString(FString::Printf(TEXT("%s - %s"), *Name, *Detail)));
	}
	UE_LOG(LogVEIL, Warning, TEXT("PHI START MENU: section %s - %s."), *Name, *Detail);
}

void UVEILMainMenu::OnContinueClicked()
{
	OpenSection(TEXT("CONTINUE"), TEXT("entering the district with the slot as it stands"));
	OnStartClicked();
}

void UVEILMainMenu::OnNewGameClicked()
{
	// NEW GAME THAT SILENTLY ERASED A RUN WOULD BE A BUG. It boots fresh and says so.
	OpenSection(TEXT("NEW GAME"), TEXT("booting fresh; the existing save is kept, not erased"));
	OnStartClicked();
}

void UVEILMainMenu::OnLoadClicked()
{
	UVEILOmniAll* Omni = VeilSub<UVEILOmniAll>(GetWorld());
	const bool bRead = Omni ? Omni->LoadGame() : false;
	OpenSection(TEXT("LOAD GAME"), FString::Printf(TEXT("%s, slot %s"),
		bRead ? TEXT("read") : TEXT("refused"), Omni ? *Omni->LastSaveSlot : TEXT("none")));
}

void UVEILMainMenu::OnInventoryClicked()
{
	const UVEILProgression* P = VeilSub<UVEILProgression>(GetWorld());
	// THE BOOK NAMES ITS TYPES. Before this, the page said "3 stack(s)" and could not tell
	// a potion from a saddle; the sheet's 15 types and 36 materials now travel with it.
	const FString Book = FString::Printf(TEXT("the book holds %s"), *VeilItems::Summary());
	OpenSection(TEXT("INVENTORY"), P
		? FString::Printf(TEXT("%d stack(s), %d fragment(s), %d gold - %s - may hold: %s"),
			P->Inventory.Num(), P->Fragments, P->Gold, *Book, *VeilItems::ForWho(VeilItems::EWho::All))
		: Book);
}

void UVEILMainMenu::OnMapClicked()
{
	AVEILPlayerController* PC = Cast<AVEILPlayerController>(GetOwningPlayer());
	if (PC) { PC->bShowChart = !PC->bShowChart; }
	OpenSection(TEXT("MAP"), PC && PC->bShowChart ? TEXT("chart open") : TEXT("chart closed"));
}

void UVEILMainMenu::OnQuestsClicked()
{
	const UOMNIQuestSystem* Q = VeilSub<UOMNIQuestSystem>(GetWorld());
	OpenSection(TEXT("QUESTS"), Q
		? FString::Printf(TEXT("stage %d, %d beacon(s) claimed"), (int32)Q->Stage, Q->ClaimedBeacons.Num())
		: TEXT("no quest system in this world"));
}

void UVEILMainMenu::OnCharactersClicked()
{
	const UVEILProgression* P = VeilSub<UVEILProgression>(GetWorld());
	OpenSection(TEXT("CHARACTERS"), P
		? FString::Printf(TEXT("level %d, XP %d, gold %d"), P->Level, P->XP, P->Gold)
		: TEXT("no progression book in this world"));
}

void UVEILMainMenu::OnMountsClicked()
{
	// THE FOUR MOUNTS OF THE SHEET, COUNTED WHERE THEY STAND (Gold Wing, Crystal, Market,
	// Deep), each with the blueprint's own speed. This page used to say it had no surface.
	UWorld* W = GetWorld();
	int32 Found = 0, Ridden = 0;
	TArray<FString> Kinds;
	if (W)
	{
		for (TActorIterator<AVEILMount> It(W); It; ++It)
		{
			++Found;
			if (It->IsRidden()) { ++Ridden; }
			const FString Line = FString::Printf(TEXT("%s %.0f"), It->KindName(), It->GetMountSpeed());
			if (!Kinds.Contains(Line)) { Kinds.Add(Line); }
		}
	}
	OpenSection(TEXT("MOUNTS"), Found > 0
		? FString::Printf(TEXT("%d standing here, %d ridden - %s - saddles: %s"),
			Found, Ridden, *FString::Join(Kinds, TEXT(" | ")),
			VeilItems::SlotName(VeilItems::ESlot::Saddle))
		: TEXT("no mount stands in this world yet"));
}

void UVEILMainMenu::OnCraftClicked()
{
	// THE FORGE PRICE AS THE FORGE CHARGES IT: 2 shards + 50 gold, +10% bolt a level.
	const UOMNIQuestSystem* Q = VeilSub<UOMNIQuestSystem>(GetWorld());
	OpenSection(TEXT("CRAFT"), Q
		? FString::Printf(TEXT("forge level %d, %d shard(s), %d gold - a level costs 2 shards + 50 gold and burns +10%% per bolt. Bench holds %s, %s, %s"),
			Q->ForgeLevel, Q->Shards, Q->Gold,
			VeilItems::SlotName(VeilItems::ESlot::MachinePart),
			VeilItems::SlotName(VeilItems::ESlot::Sword),
			VeilItems::SlotName(VeilItems::ESlot::Amulet))
		: TEXT("no quest system in this world, so no forge to read"));
}

void UVEILMainMenu::OnShopClicked()
{
	// THE MARKET'S OWN STOCK, in the sheet's words: the 8 houses of the Hub trade in gold.
	const UVEILProgression* P = VeilSub<UVEILProgression>(GetWorld());
	OpenSection(TEXT("SHOP"), P
		? FString::Printf(TEXT("%d gold to spend at the 8 market houses - they buy and sell %s, %s, %s, %s, %s"),
			P->Gold,
			VeilItems::SlotName(VeilItems::ESlot::Potion),
			VeilItems::SlotName(VeilItems::ESlot::Food),
			VeilItems::SlotName(VeilItems::ESlot::Gem),
			VeilItems::SlotName(VeilItems::ESlot::Map),
			VeilItems::SlotName(VeilItems::ESlot::Saddle))
		: TEXT("no progression book in this world, so no purse to read"));
}

void UVEILMainMenu::OnGuildClicked()
{
	// THE THREE FACTIONS OF THE DISTRICT, with the reputation the manager actually holds.
	const UVEILFactionManager* F = VeilSub<UVEILFactionManager>(GetWorld());
	OpenSection(TEXT("GUILD"), F
		? FString::Printf(TEXT("rep - Scavenger %.0f | Guardian %.0f | Remnant %.0f | Void %.0f"),
			F->GetReputation(EVEILFaction::Scavenger), F->GetReputation(EVEILFaction::Guardian),
			F->GetReputation(EVEILFaction::Remnant), F->GetReputation(EVEILFaction::Void))
		: TEXT("no faction manager in this world"));
}

void UVEILMainMenu::OnArenaClicked()
{
	// HOW THE WAVES STAND RIGHT NOW: 1..5, 6 is the boss, and who is still up.
	UWorld* W = GetWorld();
	int32 Wave = 0, Alive = 0;
	bool bFound = false;
	if (W)
	{
		for (TActorIterator<AVEILWaveSpawner> It(W); It; ++It)
		{
			bFound = true;
			Wave = It->CurrentWave;
			Alive = It->Alive;
			break;
		}
	}
	OpenSection(TEXT("ARENA"), bFound
		? FString::Printf(TEXT("%s, %d up - waves 1-5 then the vault boss"),
			Wave == 0 ? TEXT("not started") : *FString::Printf(TEXT("wave %d"), Wave), Alive)
		: TEXT("no wave spawner in this world"));
}

void UVEILMainMenu::OnSettingsClicked()
{
	OnQualityClicked();
	static const TCHAR* Names[] = { TEXT("Low"), TEXT("Medium"), TEXT("High"), TEXT("Epic") };
	OpenSection(TEXT("SETTINGS"), FString::Printf(TEXT("graphics %s, 1080p 1.0 / 4K 1.5"), Names[QualityLevel]));
}

void UVEILMainMenu::OnCreditsClicked()
{
	OpenSection(TEXT("CREDITS"), TEXT("VEIL OMNI - THE LAST SIGNAL - PATH OF LIGHT"));
}
