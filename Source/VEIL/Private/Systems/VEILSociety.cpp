#include "Systems/VEILSociety.h"

namespace
{
	FString S(const TCHAR* In) { return FString(In); }

	FVEILArchetype A(const TCHAR* Id, const TCHAR* Name, const TCHAR* Power,
		const TCHAR* Role, const TCHAR* Signature)
	{
		FVEILArchetype Out;
		Out.Id = FName(Id);
		Out.Name = S(Name);
		Out.Power = S(Power);
		Out.Role = S(Role);
		Out.Signature = S(Signature);
		return Out;
	}

	FVEILLife L(const TCHAR* Who, const TCHAR* Duty, const TCHAR* Rest,
		const TCHAR* Post, bool bNight = false)
	{
		FVEILLife Out;
		Out.Character = FName(Who);
		Out.Duty = S(Duty);
		Out.Rest = S(Rest);
		Out.Post = S(Post);
		Out.bNightWorker = bNight;
		return Out;
	}

	// Twelve ways to be dangerous. None of them owns a slot on a character sheet.
	const TArray<FVEILArchetype>& BuildArchetypes()
	{
		static const TArray<FVEILArchetype> List = {
			A(TEXT("ironwright"), TEXT("IRONWRIGHT"), TEXT("Blade and barrier, held in one hand"),
				TEXT("Front line"), TEXT("Raises a wall of light, then walks through it swinging.")),
			A(TEXT("stormcall"), TEXT("STORMCALL"), TEXT("The Signal answers as weather"),
				TEXT("Ranged control"), TEXT("Calls the charge down a corridor; it follows the walls.")),
			A(TEXT("emberwright"), TEXT("EMBERWRIGHT"), TEXT("Heat drawn out of dead machines"),
				TEXT("Area damage"), TEXT("Burns what is already broken, so nothing walks twice.")),
			A(TEXT("tidecaller"), TEXT("TIDECALLER"), TEXT("Pressure and cold"),
				TEXT("Crowd control"), TEXT("Freezes the floor the machines are standing on.")),
			A(TEXT("geomancer"), TEXT("GEOMANCER"), TEXT("The district answers the hand"),
				TEXT("Terrain"), TEXT("Raises cover where there is none, and takes it away.")),
			A(TEXT("shadowstep"), TEXT("SHADOWSTEP"), TEXT("Distance, spent in pieces"),
				TEXT("Stealth"), TEXT("Steps out of sight and back into it somewhere useful.")),
			A(TEXT("beastbound"), TEXT("BEASTBOUND"), TEXT("Something else fights too"),
				TEXT("Summoner"), TEXT("The drone's cousins remember who fed them.")),
			A(TEXT("mindwarden"), TEXT("MINDWARDEN"), TEXT("Attention is a leash"),
				TEXT("Control"), TEXT("Makes a machine turn on the one beside it.")),
			A(TEXT("songs"), TEXT("SONGSMITH"), TEXT("Words with weight"),
				TEXT("Support"), TEXT("A line held in the voice keeps a fallen runner standing.")),
			A(TEXT("trickster"), TEXT("TRICKSTER"), TEXT("A lie with a body"),
				TEXT("Deception"), TEXT("Leaves a copy of the runner standing where he is not.")),
			A(TEXT("hollow"), TEXT("HOLLOWTOUCHED"), TEXT("The Deep, answered in kind"),
				TEXT("Attrition"), TEXT("Gives the machines a little of the silence they came from.")),
			A(TEXT("runner"), TEXT("RUNNER"), TEXT("No wall is a wall for long"),
				TEXT("Traversal"), TEXT("Swing, and the map becomes a set of corners to ignore.")),
		};
		return List;
	}

