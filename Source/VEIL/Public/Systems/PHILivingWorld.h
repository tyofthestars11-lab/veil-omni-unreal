// THE LIVING WORLD.
//
// The cast used to be a table of names: authored once, identical on day 40, and
// deaf to anything the runner did. That is a codex, not a world. This subsystem
// gives every named person a life of their own that runs whether the player is
// watching or not:
//
//   ROUTINE   the Fibonacci day drives what they are doing - on post during the
//             duty beats, at their rest during the quiet ones, and the machines
//             in the district walk a wider ring and see less when the day turns.
//   MEMORY    every reply the runner gives is a deed. It is filed against the
//             person spoken to, with the day it happened on, and their next
//             greeting is not the authored one anymore - it is the one that
//             remembers.
//   POWER     nobody stands still in a phi world. Each day that closes adds
//             1/phi^3 of a rung to what a person can do, and helping or defying
//             them moves it further. They are their own beings: they grow with
//             or without the runner.
//
// Nothing here is a scripted animation. It is state on the same clock as the
// ledger, so the world breathes because it is running.
#pragma once

#include "CoreMinimal.h"
#include "Story/VEILCast.h"
#include "Subsystems/WorldSubsystem.h"
#include "PHILivingWorld.generated.h"

USTRUCT()
struct FPHILifeRecord
{
	GENERATED_BODY()

	UPROPERTY() FName Character = NAME_None;
	/** Times the runner treated this person well. */
	UPROPERTY() int32 Favours = 0;
	/** Times the runner defied them to their face. */
	UPROPERTY() int32 Harm = 0;
	/** What was actually said, last time. */
	UPROPERTY() FString LastDeed;
	UPROPERTY() int32 LastDeedDay = 0;
	/** ln-space growth: everyone climbs a little each day they live through. */
	UPROPERTY() float PowerRung = 1.f;
	UPROPERTY() int32 PowerSteps = 0;
};

UCLASS()
class VEIL_API UPHILivingWorld : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;

	/** File a deed: what the runner said, and whether it helped or stung. */
	void NoteDeed(FName Character, bool bHelped, const FString& Words);

	bool HasMemory(FName Character) const;
	/** The greeting this person actually opens with now. Empty when nothing happened. */
	FString MemoryLineFor(FName Character) const;

	/** What they can do now, as a rung - grown by days lived and by deeds. */
	float PowerRungOf(FName Character) const;
	/** "SIGNATURE  //  rung 2.4132  //  grown 3 days" - read by the codex page. */
	FString PowerOf(FName Character) const;

	/** -1..1 tint of how this person feels about the runner. */
	float RegardOf(FName Character) const;

	/** True when the Fibonacci day is outside the duty beats. */
	bool IsOffDuty() const;
	/** What they are doing at this beat, from the society table. */
	FString RoutineOf(FName Character) const;

	/** One line of world truth for the log: who is doing what, and how strong. */
	FString StatusLine(FName Character) const;

	// ---- they talk to EACH OTHER, not only to the runner ----
	//
	// Two of them, on their own clock, about the thing actually happening: the day
	// beat, the lamps, the Deep, the runner's last deed. It is not a loop - the
	// pair walks a golden stride through the cast and the wording is keyed to the
	// day, so the same exchange does not come round twice in a session.
	const FString& OverheardLine() const { return Overheard; }
	/** 0..1 fade for the strip, so a line arrives and leaves instead of snapping. */
	float OverheardAlpha() const;
	int32 ExchangeCount() const { return Exchanges; }

	int32 DeedCount() const { return TotalDeeds; }

private:
	FPHILifeRecord& Rec(FName Character);

	UPROPERTY() TMap<FName, FPHILifeRecord> Records;
	int32 LastDay = 1;
	int32 TotalDeeds = 0;
	float LogTimer = 0.f;

	void SpeakAmongThemselves();
	/** Stable cast order, so a golden stride over it is the same order every frame. */
	TArray<FName> Order;
	FString Overheard;
	float ExchangeAge = 999.f;
	float ExchangeInterval = 21.f;
	int32 Exchanges = 0;
};
