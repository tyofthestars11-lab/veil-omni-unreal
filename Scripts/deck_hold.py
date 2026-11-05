"""The deck is drawn but not solid; bodies must hold the deck, not sink to terrain."""

npc = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilNpc.cpp"
cpp = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"

# 1. a body holds its intended height when the surface under it is far below
n = open(npc, encoding="utf-8", newline="").read().replace("\r\n", "\n")
old = ("\tif (GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_Visibility))\n"
       "\t{\n"
       "\t\treturn Hit.ImpactPoint.Z + 2.0;\n"
       "\t}\n"
       "\treturn FromZ;")
new = ("\tif (GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_Visibility))\n"
       "\t{\n"
       "\t\tconst double HitZ = Hit.ImpactPoint.Z + 2.0;\n"
       "\t\t// If the surface is far below the intended seat, the piece under the body\n"
       "\t\t// is not solid: stand on the intended deck instead of sinking to the land.\n"
       "\t\tif (FromZ - HitZ > PHI_Constants::Reach(0.5))\n"
       "\t\t{\n"
       "\t\t\treturn FromZ;\n"
       "\t\t}\n"
       "\t\treturn HitZ;\n"
       "\t}\n"
       "\treturn FromZ;")
if old not in n:
    raise SystemExit("MISS probe body")
n = n.replace(old, new, 1)
open(npc, "w", encoding="utf-8", newline="").write(n)
print("people: hold the deck when the floor under them is not solid")

# 2. collision slab: mobile before the mesh is set, and top exactly on the deck
s = open(cpp, encoding="utf-8", newline="").read().replace("\r\n", "\n")
pairs = [
    ("\t\t\tFC->SetMobility(EComponentMobility::Static);\n\t\t\tFC->SetStaticMesh(CubeMesh);",
     "\t\t\t// Movable before the mesh: assigning a mesh to a Static component is\n"
     "\t\t\t// refused, which is why the plaza had no collision at all.\n"
     "\t\t\tFC->SetMobility(EComponentMobility::Movable);\n\t\t\tFC->SetStaticMesh(CubeMesh);"),
    ("FVector(0.0, 0.0, GPlazaZ - PHI_Constants::Reach(0.5)), FRotator::ZeroRotator))",
     "FVector(0.0, 0.0, GPlazaZ - 100.0), FRotator::ZeroRotator))"),
]
for old, newv in pairs:
    if old not in s:
        raise SystemExit("MISS " + old[:40])
    s = s.replace(old, newv, 1)
open(cpp, "w", encoding="utf-8", newline="").write(s)
print("hub: collision slab mobile, top flush with the deck")
