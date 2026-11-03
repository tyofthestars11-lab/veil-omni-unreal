"""Seat law in the right files this time.

The previous attempt looked for the boxed-in log in VEILGameMode.cpp; that message lives
in VEILCharacter.cpp (the unstick), and the abort meant nothing was written at all.

1. AVEILGameMode::SpawnDefaultPawnFor hands the map's PlayerStart transform through, so
   the authored seat V(0,0,100) is discarded and the capsule starts at V(-516,472,198) -
   inside geometry, above the plaza. It spawns on the authority coordinate now.
2. When the unstick reports it is boxed in, the runner is moved to V(0,0,100) once - over
   the nine kit tiles PHI GROUND lays with their top face at z=0.
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
    "\t// THE MAP'S PLAYERSTART DOES NOT DECIDE WHERE THE RUNNER BEGINS. The StartSpot\n"
    "\t// transform was handed straight through, so the authored seat V(0,0,100) was lost\n"
    "\t// and the capsule began at V(-516,472,198) - inside geometry, above the plaza,\n"
    "\t// which is the frame that never moves. RUNNER's own coordinate wins.\n"
    "\tconst FTransform Authority(FRotator(0.f, 0.f, 0.f), FVector(0.f, 0.f, 100.f));\n"
    "\tUE_LOG(LogVEIL, Warning,\n"
    "\t\tTEXT(\"PHI SEAT LAW: runner spawns on V(0,0,100) over PHI GROUND, not on %s.\"),\n"
    "\t\tStartSpot ? *StartSpot->GetName() : TEXT(\"a default start spot\"));\n"
    "\treturn SpawnDefaultPawnAtTransform_Implementation(NewPlayer, Authority);\n"
    "}"
)
o, n = OLD.replace("\n", GNL), NEW.replace("\n", GNL)
if g.count(o) != 1:
    fails.append("SpawnDefaultPawnFor count=%d" % g.count(o))
else:
    io.open(G, "w", encoding="utf-8", newline="").write(g.replace(o, n, 1))
    print("%s: seat law" % G)

C = "Source/VEIL/Private/Character/VEILCharacter.cpp"
c = io.open(C, encoding="utf-8", newline="").read()
CNL = "\r\n" if "\r\n" in c else "\n"

marker = "the capsule is boxed in"
i = c.find(marker)
if i < 0:
    fails.append("boxed-in marker missing in Character")
else:
    end = c.find(");", i)
    line_end = c.find(CNL, end)
    RESCUE = """
					// BOXED IN MEANS THE PLAZA, ONCE. He was left inside the PlayerStart's
					// corner every run, which is the stand-still frame; one teleport to his
					// own coordinate puts him on the floor PHI GROUND laid.
					static bool bPlazaRescue = false;
					if (!bPlazaRescue)
					{
						bPlazaRescue = true;
						SetActorLocation(FVector(0.f, 0.f, 100.f), false, nullptr, ETeleportType::TeleportPhysics);
						UE_LOG(LogVEIL, Warning, TEXT("PHI SEAT LAW: boxed in, so the runner stands on V(0,0,100)."));
					}""".replace("\n", CNL)
    c = c[:line_end] + RESCUE + c[line_end:]
    io.open(C, "w", encoding="utf-8", newline="").write(c)
    print("%s: boxed-in rescue" % C)

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("SEAT LAW WRITTEN")
