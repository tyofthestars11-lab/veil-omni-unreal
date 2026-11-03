#pragma once

#include "CoreMinimal.h"

class USkeletalMesh;
class USkeletalMeshComponent;
class UClass;

/**
 * THE CAST HAS BODIES.
 *
 * Before this, every NPC in the district loaded SK_Garret_FullBody - the runner's own mesh -
 * so the plaza was the same man standing at every marker, and AOMNIQuestGiver was a cylinder
 * with a sphere on top. That is not a cast; it is a clone farm.
 *
 * This resolves a real body per role from the packs that are actually installed in Content,
 * gives it that body's own locomotion graph, and logs which mesh each character received, so
 * a missing asset shows up as a named miss instead of silently becoming another clone.
 *
 * What is installed, measured on disk:
 *   /Game/Polyart/.../SK_Garret_FullBody          + ABP_MaleCharacter     (male)
 *   /Game/Polyart/.../SK_Elara_FullBody           + ABP_FemaleCharacter   (female)
 *   /Game/Polyart/.../SK_Quinn_Simple             + ABP_FemaleCharacter   (female, third silhouette)
 *   /Game/ParagonMurdock/.../Meshes/Murdock       + Murdock_AnimBlueprint (armoured male)
 *   /Game/ParagonCountess/.../SM_Countess         + ABP_FemaleCharacter   (heroine)
 *   /Game/ParagonCountess/.../SM_CountessGilded   + ABP_FemaleCharacter   (heroine, gilded skin)
 *   /Game/ParagonCountess/.../SM_Countess_RedRevo + ABP_FemaleCharacter  (heroine, red skin)
 *   /Game/ParagonCountess/.../SM_Countess_Carnivale+ ABP_FemaleCharacter  (heroine, carnival skin)
 *   /Game/MetaHumans/.../SKM_MH_Runner_BodyMesh   + ABP_MaleCharacter     (photoreal body)
 *
 * EIGHT BODIES, NOT ONE. A role that cannot load its own body walks the roster from its
 * requested entry until something loads, so a broken path costs that role a different body
 * rather than making it a copy of every other role in the district.
 */
namespace VEILCastBody
{
	enum class EBody : uint8
	{
		Garret = 0,
		Elara,
		Quinn,
		Murdock,
		Countess,
		CountessGilded,
		CountessRedRevo,
		CountessCarnivale,
		Runner,
		Count       // not a body: the size of the roster
	};

	/** How many bodies the roster holds. */
	VEIL_API int32 Count();

	/** Deterministic body for a named role: the same name always gets the same body, so the
	 *  Keeper is not the Seer on the next boot. Names without an authority spread across the
	 *  whole roster on the golden ratio. */
	VEIL_API EBody ForRole(const FString& Role);

	/** Load the body, walking the roster forward from the requested entry if it is missing,
	 *  so a failed load yields a different body instead of the same fallback for everyone. */
	VEIL_API USkeletalMesh* LoadMesh(EBody Body);

	/** The locomotion graph that belongs to a body, or nullptr if the pack ships none. */
	VEIL_API UClass* LoadGraph(EBody Body);

	/** Load the body and its graph, apply it, and log the resolved asset by name. A role's
	 *  height is varied deterministically on the golden ratio, so two characters sharing a
	 *  body are still not the same silhouette at the same size. */
	VEIL_API void Apply(USkeletalMeshComponent* Comp, EBody Body, const FString& Role, float Scale = 1.f);

	/** Short name for the log: "Garret", "Elara", ... */
	VEIL_API const TCHAR* NameOf(EBody Body);
}
