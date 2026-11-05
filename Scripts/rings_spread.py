"""The clamp crowded the camera; spread the rings again, keep them distinct."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old_r = ("\t\t// Clamped so a ring stays on the 26 m terrace instead of walking off it.\n"
         "\t\tconst double R = FMath::Min(Pitch * FMath::Sqrt(static_cast<double>(i)),\n"
         "\t\t\tPHI_Constants::Reach(2.888));")
new_r = "\t\tconst double R = Pitch * FMath::Sqrt(static_cast<double>(i));"
if old_r not in s:
    raise SystemExit("MISS clamp")
s = s.replace(old_r, new_r, 1)

for old, new in [
    ('BuildRing(FVector2D(0.0, 600.0), 8, MeshDome, MatStone, TEXT("HOUSES"), 1.0);',
     'BuildRing(FVector2D(0.0, 700.0), 8, MeshDome, MatStone, TEXT("HOUSES"), 1.0);'),
    ('BuildRing(FVector2D(0.0, -600.0), 8, MeshArch, MatDeep, TEXT("STALLS"), 0.0);',
     'BuildRing(FVector2D(0.0, -700.0), 8, MeshArch, MatDeep, TEXT("STALLS"), 0.0);'),
]:
    if old in s:
        s = s.replace(old, new, 1)
    else:
        raise SystemExit("MISS " + old[:44])

open(p, "w", encoding="utf-8", newline="").write(s)
print("rings: spread restored, rows distinct")
