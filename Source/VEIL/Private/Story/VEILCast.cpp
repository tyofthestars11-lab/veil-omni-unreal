#include "Story/VEILCast.h"

namespace
{
	FText CastT(const TCHAR* S) { return FText::FromString(S); }

	const TArray<FVEILFactionDef>& BuildFactions()
	{
		static const TArray<FVEILFactionDef> List = {
			{
				FName(TEXT("ashfall")), CastT(TEXT("THE ASHFALL CONSORTIUM")), CastT(TEXT("Salvage is a religion")),
				CastT(TEXT("Everything ends. We are what remains - and we sell it.")),
				CastT(TEXT("The Nest, beneath the dead spires")),
				FLinearColor(1.f, 0.62f, 0.20f), EVEILFaction::Scavenger
			},
			{
				FName(TEXT("watch")), CastT(TEXT("THE LANTERN WATCH")), CastT(TEXT("Keepers of the lit beacons")),
				CastT(TEXT("A light is a promise kept, not a claim made.")),
				CastT(TEXT("The Hub, around the First Beacon")),
				FLinearColor(0.35f, 0.85f, 1.f), EVEILFaction::Guardian
			},
			{
				FName(TEXT("court")), CastT(TEXT("THE QUIET COURT")), CastT(TEXT("Heirs of the last broadcast")),
				CastT(TEXT("Silence is the oldest coin. We are its bankers.")),
				CastT(TEXT("The Spire, in the sealed upper rings")),
				FLinearColor(0.72f, 0.45f, 1.f), EVEILFaction::Remnant
			},
			{
				FName(TEXT("hollow")), CastT(TEXT("THE HOLLOW")), CastT(TEXT("Not a faction. A condition")),
				CastT(TEXT("We listened first. We are still listening.")),
				CastT(TEXT("The Deep, wherever the Signal pools")),
				FLinearColor(0.75f, 0.78f, 0.82f), EVEILFaction::Void
			},
		};
		return List;
	}

