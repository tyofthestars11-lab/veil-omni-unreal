"""Bottom-left is one column, three bands, no shared pixels.

Measured positions before: map legend at ClipY-118 (growing downward), quest ribbon at
ClipY-146, ACTIVITIES at ClipY-172 - the legend's rows ran into the ribbon, which is the
"RUNNER / FRAGMENT / CACHE / WARDEN" line printed through "QUEST: THE GOLDEN MIDST".
Now: ribbon at ClipY-70, ACTIVITIES at ClipY-94, legend at ClipY-124. Each band is 24 px
clear of the next, on the same 26 px phi step the rest of the file uses.
"""

import io

P = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
fails = []


def rep(old, new, label):
    global text
    o, n = old.replace("\n", NL), new.replace("\n", NL)
    if text.count(o) != 1:
        fails.append("%s: count=%d" % (label, text.count(o)))
        return
    text = text.replace(o, n, 1)


rep("const float QY = Canvas->ClipY - 146.f;",
    "const float QY = Canvas->ClipY - 70.f;      // band 3: the quest ribbon",
    "quest ribbon")

rep("30.f, Canvas->ClipY - 146.f - 26.f, Med, 0.56f);",
    "30.f, Canvas->ClipY - 94.f, Med, 0.56f);   // band 2: activities",
    "activities")

rep("DrawMapLegend(Canvas, Med, 30.f, Canvas->ClipY - 118.f, false);",
    "DrawMapLegend(Canvas, Med, 30.f, Canvas->ClipY - 124.f, false);   // band 1: legend",
    "legend")

if text != src:
    io.open(P, "w", encoding="utf-8", newline="").write(text)
print("%s: %d -> %d" % (P, len(src), len(text)))
if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("BOTTOM LEFT RESTACKED: legend -124, activities -94, ribbon -70")
