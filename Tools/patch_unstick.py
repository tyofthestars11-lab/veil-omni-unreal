"""Add the stuck-capsule doctor to AVEILCharacter::Tick, or abort without writing.

Measured in a packaged run: the Move action delivered axis=(0.00,0.80), the body
built acceleration V(484,-443) and its velocity stayed V(0) - not a dead
controller, a capsule inside geometry. Acceleration without velocity for phi
seconds now relocates the runner to the first open ground the golden angle finds.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

ANCHOR = "\tSuper::Tick(DeltaSeconds);\n\t++PhiTickEntryCount;"

INSERT = (
    "\tSuper::Tick(DeltaSeconds);\n"
    "\t++PhiTickEntryCount;\n"
    "\n"
    "\t// STUCK DOCTOR: ACCELERATION WITHOUT VELOCITY MEANS THE CAPSULE IS INSIDE\n"
    "\t// GEOMETRY. Measured, not guessed: \"axis=(0.00,0.80) mode=1 maxWalk=450\n"
    "\t// ignored=0 story=0 accel=V(484.21, -443.58) vel=V(0)\" - the input arrives,\n"
    "\t// the body builds acceleration, the world refuses to let it move, and no key\n"
    "\t// on the machine will ever help. Held for phi seconds, the runner is moved to\n"
    "\t// the first open ground the golden angle finds, and the log names both ends.\n"
    "\t{\n"
    "\t\tstatic float StuckClock = 0.f;\n"
    "\t\tconst UCharacterMovementComponent* StuckMove = GetCharacterMovement();\n"
    "\t\tconst bool bPushing = !MoveAxis.IsNearlyZero();\n"
    "\t\tconst bool bWedged = bPushing && StuckMove\n"
    "\t\t\t&& StuckMove->Velocity.SizeSquared() < 4.f\n"
    "\t\t\t&& StuckMove->GetCurrentAcceleration().SizeSquared() > 100.f;\n"
    "\t\tif (!bWedged) { StuckClock = 0.f; }\n"
    "\t\telse\n"
    "\t\t{\n"
    "\t\t\tStuckClock += DeltaSeconds;\n"
    "\t\t\tif (StuckClock >= UPHIMito::Phi)\n"
    "\t\t\t{\n"
    "\t\t\t\tStuckClock = 0.f;\n"
    "\t\t\t\tUWorld* StuckWorld = GetWorld();\n"
    "\t\t\t\tconst FVector From = GetActorLocation();\n"
    "\t\t\t\tbool bMoved = false;\n"
    "\t\t\t\tif (StuckWorld)\n"
    "\t\t\t\t{\n"
    "\t\t\t\t\tFCollisionQueryParams Probe(SCENE_QUERY_STAT(VEILUnstick), false, this);\n"
    "\t\t\t\t\tfor (int32 i = 1; i <= 34 && !bMoved; ++i)\n"
    "\t\t\t\t\t{\n"
    "\t\t\t\t\t\tconst float Ang = FMath::DegreesToRadians(i * 137.50776405f);\n"
    "\t\t\t\t\t\tconst float Rad = 260.f * FMath::Sqrt((float)i);\n"
    "\t\t\t\t\t\tconst float PX = From.X + FMath::Cos(Ang) * Rad;\n"
    "\t\t\t\t\t\tconst float PY = From.Y + FMath::Sin(Ang) * Rad;\n"
    "\t\t\t\t\t\tFHitResult Floor;\n"
    "\t\t\t\t\t\tif (!StuckWorld->LineTraceSingleByChannel(Floor, FVector(PX, PY, From.Z + 400.f),\n"
    "\t\t\t\t\t\t\tFVector(PX, PY, From.Z - 400.f), ECC_Visibility, Probe)) { continue; }\n"
    "\t\t\t\t\t\tconst FVector Stand(PX, PY, Floor.ImpactPoint.Z + 98.f);\n"
    "\t\t\t\t\t\tFCollisionQueryParams Free = Probe;\n"
    "\t\t\t\t\t\tFree.AddIgnoredActor(Floor.GetActor());\n"
    "\t\t\t\t\t\tif (StuckWorld->OverlapBlockingTestByChannel(Stand, FQuat::Identity, ECC_Pawn,\n"
    "\t\t\t\t\t\t\tFCollisionShape::MakeCapsule(38.f, 92.f), Free)) { continue; }\n"
    "\t\t\t\t\t\tSetActorLocation(Stand, false, nullptr, ETeleportType::TeleportPhysics);\n"
    "\t\t\t\t\t\tbMoved = true;\n"
    "\t\t\t\t\t\tUE_LOG(LogVEIL, Warning,\n"
    "\t\t\t\t\t\t\tTEXT(\"PHI UNSTICK: capsule was inside geometry at %s - moved to open ground at %s (%.0f cm out).\"),\n"
    "\t\t\t\t\t\t\t*From.ToCompactString(), *Stand.ToCompactString(), Rad);\n"
    "\t\t\t\t\t}\n"
    "\t\t\t\t}\n"
    "\t\t\t\tif (!bMoved)\n"
    "\t\t\t\t{\n"
    "\t\t\t\t\tUE_LOG(LogVEIL, Warning,\n"
    "\t\t\t\t\t\tTEXT(\"PHI UNSTICK: no open ground within 1500 cm of %s - the capsule is boxed in.\"),\n"
    "\t\t\t\t\t\t*From.ToCompactString());\n"
    "\t\t\t\t}\n"
    "\t\t\t}\n"
    "\t\t}\n"
    "\t}"
)

old = ANCHOR.replace("\n", NL)
found = src.count(old)
if found != 1:
    print("ABORTED - anchor found %d times (want 1)" % found)
    raise SystemExit(1)

out = src.replace(old, INSERT.replace("\n", NL), 1)
io.open(PATH, "w", encoding="utf-8", newline="").write(out)
print("WROTE %s: %d -> %d chars" % (PATH, len(src), len(out)))
