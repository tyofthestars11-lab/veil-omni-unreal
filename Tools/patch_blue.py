"""Remove the blue placeholder boxes and add a boot audit that counts them.

The blue boxes in the district are three things, all made of engine cubes wearing
M_PHI_DeepBlue: the eight 11 m decks of the Deep, every market stall's awning and
every tent's canopy. They become cut stone and gold trim - the palette the district
already uses - while the crystal cones keep their blue, because that blue is the
vision, not a placeholder. A boot audit then counts any surviving blue-material box
so the claim is a number in the log, not an opinion.
"""

import io

fails = []


def edit(path, pairs):
    src = io.open(path, encoding="utf-8", newline="").read()
    NL = "\r\n" if "\r\n" in src else "\n"
    text = src
    for old, new, label in pairs:
        o = old.replace("\n", NL)
        n = new.replace("\n", NL)
        c = text.count(o)
        if c != 1:
            fails.append("%s [%s]: count=%d" % (path, label, c))
            continue
        text = text.replace(o, n, 1)
    if text != src:
        io.open(path, "w", encoding="utf-8", newline="").write(text)
    print("%s: %d -> %d chars" % (path, len(src), len(text)))


# 1. BuildDeep learns stone and gold.
edit("Source/VEIL/Public/World/VEILConceptWorld.h", [
    ("\tvoid BuildDeep(UStaticMesh* Cube, UStaticMesh* Cylinder, UMaterialInterface* Water,\n"
     "\t\tUMaterialInterface* Glow);",
     "\tvoid BuildDeep(UStaticMesh* Cube, UStaticMesh* Cylinder, UMaterialInterface* Water,\n"
     "\t\tUMaterialInterface* Glow, UMaterialInterface* Stone, UMaterialInterface* Gold);",
     "header"),
])

# 2. The Deep's decks are stone with a gold rim, not blue boxes.
edit("Source/VEIL/Private/World/VEILConceptWorld.cpp", [
    ("void AVEILConceptWorld::BuildDeep(UStaticMesh* Cube, UStaticMesh* Cylinder,\n"
     "\tUMaterialInterface* Water, UMaterialInterface* Glow)",
     "void AVEILConceptWorld::BuildDeep(UStaticMesh* Cube, UStaticMesh* Cylinder,\n"
     "\tUMaterialInterface* Water, UMaterialInterface* Glow, UMaterialInterface* Stone,\n"
     "\tUMaterialInterface* Gold)",
     "signature"),
    ("\t\tSpawnPiece(Cube, At, FVector(1100.f, 1100.f, 60.f), Water, FRotator(0.f, i * 17.0324f, 0.f), true);",
     "\t\t// THE DEEP IS STONE, NOT A BLUE BOX. Eight blue 11 m plates were the largest\n"
     "\t\t// placeholder blocks in the district. The deck is cut stone with a gold rim;\n"
     "\t\t// the column above it was always light.\n"
     "\t\tSpawnPiece(Cube, At, FVector(1100.f, 1100.f, 60.f), Stone, FRotator(0.f, i * 17.0324f, 0.f), true);\n"
     "\t\tSpawnPiece(Cube, At + FVector(0.f, 0.f, 34.f), FVector(1140.f, 1140.f, 16.f), Gold,\n"
     "\t\t\tFRotator(0.f, i * 17.0324f, 0.f), false);",
     "deep deck"),
    ("\tBuildDeep(Cube, Cylinder, Water, Glow);",
     "\tBuildDeep(Cube, Cylinder, Water, Glow, Stone, Gold);",
     "call"),
])

# 3. Market awnings and tent canopies: gold cloth, not blue plates.
edit("Source/VEIL/Private/World/VEILContentFamilies.cpp", [
    ("\tPart(Cube, At + FVector(0.f, 0.f, 268.f), FVector(2.7f, 1.5f, 0.10f),\n"
     "\t\tFRotator(-12.f, YawDeg, 0.f), DeepBlue, false);",
     "\t// AWNING IS CLOTH-GOLD. It was a blue plate: the clearest \"placeholder box\"\n"
     "\t// left in the market, and the one prop a player stands directly under.\n"
     "\tPart(Cube, At + FVector(0.f, 0.f, 268.f), FVector(2.7f, 1.5f, 0.10f),\n"
     "\t\tFRotator(-12.f, YawDeg, 0.f), Gold, false);",
     "stall awning"),
    ("\tPart(Cube, At + FVector(0.f, 0.f, 268.f), FVector(3.0f, 3.0f, 0.10f), Yaw, DeepBlue, false);",
     "\tPart(Cube, At + FVector(0.f, 0.f, 268.f), FVector(3.0f, 3.0f, 0.10f), Yaw, Gold, false);",
     "tent canopy"),
])

# 4. A boot audit that counts any surviving blue-material box.
edit("Source/VEIL/Private/Game/VEILGameMode.cpp", [
    ("\t\t\tif (Pass) { Pass->Populate(Composition > 0); }",
     "\t\t\tif (Pass) { Pass->Populate(Composition > 0); }\n"
     "\t\t\t// BLUE AUDIT: no placeholder box may survive a build. A cube wearing\n"
     "\t\t\t// M_PHI_DeepBlue is what \"blue placeholder box\" means; a crystal cone in\n"
     "\t\t\t// the same blue is the vision, so the mesh is part of the test.\n"
     "\t\t\tvoid();\n"
     "\t\t\t{\n"
     "\t\t\t\tFTimerHandle BlueAudit;\n"
     "\t\t\t\tGetWorldTimerManager().SetTimer(BlueAudit, [this]()\n"
     "\t\t\t\t{\n"
     "\t\t\t\t\tint32 BlueBoxes = 0, BlueSlots = 0;\n"
     "\t\t\t\t\tfor (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)\n"
     "\t\t\t\t\t{\n"
     "\t\t\t\t\t\tconst UStaticMeshComponent* Comp = It->GetStaticMeshComponent();\n"
     "\t\t\t\t\t\tconst UStaticMesh* BlueMesh = Comp ? Comp->GetStaticMesh() : nullptr;\n"
     "\t\t\t\t\t\tif (!Comp || !BlueMesh) { continue; }\n"
     "\t\t\t\t\t\tfor (int32 Slot = 0; Slot < Comp->GetNumMaterials(); ++Slot)\n"
     "\t\t\t\t\t\t{\n"
     "\t\t\t\t\t\t\tconst UMaterialInterface* Mat = Comp->GetMaterial(Slot);\n"
     "\t\t\t\t\t\t\tif (!Mat || !Mat->GetName().Contains(TEXT(\"DeepBlue\"))) { continue; }\n"
     "\t\t\t\t\t\t\t++BlueSlots;\n"
     "\t\t\t\t\t\t\tif (BlueMesh->GetName().Contains(TEXT(\"Cube\"))) { ++BlueBoxes; }\n"
     "\t\t\t\t\t\t}\n"
     "\t\t\t\t\t}\n"
     "\t\t\t\t\tUE_LOG(LogVEIL, Warning,\n"
     "\t\t\t\t\t\tTEXT(\"PHI BLUE AUDIT: %d blue placeholder box(es), %d blue material slot(s) in the district.\"),\n"
     "\t\t\t\t\t\tBlueBoxes, BlueSlots);\n"
     "\t\t\t\t}, 6.f, false);\n"
     "\t\t\t}",
     "blue audit"),
])

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("ALL EDITS APPLIED")
