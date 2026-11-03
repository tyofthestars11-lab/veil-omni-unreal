"""The vista pass: what the reference frame shows and the district did not build.

Elements taken from the images, in the order they define the silhouette:
  1. a blue crystal range on one flank, clustered on the golden angle;
  2. waterfalls pouring off cliff faces into the valley;
  3. a terraced village - shelves with enterable houses - around the market;
  4. timber/rope bridges spanning between the terraces;
  5. the three gold phi pillars in the foreground, glyph plate and crown;
  6. gold beacon shafts standing from the authority sites into the sky.

All of it uses meshes the builder already loads (wall, floor, column, monolith, pyramid crown)
and the project's own palette, so nothing here needs an editor step or a new asset.
"""

import io

H = "Source/VEIL/Public/World/VEILWorldBuilder.h"
CPP = "Source/VEIL/Private/World/VEILWorldBuilder.cpp"

# ---------------------------------------------------------------- header
hs = io.open(H, encoding="utf-8", newline="").read()
HN = "\r\n" if "\r\n" in hs else "\n"
if "void BuildVista();" not in hs:
    anchor = "\tvoid BuildDistrict();"
    if hs.count(anchor) != 1:
        print("header anchor count=%d" % hs.count(anchor))
        raise SystemExit(1)
    hs = hs.replace(anchor, anchor + HN + HN +
                    "\t/** The silhouette the reference frame asks for: crystal range, cliff falls," + HN +
                    "\t *  terraced houses, bridges, phi pillars, beacon shafts. */" + HN +
                    "\tvoid BuildVista();", 1)
    io.open(H, "w", encoding="utf-8", newline="").write(hs)
    print("header: BuildVista declared")
else:
    print("header: already declared")

