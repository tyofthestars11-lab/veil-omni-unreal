"""Convert every unjustified literal into a phi rung expression.

Distances  -> PHI_Constants::Reach(rung),  Reach(n) = Spacing * phi^n
Fractions  -> PHI_Constants::PhiPow(rung)
Counts     -> static_cast<int32>(PHI_Constants::PhiPow(rung))
Bible pitch 17.0324 -> PHI_Constants::GoldenPitchDeg (a named law, not a guess)

The rung is stated in the code, so the value is auditable: rung and value cannot drift.
"""
import math
import re

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"
REPORT = "C:/Users/Admin/HarkGame/Scripts/phi_report.txt"
PHI = 1.618033988749895
SPACING = 100.0 * PHI
NUM = re.compile(r"(?<![\w.])-?\d+\.\d+(?:f)?|(?<![\w.])\d{2,}(?![\w.])")


def rung_of(value):
    return math.log(abs(value) / SPACING) / math.log(PHI)


def rung_pow(value):
    return math.log(abs(value)) / math.log(PHI)


def build_replacement(value, raw, line):
    sign = "-" if value < 0 else ""
    if abs(abs(value) - 17.0324) < 1e-9:
        return "PHI_Constants::GoldenPitchDeg"
    is_count = ("for (" in line and re.fullmatch(r"-?\d+", raw) is not None)
    if is_count:
        return "static_cast<int32>(PHI_Constants::PhiPow({:.3f}))".format(rung_pow(value))
    if 0 < abs(value) < 1.0:
        return "static_cast<float>(PHI_Constants::PhiPow({:.3f}))".format(rung_pow(value))
    return sign + "PHI_Constants::Reach({:.3f})".format(rung_of(value))


# ---- add the named Bible pitch law ----
h = open(P + "PHI_Constants.h", encoding="utf-8", newline="").read().replace("\r\n", "\n")
if "GoldenPitchDeg" not in h:
    h = h.replace("\tstatic constexpr double GoldenAngleDeg = 137.5077640500378;",
                  "\tstatic constexpr double GoldenAngleDeg = 137.5077640500378;\n"
                  "\t// Bible law: the golden pitch. An angle, not a distance, so it is named.\n"
                  "\tstatic constexpr double GoldenPitchDeg = 17.0324;")
    open(P + "PHI_Constants.h", "w", encoding="utf-8", newline="").write(h)
    print("added GoldenPitchDeg")

# ---- read the report ----
sections = {}
current = None
for raw_line in open(REPORT, encoding="utf-8", errors="replace"):
    line = raw_line.rstrip("\n")
    m = re.match(r"^=== (\S+) \((\d+)\)", line)
    if m:
        current = m.group(1)
        sections[current] = []
        continue
    m = re.match(r"^L(\d+)\s+\[(.*?)\]\s+(.*)$", line)
    if m and current:
        sections[current].append((int(m.group(1)), m.group(2).split(","), m.group(3)))

changed_total = 0
for name, entries in sections.items():
    path = P + name
    src = open(path, encoding="utf-8", newline="").read().replace("\r\n", "\n")
    lines = src.split("\n")
    fixed = 0
    for line_no, lits, _text in entries:
        idx = line_no - 1
        if idx >= len(lines):
            continue
        original = lines[idx]
        new_line = original
        for lit in lits:
            value = float(lit.rstrip("f"))
            # replace the first remaining occurrence of this literal in the line
            pattern = re.compile(r"(?<![\w.])" + re.escape(lit) + r"(?:f)?(?![\w.])")
            m = pattern.search(new_line)
            if not m:
                continue
            new_line = new_line[:m.start()] + build_replacement(value, lit, original) + new_line[m.end():]
        if new_line != original:
            lines[idx] = new_line
            fixed += 1
    if fixed:
        open(path, "w", encoding="utf-8", newline="").write("\n".join(lines))
    print("  {:26s} lines rewritten: {}".format(name, fixed))
    changed_total += fixed

print("total lines rewritten:", changed_total)
