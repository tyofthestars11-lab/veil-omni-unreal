#include "Systems/VEILTime.h"

#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
#include "VEIL.h"
#include "Systems/VEILPhi.h"

namespace
{
	// Real seconds per beat: 1, 1, 2, 3, 5, 8, 13, 21, 34, 55. Sum 143.
	const TArray<int32> GBeatSeconds = { 1, 1, 2, 3, 5, 8, 13, 21, 34, 55 };

	const TCHAR* GBeatNames[10] = {
		TEXT("FIRST LIGHT"),
		TEXT("THE WAKING"),
		TEXT("THE FIRST WATCH"),
		TEXT("THE TRADE HOURS"),
		TEXT("THE LONG WALK"),
		TEXT("THE HIGH SUN"),
		TEXT("THE SIFTING"),
		TEXT("THE SECOND WATCH"),
		TEXT("THE QUIET HOURS"),
		TEXT("THE DEEP NIGHT"),
	};

	constexpr float GVeilTimePhi = VeilPhi::PhiF;

	// THE DAY IS STRETCHED; THE CLOCK BREATHES. Real time stays the reference - one
	// second is one second of the player's life - but the world's day runs phi^2
	// slower, so the 143-second cycle takes about 374 seconds. The beats are still
	// Fibonacci numbers (1,1,2,3,5,8,13,21,34,55); they are read on a longer second.
	// And the rate breathes by phi^-4 (0.1459 of a beat), the same law every AI stat
	// breathes on, so no two beats pass at the same speed. Nothing here is typed by
	// hand: the stretch is phi^2 and the breath is 1/phi^4, computed from the one phi.
	constexpr float GDayStretch = (float)VeilPhi::Phi2;
	constexpr float GBeatBreath =
		(float)(VeilPhi::InvPhi * VeilPhi::InvPhi * VeilPhi::InvPhi * VeilPhi::InvPhi);
	/** An unobserved field resolves itself after this many real seconds. */
	// 8 s, not 13: with 1-second beats early in the day, a 13 s hold could stall
	// the clock for longer than a whole stretch of the morning.
	constexpr float SelfCollapseSeconds = 8.f;

	// The world is busiest in the middle of the day and quiet at both ends.
	bool DutyForBeat(int32 ZeroBased)
	{
		return ZeroBased >= 1 && ZeroBased <= 7;
	}
}

const TArray<int32>& UVEILTime::BeatSeconds()
{
	return GBeatSeconds;
}

int32 UVEILTime::BeatLength() const
{
	return GBeatSeconds.IsValidIndex(BeatIndex) ? GBeatSeconds[BeatIndex] : 1;
}

FString UVEILTime::BeatNameOf(int32 ZeroBased)
{
	return GBeatNames[FMath::Clamp(ZeroBased, 0, 9)];
}

FString UVEILTime::BeatName() const
{
	return BeatNameOf(BeatIndex);
}

bool UVEILTime::IsDutyBeat() const
{
	return DutyForBeat(BeatIndex);
}

float UVEILTime::BeatProgress() const
{
	const int32 Len = FMath::Max(1, BeatLength());
	return FMath::Clamp(1.f - (BeatLeft / (float)Len), 0.f, 1.f);
}

float UVEILTime::DayProgress() const
{
	return FMath::Clamp(Elapsed / 143.f, 0.f, 1.f);
}

FString UVEILTime::ClockLine() const
{
	if (bSuperposed)
	{
		FString Beats;
		for (const FVEILBeatCandidate& C : Field)
		{
			Beats += Beats.IsEmpty()
				? FString::Printf(TEXT("%d"), C.Beat + 1)
				: FString::Printf(TEXT("|%d"), C.Beat + 1);
		}
		// The width of the field was fixed by the coherence at the moment it opened;
		// reporting the live value beside it read as a contradiction.
		return FString::Printf(TEXT("DAY %d   FIELD %s   %.0f%% COHERENT AT OPEN   %s"),
			Day, *Beats, FieldCoherence * 100.f,
			bAwaitingObserver ? TEXT("AWAITING AN OBSERVER")
				: *FString::Printf(TEXT("HELD %.0fs / %.0fs"), HoldSeconds, SelfCollapseSeconds));
	}
	return FString::Printf(TEXT("DAY %d   BEAT %d/%d   %s   %.0fs left"),
		Day, GetBeat(), BeatCount(), *BeatName(), FMath::Max(0.f, BeatLeft));
}

