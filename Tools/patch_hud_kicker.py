# -*- coding: utf-8 -*-
"""Match the plate to the HUD reference (proof/hud_pad): a letter-spaced gold
CURRENT DIRECTIVE kicker over a rule, the directive in gold beneath it, and one
quiet row of real key hints at the foot. Every advertised key is actually bound
in SetupInputComponent (E, SPACE, TAB, H, P); the plate grows 16 px to hold the
two new rows instead of squeezing the existing ones.
"""
import io
import sys

P = "D:/VEIL-OMNI/Source/VEIL/Private/Game/VEILHudCanvas.cpp"
with io.open(P, "r", encoding="utf-8", newline="") as f:
    raw = f.read()
crlf = "\r\n" in raw
t = raw.replace("\r\n", "\n")
fails = []

if "C U R R E N T" not in t:
    # 1. Room for the kicker and the hint row.
    ph = "const float PH = 140.f;"
    if t.count(ph) != 1:
        fails.append("PH anchor count=%d" % t.count(ph))
    else:
        t = t.replace(ph, "const float PH = 156.f;", 1)

    # 2. Kicker + rule above the directive.
    ystart = "\tY = PY + 14.f;\n"
    if t.count(ystart) != 1:
        fails.append("Y start anchor count=%d" % t.count(ystart))
    else:
        t = t.replace(ystart,
            "\t// THE KICKER, AS THE REFERENCE DRAWS IT. A plate that begins with its own sentence\n"
            "\t// has no masthead; the mockup leads with a letter-spaced gold label over a rule, and the\n"
            "\t// eye reads \"this is what I am doing now\" before it reads the words. Spaced with\n"
            "\t// spaces, because the canvas font has no tracking control and no second asset is worth\n"
            "\t// it for one label.\n"
            "\tDrawText(TEXT(\"C U R R E N T   D I R E C T I V E\"), FLinearColor(1.f, 0.78f, 0.30f, 0.88f),\n"
            "\t\tTX, PY + 8.f, Med, 0.46f);\n"
            "\tDrawRect(FLinearColor(1.f, 0.78f, 0.30f, 0.30f), TX, PY + 19.f, CardW, 1.f);\n"
            "\tY = PY + 26.f;\n", 1)

    ystep = "\tY += 22.f;\n"
    if t.count(ystep) != 1:
        fails.append("Y step anchor count=%d" % t.count(ystep))
    else:
        t = t.replace(ystep, "\tY += 21.f;\n", 1)

    # 3. One quiet hint row at the foot, absolute so the flow above cannot move it.
    hints_anchor = "\t// THE KEY LIST IS GONE."
    if t.count(hints_anchor) != 1:
        fails.append("hint anchor count=%d" % t.count(hints_anchor))
    else:
        HINTS = (
            "\t// ONE ROW, REAL KEYS. The reference puts the gestures along the bottom of the plate\n"
            "\t// where they can be read without leaving the frame. Every key here is bound in\n"
            "\t// SetupInputComponent, so the row teaches nothing that does not work.\n"
            "\t{\n"
            "\t\tconst FString Hints = TEXT(\"[E] SPEAK   [SPACE] NEXT   [TAB] CHART   [H] MANUAL   [P] PAUSE\");\n"
            "\t\tShadow(Fitted(Hints, Med, 0.50f, CardW), FLinearColor(0.74f, 0.79f, 0.88f, 0.66f),\n"
            "\t\t\tTX, PY + PH - 17.f, Med, 0.50f);\n"
            "\t}\n\n")
        t = t.replace(hints_anchor, HINTS + hints_anchor, 1)
else:
    print("plate kicker already present")

if fails:
    print("FAILS:", fails)
    sys.exit(1)

with io.open(P, "w", encoding="utf-8", newline="") as f:
    f.write(t.replace("\n", "\r\n") if crlf else t)

print("PH 156 =", "const float PH = 156.f;" in t)
print("kicker =", "C U R R E N T" in t)
print("hints =", "[E] SPEAK" in t)
print("PATCH: PASS")
