// VEIL OMNI - THE LAST SIGNAL - PATH OF LIGHT: one whole thing.
//
// This is the single system the five regions, the runner, the market, the
// save and the proof all go through. It does not replace the existing
// character, quest or progression code: it drives THEM, so there is one set
// of rules and no second copy of any of them.
//
// The world it addresses, at Tyree's authority coordinates (cm):
//
//   Hub market, gold phi pillars in front of the Keeper ... 900, 0
//   Nest cliffs + waterfalls, left ....................... -450, 780
//   Spire in the clouds, centre .......................... 2800, 500
//   Omni District city, right ............................ 600, -2100
//   Megastructure Deep, below right (Vault arena disc) .... 0, 0 (z -80.9)
//   The Vault, grounded .................................. 0, 0, -80.9
//   The runner's seat .................................... 0, 0, 100
//
// Every one of those numbers is also in VEILAnchors (progression) or
// UVEILRegionSubsystem::GetInfo. They are repeated here as compile-time
// constants so this system can place art and prove a walk without asking a
// subsystem that might not exist yet in the frame it is asked.
//
// Save: a real USaveGame through UGameplayStatics, superset of UVEILSaveGame.
//
// Proof runs, each a real -game session that writes its own log lines:
//   -phiomni         walk to the Keeper, claim 3 beacons, fight, die, respawn,
//                    save, quit
//   -phiomnireload   read the save back and say what survived
//   -phiomniregions  travel to all five regions through the world's own pads
//   -phiomninest     walk the Nest chain: 3 fragments, 4 machines, reward
//   -phiomnichains   walk the Spire, Omni District and Deep chains in one
//                    session, then save and quit
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Subsystems/WorldSubsystem.h"
#include "Systems/VEILProgression.h"
#include "VEILOmniAll.generated.h"

class AVEILCharacter;
class AVEILEnemy;
class AStaticMeshActor;

namespace VEILOmniAuthority
{
	// The five places, at the centres UVEILRegionSubsystem answers with.
	constexpr float HubX = 900.f, HubY = 0.f;          // market, Keeper's column
	constexpr float NestX = -450.f, NestY = 780.f;      // cliffs + water, left
	constexpr float SpireX = 2800.f, SpireY = 500.f;    // in the clouds, centre
	constexpr float OmniX = 600.f, OmniY = -2100.f;     // the city, right
	constexpr float DeepX = 0.f, DeepY = 0.f;           // the disc below right

	// The Vault and the runner's seat, exactly as the progression lock holds them.
	constexpr float VaultZ = -80.9f;
	constexpr float RunnerZ = 100.f;

	// How close the runner has to stand to a speaker for Interact to reach them.
	constexpr float SpeakerReach = 700.f;
	// How close a beacon has to be to be forged by standing at it (phi x debris ring).
	constexpr float BeaconReach = 1003.f;
	// How far from the Nest centre counts as being in the Nest.
	constexpr float NestReach = 2500.f;
}

/**
 * The one save. A superset of UVEILSaveGame, not a rival to it: everything the
 * district already saved (level, XP, gold, fragments, inventory, seat, map)
 * still travels here, and the extra fields are the ones a quit-and-reload
 * session was silently losing.
 */
UCLASS()
class VEIL_API UVEILOmniSave : public UVEILSaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	int32 BeaconsClaimed = 0;

	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	int32 QuestStage = 0;

	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	int32 Region = 0;

	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	int32 Deaths = 0;

	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	int32 Respawns = 0;

	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	float PlaySeconds = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	float Health = 100.f;

	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	bool bVaultOpen = false;

	// ---- The Nest's own chain ----
	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	int32 NestStage = 0;

	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	int32 NestFragments = 0;

	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	int32 NestMachinesDown = 0;

	// ---- Spire / Omni / Deep chains, indexed by region (0..4) ----
	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	TArray<int32> ChainStages;

	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	TArray<int32> ChainFragments;

	/** Bit per region entered: 1 Hub, 2 Nest, 4 Spire, 8 Omni, 16 Deep. */
	UPROPERTY(VisibleAnywhere, Category = "VEIL|Save")
	int32 VisitedMask = 0;
};

/**
 * The whole thing: seven verbs, five places, four chains, one save.
 *
 * Every verb routes into the system that already owns that job. Move feeds the
 * character's own movement input, Attack calls the character's own Strike,
 * Interact runs the same beacon claim and interactable chain the player's key
 * runs, Die goes through ApplyDamage so grace, attack tokens and the one-shot
 * ceiling all still apply, and Respawn calls the character's own uncapped
 * RespawnNow.
 */
