"""Widen the stuck detector: a capsule overlapping geometry is wedged with no key down.

Measured with the game's own synthetic move test after the first unstick landed:
"PHI MOVE TEST: velAfter=0.0 locAfter=(-516.158, 472.843, 198.000) moved=0.0" at
maxWalk=450 with input not ignored. The body could not be moved at all, and the
unstick never fired because it waited for a held key. A capsule that overlaps
blocking geometry while standing still is stuck now, key or no key.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src

OLD = (
    "\t\tconst bool bPushing = !MoveAxis.IsNearlyZero();\n"
    "\t\tconst bool bWedged = bPushing && StuckMove\n"
    "\t\t\t&& StuckMove->Velocity.SizeSquared() < 4.f\n"
    "\t\t\t&& StuckMove->GetCurrentAcceleration().SizeSquared() > 100.f;"
)

NEW = (
    "\t\tconst bool bPushing = !MoveAxis.IsNearlyZero();\n"
    "\t\t// A BODY THAT CANNOT BE MOVED IS STUCK EVEN WITH NO KEY DOWN. The -phimove\n"
    "\t\t// proof fed AddMovementInput for 1.2 s and measured moved=0.0 cm, vel=0.0 at\n"
    "\t\t// maxWalk=450 with input ignored=0: the keys were never the problem, the\n"
    "\t\t// capsule is inside something. So the test is geometric as well as\n"
    "\t\t// input-driven - a capsule overlapping blocking geometry is wedged.\n"
    "\t\tbool bOverlapping = false;\n"
    "\t\tif (const UWorld* StuckW = GetWorld())\n"
    "\t\t{\n"
    "\t\t\tFCollisionQueryParams Inside(SCENE_QUERY_STAT(VEILStuckProbe), false, this);\n"
    "\t\t\tbOverlapping = StuckW->OverlapBlockingTestByChannel(GetActorLocation(), GetActorQuat(),\n"
    "\t\t\t\tECC_Pawn, FCollisionShape::MakeCapsule(38.f, 92.f), Inside);\n"
    "\t\t}\n"
    "\t\tconst bool bWedged = StuckMove && StuckMove->Velocity.SizeSquared() < 4.f\n"
    "\t\t\t&& (bOverlapping || (bPushing && StuckMove->GetCurrentAcceleration().SizeSquared() > 100.f));"
)

COOLDOWN_OLD = "\t\t\t\tStuckClock = 0.f;\n\t\t\t\tUWorld* StuckWorld = GetWorld();"
COOLDOWN_NEW = ("\t\t\t\t// A GRACE PERIOD AFTER A MOVE, so a spot that is still tight does not\n"
                "\t\t\t\t// teleport him across the plaza every frame.\n"
                "\t\t\t\tStuckClock = -3.f;\n"
                "\t\t\t\tUWorld* StuckWorld = GetWorld();")

fails = []
for old, new, name in ((OLD, NEW, "wedged test"), (COOLDOWN_OLD, COOLDOWN_NEW, "cooldown")):
    o = old.replace("\n", NL)
    n = new.replace("\n", NL)
    c = text.count(o)
    if c != 1:
        fails.append("%s: found %d (want 1)" % (name, c))
        continue
    text = text.replace(o, n, 1)

if fails:
    print("ABORTED - nothing written")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(PATH, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars" % (PATH, len(src), len(text)))
