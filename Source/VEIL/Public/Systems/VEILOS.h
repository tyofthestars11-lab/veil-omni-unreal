// VEIL OMNI // ONE OS - the engine under the face.
//
// Not a copy of any menu in the project. A menu lists what the player already
// owns; this is one machine with named parts, and the phi is REVERSED into it
// rather than printed on it:
//
//   domains      rung fields        each live value, shown as log_phi(v)
//   tasks        addresses          every surface is callable by name
//   skills       round-tripped rungs a skill answers a rung, and the engine
//                                    proves the number came back unchanged
//   automations  on change          rules fire when a value MOVES, never per frame
//   memory       phi ledger         append-only, rung-stamped, capped
//   voice door   Brain / Eyes / Seat / Hands
//                                   plan -> review -> execute, with the seat
//                                   (where it happened) named before the hands move
//
// The Jarvis voice is the top layer only: one line, on the channel the world
// already speaks through. Nothing under it is rewritten.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VEILOS.generated.h"

/** DOMAINS: one live value, its address, and the rung it is read in. */
struct FVEILOSDomain
{
	FString Key;
	FString Address;
	double Value = 0.0;
	double Rung = 0.0;
};

/** TASKS: an address, the verb it carries, and its current state. */
struct FVEILOSTask
{
	FString Address;
	FString Verb;
	FString State;
};

/** SKILLS: a name, the address it is reached on, and the rung it returns. */
struct FVEILOSSkill
{
	FString Name;
	FString Address;
	double Rung = 0.0;
	bool bRoundTripped = false;
};

/** MEMORY: one ledger line. Rung-stamped, address-tagged, append-only. */
struct FVEILOSLedgerEntry
{
	double T = 0.0;
	double Rung = 0.0;
	FString Address;
	FString Note;
};

/** The plan the voice door walks: Brain drafts, Eyes reviews, Seat names the
 *  place, Hands executes. Nothing runs before the review has passed. */
struct FVEILOSPlan
{
	FString Intent;
	TArray<FString> Steps;
	bool bReviewed = false;
	FString ReviewNote;
	FString Result;
	FString Seat;
	double T = 0.0;
};

UCLASS()
class VEIL_API UVEILOS : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual TStatId GetStatId() const override;
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	// ---- the face ---------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "VEIL|OS") void Toggle();
	UFUNCTION(BlueprintCallable, Category = "VEIL|OS") void Open();
	UFUNCTION(BlueprintCallable, Category = "VEIL|OS") void Close();
	UFUNCTION(BlueprintPure, Category = "VEIL|OS") bool IsOpen() const { return bOpen; }

	// ---- the engine -------------------------------------------------------
	/** Address in, answer out. One door, so the face, the console and the proof
	 *  harness cannot disagree about what the engine just did. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OS")
	FString Dispatch(const FString& Address);

	/** BRAIN: draft the plan from the domains, one step per real surface. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OS") void Plan(const FString& Intent);

	/** EYES: every planned step must resolve to an address that answers. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OS") bool Review();

	/** SEAT then HANDS: name where it happens, then do it. Refuses unreviewed. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OS") FString Execute();

	/** The two verbs the M1 proof exercises. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OS") void AskStatus();
	UFUNCTION(BlueprintCallable, Category = "VEIL|OS") void AskSave();

	const TArray<FVEILOSDomain>& Domains() const { return DomainFields; }
	const TArray<FVEILOSTask>& Tasks() const { return TaskList; }
	const TArray<FVEILOSSkill>& Skills() const { return SkillList; }
	const TArray<FVEILOSLedgerEntry>& Ledger() const { return LedgerMemory; }
	const FVEILOSPlan& CurrentPlan() const { return PlanState; }
	const FString& Voice() const { return VoiceText; }
	const FString& LastAddress() const { return LastAnswerAddress; }
	const FString& LastAnswer() const { return LastAnswerText; }

	FString GateLine() const;
	FString DomainLine() const;
	FString MapLine() const;
	FString ControlsLine() const;
	FString ObservabilityLine() const;
	FString SkillLine() const;
	FString AutomationLine() const;
	FString M1Line() const;

	/** rung(v) = log_phi(v), the reversal the face is built on. */
	static double Rung(double Value);

	/** Every address this engine answers on: tasks first, then skills. */
	static const TArray<FString>& TaskAddresses();
	static const TArray<FString>& SkillAddresses();

private:
	void RefreshDomains();
	void RunAutomations();
	void RoundTripSkills();
	void AppendLedger(const FString& Address, const FString& Note);
	void Say(const FString& Line);
	void WriteLedgerFile();
	void MarkTask(const FString& Address);

	bool bOpen = false;
	double LastSample = 0.0;
	double PrevGold = -1.0;
	double PrevHealth = -1.0;
	int32 PrevBeacons = -1;
	int32 PrevLevel = -1;
	int32 LastAutomationRung = -1;

	FString LastAnswerAddress;
	FString LastAnswerText;
	FString VoiceText;
	FString LastAutomation;

	TArray<FVEILOSDomain> DomainFields;
	TArray<FVEILOSTask> TaskList;
	TArray<FVEILOSSkill> SkillList;
	TArray<FVEILOSLedgerEntry> LedgerMemory;
	FVEILOSPlan PlanState;

	int32 Dispatches = 0;
	int32 Answers = 0;
	int32 Automations = 0;
	int32 Saves = 0;
	int32 SkillsRoundTripped = 0;
	int32 SkillsTotal = 0;
	int32 ProofShots = 0;
};