UCLASS()
class VEIL_API UVEILOmniAll : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Tick(float DeltaSeconds) override;
	virtual TStatId GetStatId() const override { return GetStatID(); }

	// ---- The seven verbs -------------------------------------------------

	/** Write the omni slot and the progression slot. True only when both wrote. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OMNI")
	bool SaveGame();

	/** Read the omni slot back into this system and the progression book. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OMNI")
	bool LoadGame();

	/** Move on the runner's own axes. Climb is the flight axis (-1..1), 0 = hover. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OMNI")
	void Move(float ForwardAxis, float RightAxis, float ClimbAxis, float DeltaSeconds);

	/** Swing, or fire the lance bolt when bRanged. Both are the character's own. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OMNI")
	void Attack(bool bRanged = false);

	/** The interact key's chain: beacon underfoot, then a region shard, then the
	 *  nearest interactable, then the nearest speaker who is willing to talk. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OMNI")
	void Interact();

	/** A death through the real damage chain. Grace still protects; a proof
	 *  repeats the blow until the world actually lets one land. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OMNI")
	void Die();

	/** The character's own uncapped return to the seat. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OMNI")
	void Respawn();

	// ---- World ------------------------------------------------------------

	/** Walk toward a point using the same input Move uses. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OMNI")
	void MoveToPoint(const FVector& Target, float DeltaSeconds);

	/** Place the Fab meshes at the five authority centres. Idempotent per world. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OMNI")
	void DressAuthority();

	/** Forge one beacon through the quest system's own index claim. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OMNI")
	void ClaimBeaconAt(int32 Index);

	/** Travel to a region through the world builder's own arrival pads:
	 *  0 Hub, 1 Nest, 2 Spire, 3 Omni District, 4 Megastructure Deep. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OMNI")
	bool TravelToRegion(int32 Index);

	/** Build The Nest as a region with its own chain: 3 fragments to recover on
	 *  the cliffs and 4 machines holding them. Idempotent per world. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OMNI")
	void BuildNestRegion();

	/** Build one of the remaining regions' chains (2 Spire, 3 Omni, 4 Deep):
	 *  three shards to recover and three to four machines holding them, with its
	 *  own reward written to the book when it closes. Idempotent per world. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|OMNI")
	void BuildRegionChain(int32 RegionIndex);

	UFUNCTION(BlueprintPure, Category = "VEIL|OMNI")
	APawn* GetRunnerPawn() const;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	int32 BeaconsClaimed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	int32 BeaconsTotal = 3;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	float PlaySeconds = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	int32 Deaths = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	int32 Respawns = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	int32 Interactions = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	int32 Attacks = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	float DistanceWalked = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	bool bSaveResult = false;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	bool bLoadResult = false;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	FString LastSaveSlot;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	bool bProofRun = false;

	// ---- The Nest ----
	/** 0 not entered, 1 arrived, 2 fragments recovered, 3 cleared. */
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	int32 NestStage = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	int32 NestFragments = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	int32 NestFragmentsTotal = 3;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	int32 NestMachinesDown = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	int32 NestMachinesTotal = 4;

	// ---- Spire / Omni / Deep chains, indexed by region ----
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	TArray<int32> ChainStages;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	TArray<int32> ChainFragments;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	TArray<int32> ChainMachinesDown;

	/** Bit per region entered: 1 Hub, 2 Nest, 4 Spire, 8 Omni, 16 Deep. */
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	int32 VisitedMask = 0;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|OMNI")
	int32 RegionsVisited = 0;

	/** The slot this system writes. The progression slot is written too, so the
	 *  district's own HUD and any older reader keep working unchanged. */
	static const FString OmniSlotName;

private:
	AVEILCharacter* GetRunner() const;

	void MoveAxis(const FVector& WorldDir, float Scale);
	void TickProof(float DeltaSeconds);
	void ProofStageLog(const FString& Line) const;

	/** One authored mesh at one authority point, wearing the PHI palette. */
	AStaticMeshActor* PlaceAt(const FVector& At, const TCHAR* MeshPath,
		const FVector& TargetSizeCm, bool bGold, float YawDegrees);

	/** True when the save slot holds a real file this build can open. */
	bool SlotHasSave() const;

	// ---- Nest chain ----
	void TickNest(float DeltaSeconds);
	void NestAdvance(int32 NewStage);
	void MarkVisited(int32 RegionIndex);

	// ---- Chains for Spire / Omni / Deep ----
	void EnsureChainArrays();
	void TickChains(float DeltaSeconds);
	void AdvanceChain(int32 RegionIndex, int32 NewStage);

	float ProofClock = 0.f;
	int32 ProofStage = 0;
	float StageClock = 0.f;
	float LastBlowClock = 0.f;
	bool bDressPlaced = false;
	int32 DressCount = 0;
	int32 SaveFileIndex = 0;
	int32 LoadFileIndex = 0;
	FVector WalkStart = FVector::ZeroVector;

	/** Where the walk last made real progress, and how long it has been stuck.
	 *  A walk that stops making progress is a wall, and a wall is answered by
	 *  jumping and stepping, not by standing still until a timeout. */
	FVector StallFrom = FVector::ZeroVector;
	float StallClock = 0.f;
	/** A detour is walked, not nudged: while it runs the runner goes diagonally
	 *  around whatever stopped him, and the sign flips each time so two attempts
	 *  cannot grind against the same corner. */
	float DetourClock = 0.f;
	int32 DetourSign = 1;

	// ---- Nest state ----
	TArray<TWeakObjectPtr<AActor>> NestFragmentActors;
	TArray<TWeakObjectPtr<AVEILEnemy>> NestMachineActors;
	TArray<FVector> NestFragmentSpots;
	bool bNestBuilt = false;
	int32 NestProofIndex = 0;
	int32 RegionProofStep = 0;

	// ---- Chain state (5 slots, regions 2..4 used) ----
	TArray<TArray<TWeakObjectPtr<AActor>>> ChainFragmentActors;
	TArray<TArray<TWeakObjectPtr<AVEILEnemy>>> ChainMachineActors;
	TArray<TArray<FVector>> ChainFragmentSpots;
	TArray<int32> ChainMachineTotal;
	TArray<bool> ChainBuilt;
	int32 ChainProofRegion = 2;
	int32 ChainProofFragment = 0;
};
