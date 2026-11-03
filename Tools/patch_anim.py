"""Fix the animation driver guard, or abort without writing.

`if (Body->GetAnimInstance() != nullptr) return;` was meant to let an authored
AnimBlueprint win. But PlayAnimation() creates an AnimSingleNodeInstance itself, so
from the second frame on the guard was always true: the body played MM_Idle once at
speed 0 and never changed again. The runner then slides in an idle pose - the
"South Park" look - and the log says so: the only transition ever printed is
"PHI BODY MOTION: playing MM_Idle (speed 0)".
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
fails = []

OLD = (
    "\t// If a real AnimBlueprint owns the body one day, it wins; otherwise the\n"
    "\t// sequences drive it. Nothing here overrides an authored animation graph.\n"
    "\tif (Body->GetAnimInstance() != nullptr) return;"
)

NEW = (
    "\t// A REAL ANIM GRAPH WINS - A SINGLE-NODE PLAYER DOES NOT. This used to return\n"
    "\t// whenever GetAnimInstance() was non-null, and PlayAnimation() ITSELF creates an\n"
    "\t// AnimSingleNodeInstance: from the second frame the guard was always true, so the\n"
    "\t// body played MM_Idle once at speed 0 and never changed again. That is the\n"
    "\t// \"South Park\" look - an idle pose sliding at full speed - and the log proves\n"
    "\t// it: \"playing MM_Idle (speed 0)\" is the only transition ever printed.\n"
    "\tUAnimInstance* CurrentAnim = Body->GetAnimInstance();\n"
    "\tif (CurrentAnim && !CurrentAnim->IsA<UAnimSingleNodeInstance>()) { return; }"
)

o = OLD.replace("\n", NL)
n = NEW.replace("\n", NL)
if text.count(o) != 1:
    fails.append("guard count=%d" % text.count(o))
else:
    text = text.replace(o, n, 1)

inc_anchor = '#include "Animation/AnimInstance.h"'
if '#include "Animation/AnimSingleNodeInstance.h"' not in text:
    if text.count(inc_anchor) != 1:
        fails.append("include anchor count=%d" % text.count(inc_anchor))
    else:
        text = text.replace(inc_anchor,
                            inc_anchor + NL + '#include "Animation/AnimSingleNodeInstance.h"', 1)

if fails:
    print("ABORTED - nothing written")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(PATH, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars" % (PATH, len(src), len(text)))
