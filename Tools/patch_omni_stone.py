"""The district's fab dressing stops wearing the placeholder blue.

PHI OMNI FAB logged itself: every non-gold piece - cliffs, domes, shells, walls - was
loaded with M_PHI_DeepBlue and tinted (0.35, 0.66, 1.0), a bright flat blue. That is
the blue slab standing over the plaza in every photograph. Authored geometry is stone;
gold stays gold; blue is kept for crystal meshes only, because crystal blue is the
vision and a blue cliff is a placeholder.
"""

import io

PATH = "Source/VEIL/Private/Systems/VEILOmniAll.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
fails = []


def rep(old, new, label):
    global text
    o, n = old.replace("\n", NL), new.replace("\n", NL)
    c = text.count(o)
    if c == 0:
        fails.append("%s: not found" % label)
        return
    text = text.replace(o, n)


# 1. A stone path beside the palette paths.
rep(
    '\tconst TCHAR* BlueMaterialPath = TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue");',
    '\tconst TCHAR* BlueMaterialPath = TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue");\n'
    '\t// STONE FOR MASS, GOLD FOR LIGHT, BLUE ONLY FOR CRYSTAL. Every fab piece used to\n'
    '\t// fall through to the blue palette, which is why a cliff read as a placeholder box.\n'
    '\tconst TCHAR* StoneMaterialPath = TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone");',
    "stone path",
)

# 2. The material choice: crystal keeps blue, everything else is stone.
rep(
    "\tif (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, bGold ? GoldMaterialPath : BlueMaterialPath))",
    "\tconst bool bCrystal = Mesh->GetName().Contains(TEXT(\"Crystal\"));\n"
    "\tif (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr,\n"
    "\t\tbGold ? GoldMaterialPath : (bCrystal ? BlueMaterialPath : StoneMaterialPath)))",
    "material choice",
)

# 3. The tint follows the material, not the palette default.
rep(
    "\t\t\tMID->SetVectorParameterValue(TEXT(\"Color\"), bGold ? FLinearColor(1.f, 0.77f, 0.23f) : FLinearColor(0.35f, 0.66f, 1.f));\n"
    "\t\t\tMID->SetVectorParameterValue(TEXT(\"BaseColor\"), bGold ? FLinearColor(1.f, 0.77f, 0.23f) : FLinearColor(0.35f, 0.66f, 1.f));",
    "\t\t\tconst FLinearColor Tint = bGold ? FLinearColor(1.f, 0.77f, 0.23f)\n"
    "\t\t\t\t: (bCrystal ? FLinearColor(0.35f, 0.66f, 1.f) : FLinearColor(0.82f, 0.78f, 0.70f));\n"
    "\t\t\tMID->SetVectorParameterValue(TEXT(\"Color\"), Tint);\n"
    "\t\t\tMID->SetVectorParameterValue(TEXT(\"BaseColor\"), Tint);",
    "tint",
)

if fails:
    print("ABORTED - nothing written")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(PATH, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars (fab dressed stone/gold, blue kept for crystal)" % (PATH, len(src), len(text)))
