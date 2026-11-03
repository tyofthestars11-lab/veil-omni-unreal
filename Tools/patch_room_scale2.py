"""Room headroom, take two - with an anchor that cannot miss on spacing.

The first attempt aborted because the hearth-crown anchor did not match the file's exact
spacing, and it wrote nothing (that is the point of aborting). This pass uses a bounded regex
for the crown block and also repairs the include order that UHT flagged in
OMNIQuestContent.cpp: the module's own header must be the first include.
"""

import io
import re

fails = []
CAST_INC = '#include "Story/VEILCastBody.h"'

# ------------------------------------------------------------------ include order
for path, own in (
    ("Source/VEIL/Private/World/OMNIQuestContent.cpp", "OMNIQuestContent.h"),
    ("Source/VEIL/Private/AI/VEILCompanion.cpp", "VEILCompanion.h"),
    ("Source/VEIL/Private/AI/VEILFactionAI.cpp", "VEILFactionAI.h"),
):
    s = io.open(path, encoding="utf-8", newline="").read()
    NL = "\r\n" if "\r\n" in s else "\n"
    lines = [ln for ln in s.split(NL) if ln.strip() != CAST_INC]
    own_i = None
    for i, ln in enumerate(lines):
        if ln.strip().startswith("#include") and own in ln:
            own_i = i
            break
    if own_i is None:
        fails.append("%s: own header %s missing" % (path, own))
        continue
    lines.insert(own_i + 1, CAST_INC)
    io.open(path, "w", encoding="utf-8", newline="").write(NL.join(lines))
    print("%s: own header first, cast include after it" % path.split("/")[-1])

# ------------------------------------------------------------------ room scale
P = "Source/VEIL/Private/World/VEILWorldBuilder.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"


def rep(old, new, label, count=1):
    global s
    o, n = old.replace("\n", N), new.replace("\n", N)
    c = s.count(o)
    if c != count:
        fails.append("%s count=%d" % (label, c))
        return
    s = s.replace(o, n, count)


rep("""	const float T = 40.f;            // wall thickness, cm
	const float DoorHalf = 130.f;    // a 2.6 m doorway: a body walks through, no crouch
	const float DoorH = 330.f;
	const float Mid = Height * 0.5f;""",
    """	const float T = 40.f;            // wall thickness, cm
	// A ROOM A BODY CAN STAND UP IN. Walls were laid at exactly the height the caller asked
	// for and the roof sat 12 cm above that, so a shell asked for at 5.2 m finished as a low
	// room with a slab on the player's head. One number governs the interior now: 6.4 m clear
	// at minimum, a door that grows with it, and walls, lintel, roof and hearth light all read
	// from it - so no caller can build a room the player has to hunch in.
	const float RoomH = FMath::Max(Height, 640.f);
	const float DoorHalf = 150.f;    // 3.0 m wide: two abreast, no crouch
	const float DoorH = FMath::Clamp(RoomH * 0.66f, 380.f, 520.f);
	const float Mid = RoomH * 0.5f;""",
    "room header")

rep("	UStaticMeshComponent* BackWall = Wall3(FVector(-HalfD, 0.f, 0.f), T, 2.f * HalfW, Height, Mid);",
    "	UStaticMeshComponent* BackWall = Wall3(FVector(-HalfD, 0.f, 0.f), T, 2.f * HalfW, RoomH, Mid);",
    "back wall")
rep("	UStaticMeshComponent* LeftWall = Wall3(FVector(0.f, -HalfW, 0.f), 2.f * HalfD, T, Height, Mid);",
    "	UStaticMeshComponent* LeftWall = Wall3(FVector(0.f, -HalfW, 0.f), 2.f * HalfD, T, RoomH, Mid);",
    "left wall")
rep("	UStaticMeshComponent* RightWall = Wall3(FVector(0.f, HalfW, 0.f), 2.f * HalfD, T, Height, Mid);",
    "	UStaticMeshComponent* RightWall = Wall3(FVector(0.f, HalfW, 0.f), 2.f * HalfD, T, RoomH, Mid);",
    "right wall")
