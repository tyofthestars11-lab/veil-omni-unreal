"""Bible compliance: authority coordinates, blue canvas stalls, no stacked rings."""
import re

cpp = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
hdr = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.h"

s = open(cpp, encoding="utf-8", newline="").read().replace("\r\n", "\n")
h = open(hdr, encoding="utf-8", newline="").read().replace("\r\n", "\n")

# 1. a real cloth material for the market awnings (Bible: blue canvas, not stone)
if "MatCloth" not in s:
    anchor = '\tconst TCHAR* const MatDeep = TEXT("/Game/VEIL/Materials/M_VEIL_Deep.M_VEIL_Deep");'
    if anchor not in s:
        raise SystemExit("MISS MatDeep")
    s = s.replace(anchor, anchor + '\n\t// Bible: the stalls are blue canvas, so they wear the real cloth set.\n'
                  '\tconst TCHAR* const MatCloth = TEXT("/Game/VEIL/CC0/Materials/M_CC0_Cloth.M_CC0_Cloth");', 1)
    print("mat: cloth added")

# 2. BuildRing gains a radius scale so two rings can share an authority centre
if "RadiusScale" not in h:
    if "double HeightPower" not in h:
        raise SystemExit("MISS header HeightPower")
    h = h.replace("double HeightPower", "double HeightPower, double RadiusScale = 1.0", 1)
    print("ring: header radius scale")
m = re.search(r"(void AVeilGameMode::BuildRing\([^)]*?double HeightPower)\)", s)
if not m:
    raise SystemExit("MISS cpp BuildRing signature")
s = s[:m.start(1)] + m.group(1) + ", double RadiusScale" + s[m.end(1):]
print("ring: cpp signature")

old_r = "\t\tconst double R = Pitch * FMath::Sqrt(static_cast<double>(i));"
if old_r not in s:
    raise SystemExit("MISS ring radius")
s = s.replace(old_r, "\t\tconst double R = Pitch * RadiusScale * FMath::Sqrt(static_cast<double>(i));", 1)
print("ring: radius scaled")

# 3. authority centres: houses and stalls both at (900, 0), interleaved by phi
pairs = [
    ('BuildRing(FVector2D(0.0, 700.0), 8, MeshDome, MatStone, TEXT("HOUSES"), 1.0);',
     'BuildRing(FVector2D(900.0, 0.0), 8, MeshDome, MatStone, TEXT("HOUSES"), 1.0);'),
    ('BuildRing(FVector2D(0.0, -700.0), 8, MeshArch, MatStone, TEXT("STALLS"), 0.0);',
     'BuildRing(FVector2D(900.0, 0.0), 8, MeshArch, MatCloth, TEXT("STALLS"), 0.0,\n'
     '\t\tPHI_Constants::PhiPow(-1.0));'),
    # 4. crystal peaks stand at their own authority coordinate (2800, 500)
    ("\t\tif (FitPart(MeshMonolith, FVector(P.X, P.Y, GPlazaZ),",
     "\t\tif (FitPart(MeshMonolith, FVector(2800.0 + P.X, 500.0 + P.Y, GHubGroundZ),"),
]
for old, new in pairs:
    if old not in s:
        raise SystemExit("MISS " + old[:52])
    s = s.replace(old, new, 1)
    print("bible:", old.split("(")[0][:44])

open(cpp, "w", encoding="utf-8", newline="").write(s)
open(hdr, "w", encoding="utf-8", newline="").write(h)
print("bible compliance applied")
