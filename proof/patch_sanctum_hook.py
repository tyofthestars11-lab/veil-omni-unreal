# VEIL - SANCTUM HOOK PATCH
#
# The sanctum is only needed where the hub is already baked and the builder stands down - that is
# exactly the case the buried plaza comes from. It is raised right after the stage decision, before
# the seat timer fires 0.5 s later, so the probe finds the deck rather than the mass.
import io
import sys

PATH = r"D:\VEIL-OMNI\Source\VEIL\Private\Game\VEILGameMode.cpp"

OLD_INCLUDE = '#include "Game/VEILGameMode.h"\r\n'
NEW_INCLUDE = (
    '#include "Game/VEILGameMode.h"\r\n'
    '#include "World/VEILNestSanctum.h"\r\n'
)

OLD_STAGE = (
    "\t\telse if (bPlacedWorld) { Builder->BuildDistrictLayer(); }\r\n"
    "\t\telse { Builder->BuildWorld(); }\r\n"
)

NEW_STAGE = (
    "\t\telse if (bPlacedWorld) { Builder->BuildDistrictLayer(); }\r\n"
    "\t\telse { Builder->BuildWorld(); }\r\n"
    "\r\n"
    "\t\t// THE NEST HAS A FLOOR EVEN WHEN THE HUB IS BURIED.\r\n"
    "\t\t//\r\n"
    "\t\t// On a baked world the builder stands down and the level is the world - and at the\r\n"
    "\t\t// plaza centre that level stacks roofs at 500, 1250, 1751 and 2440 cm, so the\r\n"
    "\t\t// authority spawn at 0,0,100 lands inside solid geometry and the runner wakes with\r\n"
    "\t\t// the camera in a wall. The sanctum measures the surface the hub actually has and\r\n"
    "\t\t// raises its deck there, then dresses it: ring, pillars, Spire, beacon shafts and\r\n"
    "\t\t// water. It is built before the seat timer below, so the probe lands on the deck.\r\n"
    "\t\tif (bAlreadyBuilt)\r\n"
    "\t\t{\r\n"
    "\t\t\tif (AVEILNestSanctum* Sanctum = GetWorld()->SpawnActor<AVEILNestSanctum>(\r\n"
    "\t\t\t\tFVector::ZeroVector, FRotator::ZeroRotator))\r\n"
    "\t\t\t{\r\n"
    "\t\t\t\tSanctum->BuildSanctum();\r\n"
    "\t\t\t}\r\n"
    "\t\t}\r\n"
)


def main():
    with io.open(PATH, "r", encoding="utf-8", newline="") as f:
        src = f.read()

    if "World/VEILNestSanctum.h" in src and "AVEILNestSanctum" in src:
        print("already applied")
        return 0

    if OLD_INCLUDE not in src:
        print("include anchor not found")
        return 1
    if OLD_STAGE not in src:
        print("stage anchor not found")
        return 1

    src = src.replace(OLD_INCLUDE, NEW_INCLUDE, 1)
    src = src.replace(OLD_STAGE, NEW_STAGE, 1)

    with io.open(PATH, "w", encoding="utf-8", newline="") as f:
        f.write(src)
    print("hook applied")
    return 0


sys.exit(main())
