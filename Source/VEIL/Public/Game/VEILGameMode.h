// VEIL: THE LAST SIGNAL - game mode + player controller.
// The GameMode boots the world systems (Faction, Streaming, Discovery are
// WorldSubsystems / level actors that self-register; the GameMode seeds
// their starting data and runs the game-phase state machine), scales
// difficulty, and owns player spawning.
#pragma once

#include "CoreMinimal.h"
#include "AI/VEILFactionAI.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/PlayerController.h"
#include "Systems/VEILDiscoverySystem.h"
#include "VEILGameMode.generated.h"

class UUserWidget;

UENUM(BlueprintType)
enum class EVEILGamePhase : uint8
{
	Boot,
	Exploration,   // free roam, districts stream around the player
	Discovery,     // a mystery is actively being assembled
	Confrontation, // faction fight in progress near the player
	Resolution     // mystery solved / territory settled - world exhales
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVEILPhaseChanged, EVEILGamePhase, NewPhase);

// ---------------------------------------------------------------------------
// Game Mode
// ---------------------------------------------------------------------------
UCLASS()
class VEIL_API AVEILGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AVEILGameMode();
	virtual void Tick(float DeltaSeconds) override;

protected:
	/**
	 * The runner must ALWAYS exist. The district holds the placed RUNNER at
	 * 0,0,100 - the same seat the pawn spawns on - so Unreal's default spawn was
	 * refused for collision and no player was created at all: no input, no
	 * combat, no kills, no XP. A refused spawn is not a game.
	 */
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;

