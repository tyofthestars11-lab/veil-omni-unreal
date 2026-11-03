#include "Systems/VEILPerf.h"

#include "Engine/Engine.h"
#include "GameFramework/GameUserSettings.h"
#include "HAL/IConsoleManager.h"
#include "HAL/PlatformMisc.h"
#include "Misc/App.h"
#include "Scalability.h"
#include "Systems/VEILTime.h"
#include "VEIL.h"

namespace
{
	// The knob, on the phi grid. Each step is 100/phi = 61.8 % of the previous,
	// so a step is never a cliff.
	const TArray<int32> GScaleGrid = { 45, 55, 67, 82, 100 };

	void SetCVar(const TCHAR* Name, float Value)
	{
		if (IConsoleVariable* Var = IConsoleManager::Get().FindConsoleVariable(Name))
		{
			Var->Set(Value, ECVF_SetByCode);
		}
	}
}

void UVEILPerf::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LockScale = (float)ResolutionPercent;
	ApplyAll();
	UE_LOG(LogVEIL, Log, TEXT("PHI PERF: governor online. GPU=%s  budget=%.2f ms  "
		"down>%.2f ms  up<%.2f ms  cooldown=13 s"),
		*FPlatformMisc::GetPrimaryGPUBrand(),
		BudgetMs, BudgetMs * Phi, BudgetMs / Phi);
}

void UVEILPerf::ApplyAll()
{
	SetCVar(TEXT("t.MaxFPS"), (float)FrameCap);
	SetCVar(TEXT("r.VSync"), bVsync ? 1.f : 0.f);
	// With the phase lock on, the loop's continuous scale is the knob; the manual
	// integer scale is what it falls back to when the lock is switched off.
	SetCVar(TEXT("r.ScreenPercentage"), bPhaseLock ? LockScale : (float)ResolutionPercent);
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		Settings->SetVSyncEnabled(bVsync);
		Settings->SetFrameRateLimit((float)FrameCap);
		// NOT ApplyResolutionSettings: that reads the *saved* window size and on a
		// machine with none saved it snapped a 1600x900 window down to 640x480 the
		// moment the governor came online. Resolution is scaled by cvar instead.
	}
}

void UVEILPerf::SetAdaptive(bool bOn)
{
	bAdaptive = bOn;
	if (!bOn)
	{
		// Standing down returns full resolution rather than leaving a scar.
		ResolutionPercent = 100;
		SetCVar(TEXT("r.ScreenPercentage"), 100.f);
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI PERF: adaptive quality %s (resolution %d%%)."),
		bOn ? TEXT("ON") : TEXT("OFF"), ResolutionPercent);
}

void UVEILPerf::SetPreset(int32 Level)
{
	UE_LOG(LogVEIL, Log, TEXT("PHI PERF: quality preset -> %d."), Level);
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		Settings->SetOverallScalabilityLevel(Level);
		Settings->ApplySettings(false);
		Settings->SaveSettings();
	}
}

int32 UVEILPerf::GetPreset() const
{
	if (const UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		return Settings->GetOverallScalabilityLevel();
	}
	return -1;
}

