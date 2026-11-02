import io

ROOT = r"D:\VEIL-OMNI" + "\\"
P = "Source/VEIL/Private/World/VEILWilds.cpp"

t = io.open(ROOT + P, encoding="utf-8", newline=None).read()

old_start = "\t\tconst int32 HomeIndex = (int32)(Home - GMineHomes);"
old_end = "\t\tconst float X = Site.X;"

i0 = t.find(old_start)
i1 = t.find(old_end)
if i0 < 0 or i1 < 0 or i1 < i0:
    print("cpp: BLOCK NOT FOUND", i0, i1)
else:
    NEW = '''		const int32 HomeIndex = (int32)(Home - GMineHomes);
		const float Angle = FMath::DegreesToRadians(m * WildsGoldenDeg);
		const float RingFull = Home->RadiusCm * (0.55f + 0.45f * FMath::Frac(m * 0.6180339887f));
		const FVector2D Dir(FMath::Cos(Angle), FMath::Sin(Angle));

		// STANDING ON THE RIGHT GROUND.
		// A reach is wider than the distance between two anchors, so a point inside
		// "the Nest's reach" can still stand in the Hub - the first run put TIDEVANE
		// 60 cm from the Keeper. And two owners can share an anchor: the Warden holds
		// the Deep's inner disc while the Hollow works the rim, both centred on 0,0.
		// So the rule is a claim, not a nearest neighbour: every home owns everything
		// inside its reach, and the SMALLEST reach that covers a point owns it. The
		// Warden's 620 keeps the disc; the Hollow's 1,250 takes the rim outside it.
		auto ClaimOf = [](const FVector2D& Pt) -> int32
		{
			int32 Owner = INDEX_NONE;
			float Smallest = TNumericLimits<float>::Max();
			for (int32 h = 0; h < GMineHomeCount; ++h)
			{
				if (FVector2D::Distance(Pt, GMineHomes[h].Center) > GMineHomes[h].RadiusCm) { continue; }
				if (GMineHomes[h].RadiusCm < Smallest)
				{
					Smallest = GMineHomes[h].RadiusCm;
					Owner = h;
				}
			}
			return Owner;
		};

		FVector2D Site = Home->Center;
		float Ring = 0.f;
		bool bOnOwnGround = false;
		for (float Scale = 1.f; Scale >= 0.15f; Scale -= 0.05f)
		{
			const FVector2D Try = Home->Center + Dir * (RingFull * Scale);
			if (ClaimOf(Try) == HomeIndex)
			{
				Site = Try;
				Ring = RingFull * Scale;
				bOnOwnGround = true;
				break;
			}
		}
		if (!bOnOwnGround)
		{
			// Nothing on the ring is theirs: say so rather than dress a squatter up as
			// an owner. The seam goes to the edge of the reach they do hold.
			Ring = Home->RadiusCm * 0.15f;
			Site = Home->Center + Dir * Ring;
			++OffOwnGround;
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI MINE %02d %s finds no clear point of %s ground; set at the edge of the reach."),
				m, Spec.Name, Spec.Owner);
		}
'''
    t = t[:i0] + NEW + t[i1:]
    print("cpp: claim-band ground rule installed")

io.open(ROOT + P, "w", encoding="utf-8", newline="\r\n").write(t)
print("written")
