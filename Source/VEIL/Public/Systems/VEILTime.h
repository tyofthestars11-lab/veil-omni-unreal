// THE FIBONACCI CLOCK, AND THE TEMPORAL FIELD IT SITS IN.
//
// Real time is the clock. One tick is one second of the player's actual life,
// and the beats of the day are the Fibonacci numbers themselves:
//
//   beat  1  2  3  4  5  6   7   8   9  10
//   secs  1  1  2  3  5  8  13  21  34  55      sum = 143 s, then the day turns
//
// Nobody ages: the cycle closes and the frame holds, which is what "+1" means
// here - time adds, it does not wear anything down.
//
// ---------------------------------------------------------------------------
// QUANTUM TEMPORAL COHERENCE
// ---------------------------------------------------------------------------
// The linear clock is the MEASURED timeline, not the only one. At a beat
// boundary the next beat is not committed by the clock - it enters a
// superposition of 2 to 4 candidate beats, weighted by phi. While the day is
// superposed it is held: time does not advance, it waits to be observed. The
// player collapses it by doing something (speak, travel, strike, or simply
// standing still long enough), and the collapse commits ONE beat - which can
// skip beats forward, so the clock is not monotonic.
//
// Coherence is how well the present is locked:
//   still   -> coherence rises (observation settles the field)
//   moving  -> coherence falls (interaction decoheres it)
// High coherence narrows the superposition to two beats; low coherence opens it
// to four. A field nobody observes for 8 seconds collapses on its own, to the
// most probable beat, because an unobserved field still resolves.
//
// ---------------------------------------------------------------------------
// OPERATING OUTSIDE LINEAR TIME
// ---------------------------------------------------------------------------
// A collapse can land on a beat the world has not reached. The runner's present
// is then truly ahead of the measured world, and until the world catches up he
// is outside linear time: the world ticks at 1/phi^drift while he ticks at 1,
// acts he takes in that window never cross the clock and refund coherence, and
// the gap closes one beat per phi^2 seconds. It is not a status effect overlaid
// on the game - it is the same clock, read from a beat the world cannot see yet.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VEILTime.generated.h"

/** One possible beat inside a superposition, with its phi weighting. */
struct FVEILBeatCandidate
{
	/** Zero-based beat index. */
	int32 Beat = 0;
	/** Normalised weight - 1, 1/phi, 1/phi^2 ... rescaled to sum to 1. */
	float Amplitude = 1.f;
};