void UVEILPerf::SetResolutionScale(int32 Percent)
{
	ResolutionPercent = FMath::Clamp(Percent, GScaleGrid[0], GScaleGrid.Last());
	LockScale = (float)ResolutionPercent;
	if (!bPhaseLock)
	{
		SetCVar(TEXT("r.ScreenPercentage"), (float)ResolutionPercent);
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI PERF: resolution scale %d%%."), ResolutionPercent);
}

int32 UVEILPerf::GetResolutionScale() const
{
	return ResolutionPercent;
}

void UVEILPerf::SetFrameCap(int32 Fps)
{
	FrameCap = FMath::Max(0, Fps);
	SetCVar(TEXT("t.MaxFPS"), (float)FrameCap);
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		Settings->SetFrameRateLimit((float)FrameCap);
		Settings->SaveSettings();
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI PERF: frame cap %d (0 = uncapped)."), FrameCap);
}

int32 UVEILPerf::GetFrameCap() const
{
	return FrameCap;
}

void UVEILPerf::SetVsync(bool bOn)
{
	bVsync = bOn;
	SetCVar(TEXT("r.VSync"), bVsync ? 1.f : 0.f);
	if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
	{
		Settings->SetVSyncEnabled(bVsync);
		Settings->SaveSettings();
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI PERF: vsync %s."), bVsync ? TEXT("on") : TEXT("off"));
}

bool UVEILPerf::GetVsync() const
{
	return bVsync;
}

void UVEILPerf::StepResolution(int32 Delta)
{
	int32 Index = GScaleGrid.IndexOfByKey(ResolutionPercent);
	if (Index == INDEX_NONE)
	{
		// Snap to the nearest grid rung first.
		Index = 0;
		for (int32 i = 0; i < GScaleGrid.Num(); ++i)
		{
			if (GScaleGrid[i] <= ResolutionPercent) { Index = i; }
		}
	}
	const int32 Next = FMath::Clamp(Index + Delta, 0, GScaleGrid.Num() - 1);
	if (Next != Index)
	{
		SetResolutionScale(GScaleGrid[Next]);
	}
}

void UVEILPerf::RecomputeStats()
{
	if (Samples.Num() == 0) { return; }
	TArray<float> Sorted = Samples;
	Sorted.Sort();
	float Sum = 0.f;
	for (const float V : Sorted) { Sum += V; }
	AvgMs = Sum / (float)Sorted.Num();
	P95Ms = Sorted[FMath::Clamp((int32)(Sorted.Num() * 0.95f), 0, Sorted.Num() - 1)];
}

float UVEILPerf::GetHeavyMs() const
{
	const float Target = FrameCap > 0 ? 1000.f / (float)FMath::Min(FrameCap, 60) : BudgetMs;
	return Target * 1.15f;
}

float UVEILPerf::GetLightMs() const
{
	const float Target = FrameCap > 0 ? 1000.f / (float)FMath::Min(FrameCap, 60) : BudgetMs;
	return Target / Phi;
}

// A MEASUREMENT SWITCH, NOT A SHIPPING MODE. The governor rewrites r.ScreenPercentage
// every 1/phi seconds, so any outside test that sets a resolution is overwritten within
// a frame or two - two measurements of 20 %% and 100 %% both came back reading the value
// the loop had chosen. With this at 0 the loop freezes and a frame cost can be measured
// at a resolution the tester actually asked for.
static TAutoConsoleVariable<int32> CVarVeilGovernor(
	TEXT("veil.Governor"), 1,
	TEXT("1: the frame governor runs and owns the render scale. 0: frozen for measurement."),
	ECVF_Default);

void UVEILPerf::LogReport()
{
	UE_LOG(LogVEIL, Log, TEXT("PHI PERF: avg=%.2f ms  p95=%.2f ms  fps=%.1f  resolution=%d%%  preset=%d  adaptive=%d  cap=%d"),
		AvgMs, P95Ms, GetFps(), ResolutionPercent, GetPreset(), bAdaptive ? 1 : 0, FrameCap);
}

// ---------------------------------------------------------------------------
// THE 60 FPS PHI PHASE LOCK
// ---------------------------------------------------------------------------

void UVEILPerf::SetPhaseLock(bool bOn)
{
	bPhaseLock = bOn;
	if (!bOn)
	{
		bLocked = false;
		LockHoldSeconds = 0.f;
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI LOCK: phase lock %s (screen %.1f%%)."),
		bOn ? TEXT("ON") : TEXT("OFF"), LockScale);
}

float UVEILPerf::GetDriftRung() const
{
	// Same law as everything else: rung = ln(value)/ln(phi), computed fresh. The
	// value is 1 + |drift| so a positive drift and a negative one read alike.
	const float V = 1.f + FMath::Abs(DriftRate);
	return FMath::Loge(V) / FMath::Loge(Phi);
}

FString UVEILPerf::LockLine() const
{
	return FString::Printf(
		TEXT("PHI LOCK  %s  %.1f fps  p95 %.2f ms  err %+.2f ms  in phase %.1f%%  screen %.1f%%"),
		bLocked ? TEXT("LOCKED") : TEXT("ACQUIRING"), GetFps(), LockP95, LockErrorMs,
		InPhasePct, LockScale);
}

FString UVEILPerf::OmniLine() const
{
	// One line for the whole lock: the shared phase, the drift it has lost, how
	// many frames arrived late, and how many times the hold was broken. On a held
	// frame the misses stop counting and the breaks do not move at all.
	return FString::Printf(
		TEXT("OMNI LOCK  phase %.3f  drift %+.3f ms/s  rung %.4f  misses %d  breaks %d  ceiling %.1f%%  beats %.3f"),
		OmniPhase, DriftRate, GetDriftRung(), (int32)FMath::Min<int64>(LateFrames, 999999), LockBreaks, ScaleCeiling, ClockOffset);
}

void UVEILPerf::PhaseLockTick(float DeltaSeconds, float TargetMs)
{
	if (CVarVeilGovernor.GetValueOnGameThread() == 0)
	{
		// The report timer lives further down this function, so freezing the governor also
		// silences the PERF line. A frozen run logs its own line instead of looking dead.
		static double NextFrozenLog = 0.0;
		const double FrozenNow = FApp::GetCurrentTime();
		if (FrozenNow >= NextFrozenLog)
		{
			NextFrozenLog = FrozenNow + 5.0;
			UE_LOG(LogVEIL, Display, TEXT("PHI PERF (frozen governor): last avg %.2f ms p95 %.2f ms fps %.1f screen %.0f%%"),
				AvgMs, P95Ms, GetFps(), LockScale);
		}
		return;	// frozen: nothing here may rewrite the render scale
	}

	const double Now = FApp::GetCurrentTime();

	// One phase reference for the whole game: 1/phi of a second per turn, so it is
	// dense and never repeats, and every subsystem can be compared against it.
	OmniPhase = (float)FMath::Frac(Now / (double)Phi);

	// How far the Fibonacci clock has slipped from real time, in beats. A clock
	// that drifts reports it instead of quietly disagreeing with the frame.
	if (const UVEILTime* Clock = GetWorld() ? GetWorld()->GetSubsystem<UVEILTime>() : nullptr)
	{
		ClockOffset = Clock->DayProgress();
	}

	// WARM-UP GATE. The heaviest frames this game ever produces are its first
	// ones: shader compiles, level streaming, the world boot. Measured on this PC,
	// one 89 ms startup frame fed the loop and it drove the render scale from 100
	// to 45 % in a single window, then called 59.8 fps a lock while the screen sat
	// at the floor. Nothing is measured now until the ring is steady frames only.
	if (!bRingPrimed)
	{
		return;
	}

	// Phase detector: the 95th percentile against the 60 fps phase, in ms.
	LockP95 = P95Ms;
	LockErrorMs = P95Ms - TargetMs;

	// COHERENCE. The band is the deadline plus 1/phi ms - the same tolerance the
	// lock holds to - and the share of frames inside it is the number that says
	// whether errors are the norm. A held lock reads 100 %.
	{
		const float DeadlineMs = TargetMs + 1.f / Phi;
		int32 InBand = 0;
		for (const float V : Samples)
		{
			if (V <= DeadlineMs) { ++InBand; }
		}
		InPhasePct = Samples.Num() > 0 ? 100.f * (float)InBand / (float)Samples.Num() : 100.f;
	}

	// Leaky integrator: it corrects a standing error, and it forgets, so a heavy
	// start cannot wind it up and hold the frame low forever.
	const float Leak = FMath::Clamp(1.f - DeltaSeconds / (Phi * Phi), 0.f, 1.f);
	LockIntegral = FMath::Clamp(LockIntegral * Leak + LockErrorMs * DeltaSeconds, -60.f, 60.f);

	// DRIFT IS MEASURED, NOT AVERAGED OVER THE SESSION. Each second the frame is
	// late by (p95 - budget) is time actually lost against the 60 fps phase; that
	// lateness is summed over a phi^2 second window, so the number says what the
	// frame is doing NOW and a startup blip cannot masquerade as a permanent loss.
	if (Samples.Num() >= 120 && !bProbeOpen && Now >= DriftMuteUntil)
	{
		// The mean, not the p95: drift is time actually lost across frames, and the
		// p95 turns one hitch into a fake thousand milliseconds a second. Frames
		// taken while the cap is lifted are not the game's frames, so they are not
		// counted either - otherwise a probe reads as a fake gain or loss.
		const float LatenessPerSecond = (AvgMs - TargetMs) * (1000.f / TargetMs);
		DriftAccumMs += LatenessPerSecond * DeltaSeconds;
		DriftWindowSec += DeltaSeconds;
		if (DriftWindowSec >= Phi * Phi)
		{
			DriftRate = DriftAccumMs / DriftWindowSec;
			DriftMs = DriftRate;
			DriftAccumMs = 0.f;
			DriftWindowSec = 0.f;
		}
	}

	// Loop filter, applied once per 1/phi seconds. The proportional gain is 1/phi
	// (percent of render scale per ms), the integral gain 1/phi^4: the same damping
	// ratio the rest of the game uses, so it converges instead of ringing. Inside
	// a +-1/phi ms dead band nothing moves at all: that band is why a locked frame
	// stops twitching instead of chasing its own jitter.
	LockApplyTimer += DeltaSeconds;
	if (LockApplyTimer >= 1.f / Phi)
	{
		LockApplyTimer = 0.f;

		const float Kp = 1.f / Phi;
		const float Ki = 1.f / (Phi * Phi * Phi * Phi);
		const float Deadband = 1.f / Phi;
		const bool bRidingCap = FrameCap > 0 && AvgMs <= (1000.f / (float)FrameCap) * 1.02f;

		float Push = 0.f;
		if (LockErrorMs > Deadband)
		{
			// Late: give scale back in proportion to how late, phi-weighted when it
			// is badly late. This is a correction, not a step ladder.
			Push = -(Kp * (LockErrorMs - Deadband) * (LockErrorMs > 2.f ? Phi : 1.f) + Ki * LockIntegral);
		}
		else if (LockErrorMs < -Deadband)
		{
			Push = Kp * (-LockErrorMs - Deadband);
		}
		// During a headroom probe the cap is lifted, so an "early" frame is only
		// headroom if the machine actually produced one. Measured 15.5 ms at
		// 97 fps means 64 fps is the ceiling here - climbing on that reading was
		// what broke the hold right after every probe.
		if (bProbeOpen && AvgMs > TargetMs / Phi)
		{
			Push = FMath::Min(Push, 0.f);
		}
		else if (bRidingCap && LockScale < ScaleCeiling && !bProbeOpen)
		{
			// A CAPPED FRAME HIDES ITS HEADROOM. Sitting exactly on 16.67 ms means
			// the frame is early, not that the loop has finished, so while it rides
			// the cap it climbs - but only up to the ceiling this frame was actually
			// seen to hold 60 fps at, never blindly into the same wall every time.
			Push = 4.f;
		}

		// SLEW LIMIT: no single window may move the scale more than 1/phi^4 of the
		// whole range (14.6 %). That is what a shader-compile hitch or a streaming
		// spike cannot do - slam the frame to the floor and call it a lock.
		const float MaxStep = 100.f / (Phi * Phi * Phi * Phi);
		Push = FMath::Clamp(Push, -MaxStep, MaxStep);

		// THE GOLDEN PITCH, APPLIED TO THE ERROR ITSELF.
		// mu = 17.0324 deg. A correction aimed straight at the error overshoots it;
		// a correction turned away from it drifts. So the push is taken at mu off
		// the error axis: cos(mu) = 0.9561392708 corrects now, sin(mu) = 0.2929124355
		// is carried one window as quadrature and lands when the phase comes round.
		// cos^2 + sin^2 = 1.000000000000, so no part of the error is invented or
		// thrown away - it is the same error, brought in at a pitch, not squared off.
		if (Push != 0.f)
		{
			const float Mu = FMath::DegreesToRadians(17.0324f);
			const float PitchGain = FMath::Cos(Mu);      // 0.9561 in phase
			const float PitchCarry = FMath::Sin(Mu);     // 0.2929 quadrature

			const float Wanted = Push;
			const float InPhase = Wanted * PitchGain;
			const float Quadrature = Wanted * PitchCarry;

			Push = InPhase + LockQuadrature;   // last window's carry arrives now
			LockQuadrature = Quadrature;       // this window's carry waits its turn

			if (FMath::Abs(Wanted) > 0.5f)
		{
				UE_LOG(LogVEIL, Log,
					TEXT("PHI PITCH: mu 17.0324 deg  error %+.2f ms  wanted %+.2f  in-phase %+.2f (cos %.10f)  carry %+.2f (sin %.10f)  rung(err) %.4f"),
					LockErrorMs, Wanted, InPhase, PitchGain, Quadrature, PitchCarry,
					FMath::Loge(FMath::Max(1.0e-6f, FMath::Abs(LockErrorMs))) / FMath::Loge(Phi));
			}
		}

		// A climb that goes late within a few seconds of being taken means the
		// ceiling was wrong: remember the wall just under it.
		if (LockErrorMs > Deadband && Now - LastClimbTime < 6.0)
		{
			const float NewCeiling = FMath::Clamp(LockScale * 0.99f, 45.f, 100.f);
			if (NewCeiling < ScaleCeiling - 0.1f)
			{
				ScaleCeiling = NewCeiling;
				NextCeilingProbe = Now + 21.0;
				LastClimbTime = -1000.0;
				UE_LOG(LogVEIL, Log, TEXT("PHI LOCK: ceiling learned at %.1f%% - the frame went late (%+.2f ms) just after climbing."),
					ScaleCeiling, LockErrorMs);
			}
		}

		const float Before = LockScale;
		LockScale = FMath::Clamp(LockScale + Push, 45.f, 100.f);
		if (Push > 0.f && LockScale > Before + 0.05f)
		{
			LastClimbTime = Now;
		}
		if (FMath::Abs(LockScale - Before) >= 0.05f)
		{
			ResolutionPercent = FMath::Clamp(FMath::RoundToInt(LockScale), 45, 100);
			SetCVar(TEXT("r.ScreenPercentage"), LockScale);
		}

	}

	// THE HEADROOM PROBE. While the lock is held, the cap hides whether there is
	// room to give the screen back. So the cap is lifted for one phi (1.618 s);
	// a frame that still lands near 16.67 ms at 97 fps means real headroom, and
	// the loop has already raised the scale by the error it saw. Then the cap goes
	// back on. Nothing is broken to learn this, which is the difference between a
	// lock and a guess: measured, the blind climb broke the hold once a minute.
	if (!bProbeOpen && bLocked && Now >= NextCeilingProbe)
	{
		bProbeOpen = true;
		ProbeEndTime = Now + Phi;
		SetCVar(TEXT("t.MaxFPS"), (float)(FMath::Min(FrameCap, 60) * Phi));
		UE_LOG(LogVEIL, Log, TEXT("PHI LOCK: headroom probe open - cap %.0f -> %.0f fps for %.2f s at screen %.1f%%, ceiling %.1f%%."),
			(float)FrameCap, (float)(FMath::Min(FrameCap, 60) * Phi), Phi, LockScale, ScaleCeiling);
	}
	else if (bProbeOpen && Now >= ProbeEndTime)
	{
		bProbeOpen = false;
		SetCVar(TEXT("t.MaxFPS"), (float)FrameCap);
		// A frame that came in well under the 60 fps budget while uncapped had
		// headroom: the ceiling moves up to where the scale now stands. A frame
		// that was still late does not move it.
		// Headroom proven: propose the next rung and let the lock climb to it.
		// It climbs only to the ceiling, and the next probe is what proves it -
		// measure, propose, verify. Rising on the strength of "it looked fine"
		// is what made the scale ratchet up and then break the hold.
		if (AvgMs > 0.f && AvgMs < TargetMs / Phi)
		{
			const float Before = ScaleCeiling;
			// Four percent above what is actually held, not above a stale ceiling:
			// a proposal that jumps 15 % is a guaranteed break, and a break is the
			// error this lock is built to keep out of the game.
			ScaleCeiling = FMath::Min(100.f, FMath::Max(ScaleCeiling, LockScale) + 4.f);
			if (ScaleCeiling > Before + 0.01f)
			{
				UE_LOG(LogVEIL, Log, TEXT("PHI LOCK: headroom proven at %.2f ms - ceiling %.1f%% -> %.1f%%, climbing to prove it."),
					AvgMs, Before, ScaleCeiling);
			}
		}
		DriftAccumMs = 0.f;
		DriftWindowSec = 0.f;
		// The ring still holds probe frames for a moment after the cap returns;
		// drift stays quiet for that one phi so it cannot report a fake gain.
		DriftMuteUntil = Now + Phi;
		// Fibonacci backoff: 21 -> 34 -> 55 -> 89 s between probes, so a scene that
		// has no room is not interrogated every 21 s forever.
		ProbeInterval = FMath::Min(89.f, ProbeInterval * Phi);
		NextCeilingProbe = Now + ProbeInterval;
		UE_LOG(LogVEIL, Log, TEXT("PHI LOCK: headroom probe closed - avg %.2f ms at screen %.1f%%, ceiling %.1f%%, next probe in %.0f s."),
			AvgMs, LockScale, ScaleCeiling, ProbeInterval);
	}

	// Lock: error inside 1/phi ms for phi^4 seconds. Leaving needs a wider band
	// than entering, which is the hysteresis that stops a lock light blinking.
	const bool bInside = FMath::Abs(LockErrorMs) <= 1.f / Phi;
	if (bInside)
	{
		LockHoldSeconds += DeltaSeconds;
		if (!bLocked && LockHoldSeconds >= Phi * Phi * Phi * Phi)
		{
			bLocked = true;
			UE_LOG(LogVEIL, Log, TEXT("PHI LOCK: LOCKED at 60 fps - p95 %.2f ms, err %+.2f ms, in phase %.1f%%, screen %.1f%%."),
				LockP95, LockErrorMs, InPhasePct, LockScale);
		}

	}
	else
	{
		LockHoldSeconds = 0.f;
		if (bLocked && FMath::Abs(LockErrorMs) > 2.f)
		{
			bLocked = false;
			++LockBreaks;
			UE_LOG(LogVEIL, Log, TEXT("PHI LOCK: lost lock - p95 %.2f ms (err %+.2f ms), re-acquiring at screen %.1f%%."),
				LockP95, LockErrorMs, LockScale);
		}
	}

	if (Now >= NextLockLog)
	{
		NextLockLog = Now + 5.0;
		UE_LOG(LogVEIL, Log, TEXT("%s"), *LockLine());
		UE_LOG(LogVEIL, Log, TEXT("%s"), *OmniLine());
	}
}

void UVEILPerf::Tick(float DeltaTime)
{
	// Frame time as the engine measured it, not as a wall clock guesses it.
	const float Ms = FMath::Max(0.f, (float)(FApp::GetDeltaTime() * 1000.0));
	if (Ms <= 0.f) { return; }

	const float TargetMs = FrameCap > 0 ? 1000.f / (float)FMath::Min(FrameCap, 60) : BudgetMs;

	// Every frame that missed the phase is counted, never averaged away.
	if (Ms > TargetMs + 1.f / Phi)
	{
		++LateFrames;
	}

	if (Samples.Num() < 120)
	{
		Samples.Add(Ms);
	}
	else
	{
		Samples[SampleCursor] = Ms;
		SampleCursor = (SampleCursor + 1) % 120;
	}

	// WARM-UP: empty the ring once phi^3 (4.24 s) into the session, so the frames
	// that drive the loop are steady frames and never a shader compile.
	WarmupSeconds += DeltaTime;
	if (!bRingPrimed && WarmupSeconds >= Phi * Phi * Phi)
	{
		bRingPrimed = true;
		Samples.Reset();
		SampleCursor = 0;
		AvgMs = 0.f;
		P95Ms = 0.f;
		LateFrames = 0;
		UE_LOG(LogVEIL, Log, TEXT("PHI LOCK: warm-up closed at %.1f s - the phase window is steady frames only."), WarmupSeconds);
	}
	RecomputeStats();

	const double Now = FApp::GetCurrentTime();
	if (Now >= NextReportTime)
	{
		NextReportTime = Now + 21.0; // a Fibonacci number of seconds
		LogReport();
	}

	// A full window of frames before the loop is allowed to move anything: a
	// half-filled ring is mostly startup, and startup is not the frame.
	if (!bAdaptive || Samples.Num() < 120)
	{
		return;
	}
	// One move per 13 s: the frame settles instead of hunting.
	if (Now - LastAdjustTime < 13.0 && !bPhaseLock)
	{
		return;
	}
	if (bPhaseLock)
	{
		// The phase lock owns render scale while it is on and is not gated by the
		// 13 s ladder: it is a continuous control, so its own slew limit and dead
		// band keep it from hunting, and gating it would let it drift.
		PhaseLockTick(DeltaTime, TargetMs);
		return;
	}
	// THE TARGET IS THE TARGET. A first cut of this used budget*phi (26.97 ms) as
	// the "too heavy" line and happily settled at 43 fps while calling the frame
	// held - measured, and wrong. The heavy line is now 8 % over the real 60 fps
	// budget (18.00 ms), and phi is used where it belongs: as the ratio below the
	// budget that counts as genuine headroom.
	// 15 %, not 8 %: a capped 60 fps frame jitters around 16.67 by more than a
	// millisecond on its own, and a threshold inside the jitter makes the
	// governor move on noise. Measured on this machine, 8 % moved four times.
	const float HeavyMs = TargetMs * 1.15f;
	const float LightMs = TargetMs / Phi;

	// The phase lock owns render scale when it is on: one continuous control, no
	// rung stepping underneath it, so the two can never fight over the same knob.
	// MEASURED LESSON: the mean is a bad signal here. A run stepped 100 -> 82 ->
	// 67 -> 55 % on a 0.01 ms mean overshoot and got SLOWER each time (60 fps to
	// 54.8), because the frame was never GPU-bound - it was riding the cap. So:
	// the signal is the 95th percentile, one reading is never enough, and a step
	// that does not help does not get a second step under it.
	if (P95Ms > HeavyMs + 0.5f)
	{
		++HeavyStreak;
		LastHeavyTime = Now;

		// Did the last step down actually buy anything?
		if (P95AtLastStep > 0.f && P95Ms >= P95AtLastStep * 0.98f && ResolutionPercent < 100)
		{
			FloorPercent = ResolutionPercent + 15; // this rung was the floor
			UE_LOG(LogVEIL, Log, TEXT("PHI KRAKEN: lowering resolution did not help (p95 %.2f -> %.2f) - floor set at %d%%; this frame is not GPU-bound."),
				P95AtLastStep, P95Ms, ResolutionPercent);
			P95AtLastStep = 0.f;
			HeavyStreak = 0;
			return;
		}

		if (HeavyStreak >= 3 && ResolutionPercent > FloorPercent)
		{
			// A fall that follows a climb closely means the probe climbed into the
			// same wall. Back the next probe off by phi instead of hammering it
			// every 21 s: 21 -> 34 -> 55 -> 89, and it stops there.
			if (Now - LastClimbTime < 40.0 && ProbeWaitBase < 89.f)
			{
				const float Before = ProbeWaitBase;
				ProbeWaitBase = FMath::Min(89.f, ProbeWaitBase * Phi);
				UE_LOG(LogVEIL, Log, TEXT("PHI KRAKEN: the probe up did not hold - next probe waits %.0f s (was %.0f)."),
					ProbeWaitBase, Before);
				LastClimbTime = -1000.0;
			}
			const int32 Before = ResolutionPercent;
			P95AtLastStep = P95Ms;
			StepResolution(-1);
			if (ResolutionPercent != Before)
			{
				LastAdjustTime = Now;
				UE_LOG(LogVEIL, Log, TEXT("PHI KRAKEN: frame heavy (p95 %.2f ms > %.2f, %d readings) - resolution %d%% -> %d%%."),
					P95Ms, HeavyMs, HeavyStreak, Before, ResolutionPercent);
			}
		}
		return;
	}

	HeavyStreak = 0;
	P95AtLastStep = 0.f;

	// With a frame cap on, the frame can never look "light": it sits on the cap by
	// definition. So a capped frame probes upward - but only when the frame has
	// been good for a while. A probe that follows a heavy stretch just re-climbs
	// into the same wall, which is how a governor starts bouncing.
	const float CapMs = FrameCap > 0 ? 1000.f / (float)FrameCap : 0.f;
	const bool bRidingTheCap = CapMs > 0.f && AvgMs <= CapMs * 1.08f;

	// RECOVERY. A governor that only ever steps down is a governor that slowly
	// loses the frame it was protecting. When the 95th percentile sits clearly
	// under the heavy line the probe back up may come after 21 s instead of the
	// conservative 55 s; when it does not, the long wait still applies.
	const bool bClearHeadroom = P95Ms > 0.f && P95Ms < HeavyMs * 0.94f;
	const float ProbeWait = bClearHeadroom ? ProbeWaitBase : 55.f;
	const bool bLongEnoughSinceHeavy = (Now - LastHeavyTime) > ProbeWait;
	if (P95Ms < LightMs || (bRidingTheCap && bLongEnoughSinceHeavy && ResolutionPercent < 100))
	{
		const int32 Before = ResolutionPercent;
		const int32 OldFloor = FloorPercent;
		StepResolution(1);
		if (ResolutionPercent != Before)
		{
			LastAdjustTime = Now;
			LastClimbTime = Now;
			// A rung that was the floor under a heavy frame is not a floor under a
			// healthy one. Clearing it is what lets recovery reach a real 100%.
			FloorPercent = 45;
			UE_LOG(LogVEIL, Log, TEXT("PHI KRAKEN: frame holds (avg %.2f ms, p95 %.2f ms, cap %d) - resolution %d%% -> %d%% (floor %d -> 45)."),
				AvgMs, P95Ms, FrameCap, Before, ResolutionPercent, OldFloor);
		}
	}
}

TStatId UVEILPerf::GetStatId() const
{
	return TStatId();
}
