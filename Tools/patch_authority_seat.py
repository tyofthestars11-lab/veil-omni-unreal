"""Put the runner on his authority coordinate.

Measured three runs in a row: the unstick relocates him to V(-484, 106, 578) - on a
surface five metres above the plaza and inside a fall sheet, which is the dark water
filling the frame. The authority coordinate for RUNNER is 0,0,100. After the sweep, if
he is standing above 200 he is placed there, physics-teleported, and the placement is
logged so the next run states where he actually is.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

ANCHOR = "\t\tSweepBlue();"
ADD = (
    "\t\tSweepBlue();\n"
    "\t\t// HIS OWN COORDINATE, NOT WHATEVER SURFACE ANSWERS FIRST. RUNNER is 0,0,100.\n"
    "\t\t// The unstick keeps relocating him to a shelf at z=578, above the market and\n"
    "\t\t// inside a fall sheet, which is the water that filled the photographs.\n"
    "\t\tif (GetActorLocation().Z > 200.f)\n"
    "\t\t{\n"
    "\t\t\tconst FVector AuthoritySeat(0.f, 0.f, 100.f);\n"
    "\t\t\tSetActorLocation(AuthoritySeat, false, nullptr, ETeleportType::TeleportPhysics);\n"
    "\t\t\tUE_LOG(LogVEIL, Warning,\n"
    "\t\t\t\tTEXT(\"PHI AUTHORITY SEAT: runner placed on his own coordinate V(0,0,100) from %s.\"),\n"
    "\t\t\t\t*GetActorLocation().ToCompactString());\n"
    "\t\t}"
)

o, n = ANCHOR.replace("\n", NL), ADD.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - anchor count=%d" % src.count(o))
    raise SystemExit(1)
io.open(PATH, "w", encoding="utf-8", newline="").write(src.replace(o, n, 1))
print("WROTE %s - runner stands on his authority coordinate" % PATH)