	const TArray<FVEILCharacterDef>& BuildCharacters()
	{
		static const TArray<FVEILCharacterDef> List = {
			{
				FName(TEXT("sera_vahl")), CastT(TEXT("SERA VAHL")), CastT(TEXT("KEEPER OF THE FIRST BEACON")),
				FName(TEXT("watch")),
				CastT(TEXT("Four centuries ago Sera Vahl carried the first flame down the Spire while the Signal "
					"was still screaming. Her body did not survive the descent; her pattern did. The statue "
					"at the Hub is not a monument to her - it is her, still keeping count of every lamp lit "
					"in her name.")),
				{ CastT(TEXT("You stand in the one place the Signal never reached. Mind it.")),
				  CastT(TEXT("Four hundred years of lamps, runner. Do not be the one that goes out.")),
				  CastT(TEXT("The Watch does not ask you to be brave. It asks you to be on time.")) }
			},
			{
				FName(TEXT("ilva_marrow")), CastT(TEXT("ILVA MARROW")), CastT(TEXT("THE SEER OF THE NEST")),
				FName(TEXT("court")),
				CastT(TEXT("She reads ash the way the Court reads contracts: for what was left out. Marrow bought "
					"her sight from the Hollow and has been paying the interest ever since.")),
				{ CastT(TEXT("I saw you arrive. Not you - the shape of the hole you will leave.")),
				  CastT(TEXT("Ask. But I answer in what happens next, not what you would like.")) }
			},
			{
				FName(TEXT("domen_arclight")), CastT(TEXT("DOMEN ARCLIGHT")), CastT(TEXT("CARTOGRAPHER OF THE DEEP")),
				FName(TEXT("ashfall")),
				CastT(TEXT("Arclight mapped the Megastructure by walking it wrong on purpose, so the corridors "
					"would show him their second face. Half his maps are of places that only exist when "
					"nobody is looking.")),
				{ CastT(TEXT("Every corridor in the Deep is a rehearsal. Which one are you here for?")),
				  CastT(TEXT("I sell distance, runner. Not direction.")) }
			},
			{
				FName(TEXT("bram_kessler")), CastT(TEXT("BRAM KESSLER")), CastT(TEXT("QUARTERMASTER OF ASHFALL")),
				FName(TEXT("ashfall")),
				CastT(TEXT("Kessler counts what everyone else throws away, and he has never once been wrong about "
					"what a thing is worth - only about who would pay.")),
				{ CastT(TEXT("Everything on my table came off a corpse, and every corpse agreed to it.")),
				  CastT(TEXT("Bring me something the Deep doesn't want back.")) }
			},
			{
				FName(TEXT("tova_renn")), CastT(TEXT("TOVA RENN")), CastT(TEXT("CUTTER-CAPTAIN")),
				FName(TEXT("ashfall")),
				CastT(TEXT("Renn's crews open hulls with shaped charges and bad language. She has lost eleven "
					"people to the Deep and remembers all eleven names, in order.")),
				{ CastT(TEXT("If you are buying, my cutters will make you a door out of a wall.")),
				  CastT(TEXT("Eleven names. I do not intend to learn a twelfth.")) }
			},
			{
				FName(TEXT("nix")), CastT(TEXT("NIX")), CastT(TEXT("SALVAGE-RUNNER, NO SURNAME OFFERED")),
				FName(TEXT("ashfall")),
				CastT(TEXT("Nix came up out of the Nest with no memory and a perfect ear for machine rhythm. The "
					"Consortium keeps them because they can hear a failing pump through three walls.")),
				{ CastT(TEXT("Shh. Listen. That hum? That is the district thinking.")),
				  CastT(TEXT("You walk loud. The Deep notices loud.")) }
			},
			{
				FName(TEXT("odalys_crane")), CastT(TEXT("CAPTAIN ODALYS CRANE")), CastT(TEXT("WATCH-CAPTAIN OF THE HUB")),
				FName(TEXT("watch")),
				CastT(TEXT("Crane held the plaza through the Third Silence with forty volunteers and a broken "
					"lance. She has not slept a full night since, and will not say why.")),
				{ CastT(TEXT("The Watch keeps three lights. You will keep them too, or you will answer to me.")),
				  CastT(TEXT("I held this plaza once with forty. I would rather not do it again.")) }
			},
			{
				FName(TEXT("ferris_holt")), CastT(TEXT("DEACON FERRIS HOLT")), CastT(TEXT("KEEPER OF THE LEDGER")),
				FName(TEXT("watch")),
				CastT(TEXT("Holt records every lit beacon in ink and every unlit one in his memory. He believes "
					"the Signal is a debt and the Vault is the receipt.")),
				{ CastT(TEXT("Every lamp is a signature. Sign carefully.")),
				  CastT(TEXT("The Vault is not a prize. It is an invoice.")) }
			},
			{
				FName(TEXT("mirea_shen")), CastT(TEXT("SHIELD-AUNT MIREA")), CastT(TEXT("WARDEN OF THE NEST GATE")),
				FName(TEXT("watch")),
				CastT(TEXT("Mirea walked out of the Hollow with her name intact and refuses to say what she "
					"traded for it. She guards the gate because she knows what walks back through.")),
				{ CastT(TEXT("You have the Deep in your eyes. Wipe them before you come inside.")) }
			},
			{
				FName(TEXT("vespera_dal")), CastT(TEXT("LADY VESPERA DAL")), CastT(TEXT("FIRST VOICE OF THE QUIET COURT")),
				FName(TEXT("court")),
				CastT(TEXT("Dal owns nine of the ten quietest rooms in the Spire. She has never raised her voice, "
					"and three districts have gone silent on her word.")),
				{ CastT(TEXT("Speak softly. Everything you say here is already being spent.")),
				  CastT(TEXT("The Court does not ask for loyalty. It asks for discretion.")) }
			},
			{
				FName(TEXT("elias_brume")), CastT(TEXT("ELIAS BRUME")), CastT(TEXT("THE NOTARY")),
				FName(TEXT("court")),
				CastT(TEXT("Brume writes down what people intend, not what they say, and files it where it will "
					"hurt most. Nobody has caught him being wrong, which is itself suspicious.")),
				{ CastT(TEXT("State your intention. I will decide what it costs.")) }
			},
			{
				FName(TEXT("sahn")), CastT(TEXT("CIPHER-ADJUTANT SAHN")), CastT(TEXT("COURT SIGNALS OFFICER")),
				FName(TEXT("court")),
				CastT(TEXT("Sahn decodes the Court's dead channels for fun. Two of them decoded back, which is how "
					"they got the promotion and the tremor.")),
				{ CastT(TEXT("You are transmitting. Do you know to whom?")) }
			},
			{
				FName(TEXT("unit_ithaca")), CastT(TEXT("UNIT ITHACA")), CastT(TEXT("PUMP-MINDER OF THE NEST")),
				FName(TEXT("ashfall")),
				CastT(TEXT("Ithaca was a pump-minder before anyone put a voice in them, and they kept the Nest "
					"breathing for nine years without one person coming down to ask how. They speak in a low "
					"clean hum, they tell jokes badly on purpose, and they are the only construct in the "
					"district who will admit to being afraid of the dark - they have been in it longer than "
					"you have been alive.")),
				{ CastT(TEXT("You are the runner. I have been saying your name to a pump for two days. It is lighter than it looks.")),
				  CastT(TEXT("Nobody came down for nine years. Then a lamp. Then you. I am still deciding which was the good news.")),
				  CastT(TEXT("The Nest is loud to you. To me it is a room where everyone is finally talking at once.")) }
			},
			{
				FName(TEXT("the_listener")), CastT(TEXT("THE LISTENER")), CastT(TEXT("NO NAME. NO FACE. FIRST OF THE HOLLOW")),
				FName(TEXT("hollow")),
				CastT(TEXT("The Listener was the technician who first heard the Signal and answered it with a "
					"question. What came back wears them like a coat, and still asks the question.")),
				{ CastT(TEXT("We were the first to listen. You are the first to arrive.")),
				  CastT(TEXT("You are asking the question too. Good. It is the only door.")) }
			},
			{
				FName(TEXT("pale_host")), CastT(TEXT("THE PALE HOST")), CastT(TEXT("THE HOLLOW'S ANSWER")),
				FName(TEXT("hollow")),
				CastT(TEXT("Where the Signal pools it makes a body out of whatever is nearby - dust, cable, the "
					"recently dead. The Host is what the Deep sends when it wants to be understood.")),
				{ CastT(TEXT("You are made of noise. We are made of the silence after it.")) }
			},
			{
				FName(TEXT("the_third_echo")), CastT(TEXT("THE THIRD ECHO")), CastT(TEXT("HOLLOW ECHO, THREE SECONDS LATE")),
				FName(TEXT("hollow")),
				CastT(TEXT("Every answer the Hollow gives arrives a little late, and the Echo is the lateness: a voice "
					"that repeats the district three seconds after it has finished speaking, in a tone that is almost "
					"right. Almost is the part that keeps people up.")),
				{ CastT(TEXT("You said something just now. I am about to say it better.")),
				  CastT(TEXT("Do not wait for me to finish. I am always behind you, and always correct.")) }
			},
			{
				FName(TEXT("ash_witness")), CastT(TEXT("ASH-WITNESS")), CastT(TEXT("THE HOLLOW'S MEMORY")),
				FName(TEXT("hollow")),
				CastT(TEXT("The Hollow does not keep records; it keeps witnesses. Ash-Witness has watched the same "
					"square metre of the Deep for a century and remembers every foot that crossed it, including the "
					"ones that never arrived.")),
				{ CastT(TEXT("You have walked here four times. The fourth is the one I am speaking to.")),
				  CastT(TEXT("Nothing burns down here. The ash is what the Signal left when it stopped shouting.")) }
			},
			{
				FName(TEXT("pale_choir")), CastT(TEXT("THE PALE CHOIR")), CastT(TEXT("MANY VOICES, ONE THROAT")),
				FName(TEXT("hollow")),
				CastT(TEXT("Where the Signal pooled in a stairwell it learned harmony, and the Choir is what it does "
					"with a dozen borrowed throats: a hymn that is also a census, sung at whoever is closest.")),
				{ CastT(TEXT("We have your voice already. We are only waiting for the words.")),
				  CastT(TEXT("Sing back, and we will tell you which of us you are.")) }
			},
			{
				FName(TEXT("the_answerer")), CastT(TEXT("THE ANSWERER")), CastT(TEXT("HOLLOW ECHO, SPEAKING FIRST")),
				FName(TEXT("hollow")),
				CastT(TEXT("The Listener asked. The Answerer is what came back early: it answers questions nobody has "
					"asked yet, which is why the Watch fires on it before it finishes a sentence.")),
				{ CastT(TEXT("Ask me nothing. I have already answered.")),
				  CastT(TEXT("You will want the beacon at the third ring. You will not want what it costs. Both are true at once.")) }
			},
			{
				FName(TEXT("halden_rook")), CastT(TEXT("HALDEN ROOK")), CastT(TEXT("LAMP-WARDEN OF THE SPIRE STAIR")),
				FName(TEXT("watch")),
				CastT(TEXT("Rook keeps the two hundred and eleven lamps on the Spire stair, one at a time, by hand, "
					"because the last time the Watch trusted a mechanism the stair went dark for a year and the "
					"ledger still has the blank line.")),
				{ CastT(TEXT("Two hundred and eleven. Count them on the way up; it is the only prayer the Watch still says.")),
				  CastT(TEXT("Do not touch the ninth. It is not lit and it is not out - there is a difference.")) }
			},
			{
				FName(TEXT("iset_marr")), CastT(TEXT("ISET MARR")), CastT(TEXT("WATCH-SERGEANT, NIGHT LEDGER")),
				FName(TEXT("watch")),
				CastT(TEXT("Marr commands the night shift, which in this district means she commands the only people "
					"awake when the Signal is loudest. She writes down what her patrols hear and burns the pages "
					"that answer.")),
				{ CastT(TEXT("Night is a shift, not a mood. State your business before the bell.")),
				  CastT(TEXT("My patrol heard a name in the static. It was not yours. Keep it that way.")) }
			},
			{
				FName(TEXT("kesh_varo")), CastT(TEXT("KESH VARO")), CastT(TEXT("HULL-WRIGHT OF THE NEST DRY-DOCKS")),
				FName(TEXT("ashfall")),
				CastT(TEXT("Varo welds hulls that were never built to hold air, and does it well enough that the "
					"Consortium forgives the eleven ships that came back empty. She keeps their manifest bolts in a "
					"jar and calls it inventory.")),
				{ CastT(TEXT("Everything down here is a hull if you seal it hard enough. Even the district.")),
				  CastT(TEXT("I do not fix what the Deep broke. I fix what people broke trying to leave it.")) }
			},
			{
				FName(TEXT("corin_vey")), CastT(TEXT("CORIN VEY")), CastT(TEXT("QUIET BAILIFF")),
				FName(TEXT("court")),
				CastT(TEXT("Vey enforces the Court's silence with a ledger and a key. He has never drawn a weapon in "
					"the Spire and has never needed to: the rooms he locks stay locked because the people inside "
					"agree with him.")),
				{ CastT(TEXT("You are speaking at the volume of someone who has not signed anything.")),
				  CastT(TEXT("The Court fines noise. I am the fine.")) }
			},
			{
				FName(TEXT("adea_lune")), CastT(TEXT("ADEA LUNE")), CastT(TEXT("COURT ARCHIVIST OF THE SEALED RINGS")),
				FName(TEXT("court")),
				CastT(TEXT("Lune keeps every broadcast the world ever made, filed by what it was afraid of. She has "
					"read the last one and has not slept in the Spire since, which the Court records as dedication.")),
				{ CastT(TEXT("Every voice in the archive is filed by fear. Yours is not in it yet.")),
				  CastT(TEXT("The last broadcast is four seconds long. Three of them are breathing.")) }
			},
		};
		return List;
	}
}

