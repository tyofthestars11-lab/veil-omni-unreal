"""Point the save proof at UVEILProgression::SaveGame(), or abort without writing.

UPHICheats::SaveNow is private (C2248). The public writer is
UVEILProgression::SaveGame(), which returns whether the slot was written.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
fails = []

OLD = (
    "\t\t\t\tif (UPHICheats* Cheats = ProofWorld->GetSubsystem<UPHICheats>())\n"
    "\t\t\t\t{\n"
    "\t\t\t\t\tCheats->SaveNow();\n"
    "\t\t\t\t\tUE_LOG(LogVEIL, Warning, TEXT(\"PHI SAVE PROOF: SaveNow called after the move test.\"));\n"
    "\t\t\t\t}"
)

NEW = (
    "\t\t\t\t// THE PUBLIC WRITER, NOT THE CHEAT PANEL. UPHICheats::SaveNow is private;\n"
    "\t\t\t\t// UVEILProgression::SaveGame is the slot writer and returns whether it wrote.\n"
    "\t\t\t\tif (UVEILProgression* Prog = ProofWorld->GetSubsystem<UVEILProgression>())\n"
    "\t\t\t\t{\n"
    "\t\t\t\t\tconst bool bSaved = Prog->SaveGame();\n"
    "\t\t\t\t\tUE_LOG(LogVEIL, Warning,\n"
    "\t\t\t\t\t\tTEXT(\"PHI SAVE PROOF: slot %s written=%d after the move test.\"),\n"
    "\t\t\t\t\t\t*UVEILProgression::SaveSlotName, bSaved ? 1 : 0);\n"
    "\t\t\t\t}"
)

o = OLD.replace("\n", NL)
n = NEW.replace("\n", NL)
if text.count(o) != 1:
    fails.append("save block count=%d" % text.count(o))
else:
    text = text.replace(o, n, 1)

inc = '#include "Systems/PHICheats.h"'
if inc in text and '#include "Systems/VEILProgression.h"' not in text:
    text = text.replace(inc, '#include "Systems/VEILProgression.h"', 1)

if fails:
    print("ABORTED - nothing written")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(PATH, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars" % (PATH, len(src), len(text)))
