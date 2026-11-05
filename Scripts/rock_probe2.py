"""Trace far above everything, ignoring every authored piece, to find the rock."""

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(GM, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old = """	if (Idx == 0)
	{
		// The map's mountains are single meshes millions of units across, and their
		// shell crosses the plaza. Measure how high it actually rises there.
		double Lo = 1.0e9, Hi = -1.0e9;
		int32 Hits = 0;
		for (int32 gx = -3; gx <= 3; ++gx)
		{
			for (int32 gy = -3; gy <= 3; ++gy)
			{
				FHitResult H;
				const double X = gx * 700.0;
				const double Y = gy * 700.0;
				if (GetWorld()->LineTraceSingleByChannel(H,
					FVector(X, Y, GHubGroundZ + 5000.0), FVector(X, Y, GHubGroundZ - 5000.0),
					ECC_Visibility))
				{
					Lo = FMath::Min(Lo, static_cast<double>(H.ImpactPoint.Z));
					Hi = FMath::Max(Hi, static_cast<double>(H.ImpactPoint.Z));
					++Hits;
				}
			}
		}
		UE_LOG(LogVEILWorld, Display,
			TEXT("VEIL PLAZA ROCK: %d/49 samples hit, ground %.0f .. %.0f (deck %.0f)"),
			Hits, Lo, Hi, GHubGroundZ);
	}
"""

new = """	{
		// Ignore everything we authored and ask only where the map's own rock is.
		TArray<AActor*> Ignore;
		for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
		{
			if (It->ActorHasTag(TEXT("VEIL_AUTHORED")))
			{
				Ignore.Add(*It);
			}
		}
		FCollisionQueryParams Params(SCENE_QUERY_STAT(VeilRockProbe), false);
		Params.AddIgnoredActors(Ignore);

		double Lo = 1.0e9, Hi = -1.0e9, AtX = 0.0, AtY = 0.0;
		int32 Hits = 0;
		for (int32 gx = -4; gx <= 4; ++gx)
		{
			for (int32 gy = -4; gy <= 4; ++gy)
			{
				const double X = gx * 700.0;
				const double Y = gy * 700.0;
				FHitResult H;
				if (GetWorld()->LineTraceSingleByChannel(H,
					FVector(X, Y, GHubGroundZ + 30000.0), FVector(X, Y, GHubGroundZ - 30000.0),
					ECC_Visibility, Params))
				{
					if (H.ImpactPoint.Z < Lo) { Lo = H.ImpactPoint.Z; }
					if (H.ImpactPoint.Z > Hi)
					{
						Hi = H.ImpactPoint.Z;
						AtX = X;
						AtY = Y;
					}
					++Hits;
				}
			}
		}
		UE_LOG(LogVEILWorld, Display,
			TEXT("VEIL ROCK PROBE: %d/81 samples, rock %.0f .. %.0f, high point at (%.0f, %.0f), %d authored pieces ignored"),
			Hits, Lo, Hi, AtX, AtY, Ignore.Num());
	}
"""

if "VEIL ROCK PROBE" in s:
    print("already probed")
elif old in s:
    s = s.replace(old, new, 1)
    open(GM, "w", encoding="utf-8", newline="").write(s)
    print("probe replaced")
else:
    print("MISS old probe")
