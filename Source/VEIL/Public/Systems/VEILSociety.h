// THE SOCIETY LAYER: what people can do, and what they are doing right now.
//
// NO LOCKS. An archetype is a way of fighting and moving, not a box a character
// is filed into. The runner holds every one of them from the first frame, and
// the cast mixes them freely - the quartermaster is also a trickster, the
// keeper is also a stormcall. Nothing is gated, nothing is exclusive, and no
// combination is illegal.
//
// NO AGING. Everyone has a life: a post they keep while the day is busy, and a
// way they spend the quiet hours. The Fibonacci clock walks them through it -
// see VEILTime.h - and the cycle closes without anyone getting older.
#pragma once

#include "CoreMinimal.h"

struct FVEILArchetype
{
	FName Id;
	FString Name;
	FString Power;
	FString Role;
	FString Signature;
};

struct FVEILLife
{
	FName Character;
	/** What they are doing when the day is busy. */
	FString Duty;
	/** What they do in the quiet hours. */
	FString Rest;
	/** Where the post is, for a line of flavour that anchors them in the world. */
	FString Post;
	/** Night workers keep their post when everyone else has stood down. */
	bool bNightWorker = false;
};

namespace VEILSociety
{
	/** Every archetype in the game. All of them are open to everyone. */
	const TArray<FVEILArchetype>& Archetypes();
	const FVEILArchetype* FindArchetype(FName Id);

	/** A character's mix - one to three archetypes, freely combined. */
	const TArray<FName>& ArchetypesOf(FName CharacterId);

	/** A cast member's life. Null for ids that are not people. */
	const FVEILLife* LifeFor(FName CharacterId);

	/** What this character is doing at this beat. Never empty. */
	FString ActivityNow(FName CharacterId, bool bDutyBeat);

	/** The life table, for the codex page. */
	const TArray<FVEILLife>& Lives();
}
