"""Phi-whole audit, numeric and honest.

Each literal is classified once, by numeric value, not by string form:

  DERIVED      inside a PHI_Constants / PhiPow / Reach / Pace expression
  ART          colour, alpha, bloom, vignette
  ENGINE       trace extents, actor scale calls
  LAW          a Bible number phi does not govern
  STRUCT       0, 1, 2, 0.5: arithmetic identity, never art direction
  UNJUSTIFIED  a hand-typed number with no derivation. This blocks 1.0.
"""
import os
import re

SRC = "C:/Users/Admin/HarkGame/Source/VEILOMNI"

LAW = {
    439, 750, 185, -50, -980, 17.0324, 182000, 20.189, 137.5077640500378,
    1.618033988749895, 80.9, 64.7,
    900, 3500, 500, 2800, -450, 780, -5000, -6200, 809, -309, -4150, -780,
    -2000, -900, 600, -2100, -1250,
    12, 8, 5, 3, 50, 25, 100, 360,
    7.3737, 1.2364, 18.8669, 6.7549, -14.088, -12.0016,
}
STRUCT = {0.0, 1.0, 2.0, 0.5, -1.0}

NUM = re.compile(r"(?<![\w.])-?\d+\.\d+(?:f)?|(?<![\w.])\d{2,}(?![\w.])")
SKIP = ("#include", "UE_LOG", "TEXT(", "//", "UPROPERTY", "generated.h", "%")
ART_MARKS = ("FLinearColor", "SetLightColor", "LightColor", "C.A =", "ColorGain",
             "ColorSaturation", "ColorContrast", "Vignette", "Bloom")
ENGINE_MARKS = ("TraceSingle", "TraceChannel", "SetActorScale3D", "SetActorLocation",
                "SweepSingle", "LineTrace")

keys = ("DERIVED", "ART", "ENGINE", "LAW", "STRUCT", "UNJUSTIFIED")
classes = {k: 0 for k in keys}
unjustified = {}

for name in sorted(os.listdir(SRC)):
    if not name.endswith((".cpp", ".h")):
        continue
    with open(os.path.join(SRC, name), encoding="utf-8", errors="replace") as f:
        for i, line in enumerate(f, 1):
            if any(m in line for m in SKIP):
                continue
            stripped = line.lstrip()
            if stripped.startswith("*"):
                continue
            lits = [m.rstrip("f") for m in NUM.findall(line)]
            if not lits:
                continue

            if "PHI_Constants" in line or "Pow(PHI" in line or "Pace(" in line:
                classes["DERIVED"] += len(lits)
                continue
            if any(m in line for m in ART_MARKS):
                classes["ART"] += len(lits)
                continue
            if any(m in line for m in ENGINE_MARKS):
                classes["ENGINE"] += len(lits)
                continue
            for lit in lits:
                try:
                    v = float(lit)
                except ValueError:
                    continue
                if v in LAW:
                    classes["LAW"] += 1
                elif v in STRUCT:
                    classes["STRUCT"] += 1
                else:
                    classes["UNJUSTIFIED"] += 1
                    unjustified.setdefault(name, []).append((i, lit, line.strip()[:88]))

print("PHI-WHOLE AUDIT")
print("  DERIVED {:3d}   LAW {:3d}   STRUCT {:3d}   ART {:3d}   ENGINE {:3d}   UNJUSTIFIED {:3d}".format(
    classes["DERIVED"], classes["LAW"], classes["STRUCT"], classes["ART"],
    classes["ENGINE"], classes["UNJUSTIFIED"]))
print()
for name in sorted(unjustified, key=lambda n: -len(unjustified[n])):
    hits = unjustified[name]
    print("  {:26s} {:3d}".format(name, len(hits)))
    for line_no, lit, text in hits[:8]:
        print("      L{:<4d} {:>8s}  {}".format(line_no, lit, text))
print()
print("BLOCKING 1.0: {} unjustified literals".format(classes["UNJUSTIFIED"]))
