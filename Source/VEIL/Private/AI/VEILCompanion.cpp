// VEIL companion implementation: follow and fight beside the Runner.
#include "AI/VEILCompanion.h"

#include "AI/OMNIWardenPrime.h"
#include "AIController.h"
#include "Animation/AnimSequence.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/VEILEnemy.h"
#include "VEIL.h"

AVEILCompanion::AVEILCompanion(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
	GetCharacterMovement()->MaxWalkSpeed = 620.f;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	// Proven foe pattern: runtime load, silent on miss, never a dialog.
	if (USkeletalMesh* M = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Meshes/Minion_Lane_Melee_Dawn.Minion_Lane_Melee_Dawn")))
	{
		GetMesh()->SetSkeletalMesh(M);
		// Bone-driven body: keep it out of FMorphVertexBufferPool (assert :336).
		GetMesh()->bDisableMorphTarget = true;
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	}
	IdleSeq = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/NonCombat_Idle.NonCombat_Idle"));
	JogSeq = LoadObject<UAnimSequence>(nullptr, TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/NonCombat_JogFwd.NonCombat_JogFwd"));
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationSingleNode);
	if (IdleSeq) { GetMesh()->SetAnimation(IdleSeq); GetMesh()->PlayAnimation(IdleSeq, true); }
}

void AVEILCompanion::ConfigureForRole(EVEILCompanionRole InRole, FName InCallSign)
{
	CompanionRole = InRole;
	CallSign = InCallSign;
	FVector Scale(1.f);
	switch (CompanionRole)
	{
	case EVEILCompanionRole::Sage:
		MaxHealth = 120.f; Damage = 14.f; AttackCooldown = 1.3f;
		FollowDistance = 420.f; Scale = FVector(1.05f); break;
	case EVEILCompanionRole::Drone:
		MaxHealth = 70.f; Damage = 8.f; AttackCooldown = 0.7f;
		FollowDistance = 260.f; Scale = FVector(0.62f);
		BaseMoveSpeed = 760.f; break;
	case EVEILCompanionRole::Guard:
		MaxHealth = 160.f; Damage = 12.f; AttackCooldown = 1.1f;
		FollowDistance = 350.f; Scale = FVector(1.18f); break;
	case EVEILCompanionRole::Scout:
		MaxHealth = 90.f; Damage = 10.f; AttackCooldown = 0.9f;
		FollowDistance = 300.f; Scale = FVector(0.9f);
		BaseMoveSpeed = 720.f; break;
	case EVEILCompanionRole::Bulwark:
		MaxHealth = 220.f; Damage = 16.f; AttackCooldown = 1.5f;
		FollowDistance = 380.f; Scale = FVector(1.3f);
		BaseMoveSpeed = 520.f; break;
	}
	Health = MaxHealth;
	SetActorScale3D(Scale);
	UE_LOG(LogVEIL, Log, TEXT("PHI COMPANION: %s joins the party (%d)."),
		*CallSign.ToString(), (int32)CompanionRole + 1);
}

void AVEILCompanion::BeginPlay()
{
	Super::BeginPlay();
}

AActor* AVEILCompanion::FindFoe() const
{
	APawn* Leader = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Leader) return nullptr;
	const FVector Home = Leader->GetActorLocation();
	AActor* Best = nullptr;
	float BestD = 1800.f * 1800.f;
	for (TActorIterator<AVEILEnemy> It(GetWorld()); It; ++It)
	{
		if (It->IsActorBeingDestroyed() || It->Health <= 0.f) continue;
		const float dd = FVector::DistSquared(Home, It->GetActorLocation());
		if (dd < BestD) { BestD = dd; Best = *It; }
	}
	for (TActorIterator<AOMNIWardenPrime> It(GetWorld()); It; ++It)
	{
		if (It->IsActorBeingDestroyed() || !It->IsAlive()) continue;
		const float dd = FVector::DistSquared(Home, It->GetActorLocation());
		if (dd < BestD) { BestD = dd; Best = *It; }
	}
	return Best;
}

