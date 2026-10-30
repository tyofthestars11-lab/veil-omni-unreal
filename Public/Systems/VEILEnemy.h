#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VEILEnemy.generated.h"

class AVEILEnemy;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FVEILEnemyDied, AVEILEnemy*, Enemy);

UENUM(BlueprintType)
enum class EVEILEnemyKind : uint8 { Drone, Soldier, WardenPrime };

// Drone (flies), Soldier (walks on the NavMesh), WARDEN PRIME (boss).
UCLASS()
class VEIL_API AVEILEnemy : public ACharacter
{
	GENERATED_BODY()
public:
	AVEILEnemy();
	virtual void Tick(float DeltaSeconds) override;
	virtual float TakeDamage(float Damage, const FDamageEvent& Event, AController* By, AActor* Causer) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL") EVEILEnemyKind Kind = EVEILEnemyKind::Drone;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL") float MaxHealth = 60.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL") float Health = 60.f;
	UPROPERTY(EditAnywhere, Category = "VEIL") float AttackDamage = 8.f;
	UPROPERTY(EditAnywhere, Category = "VEIL") float AttackRange = 180.f;
	UPROPERTY(EditAnywhere, Category = "VEIL") float AttackCooldown = 1.2f;
	UPROPERTY(EditAnywhere, Category = "VEIL") float MoveSpeed = 380.f;

	UPROPERTY(BlueprintAssignable) FVEILEnemyDied OnEnemyDied;

	// Apply kind-based stats (called by the spawner right after spawning).
	UFUNCTION(BlueprintCallable, Category = "VEIL") void ConfigureForKind(EVEILEnemyKind InKind);

private:
	float AttackTimer = 0.f;
	float RepathTimer = 0.f;
	bool bDead = false;
	void Die();
};
