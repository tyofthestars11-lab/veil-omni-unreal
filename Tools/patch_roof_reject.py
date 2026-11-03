"""The unstick may not stand him on a roof - now that there is real ground.

The rejection (FloorZ > 200) was withdrawn when it left the ring search with nothing:
there was no collidable floor at the plaza's own height. PHI GROUND has since laid nine
kit tiles with the top face at z=0, so the rejection can go back in and the search will
find the plaza instead of a shelf at 500 that stands him at 578, above the market.
"""

import io

P = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

OLD = "\t\t\t\t\t\tif (!bFoundFloor) { continue; }"
NEW = ("\t\t\t\t\t\t// A SURFACE ABOVE TWO METRES IS A ROOF. PHI GROUND puts the plaza at\n"
       "\t\t\t\t\t\t// z=0, so a shelf at 500 is no longer the only thing that answers.\n"
       "\t\t\t\t\t\tif (!bFoundFloor || FloorZ > 200.f) { continue; }")

o, n = OLD.replace("\n", NL), NEW.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - count=%d" % src.count(o))
    raise SystemExit(1)
io.open(P, "w", encoding="utf-8", newline="").write(src.replace(o, n, 1))
print("WROTE %s - unstick rejects roofs again" % P)