namespace
{
	// Every reply the cast offers, keyed by who is speaking. Each one is a thing
	// the runner says, an answer in the speaker's own voice, and what it changes:
	// the Watch and the Court remember favour, the Hollow does not forget a slight,
	// and two of these move the main chain by lighting a beacon.
	const TMap<FName, TArray<FVEILReplyDef>>& BuildReplies()
	{
		static const TMap<FName, TArray<FVEILReplyDef>> Map = []()
		{
			TMap<FName, TArray<FVEILReplyDef>> M;
			auto R = [](const TCHAR* Label, const TCHAR* Response, EVEILReplyEffect Effect, float Amount)
			{
				FVEILReplyDef D;
				D.Label = FText::FromString(Label);
				D.Response = FText::FromString(Response);
				D.Effect = Effect;
				D.Amount = Amount;
				return D;
			};

			M.Add(FName(TEXT("sera_vahl")), {
				R(TEXT("Then I will be on time."),
				  TEXT("Good. One lamp already burns in your name. The other two are waiting on that stride."),
				  EVEILReplyEffect::LightBeacon, 0.f),
				R(TEXT("Your count is not my debt."),
				  TEXT("Spoken like every runner who went dark. We keep a light for you anyway."),
				  EVEILReplyEffect::DefyFaction, 8.f),
				R(TEXT("What does the Watch pay for a lamp?"),
				  TEXT("A line in the ledger, and my ear when it matters. Ask Holt; he signs for me."),
				  EVEILReplyEffect::FavourFaction, 10.f),
			});

			M.Add(FName(TEXT("ilva_marrow")), {
				R(TEXT("Show me the hole."),
				  TEXT("Not yet. It has to be wide enough for you to stand in first."),
				  EVEILReplyEffect::FavourFaction, 8.f),
				R(TEXT("I do not buy sight."),
				  TEXT("Then you will keep walking into what I already survived."),
				  EVEILReplyEffect::DefyFaction, 8.f),
			});

			M.Add(FName(TEXT("domen_arclight")), {
				R(TEXT("Sell me the corridor that only exists unlooked."),
				  TEXT("Walk it backwards and mind your second step. The Deep opens for that, not for courage."),
				  EVEILReplyEffect::LightBeacon, 0.f),
				R(TEXT("I will map it myself."),
				  TEXT("Then draw it in something that stays drawn. Ink has been lying to me for years."),
				  EVEILReplyEffect::FavourFaction, 6.f),
			});

			M.Add(FName(TEXT("bram_kessler")), {
				R(TEXT("I brought something the Deep does not want back."),
				  TEXT("Then it is already mine. Name your price before I decide it for you."),
				  EVEILReplyEffect::FavourFaction, 10.f),
				R(TEXT("Everything on this table came off a corpse."),
				  TEXT("Every corpse agreed to it. Do you want the paperwork as well?"),
				  EVEILReplyEffect::DefyFaction, 6.f),
			});

			M.Add(FName(TEXT("tova_renn")), {
				R(TEXT("Open the wall for me."),
				  TEXT("Cutters will make you a door. Eleven names, runner. Do not make it twelve."),
				  EVEILReplyEffect::FavourFaction, 10.f),
				R(TEXT("Eleven names is not a ledger."),
				  TEXT("Say that again when it is twelve, and say it standing in front of their crew."),
				  EVEILReplyEffect::DefyFaction, 8.f),
			});

			M.Add(FName(TEXT("nix")), {
				R(TEXT("I will walk quiet."),
				  TEXT("Then the district will start telling you things. Most runners never hear it."),
				  EVEILReplyEffect::FavourFaction, 6.f),
				R(TEXT("What is the hum saying?"),
				  TEXT("That the Vault is awake, and counting something. It counts you."),
				  EVEILReplyEffect::LightBeacon, 0.f),
			});

			M.Add(FName(TEXT("odalys_crane")), {
				R(TEXT("Three lights. I will keep them."),
				  TEXT("Then the Watch stands behind you, and so do I. That is not a small sentence."),
				  EVEILReplyEffect::FavourFaction, 12.f),
				R(TEXT("You held this plaza with forty. Hold it with one more."),
				  TEXT("One more is exactly what I had. It is what I have."),
				  EVEILReplyEffect::FavourFaction, 8.f),
				R(TEXT("Let the Court keep the plaza."),
				  TEXT("Politics over people. I have met that before, and I buried it."),
				  EVEILReplyEffect::DefyFaction, 10.f),
			});

			M.Add(FName(TEXT("ferris_holt")), {
				R(TEXT("Sign me into the ledger."),
				  TEXT("Your lamp is written. Every lamp you light from here is interest on it."),
				  EVEILReplyEffect::LightBeacon, 0.f),
				R(TEXT("The Vault is not an invoice."),
				  TEXT("Then you have not read it yet. Come back when you have."),
				  EVEILReplyEffect::DefyFaction, 6.f),
			});

			M.Add(FName(TEXT("mirea_shen")), {
				R(TEXT("I will wipe my eyes at the gate."),
				  TEXT("Good. The Nest is not for people who are still carrying it around."),
				  EVEILReplyEffect::FavourFaction, 8.f),
				R(TEXT("What did you trade for your name?"),
				  TEXT("Something I can still hear. Ask me again and I will show you."),
				  EVEILReplyEffect::DefyFaction, 4.f),
			});

			M.Add(FName(TEXT("vespera_dal")), {
				R(TEXT("I will speak softly."),
				  TEXT("Then the Court will speak for you. Loudly, which is the joke of it."),
				  EVEILReplyEffect::FavourFaction, 12.f),
				R(TEXT("Discretion is not loyalty."),
				  TEXT("No. It is better - it can be bought again and again."),
				  EVEILReplyEffect::DefyFaction, 10.f),
			});

			M.Add(FName(TEXT("elias_brume")), {
				R(TEXT("My intention: the beacon lit."),
				  TEXT("Then the Court will not stand against it. Cost: one favour, dated today."),
				  EVEILReplyEffect::FavourFaction, 8.f),
				R(TEXT("I intend to leave this room."),
				  TEXT("Everyone does. The fee applies either way; I have already filed it."),
				  EVEILReplyEffect::None, 0.f),
			});

			M.Add(FName(TEXT("sahn")), {
				R(TEXT("Tell me who I am transmitting to."),
				  TEXT("The Quiet Court. I answer in sixes and one of them is a lie."),
				  EVEILReplyEffect::FavourFaction, 6.f),
				R(TEXT("I am transmitting to no one."),
				  TEXT("That is precisely what the Signal said, one hundred years before it answered."),
				  EVEILReplyEffect::DefyFaction, 6.f),
			});

			M.Add(FName(TEXT("unit_ithaca")), {
				R(TEXT("How long were you alone down there?"),
				  TEXT("Nine years, four months. I counted in pump strokes, so the number is honest and the years are a guess."),
				  EVEILReplyEffect::FavourFaction, 12.f),
				R(TEXT("You are a machine. You do not get afraid."),
				  TEXT("Then I will stop saying it and you can keep the dark on your own. I have had practice."),
				  EVEILReplyEffect::DefyFaction, 8.f),
				R(TEXT("Come up to the plaza with me."),
				  TEXT("Give me the light first. I want to see it before I believe it, and I have waited long enough to be embarrassed about that."),
				  EVEILReplyEffect::LightBeacon, 0.f),
			});

			M.Add(FName(TEXT("the_listener")), {
				R(TEXT("What was the question?"),
				  TEXT("The one you are already asking. Say it aloud and I will stop wearing this coat."),
				  EVEILReplyEffect::None, 0.f),
				R(TEXT("You are the Hollow."),
				  TEXT("You are the door. It is the same sentence with different weather."),
				  EVEILReplyEffect::None, 0.f),
			});

			M.Add(FName(TEXT("pale_host")), {
				R(TEXT("Then I am the noise that arrives."),
				  TEXT("Finally. Something that can answer instead of merely repeat."),
				  EVEILReplyEffect::LightBeacon, 0.f),
				R(TEXT("I will not be made into a coat."),
				  TEXT("Then be a wall. Walls are also furniture, and they last longer."),
				  EVEILReplyEffect::DefyFaction, 8.f),
			});

			return M;
		}();
		return Map;
	}
}


	// THE VOICE TABLE. What each person actually says when you walk up. The backgrounds
	// above explain them; these are the lines, and they are written in four registers so
	// the district never sounds like one author: Ashfall talks in quantities and prices,
	// the Watch in liturgy and exhaustion, the Court in contracts and fees, and the Hollow
	// in second person and broken time. Keyed by cast id so a person's voice and their
	// nameplate cannot drift apart.
	const TMap<FName, TArray<FText>>& BuildVoices()
	{
		static const TMap<FName, TArray<FText>> Map = []()
		{
			TMap<FName, TArray<FText>> M;
			auto L = [](const TCHAR* A, const TCHAR* B, const TCHAR* C)
			{
				TArray<FText> Out;
				Out.Add(FText::FromString(A));
				Out.Add(FText::FromString(B));
				Out.Add(FText::FromString(C));
				return Out;
			};

			// ---- Ashfall Consortium: salvage is a religion --------------------
			M.Add(FName(TEXT("tova_renn")), L(
				TEXT("Eleven names in my head, in order. Say yours twice and I might keep it."),
				TEXT("A hull is a wall that got ambitious. I open those for a living."),
				TEXT("You walk like someone who has never heard a charge count down. Enjoy that.")));
			M.Add(FName(TEXT("bram_kessler")), L(
				TEXT("Everything on this table died owing somebody. Now it owes me."),
				TEXT("Bring me a thing the Deep won't miss and I'll tell you what it cost."),
				TEXT("I'm never wrong about worth. Only about who'd pay for it.")));
			M.Add(FName(TEXT("nix")), L(
				TEXT("Quiet. That hum under the plaza - that's the district thinking."),
				TEXT("You walk loud. Loud is how the Deep learns a name."),
				TEXT("Three walls, one failing pump, and me. That's the whole trade.")));
			M.Add(FName(TEXT("domen_arclight")), L(
				TEXT("Half my maps are streets that only exist while nobody looks at them."),
				TEXT("I sell distance, runner. Direction is free and usually wrong."),
				TEXT("Every corridor here is a rehearsal for one you haven't found yet.")));
			M.Add(FName(TEXT("kesh_varo")), L(
				TEXT("Seal it hard enough and anything holds air. Even this district."),
				TEXT("Eleven ships came back empty. I keep their bolts in a jar and call it inventory."),
				TEXT("I don't fix what the Deep broke. I fix what people broke leaving it.")));
			M.Add(FName(TEXT("unit_ithaca")), L(
				TEXT("Nine years, four months, counted in pump strokes. The years are a guess."),
				TEXT("The Nest isn't loud to me. It's a room where everyone finally talks at once."),
				TEXT("Nobody came down for nine years. Then a lamp. Then you. I'm still deciding which was good news.")));

			// ---- Lantern Watch: a light is a promise kept ---------------------
			M.Add(FName(TEXT("sera_vahl")), L(
				TEXT("Four hundred years of lamps, and I know every one that went out."),
				TEXT("You're standing where the Signal never reached. Mind it."),
				TEXT("The Watch doesn't ask you to be brave. It asks you to be on time.")));
			M.Add(FName(TEXT("odalys_crane")), L(
				TEXT("I held this plaza with forty. I'd rather not do the arithmetic again."),
				TEXT("Three lights stand in the Hub. Two of them are afraid. Guess which."),
				TEXT("Sleep is a debt I keep deferring. Don't tell Holt; he'll invoice it.")));
			M.Add(FName(TEXT("ferris_holt")), L(
				TEXT("Every lamp is a signature. Sign carefully."),
				TEXT("The Vault isn't a prize, runner. It's an invoice with your name on it."),
				TEXT("I remember the unlit ones. That is the whole of my office.")));
			M.Add(FName(TEXT("mirea_shen")), L(
				TEXT("Wipe your eyes before the gate. The Deep comes back in them."),
				TEXT("I walked out with my name. I won't say what I paid, so don't ask twice."),
				TEXT("The gate isn't to keep you out. It's to count what walks back.")));
			M.Add(FName(TEXT("halden_rook")), L(
				TEXT("Two hundred and eleven lamps. Count them on the way up; it's the only prayer we still say."),
				TEXT("The ninth is not lit and not out. There's a difference, and it costs."),
				TEXT("A mechanism kept this stair once. The stair was dark for a year after.")));
			M.Add(FName(TEXT("iset_marr")), L(
				TEXT("Night is a shift, not a mood. State your business before the bell."),
				TEXT("My patrol heard a name in the static. It wasn't yours. Keep it that way."),
				TEXT("I burn the pages that answer. You'd be surprised how many answer.")));

			// ---- Quiet Court: silence is the oldest coin ---------------------
			M.Add(FName(TEXT("vespera_dal")), L(
				TEXT("Speak softly. Everything said in this room is already being spent."),
				TEXT("The Court doesn't ask for loyalty. It asks for discretion, then audits the difference."),
				TEXT("Three districts went quiet on my word. I have never raised my voice.")));
			M.Add(FName(TEXT("elias_brume")), L(
				TEXT("State your intention. I'll decide what it costs and file it where it hurts."),
				TEXT("You said one thing and meant another. I've written down the second."),
				TEXT("The fee applies either way. I've already filed it.")));
			M.Add(FName(TEXT("sahn")), L(
				TEXT("You're transmitting. Do you know to whom?"),
				TEXT("Two dead channels decoded back. That's how I got the promotion and the tremor."),
				TEXT("I answer in sixes. One of them is a lie, and I won't say which.")));
			M.Add(FName(TEXT("ilva_marrow")), L(
				TEXT("I saw you arrive. Not you - the shape of the hole you'll leave."),
				TEXT("Ask. But I answer in what happens next, not what you'd like."),
				TEXT("I bought my sight from the Hollow. The interest is the interesting part.")));
			M.Add(FName(TEXT("corin_vey")), L(
				TEXT("You're speaking at the volume of someone who hasn't signed anything."),
				TEXT("The Court fines noise. I am the fine."),
				TEXT("I've never drawn a weapon in the Spire. The doors do the work.")));
			M.Add(FName(TEXT("adea_lune")), L(
				TEXT("Every voice in the archive is filed by what it feared. Yours isn't in it yet."),
				TEXT("The last broadcast is four seconds long. Three of them are breathing."),
				TEXT("I read it once. The Court calls my not sleeping dedication.")));

			// ---- The Hollow: not a faction, a condition ----------------------
			M.Add(FName(TEXT("the_listener")), L(
				TEXT("We listened first. You're the first to arrive."),
				TEXT("You're asking the question too. Good. It's the only door."),
				TEXT("I answered with a question. What came back still asks it.")));
			M.Add(FName(TEXT("pale_host")), L(
				TEXT("You're made of noise. We're made of the silence after it."),
				TEXT("Dust, cable, the recently dead. The Deep uses what's nearby."),
				TEXT("When it wants to be understood, it sends a body. This one.")));
			M.Add(FName(TEXT("the_third_echo")), L(
				TEXT("You said something just now. I'm about to say it better."),
				TEXT("Don't wait for me to finish. I'm always behind you, and always correct."),
				TEXT("Three seconds late is still early, if you stand in the right place.")));
			M.Add(FName(TEXT("ash_witness")), L(
				TEXT("You've walked here four times. The fourth is the one I'm speaking to."),
				TEXT("Nothing burns down here. The ash is what shouting leaves."),
				TEXT("I've watched this square metre for a century. I remember the feet that never arrived.")));
			M.Add(FName(TEXT("pale_choir")), L(
				TEXT("We have your voice already. We're only waiting for words."),
				TEXT("Sing back, and we'll tell you which of us you are."),
				TEXT("A hymn that is also a census. Stand still and be counted.")));
			M.Add(FName(TEXT("the_answerer")), L(
				TEXT("Ask me nothing. I've already answered."),
				TEXT("You'll want the beacon at the third ring. You won't want the price. Both are true."),
				TEXT("The Watch fires before I finish. It's the only courtesy left in the district.")));

			return M;
		}();
		return Map;
	}

