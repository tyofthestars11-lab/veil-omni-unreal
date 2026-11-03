"""CardA is computed before the guard that reads it.

C2065 at VEILHudCanvas.cpp(988): the visibility test asks for CardA, but the clock that
defines it sat inside the block the test guards. The declaration moves above the guard -
ordinary C++ ordering, not a new design decision.
"""

import io

P = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

BLOCK = (
    "\t\t\t// SIX SECONDS, THEN IT STANDS DOWN. The card teaches one gesture and then\n"
    "\t\t\t// stops being a window on the world; it used to sit there all run.\n"
    "\t\t\tstatic int32 CardStep = -1;\n"
    "\t\t\tstatic float CardClock = 0.f;\n"
    "\t\t\tif (Tut->GetStepIndex() != CardStep) { CardStep = Tut->GetStepIndex(); CardClock = 0.f; }\n"
    "\t\t\tCardClock += W->GetDeltaSeconds();\n"
    "\t\t\tconst float CardA = CardClock < 6.f ? 1.f : FMath::Max(0.f, 1.f - (CardClock - 6.f) / 1.5f);\n"
)
GUARD = "\t\tif (!Tut->IsHidden() && !bPageOpen && !bTalking && CardA > 0.02f)"

b = BLOCK.replace("\n", NL)
g = GUARD.replace("\n", NL)
if src.count(b) != 1 or src.count(g) != 1:
    print("ABORTED - block=%d guard=%d" % (src.count(b), src.count(g)))
    raise SystemExit(1)

moved = BLOCK.replace("\t\t\t", "\t\t").replace("\n", NL)
text = src.replace(b, "", 1).replace(g, moved + g, 1)
io.open(P, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s - CardA declared above its guard" % P)