void AVEILCompanion::PhiUpdate(float DeltaSeconds)
{
	PhiClock.Advance(DeltaSeconds);
	PhiPhase = PhiClock.Phase;

	const float Off2 = UPHIMito::PhiPow(-1.f);
	const float Off3 = UPHIMito::PhiPow(-2.f);
	const float Off4 = UPHIMito::PhiPow(-3.f);

	LiveDamage        = PhiClock.Value(Damage,         0.f,  0.75f);
	LiveAttackRange   = PhiClock.Value(AttackRange,    Off2, 0.5f);
	LiveAttackCooldown = PhiClock.Value(AttackCooldown, Off3, 0.5f);
	LiveFollowDistance = PhiClock.Value(FollowDistance, Off4, 0.5f);
	LiveMoveSpeed     = PhiClock.Value(BaseMoveSpeed,   Off2 + Off4, 1.f);

	PhiRoundTripError = FPhiDynamic::RoundTripError(LiveMoveSpeed);

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->MaxWalkSpeed = LiveMoveSpeed;
	}

	PhiLogTimer += DeltaSeconds;
	if (PhiLogTimer >= 5.f && PhiLogCount < 6)
	{
		PhiLogTimer = 0.f;
		++PhiLogCount;
		UE_LOG(LogVEIL, Log,
			TEXT("PHI AI LIVE %s phase=%.4f -> spd=%.1f dmg=%.2f range=%.1f cd=%.2f follow=%.1f round-trip=%.6f"),
			*GetActorNameOrLabel(), PhiPhase, LiveMoveSpeed, LiveDamage, LiveAttackRange, LiveAttackCooldown, LiveFollowDistance, PhiRoundTripError);
	}
}

void AVEILCompanion::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bDead || !IsAlive()) return;

	PhiUpdate(DeltaSeconds);

	APawn* Leader = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Leader) return;
	AAIController* AI = Cast<AAIController>(GetController());
	if (!AI) return;

	AActor* Foe = FindFoe();
	if (Foe)
	{
		const float D = FVector::Dist(GetActorLocation(), Foe->GetActorLocation());
		if (D > LiveAttackRange)
		{
			AI->MoveToActor(Foe, 60.f);
		}
		else
		{
			AttackTimer -= DeltaSeconds;
			if (AttackTimer <= 0.f)
			{
				AttackTimer = LiveAttackCooldown;
				Foe->TakeDamage(LiveDamage, FDamageEvent(), GetController(), this);
			}
		}
		return;
	}
	// AAA follower rule: never lost - walking distance we chase, beyond reach we appear.
	const float FarD = FVector::Dist(GetActorLocation(), Leader->GetActorLocation());
	if (FarD > 3000.f)
	{
		const FVector Behind = Leader->GetActorLocation() - Leader->GetActorForwardVector() * LiveFollowDistance;
		SetActorLocation(Behind + FVector(0.f, 0.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
		UE_LOG(LogVEIL, Log, TEXT("PHI COMPANION: %s catches up to the Runner."), *CallSign.ToString());
		return;
	}
	const FVector Anchor = Leader->GetActorLocation() - Leader->GetActorForwardVector() * LiveFollowDistance;
	if (FVector::Dist2D(GetActorLocation(), Anchor) > 90.f)
	{
		AI->MoveToLocation(Anchor, 40.f);
	}
	if (USkeletalMeshComponent* Solo = GetMesh())
	{
		UAnimSequence* Want = (GetVelocity().Size2D() > 60.f && JogSeq) ? JogSeq : IdleSeq;
		if (Want && CurAnim != Want) { CurAnim = Want; Solo->PlayAnimation(Want, true); }
	}
}

float AVEILCompanion::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (bDead || !IsAlive()) return 0.f;
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health = FMath::Clamp(Health - Applied, 0.f, MaxHealth);
	if (Health <= 0.f) { Die(); }
	return Applied;
}

void AVEILCompanion::Die()
{
	if (bDead) return;
	bDead = true;
	UE_LOG(LogVEIL, Log, TEXT("PHI COMPANION DOWN: %s falls - the party walks on."), *CallSign.ToString());
	SetLifeSpan(4.f);
}

