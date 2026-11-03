# -*- coding: utf-8 -*-
"""Stop saying it twice, and give the cast something worth hearing.

1. The ONE chip printed its own label ("ONE - click to speak or type") and then
   StateLine() underneath, whose Ready branch said "ready - click to speak or
   type": one control, two copies of the same sentence. Ready now says "ready"
   and the chip hides the second row entirely unless ONE is actually working,
   listening or speaking - so the instruction appears once, where you click it.

2. The cast's greetings were serviceable and flat ("You stand in the one place
   the Signal never reached"). Each of the twenty-four now has three lines in
   their own register: Ashfall in quantities and prices, the Watch in liturgy and
   exhaustion, the Court in contracts and fees, the Hollow in second person and
   broken time. The lines are added as a voice table keyed by cast id and applied
   over the definitions, so the data stays in one place and nothing is duplicated
   by hand.
"""
import io
import sys

ROOT = "D:/VEIL-OMNI/"
fails = []


def read(p):
    with io.open(ROOT + p, "r", encoding="utf-8", newline="") as f:
        raw = f.read()
    return raw.replace("\r\n", "\n"), ("\r\n" in raw)


def write(p, text, crlf):
    with io.open(ROOT + p, "w", encoding="utf-8", newline="") as f:
        f.write(text.replace("\n", "\r\n") if crlf else text)


# ---------------------------------------------------------------------------
# 1. HUD: no second copy of the instruction.
# ---------------------------------------------------------------------------
HUD = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
hud, hud_crlf = read(HUD)
old_hud = "\t\tif (One && !bBusy)\n"
new_hud = ("\t\t// SAY IT ONCE. The label above already says \"click to speak or type\"; the state\n"
           "\t\t// line said it again whenever ONE was idle, so the chip was one instruction printed\n"
           "\t\t// twice. The second row exists to report work, listening or an answer - nothing else.\n"
           "\t\tif (One && !bBusy && OneState != EONEState::Ready)\n")
if old_hud in hud:
    hud = hud.replace(old_hud, new_hud, 1)
elif "SAY IT ONCE" not in hud:
    fails.append("HUD state-line guard anchor missing")

# ---------------------------------------------------------------------------
# 2. ONE's own state line: "ready" is a state, not a manual.
# ---------------------------------------------------------------------------
ONE = "Source/VEIL/Private/Systems/PHIONE.cpp"
one, one_crlf = read(ONE)
if 'TEXT("ready - click to speak or type")' in one:
    one = one.replace('TEXT("ready - click to speak or type")', 'TEXT("ready")', 1)
elif 'TEXT("ready")' not in one:
    fails.append("ONE ready text anchor missing")

# ---------------------------------------------------------------------------
# 3. The voice table.
# ---------------------------------------------------------------------------
CAST = "Source/VEIL/Private/Story/VEILCast.cpp"
cast, cast_crlf = read(CAST)

VOICES = r'''
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
'''

if "BuildVoices" not in cast:
    anchor = "namespace VEILCast\n{\n\tconst TArray<FVEILFactionDef>& Factions()"
    if cast.count(anchor) != 1:
        fails.append("VEILCast.cpp voice anchor count=%d" % cast.count(anchor))
    else:
        cast = cast.replace(anchor, VOICES + "\n" + anchor, 1)

    chars_anchor = ("\tconst TArray<FVEILCharacterDef>& Characters()\n"
                    "\t{\n"
                    "\t\treturn BuildCharacters();\n"
                    "\t}")
    chars_new = ("\tconst TArray<FVEILCharacterDef>& Characters()\n"
                 "\t{\n"
                 "\t\t// The definitions carry the names and the history; the voice table carries what\n"
                 "\t\t// they say. Applying it here means every caller - the dialogue box, the HUD, the\n"
                 "\t\t// living world - hears the same person.\n"
                 "\t\tstatic const TArray<FVEILCharacterDef> List = []()\n"
                 "\t\t{\n"
                 "\t\t\tTArray<FVEILCharacterDef> Built = BuildCharacters();\n"
                 "\t\t\tconst TMap<FName, TArray<FText>>& Voices = BuildVoices();\n"
                 "\t\t\tfor (FVEILCharacterDef& C : Built)\n"
                 "\t\t\t{\n"
                 "\t\t\t\tif (const TArray<FText>* V = Voices.Find(C.Id)) { C.Greetings = *V; }\n"
                 "\t\t\t}\n"
                 "\t\t\treturn Built;\n"
                 "\t\t}();\n"
                 "\t\treturn List;\n"
                 "\t}")
    if cast.count(chars_anchor) != 1:
        fails.append("VEILCast.cpp Characters anchor count=%d" % cast.count(chars_anchor))
    else:
        cast = cast.replace(chars_anchor, chars_new, 1)
else:
    print("voice table already present")

if fails:
    print("FAILS:", fails)
    sys.exit(1)

write(HUD, hud, hud_crlf)
write(ONE, one, one_crlf)
write(CAST, cast, cast_crlf)

print("hud once =", "SAY IT ONCE" in hud)
print("one ready short =", 'TEXT("ready")' in one)
print("voices =", cast.count("M.Add(FName(TEXT("), "entries")
print("PATCH: PASS")
