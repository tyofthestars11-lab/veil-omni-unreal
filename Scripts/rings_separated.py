"""Houses and stalls were the same ring twice, and both spilled off the deck."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

# 1. Keep a ring inside the deck: phyllotaxis radius is clamped to the terrace.
old_r = "\t\tconst double R = Pitch * FMath::Sqrt(static_cast<double>(i));"
new_r = ("\t\t// Clamped so a ring stays on the 26 m terrace instead of walking off it.\n"
         "\t\tconst double R = FMath::Min(Pitch * FMath::Sqrt(static_cast<double>(i)),\n"
         "\t\t\tPHI_Constants::Reach(2.888));")
if old_r in s:
    s = s.replace(old_r, new_r, 1)
    print("ring: radius clamped to the deck")
else:
    raise SystemExit("MISS ring radius")

# 2. Two rows on the deck, not one row authored twice.
pairs = [
    ('BuildRing(FVector2D(900.0, 0.0), 8, MeshDome, MatStone, TEXT("HOUSES"), 1.0);',
     'BuildRing(FVector2D(0.0, 600.0), 8, MeshDome, MatStone, TEXT("HOUSES"), 1.0);'),
    ('BuildRing(FVector2D(900.0, 0.0), 8, MeshArch, MatDeep, TEXT("STALLS"), 0.0);',
     'BuildRing(FVector2D(0.0, -600.0), 8, MeshArch, MatDeep, TEXT("STALLS"), 0.0);'),
]
for old, new in pairs:
    if old in s:
        s = s.replace(old, new, 1)
        print("ring:", old.split(",")[0])
    else:
        raise SystemExit("MISS " + old[:40])

open(p, "w", encoding="utf-8", newline="").write(s)
print("rings separated")