namespace VEILCast
{
	const TArray<FVEILFactionDef>& Factions()
	{
		return BuildFactions();
	}

	const TArray<FVEILCharacterDef>& Characters()
	{
		// The definitions carry the names and the history; the voice table carries what
		// they say. Applying it here means every caller - the dialogue box, the HUD, the
		// living world - hears the same person.
		static const TArray<FVEILCharacterDef> List = []()
		{
			TArray<FVEILCharacterDef> Built = BuildCharacters();
			const TMap<FName, TArray<FText>>& Voices = BuildVoices();
			for (FVEILCharacterDef& C : Built)
			{
				if (const TArray<FText>* V = Voices.Find(C.Id)) { C.Greetings = *V; }
			}
			return Built;
		}();
		return List;
	}

	const FVEILCharacterDef* FindCharacter(FName Id)
	{
		for (const FVEILCharacterDef& C : BuildCharacters())
		{
			if (C.Id == Id) { return &C; }
		}
		return nullptr;
	}

	const FVEILFactionDef* FindFaction(FName FactionId)
	{
		for (const FVEILFactionDef& F : BuildFactions())
		{
			if (F.Id == FactionId) { return &F; }
		}
		return nullptr;
	}

	FName CharacterForGiver(const FName& GiverName)
	{
		// The three talking statues are three specific people, not three props.
		const FString N = GiverName.ToString();
		if (N.Contains(TEXT("Keeper"))) { return FName(TEXT("sera_vahl")); }
		if (N.Contains(TEXT("Seer"))) { return FName(TEXT("ilva_marrow")); }
		if (N.Contains(TEXT("Cartographer"))) { return FName(TEXT("domen_arclight")); }
		return FName(TEXT("sera_vahl"));
	}

