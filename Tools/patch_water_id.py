"""Name the water surface and put it under the city.

The runner stands at 104 with nothing but a surface at z=7 beneath him, and every frame is
that surface: the plaza is drowned by 7 cm and the camera looks across a purple sea. The
ocean patch did not run (no PHI SEA line), so the surface belongs to another actor. This
traces the plaza column at boot, logs the actor and mesh that answer, and lowers whatever
is holding the water by five metres.
"""

import io

P = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

ANCHOR = '\t\t\t\tUE_LOG(LogVEIL, Warning, TEXT("PHI PLAZA CLEAR: %d shell(s) removed from the spawn column."), Cleared);\n\t\t\t}'
ADD = ANCHOR + """
			// WHO IS THE WATER. The plaza reads as a flat purple sea: a surface sits at
			// z=7 over the ground the tiles laid at 0, and the camera at 1.8 m looks
			// straight across it. Name it here, then drop it under the city.
			{
				FHitResult WetHit;
				FCollisionQueryParams WetParams(SCENE_QUERY_STAT(VeilWaterId), false, this);
				const FVector WetFrom(0.f, -1500.f, 400.f);
				const FVector WetTo(0.f, -1500.f, -200.f);
				if (GroundWorld->LineTraceSingleByChannel(WetHit, WetFrom, WetTo, ECC_Visibility, WetParams))
				{
					AActor* Wet = WetHit.GetActor();
					const FString WetName = Wet ? Wet->GetName() : TEXT("none");
					const FString WetClass = Wet ? Wet->GetClass()->GetName() : TEXT("none");
					const FString WetMesh = WetHit.GetComponent() ? WetHit.GetComponent()->GetName() : TEXT("none");
					UE_LOG(LogVEIL, Warning,
						TEXT("PHI WATER ID: surface at z=%.0f is %s (%s), component %s."),
						WetHit.ImpactPoint.Z, *WetName, *WetClass, *WetMesh);

					// Anything holding a surface over the plaza that is not the tagged
					// floor is the sea. Five metres down puts it under the city.
					int32 Drowned = 0;
					for (TActorIterator<AActor> WetIt(GroundWorld); WetIt; ++WetIt)
					{
						AActor* W = *WetIt;
						if (!W || W == this) { continue; }
						const FString Cls = W->GetClass()->GetName();
						if (!Cls.Contains(TEXT("Ocean")) && !Cls.Contains(TEXT("Water")) &&
							!Cls.Contains(TEXT("Lake")) && !W->GetName().Contains(TEXT("Water")))
						{
							continue;
						}
						W->SetActorLocation(W->GetActorLocation() - FVector(0.f, 0.f, 500.f));
						++Drowned;
						UE_LOG(LogVEIL, Warning, TEXT("PHI SEA: %s (%s) lowered 500 cm under the city."),
							*W->GetName(), *Cls);
					}
					UE_LOG(LogVEIL, Warning, TEXT("PHI SEA: %d water actor(s) lowered."), Drowned);
				}
			}"""
o, n = ANCHOR.replace("\n", NL), ADD.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - water anchor count=%d" % src.count(o))
    raise SystemExit(1)
io.open(P, "w", encoding="utf-8", newline="").write(src.replace(o, n, 1))
print("WROTE %s - water identified and lowered" % P)
