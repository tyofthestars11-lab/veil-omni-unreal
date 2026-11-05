"""Lighting never changed those white surfaces: the CC0 overrides were wrong.

The kit meshes ship with authored stone materials. Our M_CC0_* overrides painted
everything flat white, and no amount of sun or exposure could fix that, which is
exactly what the record frames showed. Drop the overrides for stone pieces.
"""
FILES = ["C:/Users/Admin/HarkGame/Scripts/ed_hub_v6.py",
         "C:/Users/Admin/HarkGame/Scripts/ed_dress.py"]

DROP = [
    'C + "M_CC0_Pavement.M_CC0_Pavement"',
    'C + "M_CC0_Sandstone.M_CC0_Sandstone"',
    'C + "M_CC0_RoofPlanks.M_CC0_RoofPlanks"',
    'C + "M_CC0_Rock.M_CC0_Rock"',
    'C + "M_CC0_Cloth.M_CC0_Cloth"',
]

for path in FILES:
    s = open(path, encoding="utf-8", newline="").read().replace("\r\n", "\n")
    total = 0
    for old in DROP:
        n = s.count(old)
        if n:
            s = s.replace(old, "None")
            total += n
    open(path, "w", encoding="utf-8", newline="").write(s)
    print("%s: %d material overrides removed" % (path.rsplit("/", 1)[-1], total))
