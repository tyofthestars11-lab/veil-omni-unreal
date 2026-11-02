// VEIL: THE LAST SIGNAL - faction AI.
// Four factions (Void, Scavenger, Guardian, Remnant), six-state agents,
// territory control, and a world-level Faction Manager (a WorldSubsystem).
// Self-contained: perception runs on a light timer, no Behavior Tree assets
// required to get the prototype moving.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Subsystems/WorldSubsystem.h"
#include "Systems/PHI_NPCProcessor.h"
#include "VEILFactionAI.generated.h"

class AVEILFactionAgent;
class UPHI_NPCProcessor;

/** One machine down - the district camp counts them, and the Vault listens. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVEILAgentDown, AVEILFactionAgent*, Agent);

UENUM(BlueprintType)
enum class EVEILFaction : uint8
{
	Void,
	Scavenger,
	Guardian,
	Remnant,
	None
};

UENUM(BlueprintType)
enum class EVEILAgentState : uint8
{
	Patrol,
	Alert,
	Hunt,
	Combat,
	Defend,
	Retreat,
	Dead
};

USTRUCT(BlueprintType)
struct FVEILTerritory
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EVEILFaction Owner = EVEILFaction::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector Center = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Radius = 2000.f;

	/** Higher = factions fight harder to hold / take it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float StrategicValue = 1.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FVEILTerritoryChanged, FName, TerritoryName, EVEILFaction, OldOwner, EVEILFaction, NewOwner);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVEILAgentStateChanged, AVEILFactionAgent*, Agent, EVEILAgentState, NewState);

// ---------------------------------------------------------------------------
// Agent
// ---------------------------------------------------------------------------
UCLASS()
class VEIL_API AVEILFactionAgent : public ACharacter
{
	GENERATED_BODY()

public:
	AVEILFactionAgent();

	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	/** Let the returned runner go for a window and walk back to the ring this
	 *  machine was seated on. Six machines hold the plaza and the plaza IS the
	 *  seat, so without this a return meant being beaten down in the same spot
	 *  every sixteen seconds with no chance to move. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Faction") void StandDown(float Seconds);

	/** Walk the machine's own ring on the phi clock - a guard that stands still
	 *  is a statue, and a station is not a home unless someone walks it. */
	void OrbitHome();

	/** Hold the post: stop moving, watch the plaza, stay there. The ring is gone -
	 *  every agent used to walk a circle around its own home point for the whole
	 *  session, which is what read as NPCs running in circles. */
	void HoldPost();

	/** One-shot guard: the post is logged once per agent, not every frame. */
	bool bLoggedPost = false;

	/** THE PHI BRAIN. Every machine in the plaza decides with UPHI_NPCProcessor: identity,
	 *  mood and situation are rungs, and the intent is the nearest rung anchor - not a
	 *  threshold, not a 0/1 flag. It was only ever attached to the wave enemies, so the
	 *  plaza guards ran a plain C++ state switch instead of the framework's brain. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Phi")
	TObjectPtr<UPHI_NPCProcessor> Brain;

	/** The intent this machine chose on its own clock, for the log and the HUD. */
	UFUNCTION(BlueprintPure, Category = "VEIL|Phi")
	FString PhiStatus() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Faction")
	EVEILFaction Faction = EVEILFaction::Void;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Faction")
	EVEILAgentState State = EVEILAgentState::Patrol;

	UPROPERTY(BlueprintAssignable, Category = "VEIL|Faction")
	FVEILAgentStateChanged OnStateChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Health")
	float MaxHealth = 75.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Health")
	float Health = 75.f;

	/** Retreat below this fraction of MaxHealth. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|AI")
	float RetreatHealthFraction = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|AI")
	float SightRange = 2200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|AI")
	/** Reach is a fight's length, not a sniper lane: 1400 cm let five machines
	 *  kill a 100 HP runner in about two seconds from across the plaza. */
	float AttackRange = 700.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|AI")
	float WeaponDamage = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|AI")
	float FireInterval = 1.6f;

	/** Actors to walk between while patrolling (place Target Points). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|AI")
	TArray<TObjectPtr<AActor>> PatrolPoints;

	/** Home ground this agent falls back to when defending / retreating. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|AI")
	FName HomeTerritory = NAME_None;

	UPROPERTY(BlueprintAssignable, Category = "VEIL|Faction")
	FVEILAgentDown OnAgentDown;

	UFUNCTION(BlueprintCallable, Category = "VEIL|Faction")
	void SetState(EVEILAgentState NewState);

	UFUNCTION(BlueprintPure, Category = "VEIL|Faction")
	bool IsAlive() const { return State != EVEILAgentState::Dead; }

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;

private:
	void BindStoneBody();
	/** A machine that falls pays the runner: XP and gold, on the phi rung. */
	void AwardKill();
	void MoveToward(const FVector& Dest, float AcceptRadius);
	void Perceive();
	void ActPatrol();
	void ActHunt();
	void ActCombat(float DeltaSeconds);
	void ActDefendOrRetreat(bool bRetreat);
	bool CanSeePlayer() const;

	TWeakObjectPtr<AActor> TargetActor;
	int32 PatrolIndex = 0;
	float LastFireTime = -100.f;
	/** World time until which this machine refuses to engage. */
	float StandDownUntil = -1.f;
	/** Last intent the brain chose, so a change is one log line, not a flood. */
	EPhiIntent LastPhiIntent = EPhiIntent::Rest;
	/** Orbit phase in laps, advanced by dt / phi^4 (one lap per 6.8541 s). */
	float OrbitPhase = 0.f;
	/** Its seat's own yaw, so no two machines walk the same arc. */
	float OrbitOffsetDeg = 0.f;
	/**
	 * What this machine can do, in rungs. It climbs 1/phi^3 for every day it
	 * lives through - no level-up, no player needed. A machine that has held the
	 * plaza for a week hits harder than the one that replaced it yesterday.
	 */
	float PowerRung = 0.f;
	int32 AgentDay = 1;
	float LastOrbitLog = -100.f;
	int32 OrbitLogs = 0;
	FVector HomeLocation = FVector::ZeroVector;

	FTimerHandle PerceiveTimer;
};

