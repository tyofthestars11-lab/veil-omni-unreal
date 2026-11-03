# -*- coding: utf-8 -*-
"""ONE CAST, NOT TWO.

The plaza machines had a 24-name table of their own while the authored cast in
Story/VEILCast.cpp held the lines they actually spoke, so a patroller could wear
one name and talk under another. This patch makes VEILCast the single roster:
name, rank and CharacterId all come from it, each machine claims a person nobody
else in the district has, and nine more authored people bring the roster to 24 so
every faction has six.

Idempotent guards: every edit counts its anchor and refuses a second run.
"""
import io
import sys

ROOT = "D:/VEIL-OMNI/"
fails = []


def read(p):
    with io.open(ROOT + p, "r", encoding="utf-8", newline="") as f:
        raw = f.read()
    crlf = "\r\n" in raw
    return raw.replace("\r\n", "\n"), crlf


def write(p, text, crlf):
    if crlf:
        text = text.replace("\n", "\r\n")
    with io.open(ROOT + p, "w", encoding="utf-8", newline="") as f:
        f.write(text)


# ---------------------------------------------------------------------------
# 1. VEILCast.cpp - nine more authored people, six per faction.
# ---------------------------------------------------------------------------
CAST_PATH = "Source/VEIL/Private/Story/VEILCast.cpp"
cast, cast_crlf = read(CAST_PATH)

NEW_PEOPLE = r'''			{
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
'''

if "the_third_echo" not in cast:
    # Insert after the last authored person (the pale host), before BuildCharacters closes.
    idx = cast.find('{ CastT(TEXT("You are made of noise.')
    tail = cast.find("\t\t};\n\t\treturn List;", idx) if idx >= 0 else -1
    if idx < 0 or tail < 0:
        fails.append("VEILCast.cpp insert point %d/%d" % (idx, tail))
    else:
        cast = cast[:tail] + NEW_PEOPLE + cast[tail:]
else:
    print("VEILCast.cpp: new people already present")

# ---------------------------------------------------------------------------
# 2. VEILFactionAI.h - remember which authored person this machine is.
# ---------------------------------------------------------------------------
H_PATH = "Source/VEIL/Public/AI/VEILFactionAI.h"
h, h_crlf = read(H_PATH)
if "CastCharacterId" not in h:
    h_anchor = "\tFString CastName;\n"
    if h.count(h_anchor) != 1:
        fails.append("VEILFactionAI.h anchor count=%d" % h.count(h_anchor))
    else:
        h = h.replace(h_anchor,
            "\t/** The authored cast member this machine IS (Story/VEILCast.cpp). The nameplate and the\n"
            "\t *  conversation both read it, so the person you see is the person who answers [E]. */\n"
            "\tFName CastCharacterId;\n" + h_anchor, 1)
else:
    print("VEILFactionAI.h: CastCharacterId already present")

# ---------------------------------------------------------------------------
# 3. VEILFactionAI.cpp - one roster, claimed once per district.
# ---------------------------------------------------------------------------
CPP_PATH = "Source/VEIL/Private/AI/VEILFactionAI.cpp"
cpp, cpp_crlf = read(CPP_PATH)

if "CastCharacterId = Who->Id" not in cpp:
    start_marker = "\t// A PERSON, NOT A PLACEHOLDER."
    end_marker = "\t\tTArray<UStaticMeshComponent*> BodyMeshes;"
    si, ei = cpp.find(start_marker), cpp.find(end_marker)
    if si < 0 or ei < 0 or ei < si:
        fails.append("VEILFactionAI.cpp block markers %d/%d" % (si, ei))
    else:
        NEW_BLOCK = r'''	// A PERSON, NOT A PLACEHOLDER. The roster is the authored cast in Story/VEILCast.cpp, not
	// a second name list beside it: the name on the plate is the person who answers [E], and
	// their lines are the ones the cast file wrote for them. Faction, row and temperament come
	// from the machine's own id; no two machines in a district claim the same person; and the
	// same seat rebuilds the same roster every session. Temperament changes how it fights
	// rather than labelling it, and body scale and hue follow the row, so silhouette differs
	// before anyone reads a name.
	{
		static const TCHAR* FactionWords[4] = {
			TEXT("Ashfall Consortium"), TEXT("Lantern Watch"), TEXT("Quiet Court"), TEXT("Hollow") };
		const uint32 IdHash = (uint32)GetUniqueID() * 2654435761u;
		switch ((int32)(IdHash % 4))
		{
		case 0: Faction = EVEILFaction::Scavenger; break;
		case 1: Faction = EVEILFaction::Guardian; break;
		case 2: Faction = EVEILFaction::Remnant; break;
		default: Faction = EVEILFaction::Void; break;
		}
		// The faction's own authored people, in cast order.
		TArray<FName> Members;
		for (const FVEILCharacterDef& C : VEILCast::Characters())
		{
			if (const FVEILFactionDef* F = VEILCast::FindFaction(C.FactionId))
			{
				if (F->Faction == Faction) { Members.Add(C.Id); }
			}
		}
		int32 Row = 0;
		if (Members.Num() > 0)
		{
			// Names already claimed in this district. BeginPlay order is whatever the level
			// gives us, so each machine looks at the ones that have spoken up so far.
			TSet<FName> Taken;
			if (UWorld* W = GetWorld())
			{
				for (TActorIterator<AVEILFactionAgent> It(W); It; ++It)
				{
					if (&*It != this && !It->CastCharacterId.IsNone()) { Taken.Add(It->CastCharacterId); }
				}
			}
			Row = (int32)((IdHash >> 5) % (uint32)Members.Num());
			for (int32 Step = 0; Step < Members.Num(); ++Step)
			{
				const int32 Try = (Row + Step) % Members.Num();
				if (!Taken.Contains(Members[Try])) { Row = Try; break; }
			}
			if (const FVEILCharacterDef* Who = VEILCast::FindCharacter(Members[Row]))
			{
				CastCharacterId = Who->Id;
				CastName = Who->Name.ToString();
				CastRank = Who->Title.ToString();
			}
		}
		// Temperament is a separate rung so two of a faction's people still fight differently.
		static const TCHAR* Moods[3] = { TEXT("aggressive"), TEXT("patient"), TEXT("curious") };
		CastTemperament = Moods[(IdHash >> 11) % 3];
		if (CastTemperament == TEXT("aggressive"))
		{
			MaxHealth *= 0.85f; AttackRange *= 1.15f; WeaponDamage *= 1.35f; FireInterval *= 0.80f;
		}
		else if (CastTemperament == TEXT("patient"))
		{
			MaxHealth *= 1.25f; AttackRange *= 0.85f; WeaponDamage *= 0.80f; SightRange *= 1.30f;
		}
		else
		{
			SightRange *= 1.10f; FireInterval *= 1.15f;
		}
		Health = MaxHealth;
		const int32 CIdx = Members.Num() ? (Row % 6) : 0;
		const int32 FIdx = (int32)(IdHash % 4);
'''
        cpp = cpp[:si] + NEW_BLOCK + cpp[ei:]
