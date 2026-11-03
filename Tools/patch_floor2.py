"""Carry the lowest hit's actor to the ignore list, or abort without writing.

The multi-depth trace replaced `Floor`, but the cork that keeps the floor out of the
capsule test still asked for `Floor.GetActor()` - C2065. The lowest hit's actor is
kept in its own variable now.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
fails = []


def rep(old, new):
    global text
    o = old.replace("\n", NL)
    n = new.replace("\n", NL)
    c = text.count(o)
    if c != 1:
        fails.append("count=%d: %s" % (c, old.strip().splitlines()[0][:60]))
        return
    text = text.replace(o, n, 1)


rep("\t\t\t\t\t\tbool bFoundFloor = false;\n\t\t\t\t\t\tfloat FloorZ = 0.f;",
    "\t\t\t\t\t\tbool bFoundFloor = false;\n"
    "\t\t\t\t\t\tfloat FloorZ = 0.f;\n"
    "\t\t\t\t\t\tAActor* FloorActor = nullptr;")

rep("\t\t\t\t\t\t\tFloorZ = (float)Hit.ImpactPoint.Z;\n\t\t\t\t\t\t\tbFoundFloor = true;",
    "\t\t\t\t\t\t\tFloorZ = (float)Hit.ImpactPoint.Z;\n"
    "\t\t\t\t\t\t\tFloorActor = Hit.GetActor();\n"
    "\t\t\t\t\t\t\tbFoundFloor = true;")

rep("Free.AddIgnoredActor(Floor.GetActor());", "Free.AddIgnoredActor(FloorActor);")

if fails:
    print("ABORTED - nothing written")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(PATH, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars" % (PATH, len(src), len(text)))