TStatId UVEILTime::GetStatId() const
{
	return TStatId();
}

void UVEILTime::EnterSuperposition()
{
	// How wide the field is depends on how well the present is locked: a coherent
	// frame offers two futures, a decohered one offers four.
	int32 Width = Coherence > 0.618f ? 2 : (Coherence > 0.382f ? 3 : 4);

	// And on the world's own intent: a world measured to be tense really does
	// hold more possible futures, a resting one fewer. This is the self-updating
	// half of the field - nothing here is scripted, it is read from UPHIIntent.
	if (ChaosPressure > 0.618f)      { Width = FMath::Min(4, Width + 1); }
	else if (ChaosPressure < 0.382f) { Width = FMath::Max(2, Width - 1); }

	Field.Reset();
	float Total = 0.f;
	for (int32 k = 0; k < Width; ++k)
	{
		FVEILBeatCandidate C;
		C.Beat = (BeatIndex + 1 + k) % GBeatSeconds.Num();
		C.Amplitude = FMath::Pow(1.f / GVeilTimePhi, (float)k);
		Total += C.Amplitude;
		Field.Add(C);
	}
	for (FVEILBeatCandidate& C : Field)
	{
		C.Amplitude /= Total;
	}

	bSuperposed = true;
	FieldCoherence = Coherence;
	HoldSeconds = 0.f;

	FString Line;
	for (const FVEILBeatCandidate& C : Field)
	{
		Line += FString::Printf(TEXT("%s%d %.0f%%"), Line.IsEmpty() ? TEXT("") : TEXT("  |  "),
			C.Beat + 1, C.Amplitude * 100.f);
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI TEMPORAL: field opens - %s   (coherence %.0f%%, intent %.0f%%, %d futures)"),
		*Line, Coherence * 100.f, ChaosPressure * 100.f, Width);
}

void UVEILTime::CollapseTo(int32 CandidateIndex, const TCHAR* Reason)
{
	if (!Field.IsValidIndex(CandidateIndex))
	{
		CandidateIndex = 0;
	}
	const FVEILBeatCandidate Chosen = Field[CandidateIndex];
	const int32 From = BeatIndex;

	LastFieldCount = Field.Num();
	LastCollapsedSkip = FMath::Max(0, CandidateIndex);

	BeatIndex = Chosen.Beat;
	BeatLeft = (float)GBeatSeconds[BeatIndex];
	bSuperposed = false;
	Field.Reset();
	HoldSeconds = 0.f;

	UE_LOG(LogVEIL, Log, TEXT("PHI TEMPORAL: collapsed by %s - beat %d -> %d%s (%s)."),
		Reason, From + 1, BeatIndex + 1,
		LastCollapsedSkip > 0 ? *FString::Printf(TEXT(", skipped %d"), LastCollapsedSkip) : TEXT(""),
		*BeatName());

	// A skip is not cosmetic: the beat he landed in is one the measured world has
	// not reached yet, so he steps out of linear time for as long as it takes the
	// world to catch up to him.
	if (LastCollapsedSkip > 0)
	{
		EnterPhase(LastCollapsedSkip, Reason);
	}
}

void UVEILTime::ForceField()
{
	if (!bSuperposed)
	{
		EnterSuperposition();
	}
}

// ---------------------------------------------------------------------------
// OPERATING OUTSIDE LINEAR TIME
// ---------------------------------------------------------------------------

float UVEILTime::WorldTimeScale() const
{
	// One beat ahead runs the world at 1/phi, two at 1/phi^2, and so on. At rest
	// this is exactly 1, so normal play is never touched by the mechanic.
	return FMath::Pow(1.f / GVeilTimePhi, (float)PhaseDrift);
}

float UVEILTime::PhaseRung() const
{
	const float Scale = WorldTimeScale();
	if (Scale <= 0.f) { return -55.f; }
	// Computed fresh from the live scale, never typed: rung = ln(value)/ln(phi).
	return FMath::Loge(Scale) / FMath::Loge(GVeilTimePhi);
}

