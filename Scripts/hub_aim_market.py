"""Sixteen pieces were within 10 m of the lens; the plaza core is empty. Use it."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

pairs = [
    ("\tstatic const double BackPow[5] = { 3.566, 2.0, 2.0, 2.0, 2.0 };",
     "\tstatic const double BackPow[5] = { 0.44, 2.0, 2.0, 2.0, 2.0 };"),
    ("\tstatic const double UpPow[5] = { 3.566, 1.6, 1.6, 1.6, 1.6 };",
     "\tstatic const double UpPow[5] = { 3.04, 1.6, 1.6, 1.6, 1.6 };"),
    # the record looks at the market centre, not at the bare plaza anchor
    ("GroundZ + PHI_Constants::Reach(1.9)) - CamAt).Rotation();",
     "GroundZ + PHI_Constants::Reach(1.5)) - CamAt).Rotation();"),
]
for old, new in pairs:
    if old not in s:
        raise SystemExit("MISS " + old[:44])
    s = s.replace(old, new, 1)

# aim vector: Hub looks at the authority market centre (900, 0)
old_aim = "\t\tconst FRotator Look = (FVector(Anchor[Idx].X, Anchor[Idx].Y,"
new_aim = "\t\tconst FRotator Look = (FVector(AimAt[Idx].X, AimAt[Idx].Y,"
if old_aim not in s:
    raise SystemExit("MISS look vector")
s = s.replace(old_aim, new_aim, 1)

anchor = "\tstatic const double BackPow[5]"
aim = ("\t// Where the record looks. The Hub looks into its market, not at a bare anchor.\n"
       "\tstatic const FVector2D AimAt[5] = {\n"
       "\t\tFVector2D(900.0, 0.0), FVector2D(0.0, 3500.0), FVector2D(-450.0, 780.0),\n"
       "\t\tFVector2D(2800.0, 500.0), FVector2D(0.0, -5000.0)\n"
       "\t};\n")
if anchor not in s:
    raise SystemExit("MISS BackPow anchor")
s = s.replace(anchor, aim + anchor, 1)

open(p, "w", encoding="utf-8", newline="").write(s)
print("hub: camera in the plaza core, aimed at the market centre")
