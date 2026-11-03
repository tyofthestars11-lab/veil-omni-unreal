"""Put the runner on his authority coordinate, unique anchor.

Measured: PHI UNSTICK relocates him to V(-484, 106, 578) every run - on a surface five
metres above the plaza and inside a fall sheet, which is the water filling the frame.
RUNNER's authority coordinate is 0,0,100. Placed there after the sweep, physics
teleport, with the move recorded so the log states where he stands.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

ANCHOR = "\t\tif (UWorld* SweepWorld = GetWorld())"
ADD = """		// HIS OWN COORDINATE, NOT THE FIRST SURFACE THAT ANSWERS. RUNNER is 0,0,100.
		// The unstick relocates him to a shelf at z=578, above the market and inside a
		// fall sheet - the water that filled the photographs.
		if (GetActorLocation().Z > 200.f)
		{
			SetActorLocation(FVector(0.f, 0.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI AUTHORITY SEAT: runner stands on V(0,0,100), his own coordinate - moved from z=%.0f."),
				GetActorLocation().Z);
		}
		if (UWorld* SweepWorld = GetWorld())"""

o, n = ANCHOR.replace("\n", NL), ADD.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - anchor count=%d" % src.count(o))
    raise SystemExit(1)
io.open(PATH, "w", encoding="utf-8", newline="").write(src.replace(o, n, 1))
print("WROTE %s - authority seat before the second sweep" % PATH)
