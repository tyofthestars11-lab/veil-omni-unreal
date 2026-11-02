#include "Character/VEILCountessCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "VEIL.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "CollisionQueryParams.h"
#include "Systems/VEILInteractable.h"
#include "Systems/VEILEnemy.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

AVEILCountessCharacter::AVEILCountessCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule sits on the floor; mesh is lowered to the capsule bottom and faces forward.
	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	{ // PHI COUNTESS: her own body, crown, core, lance, cape
		// HER BODY IS OURS. It was the Paragon Countess because that tree was the only complete
		// humanoid on disk at the time; the Polyart set is complete now and is not a placeholder.
		if (USkeletalMesh* Body = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Polyart/SharedResources/Characters/Meshes/SkeletalMeshes/SK_Quinn_Simple.SK_Quinn_Simple")))
		{
			GetMesh()->SetSkeletalMesh(Body);
			// Bone-driven body: keep it out of FMorphVertexBufferPool (assert :336).
			GetMesh()->bDisableMorphTarget = true;
		}
		UStaticMesh* Cyl = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		UStaticMesh* Sph = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
		UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
		UMaterialInterface* Gold = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold"));
		UMaterialInterface* White = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive"));
		UMaterialInterface* Blue = LoadObject<UMaterialInterface>(nullptr, TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue"));
		int32 TrimN = 0;
		auto Trim = [this, &TrimN](UStaticMesh* M, UMaterialInterface* Mat, const FVector& S, const FVector& At)
		{
			UStaticMeshComponent* C = CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("Trim%d"), TrimN++));
			if (!C) return;
			C->SetupAttachment(GetMesh());
			if (M) { C->SetStaticMesh(M); }
			if (Mat) { C->SetMaterial(0, Mat); }
			C->SetRelativeScale3D(S);
			C->SetRelativeLocation(At);
		};
		Trim(Cyl, Gold, FVector(0.22f, 0.22f, 0.10f), FVector(0.f, 0.f, 158.f));
		Trim(Sph, White, FVector(0.08f, 0.08f, 0.08f), FVector(0.f, 8.f, 120.f));
		Trim(Cyl, Gold, FVector(0.06f, 0.06f, 1.60f), FVector(25.f, 0.f, 90.f));
		Trim(Cube, Blue, FVector(0.25f, 0.70f, 1.10f), FVector(-16.f, 0.f, 105.f));
		UE_LOG(LogVEIL, Log, TEXT("PHI COUNTESS: crowned, lit, lanced, caped."));
	}
	// SM_Countess is assigned on the Blueprint child (BP_Countess) or here via SetSkeletalMesh.

	bUseControllerRotationYaw = false;
	UCharacterMovementComponent* Move = GetCharacterMovement();
	Move->bOrientRotationToMovement = true;
	Move->RotationRate = FRotator(0.f, 540.f, 0.f);
	Move->MaxWalkSpeed = WalkSpeed;
	Move->JumpZVelocity = 600.f;
	Move->AirControl = 0.35f;
	Move->GravityScale = 1.6f;
	Move->SetMovementMode(MOVE_Walking); // stays on the floor
	JumpMaxCount = 2;                    // double jump

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 450.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->bEnableCameraLag = true;
	// Golden pitch from the engine: 17.0324 degrees downward.
	SpringArm->SetRelativeRotation(FRotator(-Engine.GoldenPitch, 0.f, 0.f));

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
}

void AVEILCountessCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	RunGuardBrain(DeltaSeconds);
	// Optional regen between actions (off by default so the proof values stay exact).
	if (RegenPerSecond > 0.f && Engine.ATPGradient < 100.f)
	{
		Engine.ATPGradient = FMath::Min(100.f, Engine.ATPGradient + RegenPerSecond * DeltaSeconds);
	}
}

