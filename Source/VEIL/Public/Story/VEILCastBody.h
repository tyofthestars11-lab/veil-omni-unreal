#pragma once

#include "CoreMinimal.h"

class USkeletalMesh;
class USkeletalMeshComponent;

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
 * What is installed, measured:
 *   /Game/Polyart/.../SK_Garret_FullBody         + ABP_MaleCharacter     (male)
 *   /Game/Polyart/.../SK_Elara_FullBody          + ABP_FemaleCharacter   (female)
 *   /Game/Polyart/.../SK_Quinn_Simple            + ABP_FemaleCharacter   (female, third silhouette)
 *   /Game/ParagonMurdock/.../Murdock             + Murdock_AnimBlueprint (armoured male)
 *   /Game/ParagonCountess/.../SM_Countess        + no graph of its own   (heroine, static pose)
 */
namespace VEILCastBody
{
	enum class EBody : uint8
	{
		Garret = 0,
		Elara,
		Quinn,
		Murdock,
		Countess
	};

	/** Deterministic body for a named role: the same name always gets the same body, so the
	 *  Keeper is not the Seer on the next boot. */
	VEIL_API EBody ForRole(const FString& Role);

	/** Load just the mesh for a body, with the same Garret fallback. Used where a
	 *  pawn already decides its own animation instance. */
	VEIL_API USkeletalMesh* LoadMesh(EBody Body);

	/** Load the body and its graph, apply it, and log the resolved asset by name. Falls back
	 *  to a body this project owns rather than leaving a bare capsule. */
	VEIL_API void Apply(USkeletalMeshComponent* Comp, EBody Body, const FString& Role, float Scale = 1.f);

	/** Short name for the log: "Garret", "Elara", ... */
	VEIL_API const TCHAR* NameOf(EBody Body);
}
