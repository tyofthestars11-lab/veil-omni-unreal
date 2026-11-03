"""Clear collision, not only meshes.

The 73 worn shells went, and he still stood at 598.15 - the exact top of the meshless
collision slab (Actor_1) the game mode comments describe: it spans the plaza, has no
static mesh at all, so a static-mesh sweep cannot see it, and its top face at 500 cm is
what the capsule lands on every time the hold drops him to 120. The clear now walks any
colliding primitive component, and skips pawns.
"""

import io

P = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
fails = []

PAIRS = [
    ("\t\t\t\t\tif (Shell->Tags.Contains(FName(TEXT(\"PhiGround\")))) { continue; }",
     "\t\t\t\t\tif (Shell->Tags.Contains(FName(TEXT(\"PhiGround\")))) { continue; }\n"
     "\t\t\t\t\tif (Shell->IsA(APawn::StaticClass())) { continue; }"),
    ("\t\t\t\t\tTInlineComponentArray<UStaticMeshComponent*> ShellComps;",
     "\t\t\t\t\t// ANY COLLIDING PRIMITIVE. Actor_1 is a collision slab with no static\n"
     "\t\t\t\t\t// mesh; a static-mesh sweep cannot see it, and its top face is 500 cm of\n"
     "\t\t\t\t\t// floor standing where the plaza is.\n"
     "\t\t\t\t\tTInlineComponentArray<UPrimitiveComponent*> ShellComps;"),
    ("\t\t\t\t\tfor (UStaticMeshComponent* ShellComp : ShellComps)",
     "\t\t\t\t\tfor (UPrimitiveComponent* ShellComp : ShellComps)"),
    ("\t\t\t\t\t\tif (!ShellComp) { continue; }",
     "\t\t\t\t\t\tif (!ShellComp) { continue; }\n"
     "\t\t\t\t\t\tif (ShellComp->GetCollisionEnabled() == ECollisionEnabled::NoCollision) { continue; }"),
]
for old, new in PAIRS:
    o, n = old.replace("\n", NL), new.replace("\n", NL)
    c = text.count(o)
    if c != 1:
        fails.append("[%s] count=%d" % (old.strip()[:52], c))
        continue
    text = text.replace(o, n, 1)

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(P, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars (clear walks colliding primitives)" % (P, len(src), len(text)))
