#pragma once
#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "VeilPlayerController.generated.h"

class AVeilRunner;
class UVeilSaveGame;

/**
 * The Runner's hands. Dialogue is E only, never Space and never Enter.
 * Quest of the Bible: walk to the Keeper and let her count you, then the three
 * beacons 0/3 -> 3/3. Every beacon lit is written straight into VEILOMNI_Omni0.
 */
UCLASS()
class VEILOMNI_API AVeilPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	AVeilPlayerController();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupInputComponent() override;

	/** E. Speak, continue, or be counted. This is the only dialogue key. */
	void OnInteract();
	void OnSprintStart();
	void OnSprintStop();
	void OnCombatMode();
	void OnGlideStart();
	void OnGlideStop();

	// --- what the gold ribbon reads ---
	UPROPERTY(BlueprintReadOnly) FString DialogueSpeaker;
	UPROPERTY(BlueprintReadOnly) FString DialogueLine;
	UPROPERTY(BlueprintReadOnly) int32 DialogueStep = 0;
	UPROPERTY(BlueprintReadOnly) bool bDialogueOpen = false;
	UPROPERTY(BlueprintReadOnly) bool bQuestAccepted = false;
	UPROPERTY(BlueprintReadOnly) bool bKeeperCounted = false;
	UPROPERTY(BlueprintReadOnly) int32 BeaconsLit = 0;
	UPROPERTY(BlueprintReadOnly) int32 BeaconsTotal = 3;
	UPROPERTY(BlueprintReadOnly) bool bCombat = false;
	UPROPERTY(BlueprintReadOnly) int32 Level = 1;
	UPROPERTY(BlueprintReadOnly) float VIT = 144.f;
	UPROPERTY(BlueprintReadOnly) float VITMax = 144.f;
	UPROPERTY(BlueprintReadOnly) float STM = 89.f;
	UPROPERTY(BlueprintReadOnly) float STMMax = 89.f;
	UPROPERTY(BlueprintReadOnly) bool bNearKeeper = false;

	/** Bible coordinates, never moved. */
	static const FVector KeeperAt;
	static const FVector SpireAt;
	static const FVector DeepAt;
	static FVector BeaconAt(int32 Index);

	void SaveSignal();
	void LoadSignal();

	AVeilRunner* Runner() const;

private:
	UPROPERTY() UVeilSaveGame* Save = nullptr;
	bool bBeaconLit[3] = { false, false, false };
	void TickQuest();
	void CheckBeacons();
	void Speak(const TCHAR* Who, const TCHAR* Line);
};