else:
    print("VEILFactionAI.cpp: roster already unified")

if '#include "Story/VEILCast.h"' not in cpp:
    inc_anchor = '#include "Story/VEILCastBody.h"\n'
    if cpp.count(inc_anchor) != 1:
        fails.append("VEILFactionAI.cpp include anchor count=%d" % cpp.count(inc_anchor))
    else:
        cpp = cpp.replace(inc_anchor, inc_anchor + '#include "Story/VEILCast.h"\n', 1)
if '#include "EngineUtils.h"' not in cpp:
    eu_anchor = '#include "Engine/Engine.h"\n'
    if cpp.count(eu_anchor) != 1:
        fails.append("VEILFactionAI.cpp EngineUtils anchor count=%d" % cpp.count(eu_anchor))
    else:
        cpp = cpp.replace(eu_anchor, eu_anchor + '#include "EngineUtils.h"\n', 1)

# ---------------------------------------------------------------------------
# 4. The conversation and the plate read the machine's own cast id.
# ---------------------------------------------------------------------------
GM_PATH = "Source/VEIL/Private/Game/VEILGameMode.cpp"
gm, gm_crlf = read(GM_PATH)
old_gm = "Best = VEILCast::CharacterForFaction(It->Faction, It->GetUniqueID());"
new_gm = ("Best = It->CastCharacterId.IsNone()\n"
          "\t\t\t? VEILCast::CharacterForFaction(It->Faction, It->GetUniqueID())\n"
          "\t\t\t: It->CastCharacterId;")
if old_gm in gm:
    gm = gm.replace(old_gm, new_gm, 1)
elif "It->CastCharacterId" not in gm:
    fails.append("VEILGameMode.cpp speaker anchor missing")

HUD_PATH = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
hud, hud_crlf = read(HUD_PATH)
old_hud = "\t\t\tconst FName Who = VEILCast::CharacterForFaction(It->Faction, It->GetUniqueID());"
new_hud = ("\t\t\tconst FName Who = It->CastCharacterId.IsNone()\n"
           "\t\t\t\t? VEILCast::CharacterForFaction(It->Faction, It->GetUniqueID())\n"
           "\t\t\t\t: It->CastCharacterId;")
if old_hud in hud:
    hud = hud.replace(old_hud, new_hud, 1)
elif "It->CastCharacterId" not in hud:
    fails.append("VEILHudCanvas.cpp plate anchor missing")

# ---------------------------------------------------------------------------
# Write + report.
# ---------------------------------------------------------------------------
if fails:
    print("FAILS:", fails)

write(CAST_PATH, cast, cast_crlf)
write(H_PATH, h, h_crlf)
write(CPP_PATH, cpp, cpp_crlf)
write(GM_PATH, gm, gm_crlf)
write(HUD_PATH, hud, hud_crlf)

chars = cast.count("FName(TEXT(\"")  # includes factions and characters
print("cast entries touched, characters in file =", cast.count("Greetings") or "")
print("people in VEILCast.cpp =", cast.count("CastT(TEXT(\"THE ") + cast.count("CastT(TEXT(\""))
print("unified marker =", "CastCharacterId = Who->Id" in cpp)
print("hot include =", '#include "Story/VEILCast.h"' in cpp)
print("gm reads id =", "It->CastCharacterId" in gm)
print("hud reads id =", "It->CastCharacterId" in hud)
print("PATCH: %s" % ("PASS" if not fails else "FAIL"))
sys.exit(1 if fails else 0)
