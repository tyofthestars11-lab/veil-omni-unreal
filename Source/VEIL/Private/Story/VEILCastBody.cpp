#include "Story/VEILCastBody.h"

#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "VEIL.h"

namespace
{
	struct FBodyDef
	{
		const TCHAR* Mesh;
		const TCHAR* Anim;
		const TCHAR* Name;
	};

	// Paths copied from Content, not guessed: each one was listed on disk before it was used.
	const TCHAR* const GarretMesh =
		TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody");
	const TCHAR* const ElaraMesh =
		TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Elara_FullBody.SK_Elara_FullBody");
	const TCHAR* const QuinnMesh =
		TEXT("/Game/Polyart/SharedResources/Characters/Meshes/SkeletalMeshes/SK_Quinn_Simple.SK_Quinn_Simple");
	const TCHAR* const MurdockMesh =
		TEXT("/Game/ParagonMurdock/Characters/Heroes/Murdock/Murdock.Murdock");
	const TCHAR* const CountessMesh =
		TEXT("/Game/ParagonCountess/Characters/Heroes/Countess/Meshes/SM_Countess.SM_Countess");

	const TCHAR* const MaleGraph =
		TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/AnimationBlueprints/ABP_MaleCharacter.ABP_MaleCharacter_C");
	const TCHAR* const FemaleGraph =
		TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/AnimationBlueprints/ABP_FemaleCharacter.ABP_FemaleCharacter_C");
	const TCHAR* const MurdockGraph =
		TEXT("/Game/ParagonMurdock/Characters/Heroes/Murdock/Murdock_AnimBlueprint.Murdock_AnimBlueprint_C");

	FBodyDef DefFor(VEILCastBody::EBody Body)
	{
		switch (Body)
		{
		case VEILCastBody::EBody::Elara:    return { ElaraMesh,    FemaleGraph,  TEXT("Elara") };
		case VEILCastBody::EBody::Quinn:    return { QuinnMesh,    FemaleGraph,  TEXT("Quinn") };
		case VEILCastBody::EBody::Murdock:  return { MurdockMesh,  MurdockGraph, TEXT("Murdock") };
		case VEILCastBody::EBody::Countess: return { CountessMesh, FemaleGraph,  TEXT("Countess") };
		case VEILCastBody::EBody::Garret:
		default:                            return { GarretMesh,   MaleGraph,    TEXT("Garret") };
		}
	}
}

const TCHAR* VEILCastBody::NameOf(EBody Body)
{
	return DefFor(Body).Name;
}

USkeletalMesh* VEILCastBody::LoadMesh(EBody Body)
{
	FBodyDef Def = DefFor(Body);
	USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, Def.Mesh);
	if (!Mesh && Body != EBody::Garret)
	{
		Mesh = LoadObject<USkeletalMesh>(nullptr, GarretMesh);
	}
	return Mesh;
}

VEILCastBody::EBody VEILCastBody::ForRole(const FString& Role)
{
	const FString R = Role.ToLower();

	// THE AUTHORITY, BY NAME. Fixed bodies, so the Keeper is never the Seer on a later boot.
	if (R.Contains(TEXT("keeper")))                            { return EBody::Quinn; }
	if (R.Contains(TEXT("seer")) || R.Contains(TEXT("ilva")))  { return EBody::Elara; }
	if (R.Contains(TEXT("cartographer")))                      { return EBody::Garret; }
	if (R.Contains(TEXT("countess")))                          { return EBody::Countess; }
	if (R.Contains(TEXT("warden")))                            { return EBody::Murdock; }
	if (R.Contains(TEXT("remnant")) || R.Contains(TEXT("enemy")) || R.Contains(TEXT("hostile")))
	{
		return EBody::Elara;
	}
	if (R.Contains(TEXT("guardian")) || R.Contains(TEXT("guard"))) { return EBody::Quinn; }

	// EVERYONE ELSE ON THE GOLDEN ANGLE. Different names land on different bodies, and the
	// result is stable across runs, so a crowd is not one silhouette repeated.
	const uint32 Hash = GetTypeHash(R);
	const int32 Bucket = FMath::Abs((int32)FMath::Floor(FMath::Frac((double)Hash * 0.618033988749895) * 3.0));
	switch (Bucket)
	{
	case 0:  return EBody::Garret;
	case 1:  return EBody::Elara;
	default: return EBody::Quinn;
	}
}

void VEILCastBody::Apply(USkeletalMeshComponent* Comp, EBody Body, const FString& Role, float Scale)
{
	if (!Comp)
	{
		UE_LOG(LogVEIL, Error, TEXT("PHI CAST BODY: %s has no skeletal mesh component to dress."), *Role);
		return;
	}

	FBodyDef Def = DefFor(Body);
	USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, Def.Mesh);
	UClass* Graph = Def.Anim ? LoadClass<UAnimInstance>(nullptr, Def.Anim) : nullptr;

	// A FAILED LOAD IS A NAMED MISS, NOT A SILENT CLONE. If the chosen body is not on disk the
	// character falls back to a body this project does own, and the log says which one and why.
	if (!Mesh && Body != EBody::Garret)
	{
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI CAST BODY: %s wanted %s but that mesh is not loadable - falling back to Garret."),
			*Role, Def.Name);
		Def = DefFor(EBody::Garret);
		Mesh = LoadObject<USkeletalMesh>(nullptr, Def.Mesh);
		Graph = Def.Anim ? LoadClass<UAnimInstance>(nullptr, Def.Anim) : nullptr;
	}

	if (!Mesh)
	{
		UE_LOG(LogVEIL, Error, TEXT("PHI CAST BODY: %s has no body at all - no Polyart mesh loaded."), *Role);
		return;
	}

	Comp->SetSkeletalMeshAsset(Mesh);
	// Bone-driven bodies only: leaving morph targets live put this project in
	// FMorphVertexBufferPool, whose check(Index == 1) killed the editor.
	Comp->bDisableMorphTarget = true;
	Comp->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
	Comp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	Comp->SetRelativeScale3D(FVector(Scale));
	Comp->SetVisibility(true);

	if (Graph)
	{
		Comp->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		Comp->SetAnimInstanceClass(Graph);
	}
	else
	{
		// No locomotion graph for this rig: still a real body, held in its bind pose rather
		// than pretending to walk on someone else's animation.
		Comp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI CAST BODY: %s wears %s with NO locomotion graph - standing pose, named honestly."),
			*Role, Def.Name);
	}

	UE_LOG(LogVEIL, Warning, TEXT("PHI CAST BODY: %s -> %s (scale %.2f, graph %s)."),
		*Role, *Mesh->GetName(), Scale, Graph ? TEXT("bound") : TEXT("none"));
}
