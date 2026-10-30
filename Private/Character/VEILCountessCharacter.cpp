#include "VEILCountessCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "CollisionQueryParams.h"
#include "VEILInteractable.h"

AVEILCountessCharacter::AVEILCountessCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Capsule sits on the floor; mesh is lowered to the capsule bottom and faces forward.
	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
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
	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * AttackRange;
	FHitResult Hit;
	FCollisionQueryParams P(SCENE_QUERY_STAT(CountessAttack), false, this);
	if (GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity, ECC_Pawn,
		FCollisionShape::MakeSphere(60.f), P) && Hit.GetActor())
	{
		Hit.GetActor()->TakeDamage(AttackDamage, FDamageEvent(), GetController(), this);
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
	if (Target && Target->Implements<UVEILInteractable>())
	{
		IVEILInteractable::Execute_Interact(Target, this);
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
