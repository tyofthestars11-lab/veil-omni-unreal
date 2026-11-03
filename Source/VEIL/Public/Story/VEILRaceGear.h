#pragma once

#include "CoreMinimal.h"
#include "AI/VEILFactionAI.h"

class AActor;
class USkeletalMeshComponent;

/**
 * RACES AND ROLES, READ AT A DISTANCE.
 *
 * The lineup is built on silhouette: the Runner has gold phi wings and a dark cape,
 * the Keeper carries a lantern staff, the Seer a crystal staff, the Cartographer a
 * map, the Countess wears gold, and the district's peoples are marked by what they
 * carry - crystal shards on the Hollow, gold pauldrons on the Court, a satchel on
 * Ashfall, a hand lantern on the Watch.
 *
 * All of it is geometry hung on the body that already exists, with no new assets,
 * and nothing is spawned into the world: a prop that is part of a person cannot be
 * a floating block. Offsets are in the body's own space, so it walks with them.
 */
namespace VEILRaceGear
{
	/** Attach this actor's role/race gear. CastId may be NAME_None for "a person of
	 *  this faction" (the runners and the unnamed market crowd). */
	VEIL_API void Apply(AActor* Owner, USkeletalMeshComponent* Body, FName CastId, EVEILFaction Faction);

	/** Short name of the role resolved, for the log: "WINGS", "LANTERN", "STAFF",
	 *  "MAP", "GOLD", "CRYSTAL", "SATCHEL", "HAND LANTERN", "NONE". */
	VEIL_API const TCHAR* RoleOf(FName CastId, EVEILFaction Faction);
}
