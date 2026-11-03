"""One owner per zone: the canvas HUD owns the screen, the overlays stop doubling it.

Measured in the frame: three systems draw the same corners. The canvas draws the gold
plate, the minimap, the quest ribbon and the combat pill; VEILHudMockup::Draw adds a
second quest banner, an ACTIVITIES line and a second diamond hotbar into the same
bottom-left corner; VEILOSFace fills the screen with its own face behind them. That is
the overlap - not spacing, ownership.

This pass: the canvas is the only play HUD. The mockup's one unique line - ACTIVITIES -
moves into the canvas under the quest ribbon, where it has its own band. The OS face is
a surface you raise (F1), not a second HUD, so it keeps its own draw call but no longer
paints over the play view. Sizes come from the law: minimap diameter = ClipY / phi,
quest band sits on the same margins.
"""

import io

fails = []

# ------------------------------------------------------------------ canvas
P = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src

OLD_CALLS = (
    "\tVEILHudMockup::Draw(this, Canvas);\n"
    "\tVEILOSFace::Draw(this, Canvas);"
)
NEW_CALLS = (
    "\t// ONE OWNER PER ZONE. The mockup used to draw a second quest banner, a second\n"
    "\t// diamond hotbar and its own ACTIVITIES row into the same bottom-left corner the\n"
    "\t// canvas already owned, and the OS face painted a full-screen panel behind both -\n"
    "\t// three systems, one corner, the overlap in the photograph. The canvas owns the\n"
    "\t// play HUD now; the face is a surface you raise with F1, drawn only while open.\n"
    "\tif (const AVEILPlayerController* OsPC = Cast<AVEILPlayerController>(PC))\n"
    "\t{\n"
    "\t\t		if (OsPC->bShowDebug) { VEILOSFace::Draw(this, Canvas); }\n"
    "\t}"
)
o, n = OLD_CALLS.replace("\n", NL), NEW_CALLS.replace("\n", NL)
if text.count(o) != 1:
    fails.append("overlay calls count=%d" % text.count(o))
else:
    text = text.replace(o, n, 1)

# Minimap: diameter from the law, not a hand-picked 180.
OLD_MS = "\t\tconst float MS = 180.f;                   // spec: 180 x 180"
NEW_MS = (
    "\t\t// MINIMAP DIAMETER = SCREEN HEIGHT / PHI. 1080 / 1.618033988749895 = 667 px,\n"
    "\t\t// 720 / phi = 445 px - the ring is sized by the law, not by a guess.\n"
    "\t\tconst float MS = Canvas->ClipY / UPHIMITO_PHI_SAFE;"
)
o2, n2 = OLD_MS.replace("\n", NL), NEW_MS.replace("\n", NL)
if text.count(o2) != 1:
    fails.append("minimap size count=%d" % text.count(o2))
else:
    text = text.replace(o2, n2, 1)

# ACTIVITIES gets its own band above the quest ribbon.
ANCHOR = "\t\t\t\tconst float QY = Canvas->ClipY - 146.f;"
ACT = (
    "\t\t\t\t// ACTIVITIES ON ITS OWN BAND. It was the mockup's line, drawn 16 px above\n"
    "\t\t\t\t// its own banner while the canvas ribbon sat at ClipY-146: three texts in\n"
    "\t\t\t\t// one strip. It now sits one phi step above the ribbon and nothing else does.\n"
    "\t\t\t\tif (const UVEILCampaignSubsystem* Camp = W->GetSubsystem<UVEILCampaignSubsystem>())\n"
    "\t\t\t\t{\n"
    "\t\t\t\t\tDrawText(Camp->ActivityLine(), FLinearColor(0.788f, 0.635f, 0.290f, 0.95f),\n"
    "\t\t\t\t\t\t30.f, Canvas->ClipY - 146.f - 26.f, Med, 0.56f);\n"
    "\t\t\t\t}\n"
    "\t\t\t\tconst float QY = Canvas->ClipY - 146.f;"
)
if text.count(ANCHOR) != 1:
    fails.append("quest ribbon anchor count=%d" % text.count(ANCHOR))
else:
    text = text.replace(ANCHOR, ACT.replace("\n", NL), 1)

if '#include "Systems/VEILCampaign.h"' not in text:
    inc = '#include "Systems/VEILTime.h"'
    if text.count(inc) != 1:
        fails.append("include anchor count=%d" % text.count(inc))
    else:
        text = text.replace(inc, inc + NL + '#include "Systems/VEILCampaign.h"', 1)

if text != src:
    io.open(P, "w", encoding="utf-8", newline="").write(text)
print("%s: %d -> %d" % (P, len(src), len(text)))

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("HUD ZONES REWRITTEN")