rep("	Wall3(FVector(HalfD, -(DoorHalf + SegW * 0.5f), 0.f), T, SegW, Height, Mid);",
    "	Wall3(FVector(HalfD, -(DoorHalf + SegW * 0.5f), 0.f), T, SegW, RoomH, Mid);",
    "jamb left")
rep("	Wall3(FVector(HalfD, DoorHalf + SegW * 0.5f, 0.f), T, SegW, Height, Mid);",
    "	Wall3(FVector(HalfD, DoorHalf + SegW * 0.5f, 0.f), T, SegW, RoomH, Mid);",
    "jamb right")
rep("	Wall3(FVector(HalfD, 0.f, 0.f), T, 2.f * DoorHalf, Height - DoorH, DoorH + (Height - DoorH) * 0.5f);",
    "	Wall3(FVector(HalfD, 0.f, 0.f), T, 2.f * DoorHalf, RoomH - DoorH, DoorH + (RoomH - DoorH) * 0.5f);",
    "lintel")
rep("		SpawnBlockRot(Centre + FVector(0.f, 0.f, Height + 12.f),",
    "		SpawnBlockRot(Centre + FVector(0.f, 0.f, RoomH + 12.f),",
    "roof")
rep("			APointLight::StaticClass(), Hearth + FVector(0.f, 0.f, Height * 0.72f), FRotator::ZeroRotator))",
    "			APointLight::StaticClass(), Hearth + FVector(0.f, 0.f, RoomH * 0.72f), FRotator::ZeroRotator))",
    "hearth light")

# ---------------------------------------------------------------- hearth crown variety (regex)
pat = re.compile(
    r"SpawnBlock\(Hearth \+ FVector\(0\.f, 0\.f, 145\.f\), FVector\(0\.45f, 0\.9f, 0\.9f\),\s*\r?\n"
    r"\s*ColumnMesh, FLinearColor\(1\.f, 0\.78f, 0\.30f\), 2\.0f\);")
if len(pat.findall(s)) != 1:
    fails.append("crown regex matches=%d" % len(pat.findall(s)))
else:
    block = """// VARIETY, ONE ROOM AT A TIME. Every interior wore the identical crown at the identical
	// height; the crown now steps through four kit silhouettes and two heights by room index,
	// so walking into a second building is not walking into the first one again.
	{
		const int32 Reel = RoomsBuilt % 4;
		const float CrownScale = (RoomsBuilt % 2 == 0) ? 0.9f : 1.15f;
		UStaticMesh* CrownOptions[4] = { ColumnMesh, ArchMesh ? ArchMesh.Get() : ColumnMesh,
			MonolithMesh ? MonolithMesh.Get() : ColumnMesh,
			PyramidHeadMesh ? PyramidHeadMesh.Get() : ColumnMesh };
		const FLinearColor CrownTint = (Reel == 3) ? FLinearColor(1.f, 0.62f, 0.24f)
			: (Reel == 1) ? FLinearColor(0.95f, 0.86f, 0.55f)
			: FLinearColor(1.f, 0.78f, 0.30f);
		SpawnBlock(Hearth + FVector(0.f, 0.f, 145.f),
			FVector(0.45f * CrownScale, 0.9f * CrownScale, 0.9f * CrownScale),
			CrownOptions[Reel], CrownTint, 2.0f);
	}""".replace("\n", N)
    s = pat.sub(lambda m: block, s, count=1)
    print("crown: four kit silhouettes by room index")

# ---------------------------------------------------------------- per-room numbers in the log
rep("""		FirstRoomInterior = Centre + F * 150.f + FVector(0.f, 0.f, 110.f);""",
    """		UE_LOG(LogVEIL, Warning,
			TEXT("PHI ROOM SCALE: ceiling %.0f cm, doorway %.0f wide x %.0f tall (the shell asked for %.0f cm)."),
			RoomH, 2.f * DoorHalf, DoorH, Height);
		FirstRoomInterior = Centre + F * 150.f + FVector(0.f, 0.f, 110.f);""",
    "room scale log")

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(P, "w", encoding="utf-8", newline="").write(s)
print("ROOM SCALE WRITTEN: 640 cm clear minimum, 300 cm doorway")
