"""1495 back was inside the mountain. Stand inside the ring, look down at it."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

pairs = [
    ("\tstatic const double BackPow[5] = { 4.62, 2.0, 2.0, 2.0, 2.0 };",
     "\tstatic const double BackPow[5] = { 1.2, 2.0, 2.0, 2.0, 2.0 };"),
    ("\tstatic const double UpPow[5] = { 2.24, 1.6, 1.6, 1.6, 1.6 };",
     "\tstatic const double UpPow[5] = { 2.6, 1.6, 1.6, 1.6, 1.6 };"),
    # canvas stalls, not glowing ore
    ('BuildRing(FVector2D(0.0, -700.0), 8, MeshArch, MatDeep, TEXT("STALLS"), 0.0);',
     'BuildRing(FVector2D(0.0, -700.0), 8, MeshArch, MatStone, TEXT("STALLS"), 0.0);'),
]
for old, new in pairs:
    if old not in s:
        raise SystemExit("MISS " + old[:50])
    s = s.replace(old, new, 1)

open(p, "w", encoding="utf-8", newline="").write(s)
print("hub: camera inside the ring and raised; stalls are stone")
