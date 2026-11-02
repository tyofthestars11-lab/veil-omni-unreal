# VEIL - AUTHORITY CLAMP PATCH
#
# The seat probe now does its job: at the baked hub it walks the golden angle to a point with
# floor, room and a clear view 2,235 cm out. The clamp then threw that away - anything past
# 800 cm was replaced by the address - and the address itself is buried under the hub's stacked
# masses (Actor_0 at z=500, terraces at 270-2440), so the runner woke inside a wall and the
# camera reported "floor Actor_0 at 0 cm".
#
# The Bible fixes the RUNNER's address at 0,0,100; it does not say he must stand inside the
# Megastructure's base. The clamp stays as the guard against solving dressing problems by moving
# the game kilometres away, but its radius is the plaza ring, not eight metres: a seat the probe
# proved is kept, and the log states the distance from the address either way.
import io
import sys

PATH = r"D:\VEIL-OMNI\Source\VEIL\Private\Game\VEILGameMode.cpp"

OLD = (
    "\t\t\t\tif (FVector::Dist2D(Seat, FVector::ZeroVector) > 800.f)\r\n"
)

NEW = (
    "\t\t\t\t// THE CLAMP IS A LEASH, NOT A CAGE. Eight metres was tight enough that the\r\n"
    "\t\t\t\t// probe's own answer - floor, room, clear view - was discarded and the runner\r\n"
    "\t\t\t\t// put back inside the hub's base. The plaza ring is the boundary of the\r\n"
    "\t\t\t\t// address; past it the address wins, inside it the proved seat does.\r\n"
    "\t\t\t\tconst float AddressLeash = 2600.f;\r\n"
    "\t\t\t\tif (FVector::Dist2D(Seat, FVector::ZeroVector) > AddressLeash)\r\n"
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
