#pragma once

#include "CoreMinimal.h"
#include "AI/VEILFactionAI.h"

/**
 * THE CAST OF VEIL OMNI.
 *
 * A world is not lived in because it has more meshes; it is lived in because
 * somebody wants something. This is the authored layer: four factions with
 * creeds and colours, and the people who speak for them. It is plain data so
 * the dialogue runner, the HUD and the quest content all read ONE source.
 *
 * Mechanically the four faction ids are the engine's own EVEILFaction values
 * (Void / Scavenger / Guardian / Remnant); the names here are what the world
 * calls them.
 */
struct FVEILFactionDef
{
	FName Id;
	FText Name;
	FText Epithet;
	FText Creed;
	FText Stronghold;
	FLinearColor Color = FLinearColor::White;
	EVEILFaction Faction = EVEILFaction::None;
};

struct FVEILCharacterDef
{
	FName Id;
	FText Name;
	FText Title;
	FName FactionId;
	FText Background;
	TArray<FText> Greetings;
};

/**
 * What answering somebody actually changes. A reply that changes nothing is a
 * menu, not a conversation - so every authored reply carries one of these and
 * it lands in the world before the box closes.
 */
enum class EVEILReplyEffect : uint8
{
	None,           // it is only a question answered
	FavourFaction,  // the speaker's faction trusts the runner more
	DefyFaction,    // ...and less, to the point of hunting
	LightBeacon     // the main chain moves: a beacon is claimed
};

struct FVEILReplyDef
{
	/** What the runner says. First person, short: this is a thing you chose. */
	FText Label;
	/** What they answer, in their own voice. */
	FText Response;
	EVEILReplyEffect Effect = EVEILReplyEffect::None;
	float Amount = 0.f;
};

namespace VEILCast
{
	const TArray<FVEILFactionDef>& Factions();
	const TArray<FVEILCharacterDef>& Characters();

	const FVEILCharacterDef* FindCharacter(FName Id);
	const FVEILFactionDef* FindFaction(FName FactionId);

	/** The statue at 900,0 / -450,780 / -4150,-780 is one of three voices. */
	FName CharacterForGiver(const FName& GiverName);

	/** A given machine speaks for a given member, deterministically. */
	FName CharacterForFaction(EVEILFaction Faction, int32 Index);

	/** The replies this character offers. Empty for a character with none. */
	const TArray<FVEILReplyDef>& RepliesFor(FName CharacterId);

	FLinearColor ColorOf(FName CharacterId);
	FText NameOf(FName CharacterId);
}