FString UVEILTime::PhaseLine() const
{
	if (PhaseDrift <= 0)
	{
		return FString();
	}
	return FString::Printf(
		TEXT("OUTSIDE LINEAR TIME   +%d %s AHEAD   WORLD %.3fx   RUNG %.4f   ACTS %d"),
		PhaseDrift, PhaseDrift == 1 ? TEXT("BEAT") : TEXT("BEATS"),
		WorldTimeScale(), PhaseRung(), TimelessCount);
}

void UVEILTime::EnterPhase(int32 Beats, const TCHAR* Cause)
{
	PhaseDrift = FMath::Max(0, Beats);
	// One beat of the gap closes every phi^2 seconds: the world catches up along
	// the golden step, not all at once.
	PhaseLeft = GVeilTimePhi * GVeilTimePhi;
	PhaseRefresh = 0.f;
	ApplyPhaseToWorld();

	UE_LOG(LogVEIL, Log, TEXT("PHI TEMPORAL: OUTSIDE LINEAR TIME - %s put the runner %d %s ahead; "
		"the world now runs at x%.3f (rung %.4f)."),
		Cause ? Cause : TEXT("a collapse"), PhaseDrift,
		PhaseDrift == 1 ? TEXT("beat") : TEXT("beats"), WorldTimeScale(), PhaseRung());
}

void UVEILTime::LeavePhase(const TCHAR* Cause)
{
	PhaseDrift = 0;
	PhaseLeft = 0.f;
	ApplyPhaseToWorld();
	UE_LOG(LogVEIL, Log, TEXT("PHI TEMPORAL: back in linear time at beat %d (%s); "
		"%d act%s were taken outside the clock."),
		GetBeat(), Cause ? Cause : TEXT("caught up"),
		TimelessCount, TimelessCount == 1 ? TEXT("") : TEXT("s"));
}

void UVEILTime::ApplyPhaseToWorld()
{
	UWorld* W = GetWorld();
	if (!W)
	{
		return;
	}
	const float Scale = PhaseDrift > 0 ? WorldTimeScale() : 1.f;
	const APawn* Me = UGameplayStatics::GetPlayerPawn(this, 0);
	// Everything that lives in time feels the gap; the runner does not, which is
	// what makes being ahead of the clock worth having.
	for (TActorIterator<APawn> It(W); It; ++It)
	{
		APawn* P = *It;
		if (!P || P == Me) { continue; }
		P->CustomTimeDilation = Scale;
	}
	PhaseRefresh = 1.f;
}

void UVEILTime::NoteTimelessAct(const TCHAR* Reason)
{
	++TimelessCount;
	const float Before = Coherence;
	// An act that never had to travel through time costs the field nothing: it
	// settles the present instead of scattering it. This is the reward, not a toll.
	Coherence = FMath::Clamp(Coherence + 0.05f, 0.f, 1.f);
	UE_LOG(LogVEIL, Log, TEXT("PHI TEMPORAL: act #%d outside linear time (%s) - coherence %.0f%% -> %.0f%%."),
		TimelessCount, Reason ? Reason : TEXT("act"), Before * 100.f, Coherence * 100.f);
}

void UVEILTime::SetHoldPhase(bool b)
{
	bHoldPhase = b;
	if (b && PhaseDrift <= 0)
	{
		ForcePhase(2);
	}
}

void UVEILTime::ForcePhase(int32 Beats)
{
	EnterPhase(Beats, TEXT("proof step"));
}

void UVEILTime::Observe(const TCHAR* Reason)
{
	if (!bSuperposed)
	{
		// Outside linear time there is no field to collapse - the act simply
		// happens without ever crossing the clock.
		if (PhaseDrift > 0)
		{
			NoteTimelessAct(Reason);
		}
		return;
	}
	// What the observation IS decides which future it lands in: stillness keeps
	// the nearest, a strike takes the second, travel takes the farthest. The
	// world moves through time the same way it moves through space.
	const FString R(Reason ? Reason : TEXT("still"));
	int32 Pick = 0;
	if (R.Contains(TEXT("travel")))
	{
		Pick = Field.Num() - 1;
	}
	else if (R.Contains(TEXT("strike")) || R.Contains(TEXT("speak")))
	{
		Pick = FMath::Min(1, Field.Num() - 1);
	}
	CollapseTo(Pick, *R);
}

