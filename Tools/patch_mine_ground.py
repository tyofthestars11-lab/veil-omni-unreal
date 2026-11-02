import io

ROOT = r"D:\VEIL-OMNI" + "\\"
P = "Source/VEIL/Private/World/VEILWilds.cpp"

t = io.open(ROOT + P, encoding="utf-8", newline=None).read()

old = '''		const float Angle = FMath::DegreesToRadians(m * WildsGoldenDeg);
		const float Ring = Home->RadiusCm * (0.55f + 0.45f * FMath::Frac(m * 0.6180339887f));
		const float X = Home->Center.X + FMath::Cos(Angle) * Ring;
		const float Y = Home->Center.Y + FMath::Sin(Angle) * Ring;'''

new = '''		const int32 HomeIndex = (int32)(Home - GMineHomes);
		const float Angle = FMath::DegreesToRadians(m * WildsGoldenDeg);
		const float RingFull = Home->RadiusCm * (0.55f + 0.45f * FMath::Frac(m * 0.6180339887f));
		const FVector2D Dir(FMath::Cos(Angle), FMath::Sin(Angle));

		// STANDING ON THE RIGHT GROUND.
		// A reach is wider than the distance between two anchors, so a point inside
		// "the Nest's reach" can still physically stand in the Hub - the first run put
		// TIDEVANE 60 cm from the Keeper. Being on your owner's ground is therefore
		// tested the way the region system itself decides it: the nearest anchor wins.
		// Shrink the ring until the owner's anchor is the nearest one.
		FVector2D Site = Home->Center;
		float Ring = 0.f;
		bool bOnOwnGround = false;
		for (float Scale = 1.f; Scale >= 0.15f; Scale -= 0.05f)
		{
			const FVector2D Try = Home->Center + Dir * (RingFull * Scale);
			int32 Nearest = INDEX_NONE;
			float NearestD = TNumericLimits<float>::Max();
			for (int32 h = 0; h < GMineHomeCount; ++h)
			{
				const float D = FVector2D::Distance(Try, GMineHomes[h].Center);
				if (D < NearestD) { NearestD = D; Nearest = h; }
			}
			if (Nearest == HomeIndex)
			{
				Site = Try;
				Ring = RingFull * Scale;
				bOnOwnGround = true;
				break;
			}
		}
		if (!bOnOwnGround)
		{
			// Nothing on the ring is theirs: the anchor itself is the only sure ground,
			// and saying so is better than pretending the claim is theirs.
			Site = Home->Center;
			Ring = 0.f;
			++OffOwnGround;
			UE_LOG(LogVEIL, Warning, TEXT("PHI MINE %02d %s has no point of %s ground free of a nearer anchor; set at the anchor."),
				m, Spec.Name, Spec.Owner);
		}
		const float X = Site.X;
		const float Y = Site.Y;'''

if old in t:
    t = t.replace(old, new, 1)
    print("cpp: ground test now uses the nearest anchor")
else:
    print("cpp: PATTERN MISSED")

# declare the counter before the loop
if "int32 OffOwnGround = 0;" not in t:
    t = t.replace("\tfor (int32 m = 0; m < Mines_; ++m)",
                  "\tint32 OffOwnGround = 0;\n\tfor (int32 m = 0; m < Mines_; ++m)", 1)
    print("cpp: claim counter")

# prove the claim count in the summary
old_sum = '''	TEXT("PHI WILDS: %d species bodies in %d camps, %d named mines on their owners' ground, all on phi clocks."),
		SpawnedEnemies, Camps.Num(), SpawnedMines);'''
new_sum = '''	TEXT("PHI WILDS: %d species bodies in %d camps, %d named mines, all on phi clocks."),
		SpawnedEnemies, Camps.Num(), SpawnedMines);
	UE_LOG(LogVEIL, Warning, TEXT("PHI MINE CLAIMS: %d of %d stand on their owner's ground, %d do not."),
		SpawnedMines - OffOwnGround, SpawnedMines, OffOwnGround);'''
if old_sum in t:
    t = t.replace(old_sum, new_sum, 1)
    print("cpp: claim count in the summary")
else:
    print("cpp: SUMMARY PATTERN MISSED")

io.open(ROOT + P, "w", encoding="utf-8", newline="\r\n").write(t)
print("written")
