"""Full report of unjustified literals, with the complete source line."""
import os
import re

SRC = "C:/Users/Admin/HarkGame/Source/VEILOMNI"
OUT = "C:/Users/Admin/HarkGame/Scripts/phi_report.txt"

LAW = {
    439, 750, 185, -50, -980, 17.0324, 182000, 20.189, 137.5077640500378,
    1.618033988749895, 80.9, 64.7, 900, 3500, 500, 2800, -450, 780, -5000,
    -6200, 809, -309, -4150, -780, -2000, -900, 600, -2100, -1250,
    12, 8, 5, 3, 50, 25, 100, 360,
    7.3737, 1.2364, 18.8669, 6.7549, -14.088, -12.0016,
}
STRUCT = {0.0, 1.0, 2.0, 0.5, -1.0}
NUM = re.compile(r"(?<![\w.])-?\d+\.\d+(?:f)?|(?<![\w.])\d{2,}(?![\w.])")
SKIP = ("#include", "UE_LOG", "TEXT(", "//", "UPROPERTY", "generated.h", "%")
ART = ("FLinearColor", "SetLightColor", "LightColor", "C.A =", "ColorGain",
       "ColorSaturation", "ColorContrast", "Vignette", "Bloom")
ENG = ("TraceSingle", "TraceChannel", "SetActorScale3D", "SetActorLocation",
       "SweepSingle", "LineTrace")

lines_out = []
per_file = {}
for name in sorted(os.listdir(SRC)):
    if not name.endswith((".cpp", ".h")):
        continue
    with open(os.path.join(SRC, name), encoding="utf-8", errors="replace") as f:
        for i, line in enumerate(f, 1):
            if any(m in line for m in SKIP):
                continue
            if line.lstrip().startswith("*"):
                continue
            lits = [m.rstrip("f") for m in NUM.findall(line)]
            if not lits:
                continue
            if "PHI_Constants" in line or "Pow(PHI" in line or "Pace(" in line:
                continue
            if any(m in line for m in ART) or any(m in line for m in ENG):
                continue
            bad = []
            for lit in lits:
                v = float(lit)
                if v in LAW or v in STRUCT:
                    continue
                bad.append(lit)
            if bad:
                per_file.setdefault(name, []).append(
                    "L{:<4d} [{}]  {}".format(i, ",".join(bad), line.rstrip()))

with open(OUT, "w", encoding="utf-8") as f:
    for name, rows in per_file.items():
        f.write("=== {} ({})\n".format(name, len(rows)))
        for row in rows:
            f.write(row + "\n")
        f.write("\n")

total = sum(len(v) for v in per_file.values())
print("report written to", OUT)
print("total lines to convert:", total)
for name, rows in sorted(per_file.items(), key=lambda kv: -len(kv[1])):
    print("  {:26s} {:3d}".format(name, len(rows)))
