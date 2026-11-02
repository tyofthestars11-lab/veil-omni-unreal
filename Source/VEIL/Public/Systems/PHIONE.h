#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Systems/VEILLoopSubsystem.h"
#include "PHIONE.generated.h"

/**
 * ONE - the in-game voice, inside VEILDistrict. Not a separate app, not a chatbot
 * bolted on: it has HANDS.
 *
 * The structure is Tyree's, not Claude's: domains -> tasks -> skills ->
 * automations, each one a rung.
 *
 *   DOMAIN      a rung field: game / ledger / pc / voice.
 *   TASK        one address in that field ("fly to the keeper").
 *   SKILL       a repeatable rung, round-tripped: the intent goes in, the world
 *               changes, the value comes back out - nothing typed by hand.
 *   AUTOMATION  a skill that fires ON CHANGE, not on a clock. It watches a value
 *               and speaks the moment the value moves, like the Source Hive hook.
 *
 * Voice rules: short, plain, one breath. No lists read out loud. It says what it
 * did and names the hand it used. Listening / thinking / speaking is a state, not
 * an animation of one.
 *
 * Memory is the ledger: every task is written with the rung it was measured at,
 * so what ONE did is a record, not a claim. Nothing here deletes anything, and
 * nothing touches D: outside Saved\ONE.
 */
UENUM(BlueprintType)
enum class EONEState : uint8
{
	Listening,
	Thinking,
	Speaking
};

/** One skill: an address, the rung it weighs, and the hand that performs it. */
struct FONESkill
{
	FName Id;
	FName Domain;
	/** 1.0 = rung 0. The weight is the skill's own value, never a priority hack. */
	float Value = 1.f;
	/** Named in the reply, e.g. "fly", "ledger", "spiral". */
	FString Hand;
	/** What the player can say to reach it. First entry is the canonical one. */
	TArray<FString> Says;
};

UCLASS()
class VEIL_API UPHIONE : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { return GetStatID(); }

	/** Speak an intent. ONE answers in one breath. True if a skill fired. */
	bool Say(const FString& Utterance);

	/** The skill table, as data - the HUD and the ledger read the same list. */
	const TArray<FONESkill>& Skills() const { return SkillTable; }

	EONEState GetState() const { return State; }
	FString GetReply() const { return Reply; }
	FString GetHand() const { return HandUsed; }
	/** One line for the HUD: state, reply, and the hand that did it. */
	FString StateLine() const;

	/** The ledger as text (also written to Saved/ONE/ledger.txt). */
	const TArray<FString>& Ledger() const { return LedgerLines; }

	/** rung = ln(value)/ln(phi), the same measure everything else here uses. */
	static float RungOf(float Value);

private:
	bool RunSkill(const FString& Utterance, FString& OutReply, FString& OutHand);
	void WriteLedger(const FString& Utterance, const FString& Reply, const FString& Hand, float Rung);
	void Speak(const FString& InReply, const FString& InHand);

	/** Hands. */
	void FlyToKeeper();
	void Land();
	FString ReportWhere();
	void SpawnSpiral();
	void ActivateBeacon();
	void SetGodMode(bool bOn);

	// Plain C++ structs, so no UPROPERTY here - the reflection layer does not need
	// the skill table, and a mirrored USTRUCT would be a second source of truth.
	TArray<FONESkill> SkillTable;
	TArray<FString> LedgerLines;

	EONEState State = EONEState::Listening;
	FString Reply;
	FString HandUsed;
	/** Seconds of "speaking" left, so the orb has something honest to show. */
	float SpeakTime = 0.f;

	/** The flight ONE is currently driving, if any. */
	bool bCarrying = false;
	FVector CarryTarget = FVector::ZeroVector;
	FString CarryName;

	/** Automation: the last stage the world was in. Fires on change, not on tick. */
	EVEILStage SeenStage = EVEILStage::FindSignal;
	bool bStageSeen = false;
};
