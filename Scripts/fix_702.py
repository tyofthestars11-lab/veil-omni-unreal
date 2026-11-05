paths = [
    "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp",
    "C:/Users/Admin/.freebuff/worktrees/52bfbe8a/e80fea6afa42e7c3cd60a735da7e14aecc808ccd7f84fcdd/Source/VEILOMNI/VeilGameMode.cpp",
]

FIX = [
    ("FVector(GHubShift.X, GHubShift.Y) < 1600.0",
     "FVector(GHubShift.X, GHubShift.Y, GHubGroundZ) < 1600.0"),
    ('BuildRing(FVector2D(900.0, 0.0), 8, MeshArch, MatStone, TEXT("STALLS"), 0.0);',
     'BuildRing(FVector2D(900.0, 0.0), 8, MeshArch, MatDeep, TEXT("STALLS"), 0.0);'),
]

for p in paths:
    s = open(p, encoding="utf-8", newline="").read()
    before = s
    hits = 0
    for old, new in FIX:
        n = s.count(old)
        hits += n
        s = s.replace(old, new)
    if s != before:
        open(p, "w", encoding="utf-8", newline="").write(s)
    print(p.split("/")[3], "edits:", hits, "changed:", s != before)
