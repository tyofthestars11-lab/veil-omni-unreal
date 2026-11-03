"""Give the interiors real headroom.

Measured: BuildRoom laid the walls at exactly the Height the caller passed, put the roof at
Height + 12, and cut a 3.3 m doorway with a 2.6 m gap. So a shell asked for at 5.2 m finished
as a low room with a slab on it, and every interior in the district had the ceiling sitting
on the player's head.

One number now governs the room: a floor of 6.4 m clear, a doorway that grows with it, and
the roof, walls, lintel and hearth light all read from it. The log names the result per room,
including the shell the caller asked for, so the next too-low interior is a number in the log
rather than an impression.
"""

import io

P = "Source/VEIL/Private/World/VEILWorldBuilder.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"
fails = []


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
	// A ROOM A BODY CAN STAND UP IN. The walls were laid at exactly the height the caller
	// asked for, the roof sat 12 cm above that, and the doorway was cut at 3.3 m - so a shell
	// asked for at 5.2 m finished as a low room with a slab on the player's head. One number
	// now governs the interior: 6.4 m clear at minimum, a door that grows with the ceiling,
	// and walls, lintel, roof and hearth light all reading from it.
	const float RoomH = FMath::Max(Height, 640.f);
	const float DoorHalf = 150.f;    // 3.0 m wide: two abreast, no crouch
	const float DoorH = FMath::Clamp(RoomH * 0.66f, 380.f, 520.f);
	const float Mid = RoomH * 0.5f;""",
    "room height header")

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

# Per-room variety in the hearth crown, so interiors are not one identical hero prop each.
rep("""	// The hearth crown is the kit's carved column, not the engine sphere it used to be.
	SpawnBlock(Hearth + FVector(0.f, 0.f, 145.f), FVector(0.45f, 0.9f, 0.9f),""",
    """	// The hearth crown is the kit's carved column, not the engine sphere it used to be.
	SpawnBlock(Hearth + FVector(0.f, 0.f, 145.f), FVector(0.45f, 0.9f, 0.9f),""",
    "crown probe", count=0) if False else None

rep("""	SpawnBlock(Hearth + FVector(0.f, 0.f, 145.f), FVector(0.45f, 0.9f, 0.9f),
		ColumnMesh, FLinearColor(1.f, 0.78f, 0.30f), 2.0f);""",
    """	// VARIETY, ONE ROOM AT A TIME. Every interior wore the identical crown at the identical
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
		SpawnBlock(Hearth + FVector(0.f, 0.f, 145.f), FVector(0.45f * CrownScale, 0.9f * CrownScale, 0.9f * CrownScale),
			CrownOptions[Reel], CrownTint, 2.0f);
	}""",
    "crown variety")

# Name the numbers per room.
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
print("WROTE %s: interiors now %.0f cm clear minimum" % (P, 640.0))