# ---------------------------------------------------------------- cpp
s = io.open(CPP, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"

# call it from BuildWorld, after the beacon stone and before the horizon
callAnchor = "\tUE_LOG(LogVEIL, Log, TEXT(\"VEIL: free to play under the PHI LICENSE"
if "void AVEILWorldBuilder::BuildVista" not in s:
    if s.count(callAnchor) != 1:
        print("call anchor count=%d" % s.count(callAnchor))
        raise SystemExit(1)
    s = s.replace(callAnchor, "\tBuildVista();" + N + callAnchor, 1)
    print("BuildWorld now raises the vista")

IMPL = r'''
// ---------------------------------------------------------------------------
// THE VISTA. The reference frame is not a plaza with props: it is a valley with a blue
// crystal range on one flank, water falling off the cliffs, a village stepped up the rock
// in terraces joined by bridges, three gold phi pillars in the foreground and light shafts
// standing out of the holy sites. This builds that silhouette from the kit the district
// already uses, on the golden angle, at real sizes in centimetres.
// ---------------------------------------------------------------------------
void AVEILWorldBuilder::BuildVista()
{
	UWorld* World = GetWorld();
	if (!World) return;

	UStaticMesh* Crystal = PyramidHeadMesh ? PyramidHeadMesh.Get() : MonolithMesh.Get();
	UStaticMesh* Spike = MonolithMesh ? MonolithMesh.Get() : Crystal;
	UStaticMesh* Stone = WallMesh ? WallMesh.Get() : FloorMesh.Get();
	UStaticMesh* Ground = FloorMesh ? FloorMesh.Get() : Stone;
	UStaticMesh* Pillar = ColumnMesh ? ColumnMesh.Get() : CylinderMesh.Get();
	UStaticMesh* Water = FloorMesh ? FloorMesh.Get() : Stone;
	if (!Crystal || !Stone || !Pillar || !Ground) { return; }

	const FLinearColor IceC(0.30f, 0.58f, 1.0f);
	const FLinearColor ColdC(0.18f, 0.34f, 0.72f);
	const FLinearColor Gold(0.72f, 0.56f, 0.28f);
	const FLinearColor StoneC(0.42f, 0.39f, 0.35f);
	const FLinearColor WaterC(0.52f, 0.74f, 1.0f);
	const float PhiV = 1.61803398875f;
	const float GoldDeg = 137.50776405f;

	int32 Peaks = 0, Falls = 0, Houses = 0, Bridges = 0, Pillars = 0, Shafts = 0;

	// 1. THE CRYSTAL RANGE. Blue spires on the far flank, clustered rather than scattered:
	// five shards per cluster, each shorter than the last, all leaning out of one base.
	for (int32 i = 0; i < 14; ++i)
	{
		const float Ang = FMath::DegreesToRadians(i * GoldDeg + 205.f);
		const float Rad = 5200.f * FMath::Pow(PhiV, (float)(i % 5) * 0.20f);
		const FVector Base(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);
		const float ClusterH = 1500.f + (float)(i % 4) * 700.f;
		for (int32 K = 0; K < 5; ++K)
		{
			const float KAng = FMath::DegreesToRadians(K * GoldDeg);
			const float KR = 300.f + 110.f * (float)K;
			const FVector At = Base + FVector(FMath::Cos(KAang) * KR, FMath::Sin(KAang) * KR, 0.f);
			const float KH = ClusterH * (1.f - 0.13f * (float)K);
			if (SpawnBlock(At + FVector(0.f, 0.f, KH * 0.5f),
				FVector(KH * 0.0024f, KH * 0.0024f, KH / 100.f),
				(K % 2 == 0) ? Crystal : Spike, (K % 2 == 0) ? IceC : ColdC, 0.45f))
			{
				++Peaks;
			}
		}
	}

	// 2. WATERFALLS OFF THE CLIFF FACES. A rock buttress, then the sheet down its face, then
	// the basin it lands in - so the water reads as falling rather than as a blue plate.
	for (int32 i = 0; i < 6; ++i)
	{
		const float Ang = FMath::DegreesToRadians(i * GoldDeg + 60.f);
		const float Rad = 3400.f * FMath::Pow(PhiV, (float)(i % 3) * 0.25f);
		const FVector At(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);
		const FRotator Face(0.f, FMath::RadiansToDegrees(Ang) + 180.f, 0.f);
		const float FallH = 2200.f + (float)(i % 3) * 500.f;

		SpawnBlockRot(At + FVector(0.f, 0.f, FallH * 0.5f),
			FVector(14.f, 14.f, FallH / 100.f), Stone, StoneC, 0.f, Face);
		if (SpawnBlockRot(At + FVector(-260.f, 0.f, FallH * 0.45f),
			FVector(0.5f, 7.f, FallH * 0.85f / 100.f), Water, WaterC, 0.55f, Face))
		{
			++Falls;
		}
		SpawnBlockRot(At + FVector(-330.f, 0.f, 40.f), FVector(6.f, 9.f, 0.6f), Water, WaterC, 0.25f, Face);
	}

	// 3. THE TERRACED VILLAGE. Three shelves stepped up the rock, each carrying enterable
	// houses from BuildRoom, so the village is somewhere you can walk into, not scenery.
	TArray<FVector> Terraces;
	for (int32 T = 0; T < 3; ++T)
	{
		const float Ang = FMath::DegreesToRadians(T * GoldDeg + 40.f);
		const float Rad = 2300.f * FMath::Pow(PhiV, (float)T);
		const FVector C(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);
		const float DeckZ = 320.f + 220.f * (float)T;
		Terraces.Add(C + FVector(0.f, 0.f, DeckZ));

		SpawnBlock(C + FVector(0.f, 0.f, DeckZ * 0.5f), FVector(11.f, 8.f, DeckZ / 100.f), Ground, StoneC, 0.f);
		const FVector Facing = (-C).GetSafeNormal2D();
		for (int32 H = 0; H < 5; ++H)
		{
			const float HAng = FMath::DegreesToRadians(H * GoldDeg + 20.f);
			const float HR = 620.f + 120.f * (float)H;
			const FVector At = C + FVector(FMath::Cos(HAng) * HR, FMath::Sin(HAng) * HR, DeckZ);
			UStaticMesh* WallHere = WallVariants.Num() ? WallVariants[H % WallVariants.Num()].Get() : Stone;
			UStaticMesh* FloorHere = FloorVariants.Num() ? FloorVariants[H % FloorVariants.Num()].Get() : Ground;
			BuildRoom(At, 300.f, 260.f, 640.f, Facing, WallHere, FloorHere, StoneC, false);
			++Houses;
		}
	}

	// 4. BRIDGES BETWEEN THE TERRACES. Planks on the sag line, a post at each end, a rope
	// above - the span the picture runs between the rock shelves.
	for (int32 B = 0; B + 1 < Terraces.Num(); ++B)
	{
		const FVector A = Terraces[B];
		const FVector Z = Terraces[B + 1];
		const FVector Delta = Z - A;
		const float Len = Delta.Size2D();
		if (Len < 300.f) { continue; }
		const FRotator Facing = Delta.Rotation();
		const int32 Planks = FMath::Clamp(FMath::RoundToInt(Len / 110.f), 6, 26);
		const float Sag = Len * 0.06f;
		for (int32 P = 0; P <= Planks; ++P)
		{
			const float T = (float)P / (float)Planks;
			const FVector At = FMath::Lerp(A, Z, T) - FVector(0.f, 0.f, FMath::Sin(T * PI) * Sag);
			SpawnBlockRot(At, FVector(0.9f, 0.9f, 0.10f), Ground, StoneC, 0.f, Facing);
		}
		for (int32 Side = -1; Side <= 1; Side += 2)
		{
			const FVector Off = Facing.RotateVector(FVector(0.f, Side * 90.f, 0.f));
			SpawnBlockRot(A + Off + FVector(0.f, 0.f, 190.f), FVector(0.16f, 0.16f, 3.8f), Pillar, Gold, 0.f, FRotator::ZeroRotator);
			SpawnBlockRot(Z + Off + FVector(0.f, 0.f, 190.f), FVector(0.16f, 0.16f, 3.8f), Pillar, Gold, 0.f, FRotator::ZeroRotator);
			const FVector Mid = (A + Z) * 0.5f + Off - FVector(0.f, 0.f, Sag * 0.5f);
			const FRotator RopeRot = FRotationMatrix::MakeFromZ(Delta + FVector(0.f, 0.f, -Sag)).Rotator();
			SpawnBlockRot(Mid + FVector(0.f, 0.f, 300.f), FVector(0.06f, 0.06f, Len / 100.f), Pillar, Gold, 0.f, RopeRot);
		}
		++Bridges;
	}

	// 5. THE THREE GOLD PHI PILLARS. The picture's foreground: three columns carrying a gold
	// plate, at radius 1100, 1100*phi, 1100*phi^2, on the golden angle.
	for (int32 i = 0; i < 3; ++i)
	{
		const float Ang = FMath::DegreesToRadians(i * GoldDeg + 30.f);
		const float Rad = 1100.f * FMath::Pow(PhiV, (float)i);
		const FVector Base(FMath::Cos(Ang) * Rad, FMath::Sin(Ang) * Rad, 0.f);
		SpawnBlock(Base + FVector(0.f, 0.f, 40.f), FVector(3.4f, 3.4f, 0.8f), Ground, StoneC, 0.f);
		SpawnBlock(Base + FVector(0.f, 0.f, 600.f), FVector(0.9f, 0.9f, 11.f), Pillar, Gold, 0.15f);
		SpawnBlock(Base + FVector(0.f, 0.f, 1180.f), FVector(2.2f, 2.2f, 0.5f), Crystal, Gold, 1.4f);
		++Pillars;
	}

	// 6. BEACON SHAFTS. The light standing out of the holy sites into the sky - the vertical
	// gold lines that give the picture its depth.
	struct FSite { float X, Y; };
	static const FSite Sites[] = { { 900.f, 0.f }, { -450.f, 780.f }, { -4150.f, -780.f } };
	for (const FSite& Site : Sites)
	{
		if (SpawnBlock(FVector(Site.X, Site.Y, 2600.f), FVector(0.55f, 0.55f, 46.f), Pillar, Gold, 2.6f))
		{
			++Shafts;
		}
		SpawnBlock(FVector(Site.X, Site.Y, 300.f), FVector(2.4f, 2.4f, 6.f), Crystal, Gold, 1.1f);
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI VISTA: %d crystal peaks, %d waterfall(s), %d house(s) on 3 terraces, %d bridge span(s), %d phi pillar(s), %d beacon shaft(s) - the district reads in silhouette."),
		Peaks, Falls, Houses, Bridges, Pillars, Shafts);
}
'''

if "void AVEILWorldBuilder::BuildVista" not in s:
    s = s.rstrip() + N + IMPL.replace("\n", N) + N
    print("BuildVista implementation appended")
io.open(CPP, "w", encoding="utf-8", newline="").write(s)
print("vista written")
