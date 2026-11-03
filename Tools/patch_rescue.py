"""Boxed in means: put him on his own coordinate, once.

The unstick now refuses roofs above 200 cm, so it reports the truth it always should
have: "no open ground within 1500 cm ... the capsule is boxed in" - the ring search
cannot reach the plaza from inside the plaza's shell. Leaving him there is what kept the
camera under a fall sheet. On the first such report he is placed on V(0,0,100), the
RUNNER authority coordinate, and the move is logged.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

marker = "- the capsule is boxed in."
i = src.find(marker)
if i < 0:
    print("ABORTED - boxed-in log not found")
    raise SystemExit(1)
end = src.find(");", i)
if end < 0:
    print("ABORTED - statement end not found")
    raise SystemExit(1)
endline = src.find(NL, end)
if endline < 0:
    print("ABORTED - line end not found")
    raise SystemExit(1)

INSERT = """
					// BOXED IN: TAKE HIM TO HIS OWN COORDINATE, ONCE. The plaza's shell boxes the
					// ring search out, and standing him inside it is what put the camera under a
					// fall sheet while the world around him looked unfinished.
					static bool bAuthorityRescue = false;
					if (!bAuthorityRescue)
					{
						bAuthorityRescue = true;
						SetActorLocation(FVector(0.f, 0.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
						UE_LOG(LogVEIL, Warning,
							TEXT("PHI AUTHORITY RESCUE: boxed in, so the runner stands on V(0,0,100)."));
					}""".replace("\n", NL)

out = src[:endline] + INSERT + src[endline:]
io.open(PATH, "w", encoding="utf-8", newline="").write(out)
print("WROTE %s: %d -> %d chars (boxed-in rescue)" % (PATH, len(src), len(out)))
