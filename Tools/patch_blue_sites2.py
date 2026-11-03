"""Both blue handles in VEILWorldBuilder load stone.

The first attempt aborted because the load appears twice - the palette pass and
DressRegions' own lambda - which is the same defect the file's own comments describe:
a fix applied at one site while a second site kept painting blue. Both are replaced.
Blue remains for crystals (ContentFamilies, Wilds) and water (Ocean, Nest, falls).
"""

import io

PATH = "Source/VEIL/Private/World/VEILWorldBuilder.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

OLD = (
    "\tUMaterialInterface* Blue = LoadObject<UMaterialInterface>(nullptr,\n"
    "\t\tTEXT(\"/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue\"));"
)
NEW = (
    "\t// STONE, NOT BLUE. This handle reached columns, roofs and shells as flat blue\n"
    "\t// slabs. It keeps its name so every call site is untouched; the material it\n"
    "\t// loads is the stone the district is built from. Crystal and water keep their\n"
    "\t// own blue in their own files.\n"
    "\tUMaterialInterface* Blue = LoadObject<UMaterialInterface>(nullptr,\n"
    "\t\tTEXT(\"/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone\"));"
)

o, n = OLD.replace("\n", NL), NEW.replace("\n", NL)
count = src.count(o)
if count == 0:
    print("ABORTED - no blue handle found")
    raise SystemExit(1)
io.open(PATH, "w", encoding="utf-8", newline="").write(src.replace(o, n))
print("WROTE %s - %d blue handle(s) now load stone" % (PATH, count))
