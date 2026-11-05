"""The live project spawns AVEILHUD; my header declared AVeilHud. Match the project."""
import glob
import os

SRC = "C:/Users/Admin/HarkGame/Source/VEILOMNI"
for path in glob.glob(os.path.join(SRC, "*.cpp")) + glob.glob(os.path.join(SRC, "*.h")):
    text = open(path, encoding="utf-8", errors="replace").read()
    if "AVEILHUD" in text and "VeilHud." not in os.path.basename(path):
        print("%s: uses AVEILHUD" % os.path.basename(path))

for name in ("VeilHud.h", "VeilHud.cpp"):
    path = os.path.join(SRC, name)
    text = open(path, encoding="utf-8", errors="replace").read()
    n = text.count("AVeilHud")
    if n:
        text = text.replace("AVeilHud", "AVEILHUD")
        open(path, "w", encoding="utf-8").write(text)
    print("%s: renamed %d" % (name, n))
