// VEIL OMNI: THE CAMPAIGN.
//
// The quest system knows the six beats and the side quests know four chores;
// nothing joined them into a game you can finish. This subsystem is that join:
//
//   * six acts, each with one objective line, driven by the beat counters that
//     already exist (fragments, beacons, cache, warden) - no new state the save
//     does not already carry;
//   * FIFTY ACTIVITIES, generated from the five regions and six counter kinds,
//     each one a real goal on a real counter rather than a menu of nothing;
//   * an ending: when the Warden falls and all three beacons burn, the campaign
//     closes the story, pays the epilogue reward and says so on screen.
//
// The counters are incremented from the places the events actually happen
// (enemy death, fragment pickup, beacon forge, cache, wave cleared, dialogue),
// so an activity cannot tick up by itself and a proof cannot show progress the
// player did not make.
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Tickable.h"
#include "VEILCampaign.generated.h"

/** What an activity asks for. Each kind reads one counter. */
enum class EVEILCounter : uint8
{
	Kills,
	Fragments,
	Beacons,
	Caches,
	Waves,
	Voices,
	Boss
};

/** One of the fifty. Plain data: built once, read by the HUD, never serialised. */
struct FVEILActivity
{
	FString Title;
	EVEILCounter Kind = EVEILCounter::Kills;
	int32 Region = 0;
	int32 Required = 1;
	int32 Gold = 0;
	int32 XP = 0;
	bool bDone = false;
	/** The runner has stood at this activity's marker. */
	bool bVisited = false;
};

UCLASS()
class VEIL_API UVEILCampaignSubsystem : public UWorldSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	// FTickableGameObject: one cheap scan a second, no actor, no timer asset.
	virtual void Tick(float DeltaSeconds) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;

	/** The event hooks. Called from where the event actually happens. */
	void NoteKill(bool bBoss);
	void NoteFragment();
	void NoteBeacon();
	void NoteCache();
	void NoteWave();
	void NoteVoice();
	/** The runner reached an activity marker. */
	void NoteVisited(int32 Index);

	/** The HUD's two lines. Never empty. */
	FString ObjectiveLine() const;
	FString ActivityLine() const;
	/** Nearest open activity and its bearing, for the HUD's compass line. */
	FString NearestLine(const FVector& From) const;
	FString ActTitle() const;

	/** The fifty, whole, for a console dump or a proof. */
	const TArray<FVEILActivity>& All() const { return Activities; }

	int32 DoneCount() const;
	int32 TotalCount() const { return Activities.Num(); }
	bool IsComplete() const { return bComplete; }

private:
	int32 CountOf(EVEILCounter Kind) const;
	void BuildActivities();
	void SpawnMarkers();
	/** Region anchor plus a golden step: where one of the fifty stands. */
	FVector AnchorFor(int32 Region, int32 Slot) const;
	void Scan();
	void Announce(const FString& Line) const;
	void CloseTheStory();

	TArray<FVEILActivity> Activities;
	/** One marker per activity, so the fifty exist as places. */
	TArray<TWeakObjectPtr<AActor>> Markers;

	int32 Kills = 0;
	int32 Fragments = 0;
	int32 Beacons = 0;
	int32 Caches = 0;
	int32 Waves = 0;
	int32 Voices = 0;
	int32 Bosses = 0;

	int32 LastDone = 0;
	float ScanClock = 0.f;
	bool bComplete = false;
};
