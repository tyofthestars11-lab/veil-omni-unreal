"""Guarantee the phi source of truth is included wherever the ladder is used."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"
FILES = ["VeilWater.cpp", "VeilMind.cpp", "VeilNpc.cpp", "VeilNpc.h",
         "VeilGameMode.cpp", "VeilHud.cpp", "VeilRunner.cpp", "VeilPlayerController.cpp"]

for name in FILES:
    path = P + name
    s = open(path, encoding="utf-8", newline="").read().replace("\r\n", "\n")
    if '#include "PHI_Constants.h"' in s:
        print("  ok    ", name)
        continue
    lines = s.split("\n")
    placed = False
    for i, line in enumerate(lines):
        if line.lstrip().startswith("#include"):
            lines.insert(i + 1, '#include "PHI_Constants.h"')
            placed = True
            break
    if placed:
        open(path, "w", encoding="utf-8", newline="").write("\n".join(lines))
    print("  fixed " if placed else "  NOINC ", name)
