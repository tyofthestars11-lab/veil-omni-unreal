"""The unstick may not stand him on a roof.

Every run ends with PHI UNSTICK moving him to z=578 - a surface five metres above the
plaza - and the authority seat placed in BeginPlay is undone a moment later by that
timer. The candidate floors are now filtered: a surface above 200 cm is a roof or a
shelf, not ground, so the search keeps going and finds the plaza at z=0.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

OLD = "\t\t\t\t\t\tif (!bFoundFloor) { continue; }"
NEW = (
    "\t\t\t\t\t\t// A FLOOR ABOVE TWO METRES IS A ROOF. The ring search kept accepting a\n"
    "\t\t\t\t\t\t// shelf at z=500 and standing him at 578, inside a fall sheet - which is\n"
    "\t\t\t\t\t\t// the water that filled every photograph. Reject it and keep looking.\n"
    "\t\t\t\t\t\tif (!bFoundFloor || FloorZ > 200.f) { continue; }"
)

o, n = OLD.replace("\n", NL), NEW.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - count=%d" % src.count(o))
    raise SystemExit(1)
io.open(PATH, "w", encoding="utf-8", newline="").write(src.replace(o, n, 1))
print("WROTE %s - unstick rejects roofs above 200 cm" % PATH)
