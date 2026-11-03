"""Room headroom, applied on its own.

The crown variety edit kept missing its anchor, so it is dropped from this pass - the actual
complaint is ceiling height, and that is what this changes. Walls, lintel, roof and hearth
light now all read one number: 6.4 m clear at minimum, with a doorway that grows with it.
"""

import io

P = "Source/VEIL/Private/World/VEILWorldBuilder.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"
fails = []


def rep(old, new, label):
    global s
    o, n = old.replace("\n", N), new.replace("\n", N)
    c = s.count(o)
    if c != 1:
        fails.append("%s count=%d" % (label, c))
        return
    s = s.replace(o, n, 1)


rep("""	const float T = 40.f;            // wall thickness, cm
	const float DoorHalf = 130.f;    // a 2.6 m doorway: a body walks through, no crouch
	const float DoorH = 330.f;
	const float Mid = Height * 0.5f;""",
    """	const float T = 40.f;            // wall thickness, cm
	// A ROOM A BODY CAN STAND UP IN. Walls were laid at exactly the height the caller asked
	// for and the roof sat 12 cm above that, so a shell asked for at 5.2 m finished as a low
	// room with a slab on the player's head. One number governs the interior now: 6.4 m clear
	// at minimum, a door that grows with the ceiling, and walls, lintel, roof and hearth light
	// all read from it - so no caller can build a room the player has to hunch in.
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
