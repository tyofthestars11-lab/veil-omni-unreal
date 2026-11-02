# VEIL - SEAT FREE-SPACE PATCH
#
# The seat search dismisses everything that is not street level: the slab (top 500), the
# landmark proxies (top 4220), the terraces at 270-2440. Then the final free-space check builds
# NEW collision params that ignore only the pawn - so those same dismissed masses block every
# stepping-stone, "nothing within 34 golden steps was clear", and the runner is clamped back to
# the authority spawn at 0,0 - which is inside them. The camera then reports Actor_0 at 0 cm,
# because that is exactly what is in front of the lens.
#
# The check inherits the query it is finishing: what was not a floor is not a wall either.
import io
import sys

PATH = r"D:\VEIL-OMNI\Source\VEIL\Private\Game\VEILGameMode.cpp"

OLD = (
    "\t\t\t\t\tFCollisionQueryParams FreeParams(SCENE_QUERY_STAT(VeilSeatFree), false);\r\n"
    "\t\t\t\t\tFreeParams.AddIgnoredActor(Pawn);\r\n"
)

NEW = (
    "\t\t\t\t\t// WHAT WAS NOT A FLOOR IS NOT A WALL EITHER.\r\n"
    "\t\t\t\t\t//\r\n"
    "\t\t\t\t\t// This check used to start a fresh query that ignored only the pawn, so the\r\n"
    "\t\t\t\t\t// slab (top 500), the landmark proxies (top 4220) and the terraces the seat\r\n"
    "\t\t\t\t\t// search had already dismissed were blocking again with full authority. Every\r\n"
    "\t\t\t\t\t// candidate inside 34 golden steps read as blocked, the runner was clamped\r\n"
    "\t\t\t\t\t// back to 0,0, and the camera sat inside the mass: \"floor Actor_0 at 0 cm\".\r\n"
    "\t\t\t\t\t// It now inherits the ignored set it is continuing, and adds the body.\r\n"
    "\t\t\t\t\tFCollisionQueryParams FreeParams = Params;\r\n"
    "\t\t\t\t\tFreeParams.AddIgnoredActor(Pawn);\r\n"
)


def main():
    with io.open(PATH, "r", encoding="utf-8", newline="") as f:
        src = f.read()

    if NEW in src:
        print("PATCH: already applied")
        return 0
    if OLD not in src:
        print("PATCH: anchor not found - nothing written")
        return 1

    out = src.replace(OLD, NEW, 1)
    with io.open(PATH, "w", encoding="utf-8", newline="") as f:
        f.write(out)
    print("PATCH: applied, {} -> {} bytes".format(len(src), len(out)))
    return 0


sys.exit(main())
