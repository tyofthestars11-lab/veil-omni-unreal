"""55% panels, as the brief states, everywhere a panel is raised.

The play plate and the manual card are already at 0.55. The four page surfaces (map,
chart, manual page, log) were still 0.92-0.95 - black slabs over the district, which is
exactly what the brief forbids. They join the same number.
"""

import io
import re

P = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
src = io.open(P, encoding="utf-8", newline="").read()

pattern = re.compile(r"(FVeilUI::Glass\(this, FLinearColor\(0\.0\d+f, 0\.0\d+f, 0\.0\d+f, )0\.9\d+f\)")
matches = pattern.findall(src)
out, n = pattern.subn(r"\g<1>0.55f)", src)
if n:
    io.open(P, "w", encoding="utf-8", newline="").write(out)
print("PAGE PANELS AT 55%: %d replaced" % n)
print("remaining 0.9x glass:", len(pattern.findall(out)))
