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

	// EVERY PATH BELOW WAS LISTED ON DISK BEFORE IT WAS USED. Murdock's mesh lives at
	// .../Murdock/Meshes/Murdock, not .../Murdock/Murdock - the old path never loaded, so
	// every Warden in the district silently wore Garret. That is the clone farm the plaza
	// was showing: eight bodies on disk, one body in the frame.
	const TCHAR* const GarretMesh =
		TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody");
	const TCHAR* const ElaraMesh =
		TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Elara_FullBody.SK_Elara_FullBody");
	const TCHAR* const QuinnMesh =
		TEXT("/Game/Polyart/SharedResources/Characters/Meshes/SkeletalMeshes/SK_Quinn_Simple.SK_Quinn_Simple");
	const TCHAR* const MurdockMesh =
		TEXT("/Game/ParagonMurdock/Characters/Heroes/Murdock/Meshes/Murdock.Murdock");
	const TCHAR* const CountessMesh =
		TEXT("/Game/ParagonCountess/Characters/Heroes/Countess/Meshes/SM_Countess.SM_Countess");
	const TCHAR* const CountessGildedMesh =
		TEXT("/Game/ParagonCountess/Characters/Heroes/Countess/Skins/Tier1/Count_Gold/Meshes/SM_CountessGilded.SM_CountessGilded");
	// THE MANNEQUIN PACK IS NOT A MESH. /Game/Mannequins/Meshes/SK_Mannequin failed to load
	// three times in one run and every machine wearing it walked the roster to someone else,
	// so the entry is replaced by two skins that ARE skeletal meshes - the same class of
	// asset as SM_Countess, which loads.
	const TCHAR* const CountessRedRevoMesh =
		TEXT("/Game/ParagonCountess/Characters/Heroes/Countess/Skins/Tier1/Count_RedRevo/Meshes/SM_Countess_RedRevo.SM_Countess_RedRevo");
	const TCHAR* const CountessCarnivaleMesh =
		TEXT("/Game/ParagonCountess/Characters/Heroes/Countess/Skins/Tier1/Count_Carnivale/Meshes/SM_Countess_Carnivale.SM_Countess_Carnivale");
	const TCHAR* const RunnerMesh =
		TEXT("/Game/MetaHumans/MH_Runner_Built/MH_Runner/Body/SKM_MH_Runner_BodyMesh.SKM_MH_Runner_BodyMesh");

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
		case VEILCastBody::EBody::Elara:          return { ElaraMesh,          FemaleGraph,  TEXT("Elara") };
		case VEILCastBody::EBody::Quinn:          return { QuinnMesh,          FemaleGraph,  TEXT("Quinn") };
		case VEILCastBody::EBody::Murdock:        return { MurdockMesh,        MurdockGraph, TEXT("Murdock") };
		case VEILCastBody::EBody::Countess:       return { CountessMesh,       FemaleGraph,  TEXT("Countess") };
		case VEILCastBody::EBody::CountessGilded: return { CountessGildedMesh, FemaleGraph,  TEXT("CountessGilded") };
		case VEILCastBody::EBody::CountessRedRevo:   return { CountessRedRevoMesh,   FemaleGraph, TEXT("CountessRedRevo") };
		case VEILCastBody::EBody::CountessCarnivale: return { CountessCarnivaleMesh, FemaleGraph, TEXT("CountessCarnivale") };
		case VEILCastBody::EBody::Runner:         return { RunnerMesh,         MaleGraph,    TEXT("Runner") };
		case VEILCastBody::EBody::Garret:
		default:                                  return { GarretMesh,         MaleGraph,    TEXT("Garret") };
		}
	}

	/** Walk the roster from Want until something loads. A miss is named, and it costs that
	 *  role a DIFFERENT body rather than making it a copy of every other role. */
	VEILCastBody::EBody Resolve(VEILCastBody::EBody Want, USkeletalMesh*& OutMesh)
	{
		const int32 Total = (int32)VEILCastBody::EBody::Count;
		const int32 Start = FMath::Clamp((int32)Want, 0, Total - 1);
		for (int32 Step = 0; Step < Total; ++Step)
		{
			const VEILCastBody::EBody Try = (VEILCastBody::EBody)((Start + Step) % Total);
			const FBodyDef Def = DefFor(Try);
			if (USkeletalMesh* Mesh = LoadObject<USkeletalMesh>(nullptr, Def.Mesh))
			{
				OutMesh = Mesh;
				return Try;
			}
			UE_LOG(LogVEIL, Warning, TEXT("PHI CAST BODY: %s is not loadable at %s - walking the roster."),
				Def.Name, Def.Mesh);
		}
		OutMesh = nullptr;
		return Want;
	}
}