void AVEILCountessCharacter::SetupPlayerInputComponent(UInputComponent* In)
{
	Super::SetupPlayerInputComponent(In);
	// Create these in Project Settings > Input (see README_UNREAL_PLAYER.md).
	In->BindAxis("MoveForward", this, &AVEILCountessCharacter::MoveForward);
	In->BindAxis("MoveRight", this, &AVEILCountessCharacter::MoveRight);
	In->BindAxis("LookUp", this, &AVEILCountessCharacter::LookUp);
	In->BindAxis("Turn", this, &AVEILCountessCharacter::Turn);
	In->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	In->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	In->BindAction("Sprint", IE_Pressed, this, &AVEILCountessCharacter::SprintOn);
	In->BindAction("Sprint", IE_Released, this, &AVEILCountessCharacter::SprintOff);
	In->BindAction("Dash", IE_Pressed, this, &AVEILCountessCharacter::DoDash);
	In->BindAction("Attack", IE_Pressed, this, &AVEILCountessCharacter::DoAttack);
	In->BindAction("Interact", IE_Pressed, this, &AVEILCountessCharacter::DoInteract);
}

void AVEILCountessCharacter::MoveForward(float V)
{
	if (Controller && V != 0.f)
	{
		const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
		AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::X), V);
	}
}
void AVEILCountessCharacter::MoveRight(float V)
{
	if (Controller && V != 0.f)
	{
		const FRotator Yaw(0.f, Controller->GetControlRotation().Yaw, 0.f);
		AddMovementInput(FRotationMatrix(Yaw).GetUnitAxis(EAxis::Y), V);
	}
}
void AVEILCountessCharacter::LookUp(float V) { AddControllerPitchInput(V); }
void AVEILCountessCharacter::Turn(float V) { AddControllerYawInput(V); }
void AVEILCountessCharacter::SprintOn() { GetCharacterMovement()->MaxWalkSpeed = SprintSpeed; }
void AVEILCountessCharacter::SprintOff() { GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; }

// Pump protons for an action. Returns false (and does nothing) if not enough energy.
bool AVEILCountessCharacter::Spend(float Cost)
{
	if (Engine.ATPGradient < Cost) { return false; }
	Engine.PumpProtons(Cost); // 100 -> 88 (dash) -> 70 (attack) -> 62 (interact)
	return true;
}

void AVEILCountessCharacter::DoDash()
{
	if (!Spend(DashCost)) { return; }
	FVector Dir = GetLastMovementInputVector();
	if (Dir.IsNearlyZero()) { Dir = GetActorForwardVector(); }
	LaunchCharacter(Dir.GetSafeNormal() * DashImpulse, true, false);
	OnDash();
}

void AVEILCountessCharacter::DoAttack()
{
	if (!Spend(AttackCost)) { return; }
	FVector AimDir = GetActorForwardVector();
	{ AVEILEnemy* Best = nullptr; float BestD = AttackRange + 150.f;
	  for (TActorIterator<AVEILEnemy> It(GetWorld()); It; ++It) { if (It->IsActorBeingDestroyed()) continue;
	    const float D = FVector::Dist(GetActorLocation(), It->GetActorLocation());
	    if (D < BestD) { BestD = D; Best = *It; } }
	  if (Best) { AimDir = ((Best->GetActorLocation() + FVector(0.f, 0.f, 60.f)) - (GetActorLocation() + FVector(0.f, 0.f, 90.f))).GetSafeNormal(); } }
	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, 90.f);
	const FVector End = Start + AimDir * (AttackRange + 150.f);
	FHitResult Hit;
	FCollisionQueryParams P(SCENE_QUERY_STAT(CountessAttack), false, this);
	if (GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(60.f), P) && Hit.GetActor())
	{
		const float Dealt = Hit.GetActor()->TakeDamage(AttackDamage, FDamageEvent(), GetController(), this);
		if (Dealt > 0.f) { UE_LOG(LogTemp, Display, TEXT("PHI STRIKE HIT %s for %.0f"), *Hit.GetActor()->GetName(), Dealt); }
	}
	OnAttack();
}

void AVEILCountessCharacter::DoInteract()
{
	if (!Spend(InteractCost)) { return; }
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * InteractRange;
	FHitResult Hit;
	FCollisionQueryParams P(SCENE_QUERY_STAT(CountessInteract), false, this);
	AActor* Target = nullptr;
	if (GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(80.f), P))
	{
		Target = Hit.GetActor();
	}
	if (Target && Target->Implements<UVEILVaultInteractable>())
	{
		IVEILVaultInteractable::Execute_Interact(Target, this);
	}
	OnInteract(Target);
}

