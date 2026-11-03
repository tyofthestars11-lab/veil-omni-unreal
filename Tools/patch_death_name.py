# -*- coding: utf-8 -*-
"""Name the killer, not the object.

Damage and death lines printed the raw actor name (VEILFactionAgent_8), which is
the loudest placeholder tell left in the game. A machine that has a cast id now
prints its authored name and rank in both the damage log and the death line, so
"the runner went down to Tova Renn, Cutter-Captain" is what the log says.

Also syncs VEIL-CAST.md with the nine people added to the roster.
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
# 1. VEILCharacter.cpp - the causer is a person when it can be.
# ---------------------------------------------------------------------------
CPP = "Source/VEIL/Private/Character/VEILCharacter.cpp"
cpp, cpp_crlf = read(CPP)

if "CauserName" not in cpp:
    if '#include "AI/VEILFactionAI.h"' not in cpp:
        i = cpp.find('#include "')
        if i < 0:
            fails.append("VEILCharacter.cpp has no includes")
        else:
            j = cpp.find("\n", i) + 1
            cpp = cpp[:j] + '#include "AI/VEILFactionAI.h"\n' + cpp[j:]

    HELPER = '''	// WHO HIT YOU IS A PERSON. This logged the raw object name for weeks, so a
	// death read "VEILFactionAgent_8" - the one place in the game still speaking in
	// asset names. The causer (or the projectile's owner) resolves to its cast row.
	const FString CauserName = [DamageCauser, EventInstigator]() -> FString
	{
		AActor* A = DamageCauser;
		if (!A && EventInstigator) { A = EventInstigator->GetPawn(); }
		const AVEILFactionAgent* Who = Cast<AVEILFactionAgent>(A);
		if (!Who && A) { Who = Cast<AVEILFactionAgent>(A->GetOwner()); }
		if (Who && !Who->CastName.IsEmpty())
		{
			return Who->CastRank.IsEmpty()
				? Who->CastName
				: FString::Printf(TEXT("%s, %s"), *Who->CastName, *Who->CastRank);
		}
		return A ? A->GetName() : FString(TEXT("(no causer)"));
	}();
'''
    anchor = "\t// EVERY BLOW NAMES ITSELF."
    if cpp.count(anchor) != 1:
        fails.append("VEILCharacter.cpp blow anchor count=%d" % cpp.count(anchor))
    else:
        cpp = cpp.replace(anchor, HELPER + anchor, 1)

    old = "DamageCauser ? *DamageCauser->GetName() : TEXT(\"(no causer)\")"
    n = cpp.count(old)
    if n < 2:
        fails.append("VEILCharacter.cpp causer uses=%d" % n)
    else:
        cpp = cpp.replace(old, "*CauserName")
else:
    print("VEILCharacter.cpp: causer already named")

# ---------------------------------------------------------------------------
# 2. VEIL-CAST.md - the mirror gains the nine new people.
# ---------------------------------------------------------------------------
MD = "VEIL-CAST.md"
md, md_crlf = read(MD)

WATCH = '''
**HALDEN ROOK** — *Lamp-Warden of the Spire Stair*
Keeps the two hundred and eleven lamps on the stair by hand, one at a time, because the last
time the Watch trusted a mechanism the stair went dark for a year. Counts them aloud on the way
up; it is the only prayer the Watch still says.

**ISET MARR** — *Watch-Sergeant, Night Ledger*
Commands the night shift — the only people awake when the Signal is loudest. Writes down what her
patrols hear and burns the pages that answer.
'''

COURT = '''
**CORIN VEY** — *Quiet Bailiff*
Enforces the Court's silence with a ledger and a key. Has never drawn a weapon in the Spire and
has never needed to: the rooms he locks stay locked because the people inside agree with him.

**ADEA LUNE** — *Court Archivist of the Sealed Rings*
Keeps every broadcast the world ever made, filed by what it was afraid of. She has read the last
one and has not slept in the Spire since, which the Court records as dedication.
'''

ASHFALL = '''
**KESH VARO** — *Hull-Wright of the Nest Dry-Docks*
Welds hulls that were never built to hold air, well enough that the Consortium forgives the eleven
ships that came back empty. Keeps their manifest bolts in a jar and calls it inventory.
'''

HOLLOW = '''
**THE THIRD ECHO** — *Hollow Echo, three seconds late*
Every answer the Hollow gives arrives late, and the Echo is the lateness: it repeats the district
three seconds after it has finished speaking, in a tone that is almost right.

**ASH-WITNESS** — *The Hollow's memory*
Has watched the same square metre of the Deep for a century and remembers every foot that crossed
it, including the ones that never arrived.

**THE PALE CHOIR** — *Many voices, one throat*
Where the Signal pooled in a stairwell it learned harmony. A hymn that is also a census, sung at
whoever is closest.

**THE ANSWERER** — *Hollow Echo, speaking first*
The Listener asked; the Answerer is what came back early. It answers questions nobody has asked
yet, which is why the Watch fires on it before it finishes a sentence.
'''

if "HALDEN ROOK" not in md:
    for heading, block in (("### The Quiet Court", WATCH),
                          ("### The Ashfall Consortium", COURT),
                          ("### The Hollow", ASHFALL),
                          ("## Where you meet them", HOLLOW)):
        n = md.count("\n" + heading + "\n")
        if n != 1:
            fails.append("VEIL-CAST.md heading %s count=%d" % (heading, n))
            continue
        md = md.replace("\n" + heading + "\n", block + "\n" + heading + "\n", 1)
else:
    print("VEIL-CAST.md: new people already present")

note = ("- **Nameplates and voices are the same person.** The roster this page mirrors is what the\n"
        "  machines claim: a patroller's plate, its rank and the lines it speaks under **[E]** all come\n"
        "  from one cast row, and no two machines in a district claim the same person.\n")
if "Nameplates and voices are the same person" not in md:
    n = md.count("## The conversation box")
    if n != 1:
        fails.append("VEIL-CAST.md conversation heading count=%d" % n)
    else:
        md = md.replace("## The conversation box", note + "\n## The conversation box", 1)

if fails:
    print("FAILS:", fails)

write(CPP, cpp, cpp_crlf)
write(MD, md, md_crlf)

print("causer named =", "CauserName" in cpp)
print("md people =", md.count("**"), "| new head =", "HALDEN ROOK" in md)
print("PATCH: %s" % ("PASS" if not fails else "FAIL"))
sys.exit(1 if fails else 0)
