// VEIL OMNI // ONE OS - the engine under the face.
//
// This is NOT a copy of any existing menu. Every other page in the game is a
// list of things the player already had. This is one interface with two halves:
//
//   FACE    a screen (TV / PC / game), a voice, a map, the controls
//   ENGINE  rung fields, addresses, round-tripped skills, on-change
//           automations, a phi ledger memory, and Play-proof observability
//
// The phi is reversed into the interface rather than printed on it: every
// number the face shows is a rung (log base phi) of a real engine value, and
// every surface has an ADDRESS that any caller - the face, the console, the
// proof harness - can dispatch to and get the same answer back.
//
// The voice is a thin layer on top: one line, the same subtitle channel the
// world already speaks through. Nothing below it is rewritten.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VEILOS.generated.h"

/** One line of the ledger. Rung-stamped, address-tagged, append-only. */
struct FVEILOSLedgerEntry
{
	double T = 0.0;
	double Rung = 0.0;
	FString Address;
	FString Note;
};

/** One engine value, its address, and the rung the face reads it in. */
struct FVEILOSRungField
{
	FString Key;
	FString Address;
	double Value = 0.0;
	double Rung = 0.0;
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

	/** The two verbs the proof exercises: answer, and do the thing. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OS") void AskStatus();
	UFUNCTION(BlueprintCallable, Category = "VEIL|OS") void AskSave();

	const FString& LastAddress() const { return LastAnswerAddress; }
	const FString& LastAnswer() const { return LastAnswerText; }
	const TArray<FVEILOSRungField>& Fields() const { return RungFields; }
	const TArray<FVEILOSLedgerEntry>& Ledger() const { return LedgerMemory; }
	const FString& Voice() const { return VoiceText; }

	FString ObservabilityLine() const;
	FString EngineLine() const;
	FString MapLine() const;
	FString ControlsLine() const;
	FString RungTableLine() const;

	/** rung(v) = log_phi(v), the reversal the face is built on. */
	static double Rung(double Value);

	/** Every address this engine answers on. */
	static const TArray<FString>& AddressBook();

private:
	void RefreshFields();
	void RunAutomations();
	void AppendLedger(const FString& Address, const FString& Note);
	void Say(const FString& Line);
	void RoundTripSkills();
	void WriteLedgerFile();
	void ProofStep(int32 Step);

	bool bOpen = false;
	double LastSample = 0.0;
	double PrevGold = -1.0;
	double PrevHealth = -1.0;
	int32 PrevBeacons = -1;
	int32 PrevLevel = -1;

	FString LastAnswerAddress;
	FString LastAnswerText;
	FString VoiceText;
	TArray<FVEILOSRungField> RungFields;
	TArray<FVEILOSLedgerEntry> LedgerMemory;

	int32 Dispatches = 0;
	int32 Answers = 0;
	int32 Automations = 0;
	int32 Saves = 0;
	int32 SkillsRoundTripped = 0;
	int32 SkillsTotal = 0;
	int32 ProofShots = 0;
};
