// THE KRAKEN: a frame-time governor that holds the frame, using phi as its
// thresholds and TON618 as the reason it never thrashes.
//
// It measures real frame time every tick and moves ONE knob - the resolution
// scale, on a phi grid (100 / 82 / 67 / 55 / 45 %) - because resolution is the
// cheapest lever with the most effect. The frame is "too heavy" when the average
// exceeds the 60 fps budget times phi (16.67 ms * 1.618 = 26.97 ms, i.e. below
// 37 fps); it is "too light" when it falls under the budget divided by phi
// (10.30 ms, above 97 fps). Between those two the governor does nothing at all,
// which is the whole point: a dead band phi wide is what stops a system from
// oscillating between two settings every second.
//
// TON618 law, applied honestly: phi^55 is a number nobody can measure, so it is
// not used as a speed claim. What is used is the shape of the law - a fixed
// ratio between "step down" and "step up", with a Fibonacci cooldown (13 s)
// between any two moves, so the frame settles instead of hunting.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "VEILPerf.generated.h"

UCLASS()
class VEIL_API UVEILPerf : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// ---- the knobs the options page moves ----
	void SetAdaptive(bool bOn);
	bool IsAdaptive() const { return bAdaptive; }

	/** 0 Low, 1 Medium, 2 High, 3 Epic, -1 Custom. */
	void SetPreset(int32 Level);
	int32 GetPreset() const;

	/** 45..100 percent of the render resolution. */
	void SetResolutionScale(int32 Percent);
	int32 GetResolutionScale() const;

	/** t.MaxFPS: 0 means uncapped. */
	void SetFrameCap(int32 Fps);
	int32 GetFrameCap() const;

	void SetVsync(bool bOn);
	bool GetVsync() const;

	// ---- measurement ----
	float GetAvgMs() const { return AvgMs; }
	float GetP95Ms() const { return P95Ms; }
	float GetFps() const { return AvgMs > 0.f ? 1000.f / AvgMs : 0.f; }

	/** Writes a measured line to the log: average, 95th percentile, fps, knob. */
	void LogReport();

	/** The live thresholds, so the options page can never print a stale number. */
	float GetHeavyMs() const;
	float GetLightMs() const;

	/** Applies the preset + resolution + cap + vsync as one operation and saves. */
	void ApplyAll();

	/** Moves one rung along the phi grid of render scales. */
	void StepResolution(int32 Delta);

	// ---- THE 60 FPS PHI PHASE LOCK ----
	//
	// A cap is not a lock. The frame is a phase: 60 fps is one frame every 16.6667
	// ms, and the job is to hold the MEASURED 95th percentile on that phase, not to
	// hope the cap drags it there. So frame time is fed into a phase detector and
	// the error drives render scale continuously - no rungs, no 13 s steps, no
	// hunting - with the gains set by phi so the loop is damped and cannot ring.
	//
	//   error   e  = p95 - 16.6667 ms      (phase error, ms)
	//   scale      -= (1/phi)*e + (1/phi^4)*integral(e)     each 1/phi seconds
	//   lock       |e| under 1/phi ms for phi^4 seconds
	//
	// Drift is reported, never assumed away: the integral of the error is the time
	// the frame has actually lost or gained against 60 fps, in ms/s and in rungs.

	void SetPhaseLock(bool bOn);
	bool IsPhaseLock() const { return bPhaseLock; }
	bool IsLocked() const { return bLocked; }
	/** Live render scale the loop is holding, in percent. */
	float GetLockScale() const { return LockScale; }
	/** Signed phase error of the last window: negative is headroom, positive is late. */
	float GetPhaseErrorMs() const { return LockErrorMs; }
	/** Frame time actually lost(+) or gained(-) against 60 fps, per second. */
	float GetDriftMsPerSec() const { return DriftRate; }
	float GetDriftRung() const;
	/** One shared phase for every subsystem to read: frac(time / phi). */
	float GetOmniPhase() const { return OmniPhase; }
	/** How far the Fibonacci clock has slipped from real time, in beats. */
	float GetClockOffset() const { return ClockOffset; }

	// ---- THE OMNI LOCK ----
	// The frame lock is not the only thing held on phase: the same clock drives
	// the world, the brains, the weather and the ledger, so they cannot disagree.
	// What is measured about the phase is frags, not opinions:
	//   inPhase  share of frames delivered inside the 60 fps deadline
	//   misses   frames that arrived late (a count, not a rate)
	//   breaks   times the lock was lost after having been held
	// A held lock reports misses that stay at zero; an error is then an event,
	// not the resting state of the game.
	float GetInPhasePct() const { return InPhasePct; }
	int64 GetLateFrames() const { return LateFrames; }
	int32 GetLockBreaks() const { return LockBreaks; }
	float GetLockBudgetMs() const { return BudgetMs; }
	float GetLockCeiling() const { return ScaleCeiling; }
	FString LockLine() const;
	/** One line for the whole lock: phase, coherence, drift, misses, scale. */
	FString OmniLine() const;

