// OMNI RPG: PATH OF LIGHT - WARDEN PRIME implementation.
#include "AI/OMNIWardenPrime.h"
#include "VEIL.h"

#include "GameFramework/CharacterMovementComponent.h"

AOMNIWardenPrime::AOMNIWardenPrime()
{
	// The duel: twelve times a street machine's frame, a heavier hand,
	// a longer eye. Spawned at 1.7 scale by the quest system.
	MaxHealth = 900.f;
	Health = MaxHealth;
	WeaponDamage = 18.f;
	SightRange = 3200.f;
	AttackRange = 1600.f;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
}

void AOMNIWardenPrime::BeginPlay()
{
	Super::BeginPlay();
	// The Warden wears the ChronoBoss (job 492): the boss body over the
	// agent frame, seated after the base bind so it stands last.
	// PARAGON OUT. The Warden wore Revenant's ChronoBoss skin - Fab padding for a body we
	// already own. He wears the project's own Polyart character and locomotion graph now.
	USkeletalMesh* Boss = LoadObject<USkeletalMesh>(nullptr,
		TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody"));
	UClass* BossAnim = LoadClass<UAnimInstance>(nullptr,
		TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/AnimationBlueprints/ABP_MaleCharacter.ABP_MaleCharacter_C"));
	if (Boss)
	{
		GetMesh()->SetSkeletalMeshAsset(Boss);
		// Bone-driven body: keep it out of FMorphVertexBufferPool (assert :336).
		GetMesh()->bDisableMorphTarget = true;
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		if (BossAnim) GetMesh()->SetAnimInstanceClass(BossAnim);
		UE_LOG(LogVEIL, Log, TEXT("PHI WARDEN BODY: %s stands - a body this project owns, not a Fab stand-in."), *Boss->GetName());
	}
	else
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI WARDEN BODY: Polyart body not found - agent body stands."));
	}
}

