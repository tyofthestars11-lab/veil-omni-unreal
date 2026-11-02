#include "Systems/PHI_NPCProcessor.h"
#include "Math/UnrealMathUtility.h"
#include "Systems/PHIMito.h"
#include "Systems/PHIPhinary.h"
#include "VEIL.h"

UPHI_NPCProcessor::UPHI_NPCProcessor()
{
    // A brain that cannot think is decoration. This one holds a live clock.
    PrimaryComponentTick.bCanEverTick = true;
}

// ---------------------------------------------------------------------------
// Identity
// ---------------------------------------------------------------------------
void UPHI_NPCProcessor::Seed(FName InName, int32 Ordinal)
{
    SeedOrdinal = Ordinal;

    // The name's own hash, normalised into [0,1), then lifted into rung space
    // through phi: two names are never the same creature, none is hand-picked.
    const float H = static_cast<float>(GetTypeHash(InName) % 10007u) / 10007.f;
    PersonalityRung = UPHIMito::Rung(1.f + (H + static_cast<float>(Ordinal % 64)) * (UPHIMito::Phi - 1.f));
}

// ---------------------------------------------------------------------------
// The clock
// ---------------------------------------------------------------------------
void UPHI_NPCProcessor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    PhiClock.Advance(DeltaTime);
    PhiPhase = PhiClock.Phase;

    // phi-1 is irrational, so this section of the clock is dense and never
    // repeats - variety that costs nothing and can never run out.
    Band = FMath::Frac((PhiPhase + PersonalityRung) * (UPHIMito::Phi - 1.f));

    // Moods are small rung offsets around neutral (Rung(1) = 0). They breathe on
    // their own phases, so aggression, curiosity and patience never move as one -
    // and they breathe from THIS creature's origin, not from the clock's shared
    // start. Without the origin every brain in the district printed identical
    // moods: six bodies, one temperament, only the address differing.
    const float Off2 = UPHIMito::PhiPow(-1.f);
    const float Off3 = UPHIMito::PhiPow(-2.f);
    const float OwnOrigin = PersonalityRung * UPHIMito::PhiPow(-1.f);
    AggressionRung = FPhiDynamic::Rung(1.f, PhiClock.Phase, OwnOrigin, 0.5f);
    CuriosityRung  = FPhiDynamic::Rung(1.f, PhiClock.Phase, OwnOrigin + Off2, 0.75f);
    PatienceRung   = FPhiDynamic::Rung(1.f, PhiClock.Phase, OwnOrigin + Off3, 0.5f);

    // A living reading, not a constant.
    LiveProcessRung = FPhiDynamic::Rung(1.f, PhiClock.Phase, OwnOrigin, 4.f);

    LogTimer += DeltaTime;
    if (LogTimer >= 5.f)
    {
        LogTimer = 0.f;
        UE_LOG(LogVEIL, Log, TEXT("PHI NPC BRAIN %s"), *StatusLine());
    }
}

// ---------------------------------------------------------------------------
// Decisions - no thresholds, only rung addresses
// ---------------------------------------------------------------------------
float UPHI_NPCProcessor::AnchorRung(EPhiIntent Intent)
{
    // Anchors are phi powers read as rungs: -2, -1, 0, 1, 2, 3. Computed, not typed.
    switch (Intent)
    {
    case EPhiIntent::Rest:        return UPHIMito::Rung(UPHIMito::PhiPow(-2.f));
    case EPhiIntent::Patrol:      return UPHIMito::Rung(UPHIMito::PhiPow(-1.f));
    case EPhiIntent::Investigate: return UPHIMito::Rung(UPHIMito::PhiPow(0.f));
    case EPhiIntent::Hunt:        return UPHIMito::Rung(UPHIMito::PhiPow(1.f));
    case EPhiIntent::Fight:       return UPHIMito::Rung(UPHIMito::PhiPow(2.f));
    case EPhiIntent::Flee:        return UPHIMito::Rung(UPHIMito::PhiPow(3.f));
    }
    return 0.f;
}

float UPHI_NPCProcessor::Pull(EPhiIntent Intent, float Threat01, float Health01) const
{
    // Situation as a rung address: threat lifts it, health lowers it. A hurt
    // creature sits further up the address line, closer to Flee.
    const float ThreatRung = UPHIMito::Rung(1.f + 2.f * FMath::Clamp(Threat01, 0.f, 1.f));
    const float HealthRung = UPHIMito::Rung(1.f + 2.f * FMath::Clamp(Health01, 0.f, 1.f));

    // Attention drifts along the address line on the creature's own band, +/-
    // one phi of a rung. Without this the same situation would always resolve to
    // the same answer - a brain that never changes its mind is a lookup table.
    const float Drift = (Band - 0.5f) * UPHIMito::PhiPow(1.f);
    const float SituationRung = ThreatRung - HealthRung * UPHIMito::PhiPow(-1.f) + Drift;

    // The mood shifts the anchor, not the situation: an aggressive creature's
    // Fight anchor sits lower, so less needs to happen before it fights.
    float Shift = 0.f;
    switch (Intent)
    {
    case EPhiIntent::Fight:       Shift = -AggressionRung - CuriosityRung * UPHIMito::PhiPow(-2.f); break;
    case EPhiIntent::Hunt:        Shift = -AggressionRung * UPHIMito::PhiPow(-1.f); break;
    case EPhiIntent::Investigate: Shift = -CuriosityRung * UPHIMito::PhiPow(-1.f); break;
    case EPhiIntent::Rest:
    case EPhiIntent::Patrol:      Shift = -PatienceRung * UPHIMito::PhiPow(-1.f); break;
    case EPhiIntent::Flee:        Shift = 0.f; break;
    }

    // Utility is negative rung distance: nearest address wins.
    float Score = -FMath::Abs(SituationRung - (AnchorRung(Intent) + Shift));

    // Endless variety without randomness: an irrational slice of its own band.
    const float Variety = FMath::Frac((Band + PersonalityRung) * (1.f + static_cast<float>(static_cast<uint8>(Intent))));
    Score -= Variety * UPHIMito::PhiPow(-1.f);

    return Score;
}