	FName CharacterForFaction(EVEILFaction Faction, int32 Index)
	{
		TArray<FName> Members;
		for (const FVEILCharacterDef& C : BuildCharacters())
		{
			const FVEILFactionDef* F = FindFaction(C.FactionId);
			if (F && F->Faction == Faction) { Members.Add(C.Id); }
		}
		if (Members.Num() == 0) { return FName(TEXT("sera_vahl")); }
		return Members[FMath::Abs(Index) % Members.Num()];
	}

	FLinearColor ColorOf(FName CharacterId)
	{
		if (const FVEILCharacterDef* C = FindCharacter(CharacterId))
		{
			if (const FVEILFactionDef* F = FindFaction(C->FactionId))
			{
				return F->Color;
			}
		}
		return FLinearColor(0.85f, 0.9f, 1.f);
	}

	FText NameOf(FName CharacterId)
	{
		const FVEILCharacterDef* C = FindCharacter(CharacterId);
		return C ? C->Name : FText::FromName(CharacterId);
	}

	const TArray<FVEILReplyDef>& RepliesFor(FName CharacterId)
	{
		static const TArray<FVEILReplyDef> Empty;
		const TArray<FVEILReplyDef>* Found = BuildReplies().Find(CharacterId);
		return Found ? *Found : Empty;
	}
}
