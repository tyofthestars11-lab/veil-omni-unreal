"""Land the unstick on the LOWEST floor, so the runner returns to the plaza and not
a slab five metres above it, or abort without writing.

Measured: the unstick moved the runner to V(-484, 106, 598) - 98 cm of standoff over
a surface at z=500 - and six frames of held analog input then produced no movement
because that surface is a roof, not the plaza the district is built on at z=0. The
unstick now walks down through every surface under the candidate and keeps the
lowest one, which is the ground the market stands on.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

OLD = (
    "\t\t\t\t\t\tFHitResult Floor;\n"
    "\t\t\t\t\t\tif (!StuckWorld->LineTraceSingleByChannel(Floor, FVector(PX, PY, From.Z + 400.f),\n"
    "\t\t\t\t\t\t\tFVector(PX, PY, From.Z - 400.f), ECC_Visibility, Probe)) { continue; }\n"
    "\t\t\t\t\t\tconst FVector Stand(PX, PY, Floor.ImpactPoint.Z + 98.f);"
)

NEW = (
    "\t\t\t\t\t\t// THE LOWEST FLOOR, NOT THE FIRST ONE. A first-hit trace over the\n"
    "\t\t\t\t\t\t// plaza answers with whatever is highest there - a surface at z=500 that\n"
    "\t\t\t\t\t\t// is a roof, not ground - and the runner was then stood 5 m above the\n"
    "\t\t\t\t\t\t// market and blocked by it. Walk down through every surface and keep the\n"
    "\t\t\t\t\t\t// lowest: that is the plaza the district is built on.\n"
    "\t\t\t\t\t\tbool bFoundFloor = false;\n"
    "\t\t\t\t\t\tfloat FloorZ = 0.f;\n"
    "\t\t\t\t\t\tFVector Down(PX, PY, From.Z + 400.f);\n"
    "\t\t\t\t\t\tfor (int32 Depth = 0; Depth < 5; ++Depth)\n"
    "\t\t\t\t\t\t{\n"
    "\t\t\t\t\t\t\tFHitResult Hit;\n"
    "\t\t\t\t\t\t\tif (!StuckWorld->LineTraceSingleByChannel(Hit, Down,\n"
    "\t\t\t\t\t\t\t\tFVector(PX, PY, From.Z - 4000.f), ECC_Visibility, Probe)) { break; }\n"
    "\t\t\t\t\t\t\tFloorZ = (float)Hit.ImpactPoint.Z;\n"
    "\t\t\t\t\t\t\tbFoundFloor = true;\n"
    "\t\t\t\t\t\t\tDown = FVector(PX, PY, FloorZ - 5.f);\n"
    "\t\t\t\t\t\t}\n"
    "\t\t\t\t\t\tif (!bFoundFloor) { continue; }\n"
    "\t\t\t\t\t\tconst FVector Stand(PX, PY, FloorZ + 98.f);"
)

o = OLD.replace("\n", NL)
n = NEW.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - block found %d times (want 1)" % src.count(o))
    raise SystemExit(1)

out = src.replace(o, n, 1)
io.open(PATH, "w", encoding="utf-8", newline="").write(out)
print("WROTE %s: %d -> %d chars" % (PATH, len(src), len(out)))