float UPHI_NPCProcessor::Utility(EPhiIntent Intent, float Threat01, float Health01) const
{
    return Pull(Intent, Threat01, Health01);
}

EPhiIntent UPHI_NPCProcessor::ChooseIntent(float Threat01, float Health01, bool bHasTarget)
{
    static const EPhiIntent All[] = {
        EPhiIntent::Rest, EPhiIntent::Patrol, EPhiIntent::Investigate,
        EPhiIntent::Hunt, EPhiIntent::Fight, EPhiIntent::Flee
    };

    EPhiIntent Best = EPhiIntent::Patrol;
    float BestScore = -FLT_MAX;
    for (EPhiIntent Intent : All)
    {
        // Without a target there is nothing to hunt or flee: those addresses are
        // out of reach, so the brain falls back to its own ground.
        if (!bHasTarget && (Intent == EPhiIntent::Hunt || Intent == EPhiIntent::Fight || Intent == EPhiIntent::Flee))
        {
            continue;
        }
        const float Score = Pull(Intent, Threat01, Health01);
        if (Score > BestScore)
        {
            BestScore = Score;
            Best = Intent;
        }
    }

    // The decision is not a flag, so it is not written as one. Its utility is a
    // rung, and phi^rung is the weight it carries in plain space - which is then
    // written in base phi, where every place holds its own share of the value.
    const float Weight = FMath::Pow((float)VEILPhinary::Phi, FMath::Clamp(BestScore, -12.f, 0.f));
    PhinaryState = VEILPhinary::Encode(Weight, 16);
    CurrentIntent = Best;

    return Best;
}

FString UPHI_NPCProcessor::IntentName(EPhiIntent Intent)
{
    switch (Intent)
    {
    case EPhiIntent::Rest:        return TEXT("REST");
    case EPhiIntent::Patrol:      return TEXT("PATROL");
    case EPhiIntent::Investigate: return TEXT("INVESTIGATE");
    case EPhiIntent::Hunt:        return TEXT("HUNT");
    case EPhiIntent::Fight:       return TEXT("FIGHT");
    case EPhiIntent::Flee:        return TEXT("FLEE");
    }
    return TEXT("PATROL");
}

FString UPHI_NPCProcessor::PhinaryOf(float Value) const
{
    return VEILPhinary::Encode(Value);
}

FString UPHI_NPCProcessor::StatusLine() const
{
    return FString::Printf(
        TEXT("intent %s = %s phi   rung %.4f band %.4f mood(agg %.4f cur %.4f pat %.4f) phase %.4f"),
        *IntentName(CurrentIntent),
        PhinaryState.IsEmpty() ? TEXT("-") : *PhinaryState,
        PersonalityRung, Band, AggressionRung, CuriosityRung, PatienceRung, PhiPhase);
}

// ---------------------------------------------------------------------------
// Rung tools and memory
// ---------------------------------------------------------------------------
float UPHI_NPCProcessor::RungOf(float Value) const
{
    return UPHIMito::Rung(Value);
}

float UPHI_NPCProcessor::GoldenAngle() const
{
    return UPHIMito::GoldenAngleDeg;
}

float UPHI_NPCProcessor::GoldenPitch() const
{
    return 17.0324f;
}

float UPHI_NPCProcessor::PhiProcess(float Input) const
{
    if (Input <= 0.0f)
    {
        return 0.0f;
    }
    return FPhiDynamic::Rung(Input, PhiClock.Phase, 0.f, 0.5f);
}

void UPHI_NPCProcessor::RecordDeed(const FString& Deed, float Value)
{
    if (Deed.IsEmpty())
    {
        return;
    }
    DeedRungs.Add(Deed, RungOf(Value));
}

float UPHI_NPCProcessor::RecallDeed(const FString& Deed) const
{
    const float* Found = DeedRungs.Find(Deed);
    if (Found == nullptr)
    {
        return -1.0f;
    }
    return *Found;
}

bool UPHI_NPCProcessor::HasDeed(const FString& Deed) const
{
    return DeedRungs.Contains(Deed);
}
