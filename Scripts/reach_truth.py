"""Spacing in this world is not 10. Value the constants with the real one."""

import re

SRC = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"
PHI = 1.618033988749895

# what is Spacing actually set to?
hdr = open(SRC + "PHI_Constants.h", encoding="utf-8").read()
cpp = open(SRC + "PHI_Constants.cpp", encoding="utf-8").read()
print("=== declarations ===")
for m in re.finditer(r".*Spacing.*=.*", hdr):
    print("  h:", m.group(0).strip())
for m in re.finditer(r"double\s+PHI_Constants::Spacing.*", cpp):
    print("  cpp:", m.group(0).strip())

val = re.search(r"double\s+PHI_Constants::Spacing\s*=\s*([0-9.]+)", cpp)
SPACING = float(val.group(1)) if val else 10.0
print(f"\nSpacing in use = {SPACING}")
for x in (0.8, 1.0, 1.283, 2.0, 7.0, 11.0):
    print(f"  Reach({x}) = {SPACING * PHI ** x:10.1f}")


def fix(path, pairs):
    p = SRC + path
    s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")
    for old, new in pairs:
        if old in s:
            s = s.replace(old, new)
            print(f"  {path}: {old} -> {new}")
        else:
            print(f"  {path}: MISS {old}")
    open(p, "w", encoding="utf-8", newline="").write(s)


# 300 units is the Kraken spec, and Reach(1.283) is 298 at this spacing.
fix("VeilNpc.cpp", [("PHI_Constants::Reach(7.0)", "PHI_Constants::Reach(1.283)")])
# the proof body must stand inside that 298 reach
fix("VeilGameMode.cpp", [
    ("PHI_Constants::Reach(6.0), 0.0, 0.0", "PHI_Constants::Reach(0.8), 0.0, 0.0"),
    ("const double Hold = PHI_Constants::Reach(11.0);", "const double Hold = PHI_Constants::Reach(2.0);"),
])
