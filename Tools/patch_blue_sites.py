"""Remove the last blue-material handles from the district's builders.

Two builders still resolve M_PHI_DeepBlue into geometry that is not crystal and not
water: VEILWorldBuilder::DressRegions (its own Place lambda, separate from the palette
pass) and PHIGear. Both keep their variable names so no call site changes; only what
those names load changes. Blue stays where it is the vision - crystal cones in
ContentFamilies and Wilds, water in the Ocean/Nest/falls - and nowhere else.
"""

import io

STONE = "/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"
fails = []


def edit(path, pairs):
    src = io.open(path, encoding="utf-8", newline="").read()
    NL = "\r\n" if "\r\n" in src else "\n"
    text = src
    for old, new, label in pairs:
        o, n = old.replace("\n", NL), new.replace("\n", NL)
        if text.count(o) != 1:
            fails.append("%s [%s]: count=%d" % (path, label, text.count(o)))
            continue
        text = text.replace(o, n, 1)
    if text != src:
        io.open(path, "w", encoding="utf-8", newline="").write(text)
    print("%s: %d -> %d" % (path, len(src), len(text)))


edit("Source/VEIL/Private/World/VEILWorldBuilder.cpp", [(
    "\tUMaterialInterface* Blue = LoadObject<UMaterialInterface>(nullptr,\n"
    "\t\tTEXT(\"/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue\"));",
    "\t// THE LAST BLUE HANDLE IN THE DISTRICT. This function has its own Place lambda,\n"
    "\t// separate from the palette pass that was already fixed, and it handed M_PHI_DeepBlue\n"
    "\t// to columns, roofs and shells - flat blue slabs standing over the plaza. The handle\n"
    "\t// keeps its name so every call site is untouched; what it loads is now the stone the\n"
    "\t// district is actually built from. Crystal and water keep their own blue elsewhere.\n"
    "\tUMaterialInterface* Blue = LoadObject<UMaterialInterface>(nullptr,\n"
    "\t\tTEXT(\"%s\"));" % STONE,
    "dressregions blue",
)])

edit("Source/VEIL/Private/Systems/PHIGear.cpp", [(
    "\t\tMatPath = TEXT(\"/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue\");",
    "\t\t// GEAR IS METAL, NOT BLUE PLASTIC: blue here was a placeholder tint on a physical\n"
    "\t\t// object the player picks up and wears.\n"
    "\t\tMatPath = TEXT(\"/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold\");",
    "gear blue",
)])

if fails:
    print("FAILURES (nothing half-written):")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("BLUE REMOVED FROM WORLD BUILDER AND GEAR")
