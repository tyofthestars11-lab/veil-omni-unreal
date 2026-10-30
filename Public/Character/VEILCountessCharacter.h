#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PHI_MitochondrialEngine.h"
#include "VEILCountessCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;

// Playable Countess. Mesh: assign SM_Countess (SkeletalMesh) in the Blueprint child or defaults.
UCLASS()
class VEIL_API AVEILCountessCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AVEILCountessCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// --- Phi engine (ATP gradient etc.) ---
	PHI_MitochondrialEngine Engine;

	// Action costs: dash 12 (100->88), attack 18 (->70), interact 8 (->62)
	UPROPERTY(EditAnywhere, Category = "VEIL|ATP") float DashCost = 12.f;
	UPROPERTY(EditAnywhere, Category = "VEIL|ATP") float AttackCost = 18.f;
	UPROPERTY(EditAnywhere, Category = "VEIL|ATP") float InteractCost = 8.f;

	UFUNCTION(BlueprintPure, Category = "VEIL|ATP") float GetATP() const { return Engine.ATPGradient; }

	// Health, added in the re-seat. Enemies call ApplyDamage on the Countess;
	// without this, that damage did nothing. Health bar binds to GetHealth / GetMaxHealth.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Health") float MaxHealth = 100.f;
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Health") float Health = 100.f;
	UFUNCTION(BlueprintPure, Category = "VEIL|Health") float GetHealth() const { return Health; }
	UFUNCTION(BlueprintPure, Category = "VEIL|Health") float GetMaxHealth() const { return MaxHealth; }
	virtual float TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	UFUNCTION(BlueprintImplementableEvent, Category = "VEIL") void OnHealthChanged(float NewHealth, float MaxHealthValue);

	// Cosmetic/gameplay hooks for Blueprints
	UFUNCTION(BlueprintImplementableEvent, Category = "VEIL") void OnAttack();
	UFUNCTION(BlueprintImplementableEvent, Category = "VEIL") void OnDash();
	UFUNCTION(BlueprintImplementableEvent, Category = "VEIL") void OnInteract(AActor* Target);

protected:
	UPROPERTY(VisibleAnywhere, Category = "VEIL") USpringArmComponent* SpringArm;
	UPROPERTY(VisibleAnywhere, Category = "VEIL") UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "VEIL|Move") float WalkSpeed = 450.f;
	UPROPERTY(EditAnywhere, Category = "VEIL|Move") float SprintSpeed = 800.f;
	UPROPERTY(EditAnywhere, Category = "VEIL|Move") float DashImpulse = 1800.f;
	UPROPERTY(EditAnywhere, Category = "VEIL|Move") float InteractRange = 250.f;
	UPROPERTY(EditAnywhere, Category = "VEIL|Combat") float AttackRange = 220.f;
	UPROPERTY(EditAnywhere, Category = "VEIL|Combat") float AttackDamage = 25.f;

	// ATP regained per second between actions.
	UPROPERTY(EditAnywhere, Category = "VEIL|ATP") float RegenPerSecond = 0.f; // 0 keeps the exact proof values 88/70/62; raise to regen

private:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void Turn(float Value);
	void SprintOn();
	void SprintOff();
	void DoDash();
	void DoAttack();
	void DoInteract();
	bool Spend(float Cost);
};
