"""Spawn the runner on the plaza floor at his own coordinate, not a shelf.

PHI GROUND lays nine kit tiles with the top face at z=0. The spawn path then seats him
with a golden-angle step off the anchor and a ground snap, and every run has measured
that seat landing at V(-484, 106, 578) - five metres above the plaza, inside a fall sheet.
RUNNER's authority coordinate is 0,0,100, which is 98 cm of stand-off over the new floor.
The seat is stated once, right before the pawn is spawned, so the camera, the unstick and
the movement proof all begin from the market rather than from a roof.
"""

import io

P = "Source/VEIL/Private/Game/VEILGameMode.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

ANCHOR = "\tFActorSpawnParameters SpawnInfo;"
INSERT = """	// ON THE PLAZA FLOOR, AT HIS OWN COORDINATE. PHI GROUND lays nine kit tiles with the
	// top face at z=0 and RUNNER is 0,0,100 - 98 cm of stand-off over the market. The
	// golden-angle step off the anchor seeded him onto a shelf at 578 every run, which is
	// the floating frame: a man standing five metres above his own district.
	Seat = FVector(0.f, 0.f, 100.f);
	""" + ANCHOR

o, n = ANCHOR.replace("\n", NL), INSERT.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - anchor count=%d" % src.count(o))
    raise SystemExit(1)
io.open(P, "w", encoding="utf-8", newline="").write(src.replace(o, n, 1))
print("WROTE %s - runner spawns on V(0,0,100) over the new floor" % P)
