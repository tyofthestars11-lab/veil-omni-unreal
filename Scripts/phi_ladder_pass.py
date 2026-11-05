"""Put the world on the phi ladder: Reach(n) = Spacing * phi^n.

Distances stop being typed by hand. NPC pace derives from the Bible walk number
(439) through phi, so lore and mathematics cannot drift apart.
"""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


def patch(path, pairs, add_include=None):
    s = rd(path)
    before = s
    for old, new in pairs:
        n = s.count(old)
        if n == 0:
            print("   MISS  ", path.split("/")[-1], "::", old[:52])
        s = s.replace(old, new)
    if add_include and add_include not in s:
        s = s.replace('#include "CoreMinimal.h"', '#include "CoreMinimal.h"\n' + add_include, 1)
    if s != before:
        wr(path, s)
    print("  wrote", path.split("/")[-1], "changed:", s != before)


# ---------- the ladder itself ----------
h = rd(P + "PHI_Constants.h")
if "static double Reach" not in h:
    anchor = "\tstatic double PhiPow(int32 N)"
    add = ("\t// Reach(n) = Spacing * phi^n: the only way the world states a distance.\n"
           "\tstatic double Reach(double N) { return Spacing * FMath::Pow(Phi, N); }\n\n")
    assert h.count(anchor) == 1
    h = h.replace(anchor, add + anchor)
    wr(P + "PHI_Constants.h", h)
    print("  ladder added: Reach(n)")
else:
    print("  ladder already present")

# ---------- water: lake rims and zone extent off the ladder ----------
patch(P + "VeilWater.cpp", [
    ("BuildZone(World, FVector(0.0, 0.0, 100.0), 14000.0);",
     "BuildZone(World, FVector(0.0, 0.0, PHI_Constants::Reach(0)), PHI_Constants::Reach(9));"),
    ("FMath::Cos(A) * 1400.0, 780.0 + FMath::Sin(A) * 1400.0",
     "FMath::Cos(A) * PHI_Constants::Reach(5), 780.0 + FMath::Sin(A) * PHI_Constants::Reach(5)"),
    ("FMath::Cos(A) * 900.0, 500.0 + FMath::Sin(A) * 900.0",
     "FMath::Cos(A) * PHI_Constants::Reach(4), 500.0 + FMath::Sin(A) * PHI_Constants::Reach(4)"),
], add_include='#include "PHI_Constants.h"')

# ---------- npc: wander radius and pace from the Bible walk, through phi ----------
patch(P + "VeilNpc.h", [
    ("double Radius = 400.0;", "double Radius = PHI_Constants::Reach(3);"),
    ("double Speed = 120.0;", "double Speed = 439.0 * PHI_Constants::PhiPow(-2);  // Bible walk, two rungs down"),
], add_include='#include "PHI_Constants.h"')

patch(P + "VeilNpc.cpp", [
    ("Speed *= (0.85 + 0.3 * G);",
     "Speed *= (1.0 / PHI_Constants::Phi + (1.0 / (PHI_Constants::Phi * PHI_Constants::Phi)) * G);"),
    ("Wait = 0.5f + 5.5f * Phase;",
     "Wait = static_cast<float>(1.0 / (PHI_Constants::Phi * PHI_Constants::Phi)\n\t\t+ 2.0 * PHI_Constants::Phi * PHI_Constants::Phi * Phase);"),
    ("FVector(X, Y, FromZ + 6000.0), FVector(X, Y, FromZ - 6000.0)",
     "FVector(X, Y, FromZ + PHI_Constants::Reach(6)), FVector(X, Y, FromZ - PHI_Constants::Reach(6))"),
    ("const double Rate = 0.618033988749895 + 0.382 * static_cast<double>(Phase);",
     "const double Rate = 1.0 / PHI_Constants::Phi\n\t\t+ (1.0 - 1.0 / PHI_Constants::Phi) * static_cast<double>(Phase);"),
])

# ---------- mind: perception ranges on the ladder ----------
patch(P + "VeilMind.cpp", [
    ("const int32 State = (D < 800.0) ? 2 : ((D < 2600.0) ? 1 : 0);",
     "const int32 State = (D < PHI_Constants::Reach(4)) ? 2\n\t\t: ((D < PHI_Constants::Reach(6)) ? 1 : 0);"),
], add_include='#include "PHI_Constants.h"')

# ---------- gamemode: trace extents on the ladder ----------
patch(P + "VeilGameMode.cpp", [
    ("FVector(Loc.X, Loc.Y, Loc.Z + 40000.0), FVector(Loc.X, Loc.Y, Loc.Z - 40000.0)",
     "FVector(Loc.X, Loc.Y, Loc.Z + PHI_Constants::Reach(13)), FVector(Loc.X, Loc.Y, Loc.Z - PHI_Constants::Reach(13))"),
])

print("done")