void UVEILTime::Tick(float DeltaTime)
{
	// Nobody ages and nothing decays: time is only ever added.
	Elapsed += DeltaTime;

	// Coherence follows the body: still observes, motion decoheres.
	if (const APawn* P = UGameplayStatics::GetPlayerPawn(this, 0))
	{
		const float Speed = (float)P->GetVelocity().Size2D();
		SampleSpeed = FMath::FInterpTo(SampleSpeed, Speed, DeltaTime, GVeilTimePhi);
	}
	const float Stillness = FMath::Clamp(1.f - SampleSpeed / 260.f, 0.f, 1.f);
	Coherence = FMath::Clamp(Coherence + (Stillness - 0.38f) * DeltaTime * 0.5f, 0.f, 1.f);

	// THE TIMELESS WINDOW. While the runner stands ahead of the world, the world
	// is dilated and the gap closes one beat at a time.
	if (PhaseDrift > 0)
	{
		// Newly spawned pawns must feel the same dilation as the ones already here.
		PhaseRefresh -= DeltaTime;
		if (PhaseRefresh <= 0.f)
		{
			ApplyPhaseToWorld();
		}

		if (!bHoldPhase)
		{
			PhaseLeft -= DeltaTime;
			if (PhaseLeft <= 0.f)
			{
				--PhaseDrift;
				PhaseLeft = GVeilTimePhi * GVeilTimePhi;
				if (PhaseDrift <= 0)
				{
					LeavePhase(TEXT("the world caught up"));
				}
				else
				{
					ApplyPhaseToWorld();
					UE_LOG(LogVEIL, Log, TEXT("PHI TEMPORAL: the world catches up one beat - runner now +%d "
						"(x%.3f)."), PhaseDrift, WorldTimeScale());
				}
			}
		}
	}

	if (bSuperposed)
	{
		// THE HOLD. While the field is open the clock does not move: this is the
		// "outside linear time" part, and it is the player's stillness or action
		// that ends it.
		HoldSeconds += DeltaTime;

		if (bAwaitingObserver)
		{
			return; // proof hold: the field stays open for inspection
		}

		// Standing still IS an observation: the field settles on the nearest
		// future once the runner has held still for phi^2 seconds.
		if (SampleSpeed < 30.f && HoldSeconds >= GVeilTimePhi * GVeilTimePhi)
		{
			CollapseTo(0, TEXT("stillness"));
			return;
		}

		if (HoldSeconds >= SelfCollapseSeconds)
		{
			CollapseTo(0, TEXT("no observer"));
		}
		return;
	}

	// The world's second is longer than the player's, and it varies as it goes.
	const float Breath = 1.f + FMath::Sin(Elapsed * 0.05f) * GBeatBreath;
	BeatLeft -= DeltaTime * Breath / GDayStretch;
	if (BeatLeft <= 0.f)
	{
		// A day that ends at beat 10 turns without a field: the cycle closing is
		// the one transition that is not in question.
		if ((BeatIndex + 1) % GBeatSeconds.Num() == 0)
		{
			++BeatIndex;
			if (BeatIndex >= GBeatSeconds.Num())
			{
				BeatIndex = 0;
				++Day;
				Elapsed = 0.f;
				UE_LOG(LogVEIL, Warning, TEXT("PHI TIME: DAY %d turns after %.1fs of world time (day stretched phi^2 = %.3fx real, beats breathing +/-%.1f%%) - the cycle closes, nobody aged."),
				Day, (float)(143.0 * GDayStretch), GDayStretch, GBeatBreath * 100.f);
			}
			BeatLeft += (float)GBeatSeconds[BeatIndex];
			UE_LOG(LogVEIL, Log, TEXT("PHI TIME: %s"), *ClockLine());
			return;
		}
		BeatLeft = 0.f;
		EnterSuperposition();
	}
}