private:
	void PhaseLockTick(float DeltaSeconds, float TargetMs);

	static constexpr float Phi = 1.618033988749895f;
	/** 1000 / 60: the budget for a 60 fps frame. */
	static constexpr float BudgetMs = 16.6667f;

private:
	void RecomputeStats();

	TArray<float> Samples;
	int32 SampleCursor = 0;
	float AvgMs = 0.f;
	float P95Ms = 0.f;
	bool bAdaptive = true;
	int32 ResolutionPercent = 100;
	int32 FrameCap = 60;
	bool bVsync = false;
	double LastAdjustTime = -100.0;
	double LastHeavyTime = -1000.0;
	/** Consecutive heavy readings. One spike is never a reason to move. */
	int32 HeavyStreak = 0;
	/** The p95 the last step was taken at, so the next step can ask whether it helped. */
	float P95AtLastStep = 0.f;
	/** A rung that did not help. Descending past it is what made the frame worse. */
	int32 FloorPercent = 45;
	/** How long to wait before probing back up. Grows by phi when a probe fails. */
	float ProbeWaitBase = 21.f;
	/** When the last climb was taken, so a fast fall can be blamed on it. */
	double LastClimbTime = -1000.0;

	// ---- phase lock state ----
	bool bPhaseLock = true;
	bool bLocked = false;
	float LockScale = 100.f;
	float LockIntegral = 0.f;
	/** The part of the last correction carried at the golden pitch (sin mu).
	 *  It lands one window later, so an error is brought in at 17.0324 deg
	 *  instead of being squared off by a straight push. */
	float LockQuadrature = 0.f;
	float LockErrorMs = 0.f;
	float LockP95 = 0.f;
	/** Seconds of error integrated; the drift denominator. */
	float LockSeconds = 0.f;
	/** Integral of frame-time error: ms the frame has lost against 60 fps. */
	float DriftMs = 0.f;
	float DriftRate = 0.f;
	/** How long the loop has been inside the lock band. */
	float LockHoldSeconds = 0.f;
	float LockApplyTimer = 0.f;
	float OmniPhase = 0.f;
	float ClockOffset = 0.f;
	double NextLockLog = 5.0;	double NextReportTime = 21.0;
	bool bInit = false;

	// ---- warm-up, coherence and the learned ceiling ----
	/** Seconds since the loop came online. The first frames are compile + stream. */
	float WarmupSeconds = 0.f;
	/** True once the sample ring has been emptied of startup frames. */
	bool bRingPrimed = false;
	/** Accumulated lateness (ms) and the window it was accumulated over. */
	float DriftAccumMs = 0.f;
	float DriftWindowSec = 0.f;
	/** Share of frames inside the deadline, 0..100. */
	float InPhasePct = 100.f;
	int64 LateFrames = 0;
	int32 LockBreaks = 0;
	/**
	 * The highest render scale this frame was actually seen to hold 60 fps at.
	 * Climbing blind into a wall every few seconds is what makes a governor
	 * hunt; remembering where the wall was is what makes it settle. It creeps
	 * back up after 21 s of clean frames, so a lighter scene is not punished
	 * forever by a heavier one.
	 */
	float ScaleCeiling = 100.f;
	double NextCeilingProbe = 0.0;
	/**
	 * HEADROOM PROBE. A capped frame cannot show headroom: it sits on 16.67 ms
	 * whether the GPU is idle or dying. So while locked, the cap is lifted for
	 * one phi of truth, the loop reads the frame it really gets, and then the cap
	 * goes back on. That is a measurement, not a guess - which is why it can raise
	 * the ceiling without climbing into a wall and breaking the lock to find out.
	 */
	bool bProbeOpen = false;
	double ProbeEndTime = 0.0;
	/** Drift stays quiet until this time: the ring still holds probe frames. */
	double DriftMuteUntil = 0.0;
	/** Fibonacci backoff between probes, so a wall is not hit every 21 s. */
	float ProbeInterval = 21.f;
};
