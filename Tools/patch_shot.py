"""-phishot photographs the world, so it must not summon the test rig.

bGoldenProof treated -phishot as -phigolden, which spawns the 34-cube sunflower near
the runner: every frame meant to prove the district contained a proof object instead.
"""

import io

PATH = "Source/VEIL/Private/Game/VEILGameMode.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

OLD = (
    "\t\tconst bool bGoldenProof = FParse::Param(FCommandLine::Get(), TEXT(\"phigolden\"))\n"
    "\t\t\t|| FParse::Param(FCommandLine::Get(), TEXT(\"phishot\"));"
)
NEW = (
    "\t\t// A PHOTOGRAPH IS NOT A TEST RIG. -phishot used to imply -phigolden, so the\n"
    "\t\t// 34-cube sunflower spawned into every frame meant to prove the district was\n"
    "\t\t// free of placeholder boxes. The rig asks for its own flag now.\n"
    "\t\tconst bool bGoldenProof = FParse::Param(FCommandLine::Get(), TEXT(\"phigolden\"));"
)

o, n = OLD.replace("\n", NL), NEW.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - count=%d" % src.count(o))
    raise SystemExit(1)
io.open(PATH, "w", encoding="utf-8", newline="").write(src.replace(o, n, 1))
print("WROTE %s - photo no longer spawns the sunflower" % PATH)
