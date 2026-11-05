"""A camera outside the rim photographs the mesa wall. Keep it over the paving."""

GM = "C:/Users/Admin/HarkGame/Source/VEILNI/VeilGameMode.cpp"
GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(GM, encoding="utf-8", newline="").read().replace("\r\n", "\n")

anchor = "\t\tFVector CamAt(Anchor[Idx].X - FMath::Cos(YawRad) * Back,\n"
block = """\t\tFVector CamAt(Anchor[Idx].X - FMath::Cos(YawRad) * Back,
			Anchor[Idx].Y - FMath::Sin(YawRad) * Back,
			GroundZ + PHI_Constants::Reach(UpPow[Idx]));

		// Every record camera stays over the paving: outside the rim it would be
		// photographing the outside of the terrace instead of the town.
		{
			const double Rad = FMath::Sqrt(CamAt.X * CamAt.X + CamAt.Y * CamAt.Y);
			if (Rad > 3200.0)
			{
				CamAt.X *= 3200.0 / Rad;
				CamAt.Y *= 3200.0 / Rad;
				UE_LOG(LogVEILWorld, Display,
					TEXT("VEIL SHOT CLAMP: %s pulled inside the rim"), Names[Idx]);
			}
		}
"""

if "VEIL SHOT CLAMP" in s:
    msg = "clamp already present"
elif anchor in s:
    s = s.replace(anchor, block, 1)
    open(GM, "w", encoding="utf-8", newline="").write(s)
    msg = "clamp installed"
else:
    msg = "MISS camera anchor"
print(msg)
