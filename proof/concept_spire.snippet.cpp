// ---------------------------------------------------------------------------
// THE SPIRE. The image's centre: a gold tower that can be seen from the market
// and that reads as a tower, not a block. Eight tiers, each narrower than the
// last by the square root of phi, each capped with a gold band, then a beacon
// shaft and a crown. Height is a phi ladder, so the silhouette narrows the way
// a spire actually narrows instead of stepping like a ziggurat.
// ---------------------------------------------------------------------------
void AVEILConceptWorld::BuildSpire(UStaticMesh* Cube, UStaticMesh* Cylinder, UStaticMesh* Sphere,
	UMaterialInterface* Gold, UMaterialInterface* Stone, UMaterialInterface* Glow)
{
	float Z = 0.f;
	float Width = 1900.f;

	for (int32 Tier = 0; Tier < 8; ++Tier)
	{
		const float Height = 640.f / FMath::Pow(ConceptPhi, Tier * 0.35f);
		SpawnPiece(Cube, SpireAt + FVector(0.f, 0.f, Z + Height * 0.5f),
			FVector(Width, Width, Height), Stone, FRotator(0.f, Tier * 7.5f, 0.f), true);

		// A gold band on every tier: the trim that makes stone read as the Spire.
		SpawnPiece(Cube, SpireAt + FVector(0.f, 0.f, Z + Height + 25.f),
			FVector(Width * 1.08f, Width * 1.08f, 50.f), Gold, FRotator(0.f, Tier * 7.5f, 0.f), false);

		Z += Height + 50.f;
		Width /= FMath::Sqrt(ConceptPhi);
	}

	// THE BEACON: a lit shaft out of the crown, then the crown itself.
	SpawnPiece(Cylinder, SpireAt + FVector(0.f, 0.f, Z + 1500.f), FVector(520.f, 520.f, 3000.f), Glow, FRotator::ZeroRotator, false);
	SpawnPiece(Sphere, SpireAt + FVector(0.f, 0.f, Z + 3200.f), FVector(1100.f, 1100.f, 1100.f), Gold, FRotator::ZeroRotator, false);
	SpawnPiece(Cube, SpireAt + FVector(0.f, 0.f, Z + 120.f), FVector(Width * 2.2f, Width * 2.2f, 240.f), Gold, FRotator(0.f, 45.f, 0.f), false);
}

// ---------------------------------------------------------------------------
// THE TERRACES. The image's left side: the market sits on shelves cut into a
// cliff, with water leaving each shelf. Six shelves on the golden angle, each
// further out and lower than the last, each with a fall on its inner face. This
// is the terrain the district was missing - a horizon with height in it.
// ---------------------------------------------------------------------------
void AVEILConceptWorld::BuildTerraces(UStaticMesh* Cube, UStaticMesh* Cylinder,
	UMaterialInterface* Stone, UMaterialInterface* Water, UMaterialInterface* Gold)
{
	for (int32 i = 0; i < 6; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * ConceptGoldenDeg + 28.f);
		const float Radius = 2600.f * FMath::Pow(ConceptPhi, i * 0.35f);
		const float Drop = -220.f * i;
		const FVector At(Plaza.X + FMath::Cos(Angle) * Radius,
			Plaza.Y + FMath::Sin(Angle) * Radius, Drop);

		// The shelf: a slab wide enough to stand a market on.
		SpawnPiece(Cube, At + FVector(0.f, 0.f, -260.f),
			FVector(3200.f, 3200.f, 520.f), Stone, FRotator(0.f, i * 13.7f, 0.f), true);

		// The lip, in gold, so a shelf edge reads from a distance.
		SpawnPiece(Cube, At + FVector(0.f, 0.f, 30.f),
			FVector(3300.f, 3300.f, 60.f), Gold, FRotator(0.f, i * 13.7f, 0.f), false);

		// The fall leaving the shelf, on the face toward the plaza.
		const FVector Inward = (Plaza - At).GetSafeNormal2D();
		SpawnPiece(Cube, At + Inward * 1600.f + FVector(0.f, 0.f, -700.f),
			FVector(900.f, 120.f, 1500.f), Water,
			FRotator(0.f, FMath::RadiansToDegrees(Angle) + 90.f, 0.f), false);
	}
}

// ---------------------------------------------------------------------------
// THE CRYSTAL RANGE. The image's right: blue peaks with real mass, not pebbles.
// Seven great spires between eight and fourteen kilometres out, heights climbing
// the phi ladder to more than a hundred metres, each with a lit core. Twenty
// shards around them at the same rule, so the range has a skirt.
// ---------------------------------------------------------------------------
void AVEILConceptWorld::BuildCrystalRange(UStaticMesh* Cone, UStaticMesh* Cylinder,
	UMaterialInterface* Water, UMaterialInterface* Glow)
{
	for (int32 i = 0; i < 7; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * ConceptGoldenDeg + 215.f);
		const float Radius = 8000.f + 900.f * i;
		const FVector At(Plaza.X + FMath::Cos(Angle) * Radius,
			Plaza.Y + FMath::Sin(Angle) * Radius, 0.f);
		const float Height = 5000.f * FMath::Pow(ConceptPhi, (i % 3) * 0.5f);
		const float Base = Height * 0.55f;

		SpawnPiece(Cone, At + FVector(0.f, 0.f, Height * 0.5f), FVector(Base, Base, Height), Water,
			FRotator(0.f, i * 23.f, 0.f), true);
		// The core: a narrower lit cone inside the mass, so the peak glows at dusk.
		SpawnPiece(Cone, At + FVector(0.f, 0.f, Height * 0.42f),
			FVector(Base * 0.45f, Base * 0.45f, Height * 0.84f), Glow,
			FRotator(0.f, i * 23.f, 0.f), false);
	}

	for (int32 i = 0; i < 20; ++i)
	{
		const float Angle = FMath::DegreesToRadians(i * ConceptGoldenDeg + 190.f);
		const float Radius = 6500.f + 260.f * i;
		const FVector At(Plaza.X + FMath::Cos(Angle) * Radius,
			Plaza.Y + FMath::Sin(Angle) * Radius, 0.f);
		const float Height = 700.f * FMath::Pow(ConceptPhi, (i % 4) * 0.5f);
		SpawnPiece(Cone, At + FVector(0.f, 0.f, Height * 0.5f),
			FVector(Height * 0.6f, Height * 0.6f, Height), Water,
			FRotator(0.f, i * 31.f, 0.f), false);
	}
}
