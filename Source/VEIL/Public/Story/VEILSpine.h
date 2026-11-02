#pragma once

#include "CoreMinimal.h"
#include "Systems/VEILLoopSubsystem.h"

/**
 * THE SPINE OF VEIL OMNI: six acts, told by the three statues.
 *
 * The loop already knows WHAT happens (signal, fragments, artifact, waves,
 * beacon, won). This is what it MEANS - the scene each act plays, in the voice
 * of whoever is standing in front of the runner. Sera Vahl counts, Ilva Marrow
 * reads what was left out, Domen Arclight sells distance; each of them says the
 * act the world is actually in, so the statues are never just plaques with a
 * greeting attached.
 *
 * It is plain data on purpose: the dialogue runner reads it, the HUD reads the
 * directive, and there is one source for both.
 */
struct FVEILActDef
{
	EVEILStage Stage = EVEILStage::FindSignal;

	/** ACT I - THE LAMP THAT WAITED. */
	FText Title;
	/** One line of prose, the act in a sentence. */
	FText Epigraph;
	/** What the runner is asked to do, in the world's voice - kept short: the HUD draws it. */
	FText Directive;

	/** SERA VAHL, KEEPER OF THE FIRST BEACON - the count, the lamps, the ledger. */
	TArray<FText> Keeper;
	/** ILVA MARROW, SEER OF THE NEST - futures, omissions, what the ash leaves out. */
	TArray<FText> Seer;
	/** DOMEN ARCLIGHT, CARTOGRAPHER OF THE DEEP - distances, second faces, roads. */
	TArray<FText> Cartographer;
};

namespace VEILSpine
{
	const TArray<FVEILActDef>& Acts();

	/** The act the world is in. Never null: an unknown stage answers with the first act. */
	const FVEILActDef& ForStage(EVEILStage Stage);

	/** The lines this speaker says in this act. Empty for anyone who is not a statue. */
	const TArray<FText>& SceneFor(EVEILStage Stage, FName SpeakerId);

	/** The HUD's directive line for the act. */
	FText DirectiveFor(EVEILStage Stage);

	/** The act's title, for the log and for a banner. */
	FText TitleFor(EVEILStage Stage);

	/** The act's epigraph - the prose line under the title. */
	FText EpigraphFor(EVEILStage Stage);
}
