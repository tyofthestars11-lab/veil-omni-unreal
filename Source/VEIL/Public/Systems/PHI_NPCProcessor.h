#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Systems/PHIMito.h"
#include "PHI_NPCProcessor.generated.h"

/** What a phi-brain decided to do. Six shapes, the same law for every NPC. */
UENUM(BlueprintType)
enum class EPhiIntent : uint8
{
    Rest,        // nothing pulls it
    Patrol,      // its own ground
    Investigate, // something moved
    Hunt,        // it knows where you are
    Fight,       // it is close enough to hurt you
    Flee         // it cannot win
};

// ---------------------------------------------------------------------------
// UPHI_NPCProcessor - the phi brain.
//
// No cloud, no model, no bill: every decision is arithmetic on the same rung
// space as the ledger, computed on the same clock as the machines. Free by
// construction, and endless by construction - the clock's band is an irrational
// section of an accumulating phase, so a brain never repeats a sequence, and a
// different seed is a different creature from the same law.
//
//   identity   rung of its seed          (who this NPC is)
//   mood       rungs that breathe        (how it feels right now)
//   situation  a rung address            (threat lifts it, health lowers it)
//   decision   nearest rung anchor       (intent = argmax utility, no thresholds)
// ---------------------------------------------------------------------------
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VEIL_API UPHI_NPCProcessor : public UActorComponent
{
    GENERATED_BODY()
public:
    UPHI_NPCProcessor();

    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** Give this NPC its own rung address. Same seed twice = the same creature. */
    UFUNCTION(BlueprintCallable, Category="Phi")
    void Seed(FName InName, int32 Ordinal = 0);

    // ---------------- what it is ----------------
    UPROPERTY(BlueprintReadOnly, Category="Phi") float PersonalityRung = 0.f;

    // ---------------- the clock ----------------
    UPROPERTY(BlueprintReadOnly, Category="Phi") float PhiPhase = 0.f;
    /** 0..1 golden section of the clock: dense, irrational, never repeating. */
    UPROPERTY(BlueprintReadOnly, Category="Phi") float Band = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="Phi") float LiveProcessRung = 0.f;

    // ---------------- how it feels ----------------
    UPROPERTY(BlueprintReadOnly, Category="Phi") float AggressionRung = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="Phi") float CuriosityRung = 0.f;
    UPROPERTY(BlueprintReadOnly, Category="Phi") float PatienceRung = 0.f;

    // ---------------- what it decides ----------------
    /** The decision itself. No thresholds anywhere: utilities at rung addresses. */
    UFUNCTION(BlueprintCallable, Category="Phi")
    EPhiIntent ChooseIntent(float Threat01, float Health01, bool bHasTarget);

    UFUNCTION(BlueprintPure, Category="Phi")
    float Utility(EPhiIntent Intent, float Threat01, float Health01) const;

    /** One line for the log: who it is, where its clock is, what it wants. */
    UFUNCTION(BlueprintPure, Category="Phi")
    FString StatusLine() const;

    /** The decision as a base-phi numeral. Not a flag: every place carries weight. */
    UPROPERTY(BlueprintReadOnly, Category="Phi")
    FString PhinaryState;

    /** What the last ChooseIntent landed on. */
    UPROPERTY(BlueprintReadOnly, Category="Phi")
    EPhiIntent CurrentIntent = EPhiIntent::Rest;

    UFUNCTION(BlueprintPure, Category="Phi")
    EPhiIntent GetIntent() const { return CurrentIntent; }

    /** The intent's name, for logs and the HUD. */
    static FString IntentName(EPhiIntent Intent);

    /** Any number this brain holds, written in base phi. */
    UFUNCTION(BlueprintPure, Category="Phi")
    FString PhinaryOf(float Value) const;

    // ---------------- the rung tools (one law, shared) ----------------
    UFUNCTION(BlueprintCallable, Category="Phi")
    float RungOf(float Value) const;
    UFUNCTION(BlueprintCallable, Category="Phi")
    float GoldenAngle() const;
    UFUNCTION(BlueprintCallable, Category="Phi")
    float GoldenPitch() const;
    UFUNCTION(BlueprintCallable, Category="Phi")
    float PhiProcess(float Input) const;

    /** Memory is a rung too: a deed is remembered as where it sat, not as a flag. */
    UFUNCTION(BlueprintCallable, Category="Phi")
    void RecordDeed(const FString& Deed, float Value);
    UFUNCTION(BlueprintCallable, Category="Phi")
    float RecallDeed(const FString& Deed) const;
    UFUNCTION(BlueprintCallable, Category="Phi")
    bool HasDeed(const FString& Deed) const;

private:
    /** Utility core: negative rung distance to the intent's anchor, mood-shifted. */
    float Pull(EPhiIntent Intent, float Threat01, float Health01) const;

    /** Anchor rung of an intent, in the shared rung space. */
    static float AnchorRung(EPhiIntent Intent);

    FPhiDynamic PhiClock;
    int32 SeedOrdinal = 0;
    float LogTimer = 0.f;

    UPROPERTY()
    TMap<FString, float> DeedRungs;
};
