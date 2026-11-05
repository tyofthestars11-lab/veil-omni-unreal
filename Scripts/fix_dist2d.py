import io

paths = [
    "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp",
    "C:/Users/Admin/.freebuff/worktrees/52bfbe8a/e80fea6afa42e7c3cd60a735da7e14aecc808ccd7f84fcdd/Source/VEILOMNI/VeilGameMode.cpp",
]

REPLACEMENT = "\t\tif (A && FVector::Dist2D(A->GetActorLocation(), FVector(GHubShift.X, GHubShift.Y, GHubGroundZ)) < 1600.0\n"

for p in paths:
    lines = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n").split("\n")
    hits = 0
    for i, line in enumerate(lines):
        if "Dist2D(A->GetActorLocation()" in line and "GHubShift" in line:
            print(p.split("/")[3], "was:", line.strip()[:90])
            lines[i] = REPLACEMENT.rstrip("\n")
            hits += 1
    if hits:
        open(p, "w", encoding="utf-8", newline="").write("\n".join(lines))
    print(p.split("/")[3], "rewritten:", hits)