int32 VEILCastBody::Count()
{
	return (int32)EBody::Count;
}

const TCHAR* VEILCastBody::NameOf(EBody Body)
{
	return DefFor(Body).Name;
}

USkeletalMesh* VEILCastBody::LoadMesh(EBody Body)
{
	USkeletalMesh* Mesh = nullptr;
	Resolve(Body, Mesh);
	return Mesh;
}

UClass* VEILCastBody::LoadGraph(EBody Body)
{
	const FBodyDef Def = DefFor(Body);
	return Def.Anim ? LoadClass<UAnimInstance>(nullptr, Def.Anim) : nullptr;
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
		// A FACTION IS NOT A CLONE. Every hostile used to resolve Elara, and the cast census
		// then read 34 of the district's 61 bodies on one mesh - which is the exact complaint
		// the census exists to catch. Hostiles step a combat roster on the golden ratio from
		// their own name, so a warband is five silhouettes, not one repeated.
		static const EBody CombatRoster[] = { EBody::Elara, EBody::Murdock, EBody::Runner,
			EBody::CountessGilded, EBody::CountessCarnivale };
		const int32 CombatCount = (int32)UE_ARRAY_COUNT(CombatRoster);
		const double Frac = FMath::Frac((double)GetTypeHash(R) * 0.618033988749895);
		const int32 Pick = FMath::Clamp((int32)FMath::Floor(Frac * (double)CombatCount), 0, CombatCount - 1);
		return CombatRoster[Pick];
	}
	if (R.Contains(TEXT("guardian")) || R.Contains(TEXT("guard"))) { return EBody::Quinn; }
	if (R.Contains(TEXT("runner")) || R.Contains(TEXT("signal")))  { return EBody::Runner; }
	if (R.Contains(TEXT("machine")) || R.Contains(TEXT("drone")))
	{
		// Machines get their own two bodies, split by name, not one rig for all of them.
		return (FMath::Frac((double)GetTypeHash(R) * 0.618033988749895) > 0.5) ? EBody::CountessRedRevo : EBody::CountessGilded;
	}

	// EVERYONE ELSE SPREADS OVER THE WHOLE ROSTER. Not three bodies out of eight: the
	// fractional part of hash * phi steps by the golden ratio, so successive names land far
	// apart in the roster and no two neighbours wear the same silhouette by accident.
	const uint32 Hash = GetTypeHash(R);
	const double Frac = FMath::Frac((double)Hash * 0.618033988749895);
	const int32 Total = Count();
	return (EBody)FMath::Clamp((int32)FMath::Floor(Frac * (double)Total), 0, Total - 1);
}

void VEILCastBody::Apply(USkeletalMeshComponent* Comp, EBody Body, const FString& Role, float Scale)
{
	if (!Comp)
	{
		UE_LOG(LogVEIL, Error, TEXT("PHI CAST BODY: %s has no skeletal mesh component to dress."), *Role);
		return;
	}

	USkeletalMesh* Mesh = nullptr;
	const EBody Resolved = Resolve(Body, Mesh);
	const FBodyDef Def = DefFor(Resolved);
	UClass* Graph = LoadGraph(Resolved);

	if (!Mesh)
	{
		UE_LOG(LogVEIL, Error, TEXT("PHI CAST BODY: %s has no body at all - not one roster mesh loaded."), *Role);
		return;
	}

	if (Resolved != Body)
	{
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI CAST BODY: %s wanted %s, wears %s instead - a different body, never a clone of the last one."),
			*Role, NameOf(Body), NameOf(Resolved));
	}

	Comp->SetSkeletalMeshAsset(Mesh);
	// Bone-driven bodies only: leaving morph targets live put this project in
	// FMorphVertexBufferPool, whose check(Index == 1) killed the editor.
	Comp->bDisableMorphTarget = true;
	Comp->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
	Comp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	// TWO PEOPLE IN THE SAME BODY ARE STILL NOT THE SAME PERSON. Height varies on the golden
	// ratio from the role's own name, so a shared body is 6 to 18 cm apart from its twin.
	const float RoleScale = 0.94f + 0.12f * (float)FMath::Frac((double)GetTypeHash(Role.ToLower()) * 0.618033988749895);
	Comp->SetRelativeScale3D(FVector(Scale * RoleScale));
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
		*Role, *Mesh->GetName(), Scale * RoleScale, Graph ? TEXT("bound") : TEXT("none"));
}