UCLASS()
class VEIL_API UVEILTime : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** 1..10, the beat the world is standing in (the first candidate while superposed). */
	int32 GetBeat() const { return BeatIndex + 1; }
	int32 BeatCount() const { return 10; }
	/** Seconds of real time left in this beat (frozen while superposed). */
	float BeatRemaining() const { return BeatLeft; }
	/** How long this beat is, in real seconds - a Fibonacci number. */
	int32 BeatLength() const;
	/** 0..1 through the whole 143-second day. */
	float DayProgress() const;
	/** How many days have turned since the world started. */
	int32 GetDay() const { return Day; }

	FString BeatName() const;
	static FString BeatNameOf(int32 ZeroBased);
	/** "DAY 2  BEAT 6/10  THE HIGH SUN  3s left" - or, superposed, the field. */
	FString ClockLine() const;
	/** True during the beats when most people are at their post. */
	bool IsDutyBeat() const;
	/** 0..1 through this beat. */
	float BeatProgress() const;

	static const TArray<int32>& BeatSeconds();

	// ---- the temporal field ----

	/** True when the next beat is undecided and time is held. */
	bool IsSuperposed() const { return bSuperposed; }
	const TArray<FVEILBeatCandidate>& Candidates() const { return Field; }
	/** 0..1: how locked the present is. Stillness raises it, action lowers it. */
	float GetCoherence() const { return Coherence; }
	/** Beats this collapse skipped forward (0 when it simply advanced). */
	int32 LastSkip() const { return LastCollapsedSkip; }
	/** How many beats were waiting when the last field collapsed. */
	int32 LastFieldSize() const { return LastFieldCount; }

	/**
	 * Collapse the superposition by observing it. Reason selects which candidate
	 * is committed: "still" keeps the nearest, "strike" takes the second,
	 * "travel" takes the farthest - so movement through the world moves through
	 * time as well. Does nothing when the field is already resolved.
	 */
	void Observe(const TCHAR* Reason);

	/** Proof hook: hold the field open until something observes it. */
	void SetAwaitingObserver(bool b) { bAwaitingObserver = b; }
	/** Proof hook: open a field now, if none is open. */
	void ForceField();

	// ---- operating outside linear time ----

	// A collapse can land on a beat that has not been reached yet: the runner's
	// committed present is then AHEAD of the world's measured present. That gap
	// is the timeless window. While it is open the world runs at 1/phi^drift
	// (the runner is not dilated, so he moves through a slower world), the beats
	// he is ahead of tick away one at a time, and every act he takes inside the
	// window is an act that never had to travel through time - so it refunds
	// coherence instead of spending it.

	/** True while the runner's committed beat is ahead of the world's. */
	bool IsOutsideTime() const { return PhaseDrift > 0; }
	/** Beats the runner stands ahead of the measured world. */
	int32 GetPhaseDrift() const { return PhaseDrift; }
	/** How fast the world is running right now: 1/phi^drift, exactly 1 at rest. */
	float WorldTimeScale() const;
	/** The same gap as a rung: ln(scale)/ln(phi), which is -drift by construction. */
	float PhaseRung() const;
	/** Acts taken inside the window; each one refunds coherence. */
	int32 TimelessActs() const { return TimelessCount; }
	/** "OUTSIDE LINEAR TIME   +2 BEATS   WORLD 0.382x   RUNG -2.0000   ACTS 3". */
	FString PhaseLine() const;

	/** The world's measured chaos (0..1), set by UPHIIntent each cycle. A tense
	 *  world genuinely has more possible futures, so this widens the field. */
	void SetChaosPressure(float P) { ChaosPressure = FMath::Clamp(P, 0.f, 1.f); }
	float GetChaosPressure() const { return ChaosPressure; }

	/** Proof hook: keep the window open so a still frame can show it. */
	void SetHoldPhase(bool b);
	/** Proof hook: put the runner this many beats ahead of the world now. */
	void ForcePhase(int32 Beats);

private:
	void EnterSuperposition();
	void CollapseTo(int32 CandidateIndex, const TCHAR* Reason);
	void RecomputeDutyFlag();
	void EnterPhase(int32 Beats, const TCHAR* Cause);
	void ApplyPhaseToWorld();
	void LeavePhase(const TCHAR* Cause);
	void NoteTimelessAct(const TCHAR* Reason);

	int32 BeatIndex = 0;
	float BeatLeft = 1.f;
	float Elapsed = 0.f;
	int32 Day = 1;

	// The temporal field.
	TArray<FVEILBeatCandidate> Field;
	bool bSuperposed = false;
	float Coherence = 0.618f;       // starts at 1/phi: neither locked nor scattered
	/** The coherence the field was opened at - the width follows this, not the live value. */
	float FieldCoherence = 0.618f;
	float HoldSeconds = 0.f;        // how long this field has waited to be observed
	float SampleSpeed = 0.f;        // smoothed player speed, for coherence
	int32 LastCollapsedSkip = 0;
	int32 LastFieldCount = 0;
	/** Set only by the proof switch, never by play. */
	bool bAwaitingObserver = false;

	// The timeless window: beats the runner is ahead of the world.
	int32 PhaseDrift = 0;
	/** Real seconds left before the world closes one beat of the gap. */
	float PhaseLeft = 0.f;
	int32 TimelessCount = 0;
	/** Proof hold: the window does not close on its own. */
	bool bHoldPhase = false;
	/** Seconds until the dilation is re-stamped onto newly spawned pawns. */
	float PhaseRefresh = 0.f;
	/** The world's intent, folded from measured chaos by UPHIIntent. */
	float ChaosPressure = 0.618f;
};
