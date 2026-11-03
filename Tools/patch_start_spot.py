"""The map's PlayerStart must not decide where the Runner begins, and boxed-in means plaza.

Measured: PHI RUNNER SPAWN logged V(0,0,100), and one frame later the capsule was at
V(-516.16, 472.84, 198.00) - the map's PlayerStart, which SpawnDefaultPawnFor hands to
SpawnDefaultPawnAtTransform whenever StartSpot is set. So the authored seat was thrown
away and the unstick then reported "boxed in" from the PlayerStart's corner.

1. SpawnDefaultPawnFor spawns at V(0,0,100) - RUNNER's authority coordinate - over the
   nine kit tiles PHI GROUND lays with their top face at z=0.
2. When the unstick is genuinely boxed in, it moves him there once instead of leaving him
   inside geometry, which is the state that produced the floating frames.
"""

import io

fails = []

G = "Source/VEIL/Private/Game/VEILGameMode.cpp"
g = io.open(G, encoding="utf-8", newline="").read()
GNL = "\r\n" if "\r\n" in g else "\n"

OLD = (
    "APawn* AVEILGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)\n"
    "{\n"
    "\tif (StartSpot)\n"
    "\t{\n"
    "\t\treturn SpawnDefaultPawnAtTransform_Implementation(NewPlayer, StartSpot->GetActorTransform());\n"
    "\t}\n"
    "\treturn Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);\n"
    "}"
)
NEW = (
    "APawn* AVEILGameMode::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)\n"
    "{\n"
    "\t// THE MAP'S PLAYERSTART DOES NOT DECIDE WHERE THE RUNNER BEGINS. This handed\n"
    "\t// StartSpot's transform straight through, so the authored seat V(0,0,100) was\n"
    "\t// thrown away and the capsule began at the map's PlayerStart, V(-516,472,198) -\n"
    "\t// inside geometry, five metres above the plaza. RUNNER's coordinate wins.\n"
    "\tconst FTransform AuthoritySeat(FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 100.f));\n"
    "\tUE_LOG(LogVEIL, Warning,\n"
    "\t\tTEXT(\"PHI SEAT LAW: runner spawns on V(0,0,100) over PHI GROUND, not on %s.\"),\n"
    "\t\tStartSpot ? *StartSpot->GetName() : TEXT(\"a default start spot\"));\n"
    "\treturn SpawnDefaultPawnAtTransform_Implementation(NewPlayer, AuthoritySeat);\n"
    "}"
)
o, n = OLD.replace("\n", GNL), NEW.replace("\n", GNL)
if g.count(o) != 1:
    fails.append("SpawnDefaultPawnFor count=%d" % g.count(o))
else:
    g = g.replace(o, n, 1)

marker = "- the capsule is boxed in."
i = g.find(marker)
if i < 0:
    fails.append("boxed-in log missing")
else:
    end = g.find(");", i)
    line_end = g.find(GNL, end)
    RESCUE = """
					// BOXED IN MEANS THE PLAZA, ONCE. He was left inside the PlayerStart's
					// corner every run, which is the stand-still frame. One teleport to his
					// own coordinate puts him on the new floor.
					static bool bPlazaRescue = false;
					if (!bPlazaRescue)
					{
						bPlazaRescue = true;
						SetActorLocation(FVector(0.f, 0.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
						UE_LOG(LogVEIL, Warning, TEXT("PHI SEAT LAW: boxed in, so the runner stands on V(0,0,100)."));
					}""".replace("\n", GNL)
    g = g[:line_end] + RESCUE + g[line_end:]

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(G, "w", encoding="utf-8", newline="").write(g)
print("%s: seat law + boxed-in rescue written" % G)