public:

	UPROPERTY(BlueprintAssignable, Category = "VEIL|Game")
	FVEILPhaseChanged OnPhaseChanged;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Game")
	EVEILGamePhase Phase = EVEILGamePhase::Boot;

	UFUNCTION(BlueprintCallable, Category = "VEIL|Game")
	void SetPhase(EVEILGamePhase NewPhase);

	/**
	 * THE LOOK, DERIVED FROM THE LAW.
	 *
	 * Two captures of the same world disagree, and the disagreement is measurable: the
	 * editor's -game session read mean luminance 81.9 / chroma 38.8, the packaged build
	 * 39.5 / 13.1. Through UPHIMito::Rung that is a 1.5156-rung light gap (3/2 within
	 * 1%) and a 2.2565-rung colour gap, and the colour gap is exactly phi^(3/2) of the
	 * light gap because chroma scales with exposure. So exposure descends the measured
	 * light gap and saturation takes the REMAINDER of the colour gap as gain. Every
	 * number is computed from those four measurements; none is a hand-set preference.
	 */
	void ApplyPhiGrade();

	/** The district camp: machines holding the plaza. The Vault stays sealed
	 *  until the last of them falls. */
	int32 CampStanding = 0;

	/** -phiwin: proves the ending without a hand on the keyboard. */
	bool bPhiWinProof = false;

	UFUNCTION()
	void HandleCampMachineDown(AVEILFactionAgent* Agent);

	/** Proof harness (-phikill): the nearest machine is answered through the real
	 *  damage chain, so the kill and its pay are proven in a live frame. */
	void PhiKillStep();
	FTimerHandle PhiKillTimer;
	int32 PhiKillPhase = 0;

	/** 1.0 = authored values. Scales agent health + damage at registration. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Game")
	float DifficultyScale = 1.f;

	/** Starting world data - fill in the GameMode defaults or a BP subclass. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Game")
	TArray<FVEILMystery> MysteriesToRegister;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Game")
	TArray<FVEILTerritory> TerritoriesToRegister;

	/** Agent class handed to the Faction Manager for reinforcement waves. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Game")
	TSubclassOf<AVEILFactionAgent> ReinforcementAgentClass;

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleMysterySolved(FName MysteryId);
};

// ---------------------------------------------------------------------------
// Player Controller - HUD, pause menu, input-mode housekeeping.
// ---------------------------------------------------------------------------
UCLASS()
class VEIL_API AVEILPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AVEILPlayerController();
	virtual void Tick(float DeltaSeconds) override;

	/** Re-asserts who owns the mouse every frame. A cursor left switched on with
	 *  no page owning it is the bug the player calls "the mouse is not working". */
	virtual void PlayerTick(float DeltaTime) override;

	/** True while a page that actually wants a pointer is up (menu, pause, win). */
	bool UIWantsCursor() const;
	/** Fires only if the world's timer manager advances: the timer cross-check. */
	void PhiOnTimerCheck();

	/** Device of the last real press. Everything on screen that names a button
	 *  asks this, so a pad player is never told to press TAB. */
	virtual bool InputKey(const FInputKeyEventArgs& Params) override;

	UFUNCTION(BlueprintCallable, Category = "VEIL|UI")
	void ToggleChart();

	UFUNCTION(BlueprintCallable, Category = "VEIL|UI")
	void ToggleFieldManual();

	UFUNCTION(BlueprintCallable, Category = "VEIL|UI")
	void ToggleMuteNarrator();

	UFUNCTION(BlueprintCallable, Category = "VEIL|Party")
	void SwitchLead();

	UFUNCTION(BlueprintCallable, Category = "VEIL|UI")
	void ToggleTips();

	/** The field manual page: every action, both devices. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|UI")
	bool bShowFieldManual = false;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Input")
	bool bUsingGamepad = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|UI")
	TSubclassOf<UUserWidget> HUDWidgetClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|UI")
	TSubclassOf<UUserWidget> PauseWidgetClass;

	/** Main menu shown at boot. Defaults to the C++ menu (UVEILMainMenu),
	 *  so the game has a working menu with zero widget assets. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|UI")
	TSubclassOf<UUserWidget> MainMenuWidgetClass;

	UFUNCTION(BlueprintCallable, Category = "VEIL|UI")
	void TogglePauseMenu();

	/** The win screen is a page: it owns the pointer while it is up. */
	bool bWinScreenOpen = false;

	UFUNCTION(BlueprintCallable, Category = "VEIL|UI")
	void ShowWinScreen(float Gold, int32 Level, float TimeSeconds);

	UFUNCTION(BlueprintCallable, Category = "VEIL|UI")
	void CloseWinScreen();

	UFUNCTION(BlueprintCallable, Category = "VEIL|UI")
	void RespawnRunner();

	/** [E] Opens a conversation with whoever is nearest and willing to speak;
	 *  while one is open it advances instead, so one key reads a whole scene. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Story")
	void TalkToNearest();

	/** [SPACE] Advances the open conversation (completes the line, then next). */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Story")
	void AdvanceDialogue();

	/** The living map zooms on a phi ladder: wheel, or the pad's D-pad up/down. */
	void MapZoomIn();
	void MapZoomOut();

	/** Chart navigation: the beacons are listed, one is highlighted, one key
	 *  carries the runner to it. Free travel between every lit beacon. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Travel")
	void ChartMove(int32 Delta);

	UFUNCTION(BlueprintCallable, Category = "VEIL|Travel")
	void ChartMovePrev();

	UFUNCTION(BlueprintCallable, Category = "VEIL|Travel")
	void ChartMoveNext();

	UFUNCTION(BlueprintCallable, Category = "VEIL|Travel")
	void ChartTravel();

	/** [A]/[SPACE]: advance a line if one is open, else travel to the chart's
	 *  highlighted beacon. One button, the thing in front of the player. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Travel")
	void TravelOrAdvance();

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Travel")
	int32 ChartSel = 0;

	/** 0 = the chart, 1 = the living world. One button walks the pages. */
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|World")
	int32 ChartPage = 0;

	UFUNCTION(BlueprintCallable, Category = "VEIL|World")
	void NextChartPage();

	/** No class lock: every archetype is open, and the runner can stand in any
	 *  of them. Selecting one only changes what he is currently wearing. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|World")
	void SelectArchetype(int32 Index);

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|World")
	int32 ActiveArchetype = 0;

	UFUNCTION(BlueprintCallable, Category = "VEIL|World") void SelectArchetype0();
	UFUNCTION(BlueprintCallable, Category = "VEIL|World") void SelectArchetype1();
	UFUNCTION(BlueprintCallable, Category = "VEIL|World") void SelectArchetype2();
	UFUNCTION(BlueprintCallable, Category = "VEIL|World") void SelectArchetype3();
	UFUNCTION(BlueprintCallable, Category = "VEIL|World") void SelectArchetype4();
	UFUNCTION(BlueprintCallable, Category = "VEIL|World") void SelectArchetype5();
	UFUNCTION(BlueprintCallable, Category = "VEIL|World") void SelectArchetype6();
	UFUNCTION(BlueprintCallable, Category = "VEIL|World") void SelectArchetype7();
	UFUNCTION(BlueprintCallable, Category = "VEIL|World") void SelectArchetype8();
	UFUNCTION(BlueprintCallable, Category = "VEIL|World") void SelectArchetype9();

	// ---- PHI CHEATS: the GODMODE page ----
	// One key, drawn in the game's own canvas UI, every row a real switch. The
	// list itself lives on UPHICheats, so the menu, the console and any proof run
	// all drive the same state.
	// ---- A PAD CAN ANSWER, TOO ----
	// The reply list has always been 1 / 2 / 3 on the keyboard. A controller had
	// no way to speak, which makes every conversation a dead end on a pad.
	// A / X / Y pick replies 1 / 2 / 3 while a choice is waiting, and do their
	// own jobs (jump / interact / swing) the moment it is not.
	UFUNCTION(BlueprintCallable, Category = "VEIL|Dialogue") void ChooseReply(int32 Index);
	UFUNCTION(BlueprintCallable, Category = "VEIL|Dialogue") void PadReply1();
	UFUNCTION(BlueprintCallable, Category = "VEIL|Dialogue") void PadReply2();
	UFUNCTION(BlueprintCallable, Category = "VEIL|Dialogue") void PadReply3();

	UFUNCTION(BlueprintCallable, Category = "VEIL|Cheats") void ToggleGodMenu();

	/** [F1] The one OS: screen, voice, map and controls over the engine's own
	 *  rungs, addresses, skills, automations and ledger. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OS") void ToggleOS();
	UFUNCTION(BlueprintCallable, Category = "VEIL|Cheats") void GodMenuMove(int32 Delta);
	UFUNCTION(BlueprintCallable, Category = "VEIL|Cheats") void GodMenuUp();
	UFUNCTION(BlueprintCallable, Category = "VEIL|Cheats") void GodMenuDown();
	/** Two columns on the page: left and right jump a whole column at a time. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Cheats") void GodMenuLeft();
	UFUNCTION(BlueprintCallable, Category = "VEIL|Cheats") void GodMenuRight();
	UFUNCTION(BlueprintCallable, Category = "VEIL|Cheats") void GodMenuActivate();

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Cheats")
	bool bGodMenuOpen = false;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Cheats")
	int32 GodSel = 0;

	/** OPTIONS page: eight real settings, changed in place, saved on change. */
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Options")
	int32 OptionsSel = 0;

	UFUNCTION(BlueprintCallable, Category = "VEIL|Options") void OptionsMove(int32 Delta);
	UFUNCTION(BlueprintCallable, Category = "VEIL|Options") void OptionsAdjust(int32 Delta);
	UFUNCTION(BlueprintCallable, Category = "VEIL|Options") void OptionsActivate();
	UFUNCTION(BlueprintCallable, Category = "VEIL|Options") void OpenOptions();
	/** Arrows are named for the direction the page moves, not the value. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Options") void OptionsDown();
	UFUNCTION(BlueprintCallable, Category = "VEIL|Options") void OptionsUp();

	// PC call site VEILHudCanvas.cpp(102): PC->bShowChart
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|UI")
	bool bShowChart = false;

	UFUNCTION(BlueprintPure, Category = "VEIL|UI")
	bool IsPauseMenuOpen() const { return bPauseOpen; }

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

private:
	TObjectPtr<UUserWidget> HUDWidget;
	TObjectPtr<UUserWidget> PauseWidget;
	TObjectPtr<UUserWidget> MainMenuWidget;
	bool bPauseOpen = false;
};