	const TArray<FVEILLife>& BuildLives()
	{
		static const TArray<FVEILLife> List = {
			L(TEXT("sera_vahl"), TEXT("KEEPS THE FIRST BEACON, counting every lamp lit in her name"),
				TEXT("listens to the plaza without answering it"), TEXT("the Hub, at the First Beacon")),
			L(TEXT("ilva_marrow"), TEXT("READS ASH FOR THE COURT, and for what was left out of it"),
				TEXT("sits with the Hollow's interest and pays a little of it down"), TEXT("the Nest"),
				true),
			L(TEXT("domen_arclight"), TEXT("WALKS THE DEEP WRONG ON PURPOSE, mapping the second face"),
				TEXT("sells distance to whoever is buying, and keeps the true routes"), TEXT("the Deep")),
			L(TEXT("bram_kessler"), TEXT("COUNTS WHAT ASHFALL DRAGS HOME and prices it"),
				TEXT("plays cards he is not very good at, and loses on purpose"), TEXT("the Nest")),
			L(TEXT("tova_renn"), TEXT("OPENS HULLS with charges and language"),
				TEXT("reads the eleven names, in order"), TEXT("the Nest, at the cutters' racks")),
			L(TEXT("nix"), TEXT("LISTENS TO THE DISTRICT breathe and hears what is failing"),
				TEXT("hums back at the pumps"), TEXT("wherever the rhythm is wrong"), true),
			L(TEXT("odalys_crane"), TEXT("HOLDS THE PLAZA with three lamps and forty volunteers"),
				TEXT("does not sleep, and will not say why"), TEXT("the Hub, at the watch post")),
			L(TEXT("ferris_holt"), TEXT("WRITES EVERY LIT BEACON IN INK and every unlit one in memory"),
				TEXT("reads the ledger back to the beacons one at a time"), TEXT("the Hub, at the ledger")),
			L(TEXT("mirea_shen"), TEXT("GUARDS THE NEST GATE against what walks back through"),
				TEXT("sits with her back to the wall, facing the door"), TEXT("the Nest gate"), true),
			L(TEXT("vespera_dal"), TEXT("OWNS THE QUIET and spends it carefully"),
				TEXT("raises her voice to nobody, all evening"), TEXT("the Spire, upper rings")),
			L(TEXT("elias_brume"), TEXT("FILES WHAT PEOPLE MEANT, where it will hurt most"),
				TEXT("rewrites the day's intentions into something worse"), TEXT("the Spire")),
			L(TEXT("sahn"), TEXT("DECODES THE DEAD CHANNELS and answers two of them"),
				TEXT("listens to channels that stopped existing"), TEXT("the Spire, signals room")),
			L(TEXT("the_listener"), TEXT("ASKS THE QUESTION that opened all of this"),
				TEXT("asks it again, quieter"), TEXT("the Deep"), true),
			L(TEXT("pale_host"), TEXT("WEARS THE SILENCE after the noise, and speaks for it"),
				TEXT("stands where the Signal pools and waits"), TEXT("the Deep"), true),
		};
		return List;
	}

	const TArray<FName>& EmptyMix()
	{
		static const TArray<FName> None;
		return None;
	}
}

namespace VEILSociety
{
	const TArray<FVEILArchetype>& Archetypes()
	{
		return BuildArchetypes();
	}

	const FVEILArchetype* FindArchetype(FName Id)
	{
		for (const FVEILArchetype& Arch : BuildArchetypes())
		{
			if (Arch.Id == Id) { return &Arch; }
		}
		return nullptr;
	}

	const TArray<FName>& ArchetypesOf(FName CharacterId)
	{
		// Freely mixed, and deliberately not symmetrical: the quartermaster is a
		// trickster, the keeper calls storms, the notary steps through shadows.
		static const TMap<FName, TArray<FName>> Table = {
			{ FName(TEXT("sera_vahl")),      { TEXT("stormcall"), TEXT("songs") } },
			{ FName(TEXT("ilva_marrow")),    { TEXT("shadowstep"), TEXT("mindwarden") } },
			{ FName(TEXT("domen_arclight")), { TEXT("runner"), TEXT("geomancer") } },
			{ FName(TEXT("bram_kessler")),   { TEXT("ironwright"), TEXT("trickster") } },
			{ FName(TEXT("tova_renn")),      { TEXT("emberwright"), TEXT("ironwright") } },
			{ FName(TEXT("nix")),            { TEXT("runner"), TEXT("tidecaller") } },
			{ FName(TEXT("odalys_crane")),   { TEXT("ironwright"), TEXT("stormcall") } },
			{ FName(TEXT("ferris_holt")),    { TEXT("songs"), TEXT("mindwarden") } },
			{ FName(TEXT("mirea_shen")),     { TEXT("beastbound"), TEXT("ironwright") } },
			{ FName(TEXT("vespera_dal")),    { TEXT("mindwarden"), TEXT("trickster") } },
			{ FName(TEXT("elias_brume")),    { TEXT("trickster"), TEXT("shadowstep") } },
			{ FName(TEXT("sahn")),           { TEXT("tidecaller"), TEXT("mindwarden") } },
			{ FName(TEXT("the_listener")),   { TEXT("hollow"), TEXT("songs") } },
			{ FName(TEXT("pale_host")),      { TEXT("hollow"), TEXT("beastbound") } },
		};
		const TArray<FName>* Found = Table.Find(CharacterId);
		return Found ? *Found : EmptyMix();
	}

	const TArray<FVEILLife>& Lives()
	{
		return BuildLives();
	}

	const FVEILLife* LifeFor(FName CharacterId)
	{
		for (const FVEILLife& L2 : BuildLives())
		{
			if (L2.Character == CharacterId) { return &L2; }
		}
		return nullptr;
	}

	FString ActivityNow(FName CharacterId, bool bDutyBeat)
	{
		const FVEILLife* Their = LifeFor(CharacterId);
		if (!Their) { return S(TEXT("no record")); }
		// A night worker is on post in the quiet hours and off it in the day.
		const bool bOnPost = Their->bNightWorker ? !bDutyBeat : bDutyBeat;
		return bOnPost ? Their->Duty : Their->Rest;
	}
}
