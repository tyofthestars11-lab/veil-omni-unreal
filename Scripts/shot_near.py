"""Name the geometry the lens is inside, and probe ground on the working channel."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

# FitPart's ECC_Visibility probe hits; the ECC_WorldStatic probe always missed.
old_t = ("GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_WorldStatic))\n"
         "\t\t{\n"
         "\t\t\tGroundZ = Hit.ImpactPoint.Z;")
new_t = ("GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_Visibility))\n"
         "\t\t{\n"
         "\t\t\tGroundZ = Hit.ImpactPoint.Z;")
if old_t not in s:
    raise SystemExit("MISS shot trace channel")
s = s.replace(old_t, new_t, 1)
print("shot: ground probe on ECC_Visibility")

anchor = "\t\tif (APawn* Pawn = PC->GetPawn())"
if anchor not in s:
    raise SystemExit("MISS pawn anchor")

near = '''		{
			// What is actually in front of the lens, named, with real bounds.
			int32 Near = 0;
			for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
			{
				const double D = FVector::Dist(It->GetActorLocation(), CamAt);
				if (D < 1000.0)
				{
					FVector O, E;
					It->GetActorBounds(true, O, E);
					UE_LOG(LogVEILWorld, Display,
						TEXT("VEIL SHOT NEAR: %s at %.0f | bounds %.0f x %.0f x %.0f"),
						*It->GetName(), D, E.X * 2.0, E.Y * 2.0, E.Z * 2.0);
					++Near;
				}
			}
			UE_LOG(LogVEILWorld, Display, TEXT("VEIL SHOT NEAR: %d piece(s) within 10 m of the lens"), Near);
		}
'''
s = s.replace(anchor, near + anchor, 1)
open(p, "w", encoding="utf-8", newline="").write(s)
print("shot: near-lens inventory armed")
