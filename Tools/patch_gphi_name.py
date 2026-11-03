"""Two anonymous namespaces, one unity blob, one symbol name: GPhi.

C2374 at VEILTime.cpp(27): `constexpr float GPhi` is declared in VEILTime.cpp's
anonymous namespace and again in PHIIntent.cpp's, and the unity build merged those
translation units, so the second definition collides with the first. The time file's
copy is renamed GVeilTimePhi; PHIIntent keeps its own. Nothing else changes, and every
use in the file follows.
"""

import io
import re

P = "Source/VEIL/Private/Systems/VEILTime.cpp"
src = io.open(P, encoding="utf-8", newline="").read()

out, n = re.subn(r"\bGPhi\b", "GVeilTimePhi", src)
if n == 0:
    print("ABORTED - no GPhi in VEILTime.cpp")
    raise SystemExit(1)
io.open(P, "w", encoding="utf-8", newline="").write(out)
print("WROTE %s - %d GPhi reference(s) renamed to GVeilTimePhi" % (P, n))
