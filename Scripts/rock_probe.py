"""Measure the mountain shell that crosses the plaza, on a grid, at runtime."""

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(GM, encoding="utf-8", newline="").read().replace("\r\n", "\n")

anchor = "\tconst int32 Idx = FMath::Clamp(ShotIdx, 0, 4);\n"
probe = anchor + """
	if (Idx == 0)
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

if "VEIL PLAZA ROCK" in s:
    print("already probed")
elif anchor in s:
    s = s.replace(anchor, probe, 1)
    open(GM, "w", encoding="utf-8", newline="").write(s)
    print("probe installed")
else:
    print("MISS anchor")
