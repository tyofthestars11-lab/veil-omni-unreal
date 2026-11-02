// VEIL companions: GR MUSE (1, sage), DRONE (2), GUARD, SCOUT, BULWARK (5 max).
// Engine AI throughout: AIController MoveTo on the NavMesh for follow and
// fight, distance-gated engagement beside the Runner. No scripts, no teleports.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Systems/PHIMito.h"
#include "Systems/PHIMito.h"
#include "VEILCompanion.generated.h"

UENUM(BlueprintType)
enum class EVEILCompanionRole : uint8
{
	Sage,
	Drone,
	Guard,
	Scout,
	Bulwark
};

UCLASS()
class VEIL_API AVEILCompanion : public ACharacter
{
	GENERATED_BODY()

public:
	AVEILCompanion(const FObjectInitializer& ObjectInitializer);

	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Companion")
	EVEILCompanionRole CompanionRole = EVEILCompanionRole::Guard;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Companion")
	FName CallSign = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Companion")
	float MaxHealth = 120.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Companion")
	float Health = 120.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Companion")
	float Damage = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Companion")
	float AttackRange = 220.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Companion")
	float AttackCooldown = 1.1f;

	/** Follow distance behind the Runner. Drone rides close and fast. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Companion")
	float FollowDistance = 350.f;

	// ---------------- phi dynamics ----------------
	// The stats above are seeds. These are what the companion actually uses,
	// recomputed from their rungs on the shared phi clock every tick.
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float PhiPhase = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float LiveDamage = 10.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float LiveAttackRange = 220.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float LiveAttackCooldown = 1.1f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float LiveFollowDistance = 350.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float LiveMoveSpeed = 620.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Phi") float PhiRoundTripError = 0.f;

	UFUNCTION(BlueprintPure, Category = "VEIL|Companion")
	bool IsAlive() const { return Health > 0.f; }

	UFUNCTION(BlueprintCallable, Category = "VEIL|Companion")
	void ConfigureForRole(EVEILCompanionRole InRole, FName InCallSign);

protected:
	virtual void BeginPlay() override;

private:
	FPhiDynamic PhiClock;
	void PhiUpdate(float DeltaSeconds);
	float BaseMoveSpeed = 620.f;
	float PhiLogTimer = 0.f;
	int32 PhiLogCount = 0;
	float AttackTimer = 0.f;
	float BarkTimer = 0.f;
	bool bDead = false;
	UAnimSequence* IdleSeq = nullptr;
	UAnimSequence* JogSeq = nullptr;
	UAnimSequence* CurAnim = nullptr;
	void Die();
	AActor* FindFoe() const;
};
