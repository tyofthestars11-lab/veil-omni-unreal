#include "Story/VEILSpine.h"

// THE SIX ACTS, WHOLE.
//
// Rule of the writing, tightened: a line that explains the plot is a line the
// player skips. Nothing here explains; each line reveals a person mid-habit.
// Sera Vahl counts because counting kept her alive as a pattern. Marrow answers
// sideways because she is paying interest on her sight. Arclight talks in
// distance because distance is the only thing he has ever owned. Lines are short
// enough to be read before the next one lands, and every one of them turns.

namespace
{
	FText SpineT(const TCHAR* S) { return FText::FromString(S); }

	const TArray<FVEILActDef>& Build()
	{
		static const TArray<FVEILActDef> List = []()
		{
			TArray<FVEILActDef> A;

			// ---------------------------------------------------------------
			// ACT I - the runner arrives, and is counted.
			// ---------------------------------------------------------------
			{
				FVEILActDef Act;
				Act.Stage = EVEILStage::FindSignal;
				Act.Title = SpineT(TEXT("ACT I  -  THE LAMP THAT WAITED"));
				Act.Epigraph = SpineT(TEXT("Four hundred years of lamps, and the last one was lit for somebody who never came back."));
				Act.Directive = SpineT(TEXT("Walk to the KEEPER and let her count you."));
				Act.Keeper = {
					SpineT(TEXT("Stand still. I count the living first.")),
					SpineT(TEXT("Four hundred years of lamps, and you're the first number I've had to add. Try not to be a subtraction.")),
					SpineT(TEXT("The Vault answers the Signal. Lately the Signal answers back. Light the three beacons before it learns your name.")),
				};
				Act.Seer = {
					SpineT(TEXT("You're early. That's a measurement, not a compliment.")),
					SpineT(TEXT("I read ash. Fire leaves out the truth every time; ash keeps the omission.")),
					SpineT(TEXT("There's a hole where the Vault should be and it has your shape. Find the Keeper before the Deep counts you itself.")),
				};
				Act.Cartographer = {
					SpineT(TEXT("You walk like a man following a map. There isn't one. There are corridors that agree to be walked.")),
					SpineT(TEXT("I map by going wrong on purpose. Every wrong turn shows a place's second face, and the Deep is mostly second face.")),
					SpineT(TEXT("Four roads leave this plaza. Take one. The one you take was always yours.")),
				};
				A.Add(Act);
			}

			// ---------------------------------------------------------------
			// ACT II - ten syllables of an interrupted sentence.
			// ---------------------------------------------------------------
			{
				FVEILActDef Act;
				Act.Stage = EVEILStage::Fragments;
				Act.Title = SpineT(TEXT("ACT II  -  TEN PIECES OF A VOICE"));
				Act.Epigraph = SpineT(TEXT("The Signal never spoke in words. It spoke in intervals, and the Vault is the interval it could not close."));
				Act.Directive = SpineT(TEXT("Recover ten fragments of a broken voice."));
				Act.Keeper = {
					SpineT(TEXT("Ten fragments. Not treasure - grammar.")),
					SpineT(TEXT("Each one hums at the pitch of a lamp about to fail. You'll hear them before you see them.")),
					SpineT(TEXT("Bring the tenth last, and bring it yourself. Don't send a machine for the syllable that knows your name.")),
				};
				Act.Seer = {
					SpineT(TEXT("Four of the ten are in places that don't want to be found. That's how you'll know them.")),
					SpineT(TEXT("A place that wants to be found is a trap with better manners.")),
					SpineT(TEXT("You'll lose one and go back, and on the way back you'll see something I've decided not to tell you about.")),
				};
				Act.Cartographer = {
					SpineT(TEXT("I sell distance. The fragments aren't far - they're badly placed. Different trade entirely.")),
					SpineT(TEXT("Two below the Deep line, two above the weather, the rest pretending to be ordinary stone.")),
					SpineT(TEXT("Walk the roads I drew. Where one ends without arriving, that's the map being honest.")),
				};
				A.Add(Act);
			}

			// ---------------------------------------------------------------
			// ACT III - the forge, and the thing you will have to carry.
			// ---------------------------------------------------------------
			{
				FVEILActDef Act;
				Act.Stage = EVEILStage::Artifact;
				Act.Title = SpineT(TEXT("ACT III  -  THE FORGE THAT REMEMBERS"));
				Act.Epigraph = SpineT(TEXT("Ten in, one out. It is the arithmetic of every forge ever built, and the only honest arithmetic there is."));
				Act.Directive = SpineT(TEXT("Forge the artifact at the Spire anvil."));
				Act.Keeper = {
					SpineT(TEXT("The Spire forge doesn't make weapons. It makes the thing you'll have to carry.")),
					SpineT(TEXT("Feed it the fragments one at a time. If it asks a question, answer in your own words. It can tell.")),
					SpineT(TEXT("It'll weigh more than what went into it. That's not magic, that's consequence. Lift with your legs.")),
				};
				Act.Seer = {
					SpineT(TEXT("It's not a key. It's a receipt. The Vault will read it and decide what you owe.")),
					SpineT(TEXT("Don't make it beautiful. Beautiful things survive a fire and then lie about being in it.")),
					SpineT(TEXT("When it's done, hold it up. If your shadow disagrees with you, start again.")),
				};
				Act.Cartographer = {
					SpineT(TEXT("The stairs to the forge don't exist all the way up. I've marked the three that do.")),
					SpineT(TEXT("Carry it out slowly, as though it might be listening. It is. Everything down here is.")),
					SpineT(TEXT("Nothing made in the Deep is only yours. Remember that when it starts giving orders.")),
				};
				A.Add(Act);
			}

			// ---------------------------------------------------------------
			// ACT IV - the deep answers, and it answers with a face.
			// ---------------------------------------------------------------
			{
				FVEILActDef Act;
				Act.Stage = EVEILStage::FinalWave;
				Act.Title = SpineT(TEXT("ACT IV  -  WHAT THE DEEP SENDS"));
				Act.Epigraph = SpineT(TEXT("WARDEN PRIME is not a monster. It is the Vault's opinion of you, standing up."));
				Act.Directive = SpineT(TEXT("Hold the plaza: five waves, then the WARDEN."));
				Act.Keeper = {
					SpineT(TEXT("Five waves. Count them out loud if it helps. Counting is what's kept me alive.")),
					SpineT(TEXT("They're not attacking the plaza. They're attacking the lamps. They know what light does to the dark they crawl out of.")),
					SpineT(TEXT("Hold. Not for glory - arithmetic. Every wave you stop is one the Vault doesn't get to spend.")),
				};
				Act.Seer = {
					SpineT(TEXT("This is the part I saw. I didn't tell you, because you'd have walked differently.")),
					SpineT(TEXT("Different walking is how people die in the way I saw.")),
					SpineT(TEXT("It'll use your own words. Don't answer it. Answer the frightened thing behind it.")),
				};
				Act.Cartographer = {
					SpineT(TEXT("I've mapped six approaches to this plaza. They'll use the seventh.")),
					SpineT(TEXT("The seventh comes from above, through the gap in the rings. Watch the sky and it's just a large thing arriving.")),
					SpineT(TEXT("If the ground moves, that's not the ground. That's the Deep come up to look at you.")),
				};
				A.Add(Act);
			}

			// ---------------------------------------------------------------
			// ACT V - the last lamp, and the only one that matters.
			// ---------------------------------------------------------------
			{
				FVEILActDef Act;
				Act.Stage = EVEILStage::Beacon;
				Act.Title = SpineT(TEXT("ACT V  -  LIGHT THE THING YOU CARRIED"));
				Act.Epigraph = SpineT(TEXT("Everything before now was rehearsal. The beacon does not read minds - it reads intention, and it has been starved of both."));
				Act.Directive = SpineT(TEXT("Put the artifact in the cradle. Say what you intend."));
				Act.Keeper = {
					SpineT(TEXT("Cradle. Hand on it. Say what you intend - plainly, out loud, your own voice.")),
					SpineT(TEXT("When it takes, don't step back. The light grabs the nearest living thing. That's you. It was always going to be you.")),
					SpineT(TEXT("Four hundred years of lamps, and I'd like to be watching when the last one I count goes up.")),
				};
				Act.Seer = {
					SpineT(TEXT("The moment it lights, the hole fills. I don't know with what. I've never seen past here.")),
					SpineT(TEXT("Whatever answers, answer it plainly. Four hundred years of clever people have handled it enough.")),
				};
				Act.Cartographer = {
					SpineT(TEXT("Every road I ever drew ends here. I didn't know that until now. Embarrassing, for a cartographer.")),
					SpineT(TEXT("Light it. I'd like to see one map finish.")),
				};
				A.Add(Act);
			}

			// ---------------------------------------------------------------
			// ACT VI - the path of light, and what a finished map is for.
			// ---------------------------------------------------------------
			{
				FVEILActDef Act;
				Act.Stage = EVEILStage::Won;
				Act.Title = SpineT(TEXT("ACT VI  -  PATH OF LIGHT"));
				Act.Epigraph = SpineT(TEXT("The lamps are lit. Not because you were brave - because you were on time, every time, and that is the whole religion."));
				Act.Directive = SpineT(TEXT("Signal restored. Walk the path of light."));
				Act.Keeper = {
					SpineT(TEXT("Four hundred years, runner. I've added you to the count, and for once it isn't closed.")),
					SpineT(TEXT("The Watch will want a ceremony. Let them. Then come back - there are lamps waiting on this one.")),
				};
				Act.Seer = {
					SpineT(TEXT("The hole is filled. You were the shape of it, and you didn't fall through.")),
					SpineT(TEXT("I can see past this now. It looks like more road.")),
				};
				Act.Cartographer = {
					SpineT(TEXT("The map is finished. I have no idea what to do tomorrow.")),
					SpineT(TEXT("Walk it anyway. Somebody has to draw the next one, and you've got the boots.")),
				};
				A.Add(Act);
			}

			return A;
		}();
		return List;
	}
}

namespace VEILSpine
{
	const TArray<FVEILActDef>& Acts()
	{
		return Build();
	}

	const FVEILActDef& ForStage(EVEILStage Stage)
	{
		for (const FVEILActDef& Act : Build())
		{
			if (Act.Stage == Stage) { return Act; }
		}
		return Build()[0];
	}

	const TArray<FText>& SceneFor(EVEILStage Stage, FName SpeakerId)
	{
		static const TArray<FText> Empty;
		const FVEILActDef& Act = ForStage(Stage);

		if (SpeakerId == FName(TEXT("sera_vahl"))) { return Act.Keeper; }
		if (SpeakerId == FName(TEXT("ilva_marrow"))) { return Act.Seer; }
		if (SpeakerId == FName(TEXT("domen_arclight"))) { return Act.Cartographer; }
		return Empty;
	}

	FText DirectiveFor(EVEILStage Stage)
	{
		return ForStage(Stage).Directive;
	}

	FText TitleFor(EVEILStage Stage)
	{
		return ForStage(Stage).Title;
	}

	FText EpigraphFor(EVEILStage Stage)
	{
		return ForStage(Stage).Epigraph;
	}
}
