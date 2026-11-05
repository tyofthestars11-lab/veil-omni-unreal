"""The aerial still shows a dark mass on the plaza. Name what is standing there."""

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(GM, encoding="utf-8", newline="").read().replace("\r\n", "\n")

anchor = "\tconst int32 Idx = FMath::Clamp(ShotIdx, 0, 4);\n"
block = anchor + """
	if (Idx == 0)
	{
		// Name every authored piece standing near the point the aerial looks at.
		const FVector Aim(AimAt[Idx].X, AimAt[Idx].Y, GHubGroundZ);
		for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
		{
			const double D = FVector::Dist2D(It->GetActorLocation(), Aim);
			if (D < 900.0)
			{
				FVector O, E;
				It->GetActorBounds(true, O, E);
				UE_LOG(LogVEILWorld, Display,
					TEXT("VEIL PLAZA BODY: %s at %.0f | size %.0f x %.0f x %.0f | top %.0f | centre %.0f"),
					*It->GetActorNameOrLabel(), D, E.X * 2.0, E.Y * 2.0, E.Z * 2.0,
					O.Z + E.Z, O.Z);
			}
		}
	}
"""

if "VEIL PLAZA BODY" in s:
    print("already installed")
elif anchor in s:
    s = s.replace(anchor, block, 1)
    open(GM, "w", encoding="utf-8", newline="").write(s)
    print("plaza body probe installed")
else:
    print("MISS anchor")