float AVEILCountessCharacter::TakeDamage(float Damage, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float Applied = Super::TakeDamage(Damage, DamageEvent, EventInstigator, DamageCauser);
    if (Applied > 0.f)
    {
        Health = FMath::Clamp(Health - Applied, 0.f, MaxHealth);
        OnHealthChanged(Health, MaxHealth);
    }
    return Applied;
}



// Countess NPC brain: guard the plaza ring. Faces the player, greets on
// approach, patrols waypoints on the NavMesh, engages enemies in range.
// Runs only for unpossessed instances (the seated NPC, not a player).
void AVEILCountessCharacter::RunGuardBrain(float DeltaSeconds)
{
	if (Controller && Controller->IsPlayerController()) return;
	if (!Controller) SpawnDefaultController();
	AAIController* AI = Cast<AAIController>(Controller);
	if (!AI) return;

	BrainClock += DeltaSeconds;
	if (Engine.ATPGradient < AttackCost) { Engine.ATPGradient = FMath::Min(100.f, Engine.ATPGradient + 25.f * DeltaSeconds); }

	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	AVEILEnemy* Foe = nullptr;
	float FoeDist = 1e9f;
	for (TActorIterator<AVEILEnemy> It(GetWorld()); It; ++It)
	{
		if (It->IsActorBeingDestroyed() || It->Health <= 0.f) continue;
		const float D = FVector::Dist2D(GetActorLocation(), It->GetActorLocation());
		if (D < FoeDist) { FoeDist = D; Foe = *It; }
	}

	if (BrainClock < 0.5f) return;
	BrainClock = 0.f;
	const FVector Here = GetActorLocation();

	if (Foe && FoeDist < 1400.f)
	{
		FVector To = Foe->GetActorLocation() - Here; To.Z = 0.f;
		if (!To.IsNearlyZero()) SetActorRotation(FRotator(0.f, To.Rotation().Yaw, 0.f));
		if (FoeDist > AttackRange)
		{
			AI->MoveToActor(Foe, AttackRange * 0.7f);
		}
		else
		{
			AI->StopMovement();
			if (GetWorld()->GetTimeSeconds() - LastBrainAttack > 1.2f)
		{
			FVector Lunge = Foe->GetActorLocation() - Here; Lunge.Z = 0.f;
			if (!Lunge.IsNearlyZero()) { LaunchCharacter(Lunge.GetSafeNormal() * 380.f + FVector(0.f, 0.f, 120.f), true, true); }
				LastBrainAttack = GetWorld()->GetTimeSeconds();
				DoAttack();
				UE_LOG(LogTemp, Display, TEXT("PHI COUNTESS: engages %s"), *Foe->GetName());
			}
		}
		bGreeted = false;
		return;
	}
	if (Player)
	{
		const float PD = FVector::Dist2D(Here, Player->GetActorLocation());
		if (PD < 900.f)
		{
			FVector To = Player->GetActorLocation() - Here; To.Z = 0.f;
			if (!To.IsNearlyZero()) SetActorRotation(FRotator(0.f, To.Rotation().Yaw, 0.f));
			AI->StopMovement();
			if (!bGreeted)
			{
				bGreeted = true;
				UE_LOG(LogTemp, Display, TEXT("PHI COUNTESS: Runner, the Vault hums - the fragments remember you. Walk into them."));
			}
			return;
		}
	}
	bGreeted = false;
	static const FVector Ring[4] = { FVector(700.f, 300.f, 150.f), FVector(100.f, 300.f, 150.f), FVector(100.f, -300.f, 150.f), FVector(700.f, -300.f, 150.f) };
	const FVector Goal = Ring[BrainWp % 4];
	if (FVector::Dist2D(Here, Goal) < 120.f) { BrainWp++; }
	else { AI->MoveToLocation(Goal, 80.f); }
}

