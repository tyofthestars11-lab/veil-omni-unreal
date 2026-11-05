"""Prove the pause path runs: the same calls the P and Escape keys are bound to."""

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(GM, encoding="utf-8", newline="").read().replace("\r\n", "\n")
notes = []

if '#include "VeilPlayerController.h"' not in s:
    for inc in ('#include "VeilGameMode.h"', '#include "CoreMinimal.h"'):
        if inc in s:
            s = s.replace(inc, inc + '\n#include "VeilPlayerController.h"', 1)
            notes.append("include added after " + inc)
            break
    else:
        notes.append("MISS include anchor")
else:
    notes.append("include present")

anchor = "\tconst int32 Idx = FMath::Clamp(ShotIdx, 0, 4);\n"
block = anchor + """
	if (Idx == 1)
	{
		// The P and Escape keys are bound to this; a record run cannot press a key,
		// so drive the same entry point and let the log prove pause and resume.
		if (AVeilPlayerController* VP = Cast<AVeilPlayerController>(GetWorld()->GetFirstPlayerController()))
		{
			VP->OnPauseToggle();
			VP->OnPauseToggle();
		}
	}
"""

if "drive the same entry point" in s:
    notes.append("pause exercise already present")
elif anchor in s:
    s = s.replace(anchor, block, 1)
    notes.append("pause exercise installed")
else:
    notes.append("MISS Idx anchor")

open(GM, "w", encoding="utf-8", newline="").write(s)
print("\n".join(notes))