// ---------------------------------------------------------------------------
// Faction Manager - one per world. Tracks every agent, faction power,
// territory ownership, and reinforcement spawns.
// ---------------------------------------------------------------------------
UCLASS()
class VEIL_API UVEILFactionManager : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UPROPERTY(BlueprintAssignable, Category = "VEIL|Faction")
	FVEILTerritoryChanged OnTerritoryChanged;

	/** Agent class used for reinforcements (set by GameMode or in a BP subclass). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Faction")
	TSubclassOf<AVEILFactionAgent> ReinforcementClass;

	void RegisterAgent(AVEILFactionAgent* Agent);
	void UnregisterAgent(AVEILFactionAgent* Agent);

	UFUNCTION(BlueprintCallable, Category = "VEIL|Faction")
	void RegisterTerritory(const FVEILTerritory& Territory);

	UFUNCTION(BlueprintCallable, Category = "VEIL|Faction")
	bool ClaimTerritory(FName TerritoryName, EVEILFaction NewOwner);

	/** Reputation with a faction, used by the quest system.
	 *  Added for OMNIQuestSystem: the PC file called AdjustRep, but no
	 *  such method existed, so the call could never compile. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Faction")
	void AdjustRep(EVEILFaction Faction, float Amount);

	UFUNCTION(BlueprintPure, Category = "VEIL|Faction")
	float GetReputation(EVEILFaction Faction) const;

	/** Past +20 a faction stops treating the runner as a target: what you SAID to
	 *  them is worth more than a checkbox. Below -20 they hunt on sight. */
	UFUNCTION(BlueprintPure, Category = "VEIL|Faction")
	bool IsFriendlyToRunner(EVEILFaction Faction) const;

	UFUNCTION(BlueprintPure, Category = "VEIL|Faction")
	bool IsHuntingRunner(EVEILFaction Faction) const;

	UFUNCTION(BlueprintPure, Category = "VEIL|Faction")
	EVEILFaction GetTerritoryOwner(FName TerritoryName) const;

	/** Living agent count x average health: the faction's weight in the world. */
	UFUNCTION(BlueprintPure, Category = "VEIL|Faction")
	float GetFactionPower(EVEILFaction Faction) const;

	UFUNCTION(BlueprintPure, Category = "VEIL|Faction")
	int32 GetLivingAgentCount(EVEILFaction Faction) const;

	/** Spawn a fresh agent for a faction near a location (reinforcement wave). */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Faction")
	AVEILFactionAgent* SpawnReinforcement(EVEILFaction Faction, FVector Location);

	/** Hostility matrix: who fights whom. Default: everyone fights Void;
	 *  Scavenger and Guardian are rivals; Remnant is defensive-neutral
	 *  (fights only when its territory is entered - handled in agent logic). */
	UFUNCTION(BlueprintPure, Category = "VEIL|Faction")
	bool IsHostile(EVEILFaction A, EVEILFaction B) const;

private:
	TArray<TWeakObjectPtr<AVEILFactionAgent>> Agents;
	TArray<FVEILTerritory> Territories;
	TMap<EVEILFaction, float> Reputation;
};
