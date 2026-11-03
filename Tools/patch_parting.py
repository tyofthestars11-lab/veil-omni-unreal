"""Clear the two UHT errors, without deleting anything.

1. AVEILPlayerController was defined twice: whole in VEILGameMode.h and again in an
   untracked stub Source/VEIL/Public/Game/VEILPlayerController.h (+ its .cpp). UHT:
   "Class 'AVEILPlayerController' shares engine name 'VEILPlayerController'". The
   untracked pair is MOVED to Docs/golden_keep as a backup - one definition remains,
   the one GameMode.cpp implements.
2. OMNIQuestSystem.cpp opened with a comment block and another include before its own
   header. UHT requires the module header first, so it is inserted ahead of the first
   include and any later duplicate of it is removed.
"""

import io
import shutil
from pathlib import Path

root = Path(r"D:\VEIL-OMNI")
keep = root / "Docs" / "golden_keep"
keep.mkdir(parents=True, exist_ok=True)

for rel in ["Source/VEIL/Public/Game/VEILPlayerController.h",
            "Source/VEIL/Private/Game/VEILPlayerController.cpp"]:
    p = root / rel
    if p.exists():
        shutil.copy2(p, keep / p.name)
        p.unlink()
        print("MOVED %s -> Docs/golden_keep (backup kept)" % rel)
    else:
        print("absent (already one definition): %s" % rel)

q = root / "Source/VEIL/Private/Systems/OMNIQuestSystem.cpp"
src = io.open(q, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
lines = src.split(NL)
own = '#include "Systems/OMNIQuestSystem.h"'
if lines and lines[0].strip() == own:
    print("OMNIQuestSystem.cpp already opens with its own header")
else:
    first = next((i for i, l in enumerate(lines) if l.lstrip().startswith("#include")), None)
    if first is None:
        print("ABORTED - no includes in OMNIQuestSystem.cpp")
        raise SystemExit(1)
    lines = [l for i, l in enumerate(lines) if not (l.strip() == own and i != first)]
    idx = next((i for i, l in enumerate(lines) if l.lstrip().startswith("#include")), 0)
    lines.insert(idx, own)
    io.open(q, "w", encoding="utf-8", newline="").write(NL.join(lines))
    print("OMNIQuestSystem.cpp: own header now first include")

print("ONE DEFINITION: AVEILPlayerController in Public/Game/VEILGameMode.h")
