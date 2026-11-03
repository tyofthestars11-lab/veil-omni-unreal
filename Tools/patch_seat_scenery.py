"""Scenery is not floor.

The unstick lands the runner on the highest surface under a candidate, and the concept
composition's shelves are 640 cm stone platforms ringing the plaza. That is why he
stands at z=598 - above the market and inside a fall sheet, which is the dark water
filling the photograph. The composition is scenery: the trace now ignores every
AVEILConceptWorld actor, so the lowest real floor is the plaza at z=0 and the stand-off
puts him at z=98.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
fails = []

if '#include "World/VEILConceptWorld.h"' not in text:
    inc = '#include "World/VEILWilds.h"'
    if text.count(inc) != 1:
        fails.append("include anchor count=%d" % text.count(inc))
    else:
        text = text.replace(inc, inc + NL + '#include "World/VEILConceptWorld.h"', 1)

ANCHOR = "\t\t\t\t\tFCollisionQueryParams Probe(SCENE_QUERY_STAT(VEILUnstick), false, this);"
ADD = (
    "\t\t\t\t\tFCollisionQueryParams Probe(SCENE_QUERY_STAT(VEILUnstick), false, this);\n"
    "\t\t\t\t\t// THE COMPOSITION IS SCENERY, NOT FLOOR. Its shelves are 640 cm stone\n"
    "\t\t\t\t\t// platforms around the plaza, and the trace kept standing the runner on one\n"
    "\t\t\t\t\t// at z=598 - above the market and inside a fall sheet, which is the dark\n"
    "\t\t\t\t\t// water that filled the frame. Ignoring them finds the plaza at z=0.\n"
    "\t\t\t\t\tfor (TActorIterator<AVEILConceptWorld> SceneryIt(StuckWorld); SceneryIt; ++SceneryIt)\n"
    "\t\t\t\t\t{\n"
    "\t\t\t\t\t\tProbe.AddIgnoredActor(*SceneryIt);\n"
    "\t\t\t\t\t}"
)

o, n = ANCHOR.replace("\n", NL), ADD.replace("\n", NL)
if text.count(o) != 1:
    fails.append("probe anchor count=%d" % text.count(o))
else:
    text = text.replace(o, n, 1)

if fails:
    print("ABORTED - nothing written")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(PATH, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars (seat ignores composition scenery)" % (PATH, len(src), len(text)))
