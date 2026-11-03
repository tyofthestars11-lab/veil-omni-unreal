"""Two fixes, both aimed at the frames the user keeps seeing.

1. THE SEA WAS OVER THE CITY. Every frame at the plaza was a flat purple wash with the
   gold HUD on top, and the glowing teal shapes above it were the far side of the ocean
   surface: the water plane sits above z=100, so the market at z=0 is under water and the
   camera looks along the underside of the sea. The ocean is dropped 400 cm at BeginPlay,
   which puts its surface well below the plaza and leaves the city standing in the air
   above it, the way the painting has it.

2. THE EYE CLEAR NO LONGER DESTROYS THE MARKET. The 600 cm column around the runner took
   79 actors with it, including the market itself, and the next frame was an empty blue
   void. It now only seats him on floor at or below 200 cm and touches nothing.
"""

import io

fails = []

# 1. lower the sea
P1 = "Source/VEIL/Private/Systems/PHIOcean.cpp"
s1 = io.open(P1, encoding="utf-8", newline="").read()
NL1 = "\r\n" if "\r\n" in s1 else "\n"
OLD1 = """void APhiOcean::BeginPlay()
{
	Super::BeginPlay();"""
NEW1 = """void APhiOcean::BeginPlay()
{
	Super::BeginPlay();

	// THE SEA WAS OVER THE CITY. The water surface sat above the plaza, so every frame
	// from the market was shot along the underside of the sea - a flat purple wash with
	// glowing shapes in it, which reads as "floating geometry" from inside. Four metres
	// down puts the surface below the plaza the runner stands on.
	SetActorLocation(GetActorLocation() - FVector(0.f, 0.f, 400.f));
	UE_LOG(LogTemp, Warning, TEXT("PHI SEA: ocean dropped 400 cm, surface now below the plaza."));"""
o1, n1 = OLD1.replace("\n", NL1), NEW1.replace("\n", NL1)
if s1.count(o1) != 1:
    fails.append("ocean beginplay count=%d" % s1.count(o1))
else:
    io.open(P1, "w", encoding="utf-8", newline="").write(s1.replace(o1, n1, 1))

# 2. the eye clear must not destroy anything
P2 = "Source/VEIL/Private/Character/VEILCharacter.cpp"
s2 = io.open(P2, encoding="utf-8", newline="").read()
NL2 = "\r\n" if "\r\n" in s2 else "\n"
START = "\t\t\t\t\tconst FVector Me = GetActorLocation();"
END = '\t\t\t\t\tUE_LOG(LogVEIL, Warning, TEXT("PHI EYE CLEAR: %d actor(s) cleared around the runner at %s, floor z=%.0f."),\n\t\t\t\t\t\tGone, *Me.ToCompactString(), FloorHit.ImpactPoint.Z);'
i, j = s2.find(START.replace("\n", NL2)), s2.find(END.replace("\n", NL2))
if i == -1 or j == -1:
    fails.append("eye sweep markers i=%d j=%d" % (i, j))
else:
    REPL = """					// SEAT HIM ON FLOOR, AND TOUCH NOTHING. The destructive version of this
					// took the whole market with it and left an empty blue void; the sea was
					// what filled the frame, and that is fixed in the ocean itself.
					const FVector Me = GetActorLocation();
					FHitResult FloorHit;
					FCollisionQueryParams Sp(SCENE_QUERY_STAT(VeilSettle), false, this);
					if (GroundWorld->LineTraceSingleByChannel(FloorHit, Me + FVector(0.f, 0.f, 1500.f),
						Me - FVector(0.f, 0.f, 3000.f), ECC_Visibility, Sp))
					{
						if (FloorHit.ImpactPoint.Z <= 200.f)
						{
							SetActorLocation(FVector(Me.X, Me.Y, FloorHit.ImpactPoint.Z + 98.f),
								false, nullptr, ETeleportType::TeleportPhysics);
						}
					}
					UE_LOG(LogVEIL, Warning, TEXT("PHI EYE CLEAR: runner seated at %s, floor z=%.0f."),
						*Me.ToCompactString(), FloorHit.ImpactPoint.Z);"""
    s2 = s2[:i] + REPL.replace("\n", NL2) + s2[j + len(END.replace("\n", NL2)):]
    io.open(P2, "w", encoding="utf-8", newline="").write(s2)

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("WROTE ocean (-400 cm) and character (non-destructive seat)")
