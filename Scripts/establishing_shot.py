"""The shot camera stood inside the r=423.6 pillar ring; pull it back and up."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old_arrays = "\tstatic const double Pitch[5] = { -8.0, -6.0, -7.0, -7.0, -14.0 };"
new_arrays = ("\tstatic const double Pitch[5] = { -8.0, -6.0, -7.0, -7.0, -14.0 };\n"
              "\t// How far back and how high the record camera stands at each site. The\n"
              "\t// Hub needs to clear its own market ring (r = 423.6) to see the plaza.\n"
              "\tstatic const double BackPow[5] = { 4.62, 2.0, 2.0, 2.0, 2.0 };\n"
              "\tstatic const double UpPow[5] = { 2.24, 1.6, 1.6, 1.6, 1.6 };")
if old_arrays not in s:
    raise SystemExit("MISS pitch array")
s = s.replace(old_arrays, new_arrays, 1)

pairs = [
    ("\t\tconst double Back = PHI_Constants::Reach(2.0);",
     "\t\tconst double Back = PHI_Constants::Reach(BackPow[Idx]);"),
    ("\t\t\tGroundZ + PHI_Constants::Reach(1.6));",
     "\t\t\tGroundZ + PHI_Constants::Reach(UpPow[Idx]));"),
    ("\t\tconst FRotator Look = (FVector(Anchor[Idx].X, Anchor[Idx].Y, GroundZ + PHI_Constants::Reach(0.5)) - CamAt).Rotation();",
     "\t\tconst FRotator Look = (FVector(Anchor[Idx].X, Anchor[Idx].Y, GroundZ + PHI_Constants::Reach(1.4)) - CamAt).Rotation();"),
]
for old, new in pairs:
    if old not in s:
        raise SystemExit("MISS " + old[:60])
    s = s.replace(old, new, 1)

open(p, "w", encoding="utf-8", newline="").write(s)
print("shot: per-site camera distance, Hub pulled clear of its ring")
