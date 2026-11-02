#include "Story/VEILSpine.h"

// THE SIX ACTS, WHOLE.
//
// Rule of the writing: no line explains the plot, every line reveals a person.
// Sera Vahl counts because counting is what kept her alive as a pattern. Marrow
// answers sideways because she is paying interest on her sight. Arclight talks
// in distance because distance is the only thing he has ever been able to own.
// The plot is what those three habits collide with.

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
					SpineT(TEXT("Stand still. You arrived on time, which is rarer than arriving at all.")),
					SpineT(TEXT("This is the First Beacon. I carried it down the Spire while the Signal was still screaming, and I have counted every lamp lit from it since. Four hundred years of arithmetic, runner, and you are the first number I have had to add.")),
					SpineT(TEXT("The Vault at the centre of this district is answering something. Not me. Not the Watch. It answers the Signal, and lately the Signal has begun to answer back.")),
					SpineT(TEXT("Light the three beacons. Not for me - because a light nobody tends becomes a door, and the Hollow is already leaning on it.")),
				};
				Act.Seer = {
					SpineT(TEXT("You are early. That is not a compliment, it is a measurement.")),
					SpineT(TEXT("I read ash. The Court calls it fraud and pays me anyway. Ash keeps what fire leaves out, and fire leaves out the truth every single time.")),
					SpineT(TEXT("There is a hole where the Vault should be, and it has your shape in it. I cannot tell whether you fill it or fall through it. Ask me again when you are standing in it.")),
					SpineT(TEXT("Go and find the Keeper. She will count you, and I would rather you were counted before the Deep starts counting you itself.")),
				};
				Act.Cartographer = {
					SpineT(TEXT("You walk like a man following a map. There is no map. There are only corridors that agree to be walked.")),
					SpineT(TEXT("I mapped the Megastructure by going the wrong way on purpose. Every wrong turn shows the second face of a place, and the Deep is mostly second face.")),
					SpineT(TEXT("The four roads out of this plaza were laid by somebody who understood that a city is a sentence, not a list. Take one. The one you take was always yours.")),
					SpineT(TEXT("When you find the Vault, do not enter it forward. Nobody has ever come out of a door they walked through front-first.")),
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
					SpineT(TEXT("Ten fragments. Not treasure - grammar. Bring them back and I will show you the sentence we were told to forget.")),
					SpineT(TEXT("Every one you pick up is a syllable of something interrupted four centuries ago. They still hum at the pitch of a lamp about to fail. If you listen on the road, you will hear them before you see them.")),
					SpineT(TEXT("The tenth is the one that knows your name. Bring it last, and bring it yourself. Do not send a machine for it.")),
				};
				Act.Seer = {
					SpineT(TEXT("Four of the ten are in places that do not want to be found. That is how you will know them - a place that wants to be found is a trap with better manners.")),
					SpineT(TEXT("The Spire keeps one under the sealed rings. The Court will tell you it is theirs. It is nobody's. It is simply older than ownership.")),
					SpineT(TEXT("You will lose one and go back for it, and on the way back you will see something you were not meant to see. I have already decided not to tell you what.")),
				};
				Act.Cartographer = {
					SpineT(TEXT("Distances, runner. I sell distances. The fragments are not far - they are badly placed. There is a difference, and that difference is the whole of my profession.")),
					SpineT(TEXT("Two are below the Deep line. Two are above the weather. The rest are in the district, pretending to be ordinary stone, which is what ordinary stone is for.")),
					SpineT(TEXT("Walk the roads I drew. Where a road ends without arriving, that is not a mistake. That is the map being honest.")),
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
					SpineT(TEXT("The Spire forge belonged to the Court before it belonged to anyone. It does not make weapons. It makes the thing you will have to carry.")),
					SpineT(TEXT("Stand at the anvil and give it the fragments one at a time. If it asks you a question, answer in your own words. It can tell the difference; that is the only test it has ever had.")),
					SpineT(TEXT("When it is finished it will be heavier than the sum of what went into it. That is not magic, that is consequence, and you should get used to the weight before you lift it.")),
				};
				Act.Seer = {
					SpineT(TEXT("The artifact is not a key. It is a receipt. The Vault will read it and decide what you owe.")),
					SpineT(TEXT("You will want to make it beautiful. Do not. Make it true. Beautiful things survive the fire and then lie about having been in it.")),
					SpineT(TEXT("When it is done, hold it up. If your shadow disagrees with you, put it down and start again.")),
				};
				Act.Cartographer = {
					SpineT(TEXT("The forge is at the top of the Spire and the stairs do not exist all the way up. I have marked the three steps that do.")),
					SpineT(TEXT("Carry it out the way you carried the fragments in: slowly, and as though it might be listening. It is listening. Everything in the Deep is.")),
					SpineT(TEXT("A thing made down here is never only yours. Remember that when it starts giving orders - and it will start.")),
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
					SpineT(TEXT("Five waves. Count them out loud if it helps. I have counted everything for four hundred years and it has kept me alive, so I am not going to stop now.")),
					SpineT(TEXT("They are not attacking the plaza. They are attacking the beacons, because they know exactly what a lit lamp does to the dark they come out of.")),
					SpineT(TEXT("Hold. Not for glory - for arithmetic. Every wave you stop is one the Vault does not get to spend on the next.")),
				};
				Act.Seer = {
					SpineT(TEXT("This is the part I saw. I did not tell you, because you would have walked differently, and walking differently is how people die in the way I saw.")),
					SpineT(TEXT("When it speaks, it will use your own words back at you. Do not answer it. Answer the thing standing behind it, which is afraid of you and has been for four hundred years.")),
					SpineT(TEXT("If you lose, you get up again. I have seen that too. It is the only future I have ever been certain of and it is the only one I would swear to.")),
				};
				Act.Cartographer = {
					SpineT(TEXT("I have mapped every approach to this plaza. There are six, and they will use the seventh.")),
					SpineT(TEXT("The seventh comes from above, through the gap in the rings. Stand where you can see the sky and it will not be a surprise, only a large thing arriving.")),
					SpineT(TEXT("If the ground moves, it is not the ground. It is the Deep, coming up to have a look at you.")),
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
					SpineT(TEXT("Put it in the cradle. Put your hand on it. Say what you intend - plainly, out loud, in your own voice.")),
					SpineT(TEXT("When it takes, do not step back. The light will look for something to hold on to and it will choose the nearest living thing. That is you. That was always going to be you.")),
					SpineT(TEXT("Four hundred years I have kept a count of lamps. This is the last one I will ever have to add, and I would like to be watching when it happens.")),
				};
				Act.Seer = {
					SpineT(TEXT("The moment it lights, the hole I saw fills. I do not know with what. I have never seen past this, and I have been dreading the not-knowing more than the seeing.")),
					SpineT(TEXT("Whatever answers, answer it plainly. It has had four hundred years of nothing but clever people and it is tired of being handled.")),
				};
				Act.Cartographer = {
					SpineT(TEXT("Every road I ever drew ends here. I did not know that until this moment, which is deeply embarrassing for a cartographer.")),
					SpineT(TEXT("Light it. I would like to see one map that finishes.")),
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
					SpineT(TEXT("Four hundred years, runner. I have added you to the count, and for the first time the count is not closed.")),
					SpineT(TEXT("The Watch will want a ceremony. Let them have it. Then come back and help me light the lamps that were waiting on this one.")),
				};
				Act.Seer = {
					SpineT(TEXT("The hole is filled. You were the shape of it, and you did not fall through. I have never been so glad to be wrong about a measurement.")),
					SpineT(TEXT("I can see past this now. It looks like more road.")),
				};
				Act.Cartographer = {
					SpineT(TEXT("The map is finished. I have absolutely no idea what to do tomorrow.")),
					SpineT(TEXT("Walk it anyway. Somebody has to draw the next one, and you have the boots for it.")),
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
