#include "VEILEnemy.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

AVEILEnemy::AVEILEnemy()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AVEILEnemy::ConfigureForKind(EVEILEnemyKind InKind)
{
	Kind = InKind;
	UCharacterMovementComponent* M = GetCharacterMovement();
	switch (Kind)
	{
	case EVEILEnemyKind::Drone:
		MaxHealth = 60.f; AttackDamage = 8.f; MoveSpeed = 420.f;
		M->SetMovementMode(MOVE_Flying); // drones fly straight at her
		break;
	case EVEILEnemyKind::Soldier:
		MaxHealth = 140.f; AttackDamage = 14.f; MoveSpeed = 340.f;
		break;
	case EVEILEnemyKind::WardenPrime:
		MaxHealth = 2500.f; AttackDamage = 35.f; MoveSpeed = 300.f;
		AttackRange = 300.f; AttackCooldown = 1.6f;
		SetActorScale3D(FVector(2.2f));
		break;
	}
	M->MaxWalkSpeed = MoveSpeed;
	M->MaxFlySpeed = MoveSpeed;
	Health = MaxHealth;
}

void AVEILEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bDead) { return; }

	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player) { return; }

	const FVector To = Player->GetActorLocation() - GetActorLocation();
	const float Dist = To.Size();

	if (Dist > AttackRange * 0.8f)
	{
		if (Kind == EVEILEnemyKind::Drone)
		{
			AddMovementInput(To.GetSafeNormal(), 1.f);
		}
		else
		{
			RepathTimer -= DeltaSeconds;
			if (RepathTimer <= 0.f)
			{
				RepathTimer = 0.4f;
				if (AAIController* C = Cast<AAIController>(GetController()))
				{
					C->MoveToActor(Player, AttackRange * 0.7f); // follows the NavMesh
				}
			}
		}
	}

	AttackTimer -= DeltaSeconds;
	if (Dist <= AttackRange && AttackTimer <= 0.f)
	{
		AttackTimer = AttackCooldown;
		UGameplayStatics::ApplyDamage(Player, AttackDamage, GetController(), this, nullptr);
	}
}

float AVEILEnemy::TakeDamage(float Damage, const FDamageEvent& Event, AController* By, AActor* Causer)
{
	const float Applied = Super::TakeDamage(Damage, Event, By, Causer);
	if (bDead || Applied <= 0.f) { return Applied; }
	Health -= Applied;
	if (Health <= 0.f) { Die(); }
	return Applied;
}

void AVEILEnemy::Die()
{
	if (bDead) { return; }
	bDead = true;
	OnEnemyDied.Broadcast(this);
	SetLifeSpan(0.2f);
}
