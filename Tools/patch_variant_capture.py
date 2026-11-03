"""C3493: the variant lambdas use GoldenPick without capturing it.

`WallAt`, `FloorAt` and `RockAt` capture only `this` and their mesh fallback, so a call
to the golden picker inside them has no name to bind to. The picker joins each capture
list - a capture list change, not a new design.
"""

import io

P = "Source/VEIL/Private/World/VEILWorldBuilder.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
fails = []

for old, new, label in [
    ("auto WallAt = [this, Wall](int32 i) -> UStaticMesh*",
     "auto WallAt = [this, Wall, GoldenPick](int32 i) -> UStaticMesh*", "WallAt"),
    ("auto FloorAt = [this, FloorArt](int32 i) -> UStaticMesh*",
     "auto FloorAt = [this, FloorArt, GoldenPick](int32 i) -> UStaticMesh*", "FloorAt"),
    ("auto RockAt = [this, Rock](int32 i) -> UStaticMesh*",
     "auto RockAt = [this, Rock, GoldenPick](int32 i) -> UStaticMesh*", "RockAt"),
]:
    if text.count(old) != 1:
        fails.append("%s count=%d" % (label, text.count(old)))
    else:
        text = text.replace(old, new, 1)

if fails:
    print("ABORTED")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(P, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s - GoldenPick captured by all three variant lambdas" % P)
