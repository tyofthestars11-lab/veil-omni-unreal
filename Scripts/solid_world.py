"""A mesh assigned at runtime has no body unless default collision is used."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old = ("\tUStaticMeshComponent* C = A->GetStaticMeshComponent();\n"
       "\tC->SetMobility(EComponentMobility::Movable);\n"
       "\tC->SetStaticMesh(Mesh);")
new = ("\tUStaticMeshComponent* C = A->GetStaticMeshComponent();\n"
       "\tC->SetMobility(EComponentMobility::Movable);\n"
       "\tC->SetStaticMesh(Mesh);\n"
       "\t// Without this the component renders but has no collision body, so the\n"
       "\t// world is scenery you fall through. Every piece must be solid ground.\n"
       "\tC->bUseDefaultCollision = true;")
if old not in s:
    raise SystemExit("MISS component setup")
s = s.replace(old, new, 1)

# profile must be set after the body exists, so order it explicitly
old2 = ("\tC->SetCollisionEnabled(bSolid ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);\n"
        "\tif (bSolid)\n"
        "\t{\n"
        "\t\tC->SetCollisionProfileName(TEXT(\"BlockAll\"));\n"
        "\t}")
new2 = ("\tC->SetCollisionProfileName(bSolid ? TEXT(\"BlockAll\") : TEXT(\"NoCollision\"));\n"
        "\tC->SetCollisionEnabled(bSolid ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);")
if old2 not in s:
    raise SystemExit("MISS collision block")
s = s.replace(old2, new2, 1)

open(p, "w", encoding="utf-8", newline="").write(s)
print("world: runtime meshes now carry their default collision")
