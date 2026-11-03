#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Perception/AISense.h"
#include "VEILEnemy.generated.h"

class AVEILEnemy;
class UAISenseConfig_Sight;
class UVEILLocomotionAnimInstance;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVEILEnemyDied, AVEILEnemy*, Enemy);

UENUM(BlueprintType)
enum class EVEILEnemyKind : uint8 { Drone, Soldier, WardenPrime };

// Drone (flies), Soldier (walks on the NavMesh), WARDEN PRIME (boss).
UCLASS()
class VEIL_API AVEILEnemy : public ACharacter
{
	GENERATED_BODY()
public:
	AVEILEnemy(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float Damage, const FDamageEvent& Event, AController* By, AActor* Causer) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL") EVEILEnemyKind Kind = EVEILEnemyKind::Drone;
	/** Champions drop epic spoils. Set by the world boot. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL")
	bool bChampion = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL")
	int32 ChampTier = 2;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL") float MaxHealth = 60.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL") float Health = 60.f;
	UPROPERTY(EditAnywhere, Category = "VEIL") float AttackDamage = 8.f;
	UPROPERTY(EditAnywhere, Category = "VEIL") float AttackRange = 300.f;
	UPROPERTY(EditAnywhere, Category = "VEIL") float AttackCooldown = 1.2f;
	UPROPERTY(EditAnywhere, Category = "VEIL") float MoveSpeed = 380.f;

	// ---------------- phi dynamics ----------------
	// An AI is not a static number. Every live stat below is a rung address that
	// breathes on a phi clock: rung = Rung(base) +/- phi^-4 * sin(phase), and the
	// quantity the machine actually uses is phi^rung, computed fresh every update
	// and round-tripped back (no typed constants, no fixed behaviour).

	/** Phase advanced by DeltaSeconds / phi. Accumulated, so it never drifts. */
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float PhiPhase = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float RungSpeed = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float RungDamage = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float RungCooldown = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float RungReach = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float RungSightRadius = 0.f;

	/** What this machine is using right now - recomputed from rungs each update. */
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float LiveMoveSpeed = 380.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float LiveAttackDamage = 8.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float LiveAttackCooldown = 1.2f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float LiveAttackRange = 180.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float LiveSightRange = 2200.f;

	/** PhiPow(Rung(LiveMoveSpeed)) - LiveMoveSpeed: the round-trip error, in cm/s. */
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float PhiRoundTripError = 0.f;

	UPROPERTY(BlueprintAssignable) FVEILEnemyDied OnEnemyDied;

	// Engine sight: the foe aggros what it SEES (Unreal Perception),
	// not a raw distance check. Close range (<500) still counts as heard.
	UPROPERTY(VisibleAnywhere, Category = "VEIL|AI")
	class UAIPerceptionComponent* Sight;

	/** The phinary brain: its decision is a base-phi numeral, not a set of flags. */
	UPROPERTY(VisibleAnywhere, Category = "VEIL|Phi")
	class UPHI_NPCProcessor* Brain;

	UFUNCTION()
	void OnTargetSensed(AActor* Seen, FAIStimulus Stimulus);

	// Apply kind-based stats (called by the spawner right after spawning).
	UFUNCTION(BlueprintCallable, Category = "VEIL") void ConfigureForKind(EVEILEnemyKind InKind);
	virtual void BeginPlay() override;

	/** Let the runner go for a window. Anything inside the machine's own 500 cm
	 *  "heard" range never loses him, so without this a return to the seat was a
	 *  feeding loop: he stood, he was beaten down, he stood again, forever. */
	UFUNCTION(BlueprintCallable, Category = "VEIL") void StandDown(float Seconds);

private:
	float AttackTimer = 0.f;
	FVector HomeLoc = FVector::ZeroVector;
	/** World time until which this machine refuses to engage. */
	float StandDownUntil = -1.f;

	// The seeds the rungs are anchored to (kind stats, set once per kind).
	float BaseSpeed = 380.f;
	float BaseDamage = 8.f;
	float BaseCooldown = 1.2f;
	float BaseReach = 180.f;
	float BaseSightRadius = 2200.f;

	/** Recompute every live stat from its rung. Called every tick. */
	void PhiUpdate(float DeltaSeconds);

	/** Rung of a base value, displaced by a phi-clock wave; the caller turns it back into a quantity. */
	float PhiBreath(float InBaseValue, float PhaseOffset, float AmplitudeScale) const;

	// The last reach that was actually written to the log. Logging is gated on the
	// VALUE moving by more than 1/phi^5, never on a timer: a clock writes the same
	// line every five seconds forever, a value gate writes it when it means something.
	float PhiLastLogReach = -1.f;
	UPROPERTY() class UAISenseConfig_Sight* SightConfig = nullptr;
	int32 AttackLogs = 0;
	UAnimSequence* IdleSeq = nullptr;
	UAnimSequence* JogSeq = nullptr;
	/** The blended locomotion instance (idle <-> jog), cached at BeginPlay. */
	TWeakObjectPtr<UVEILLocomotionAnimInstance> LocoAnim;
	float RepathTimer = 0.f;
	float LastSeenTime = -1000.f;
	bool bDead = false;
	bool bCalledFriends = false;
	void Die();

	/** True when nothing solid stands between this machine and the runner. */
	bool HasClearSight(const APawn* Target) const;

	/** Ground machines call their friends: one sighting wakes every machine
	 *  within earshot, so a patrol answers as a patrol instead of one at a time. */
	void AlertNearby();
};
