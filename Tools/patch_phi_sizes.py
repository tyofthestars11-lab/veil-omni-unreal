"""The phi rungs, applied to size and to variant choice.

minimap diameter = screen height / phi   (golden pitch rung 5.8916 lives on the same law)
quest panel width = screen width / phi^2  (phi^2 = phi + 1)
variant pick = Fmod(i * 137.5077640500378, Count)   (golden angle, rung 10.2318)

The 180 px map and the 480 px band are replaced by those two expressions; the world's
wall/floor/cliff choice stops being `i % Count`, which puts two adjacent pieces on the
same variant, and becomes a golden-angle stride.
"""

import io

fails = []

# ---------------------------------------------------------------- HUD sizes
H = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
h = io.open(H, encoding="utf-8", newline="").read()
HNL = "\r\n" if "\r\n" in h else "\n"

MS_OLD = "\t\tconst float MS = 180.f;"
MS_NEW = ("\t\t// MINIMAP DIAMETER = SCREEN HEIGHT / PHI. 720 / 1.618033988749895 = 445 px.\n"
          "\t\tconst float MS = Canvas->ClipY / UPHIMITO_PHI_SAFE;")
if h.count(MS_OLD) != 1:
    fails.append("minimap size count=%d" % h.count(MS_OLD))
else:
    h = h.replace(MS_OLD, MS_NEW.replace("\n", HNL), 1)

QW_OLD = "QY, 480.f, 21.f, true);"
QW_NEW = "QY, Canvas->ClipX / (UPHIMITO_PHI_SAFE * UPHIMITO_PHI_SAFE), 21.f, true);"
if h.count(QW_OLD) != 1:
    fails.append("quest width count=%d" % h.count(QW_OLD))
else:
    h = h.replace(QW_OLD, QW_NEW, 1)

# The manual card sits under the ring, so it must follow the new diameter.
CY_OLD = "const float CY = 30.f + 180.f + 18.f;   // under the 180 px ring"
CY_NEW = "const float CY = 30.f + Canvas->ClipY / UPHIMITO_PHI_SAFE + 18.f;   // under the ring"
if h.count(CY_OLD) != 1:
    fails.append("card position count=%d" % h.count(CY_OLD))
else:
    h = h.replace(CY_OLD, CY_NEW, 1)

if h != io.open(H, encoding="utf-8", newline="").read():
    io.open(H, "w", encoding="utf-8", newline="").write(h)
print("%s: phi sizes applied" % H)

# ---------------------------------------------------------------- golden-angle variants
W = "Source/VEIL/Private/World/VEILWorldBuilder.cpp"
w = io.open(W, encoding="utf-8", newline="").read()
WNL = "\r\n" if "\r\n" in w else "\n"

HELPER_ANCHOR = "\tauto WallAt = [this, Wall](int32 i) -> UStaticMesh*"
HELPER = (
    "\t// GOLDEN ANGLE VARIANT PICK. `i % Count` repeats every Count pieces, so two\n"
    "\t// neighbours could wear the same wall. Fmod(i * 137.5077640500378, Count) strides\n"
    "\t// the kit by the golden angle, rung 10.2318, which never lands back on itself.\n"
    "\tauto GoldenPick = [](int32 Index, int32 Count) -> int32\n"
    "\t{\n"
    "\t\treturn Count > 0 ? FMath::Abs((int32)FMath::Fmod((float)Index * 137.5077640500378f, (float)Count)) : 0;\n"
    "\t};\n"
)
if "GoldenPick" not in w:
    if w.count(HELPER_ANCHOR) != 1:
        fails.append("variant helper anchor count=%d" % w.count(HELPER_ANCHOR))
    else:
        w = w.replace(HELPER_ANCHOR, HELPER.replace("\n", WNL) + HELPER_ANCHOR, 1)

for name in ("WallVariants", "FloorVariants", "CliffVariants"):
    old = "%s[FMath::Abs(i) %% %s.Num()]" % (name, name)
    new = "%s[GoldenPick(i, %s.Num())]" % (name, name)
    if w.count(old) == 1:
        w = w.replace(old, new, 1)
    else:
        fails.append("%s pick count=%d" % (name, w.count(old)))

io.open(W, "w", encoding="utf-8", newline="").write(w)
print("%s: golden-angle variants" % W)

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("PHI SIZES + GOLDEN ANGLE VARIANTS WRITTEN")
