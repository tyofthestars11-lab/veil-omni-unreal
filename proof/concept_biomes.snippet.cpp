// ---------------------------------------------------------------------------
// THE WILDS. The image is not only landmarks: between them there is country -
// groves, broken ruins, boulders, shards. Hand-placing a few thousand of those
// is how a world ends up looking like a diorama, so they are scattered on the
// golden spiral and drawn as INSTANCES: one component per shape instead of one
// actor per stone, with cull distances so the far country costs nothing.
//
// Placement rule: angle = i * 137.50776405 degrees, radius grows like sqrt(i),
// which is the same sunflower rule the district ring uses. Nothing lands in the
// plaza, on the Spire, or in the Deep, and everything sits on traced ground.
// ---------------------------------------------------------------------------
void AVEILConceptWorld::BuildBiomes(UStaticMesh* Cube, UStaticMesh* Cone, UStaticMesh* Cylinder,
	UStaticMesh* Sphere, UMaterialInterface* Stone, UMaterialInterface* Water,
	UMaterialInterface* Gold)
{
	UWorld* W = GetWorld();
	if (!W) { return; }

	auto MakeSet = [this](UStaticMesh* Mesh, UMaterialInterface* Mat, float Cull) -> UInstancedStaticMeshComponent*
	{
		if (!Mesh) { return nullptr; }
		UInstancedStaticMeshComponent* Set = NewObject<UInstancedStaticMeshComponent>(this);
		Set->SetStaticMesh(Mesh);
		if (Mat) { Set->SetMaterial(0, Mat); }
		Set->SetMobility(EComponentMobility::Movable);
		Set->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		// Distance culling is the difference between a world and a slideshow.
		Set->SetCullDistances(0, (int32)Cull);
		Set->SetupAttachment(RootComponent);
		Set->RegisterComponent();
		return Set;
	};

	UInstancedStaticMeshComponent* Trunks   = MakeSet(Cylinder, Stone, 22000.f);
	UInstancedStaticMeshComponent* Canopies = MakeSet(Cone, Water, 24000.f);
	UInstancedStaticMeshComponent* Ruins    = MakeSet(Cube, Stone, 32000.f);
	UInstancedStaticMeshComponent* Boulders = MakeSet(Sphere, Stone, 26000.f);
	UInstancedStaticMeshComponent* Shards   = MakeSet(Cone, Water, 30000.f);
	UInstancedStaticMeshComponent* Posts    = MakeSet(Cylinder, Gold, 30000.f);
	if (!Trunks || !Canopies || !Ruins || !Boulders || !Shards) { return; }

	// Keep the country out of the places that are already composed.
	auto Blocked = [](const FVector2D& P) -> bool
	{
		if (P.Size() < 2400.f) { return true; }                                  // plaza + market
		if ((P - FVector2D(2800.f, 500.f)).Size() < 1700.f) { return true; }     // Spire
		if ((P - FVector2D(0.f, -6200.f)).Size() < 1900.f) { return true; }      // Deep
		if ((P - FVector2D(-4150.f, -780.f)).Size() < 900.f) { return true; }    // survey ground
		return false;
	};

	auto Ground = [W](float X, float Y, float& OutZ) -> bool
	{
		FHitResult Hit;
		const FVector From(X, Y, 30000.f);
		const FVector To(X, Y, -4000.f);
		if (!W->LineTraceSingleByChannel(Hit, From, To, ECC_Visibility, FCollisionQueryParams::DefaultQueryParam))
		{
			return false;
		}
		OutZ = Hit.ImpactPoint.Z;
		return true;
	};

	int32 Trees = 0, Broken = 0, Stones = 0, Spikes = 0;

	// GROVES: trunk plus canopy, scaled along the phi ladder so a grove has tall
	// and short trees in it rather than one tree repeated.
	for (int32 i = 0; i < 260; ++i)
	{
		const double Angle = FMath::DegreesToRadians(i * ConceptGoldenDeg);
		const double Radius = 2600.0 + 62.0 * FMath::Sqrt((double)i) * 12.0;
		const FVector2D P(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius);
		if (Blocked(P)) { continue; }
		float Z = 0.f;
		if (!Ground(P.X, P.Y, Z)) { continue; }

		const float H = 620.f * FMath::Pow(ConceptPhi, (float)(i % 5) * 0.18f);
		const FRotator Spin(0.f, (float)(i * ConceptGoldenDeg), 0.f);
		Trunks->AddInstance(FTransform(Spin, FVector(P.X, P.Y, Z + H * 0.35f), FVector(0.28f, 0.28f, H * 0.7f / 100.f)));
		Canopies->AddInstance(FTransform(Spin, FVector(P.X, P.Y, Z + H * 0.9f), FVector(H * 0.55f / 100.f, H * 0.55f / 100.f, H * 0.8f / 100.f)));
		++Trees;
	}

	// RUINS: fallen blocks on the same spiral, rotated on the golden angle, some
	// standing, some toppled - the difference is the roll, not the model.
	for (int32 i = 0; i < 200; ++i)
	{
		const double Angle = FMath::DegreesToRadians(i * ConceptGoldenDeg + 41.0);
		const double Radius = 3200.0 + 70.0 * FMath::Sqrt((double)i) * 12.0;
		const FVector2D P(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius);
		if (Blocked(P)) { continue; }
		float Z = 0.f;
		if (!Ground(P.X, P.Y, Z)) { continue; }

		const bool bToppled = (i % 3) == 0;
		const float S = 220.f * FMath::Pow(ConceptPhi, (float)(i % 4) * 0.22f);
		const FRotator Rot(bToppled ? 78.f : 0.f, (float)(i * ConceptGoldenDeg), bToppled ? 12.f : 0.f);
		Ruins->AddInstance(FTransform(Rot, FVector(P.X, P.Y, Z + (bToppled ? S * 0.3f : S * 0.5f)),
			FVector(S / 100.f, S / 100.f, S / 100.f)));
		++Broken;
	}

	// BOULDERS and SHARDS: the ground clutter that makes distance read as ground.
	for (int32 i = 0; i < 240; ++i)
	{
		const double Angle = FMath::DegreesToRadians(i * ConceptGoldenDeg + 90.0);
		const double Radius = 2800.0 + 64.0 * FMath::Sqrt((double)i) * 12.0;
		const FVector2D P(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius);
		if (Blocked(P)) { continue; }
		float Z = 0.f;
		if (!Ground(P.X, P.Y, Z)) { continue; }

		const float S = 180.f * FMath::Pow(ConceptPhi, (float)(i % 3) * 0.3f);
		if ((i % 2) == 0)
		{
			Boulders->AddInstance(FTransform(FRotator(0.f, (float)(i * 23.f), 0.f),
				FVector(P.X, P.Y, Z + S * 0.22f), FVector(S / 100.f, S / 100.f * 0.8f, S / 100.f * 0.55f)));
			++Stones;
		}
		else
		{
			Shards->AddInstance(FTransform(FRotator(0.f, (float)(i * 31.f), 0.f),
				FVector(P.X, P.Y, Z + S * 0.6f), FVector(S / 100.f * 0.5f, S / 100.f * 0.5f, S / 100.f * 1.4f)));
			++Spikes;
		}
	}

	// WAYPOSTS: gold markers along the outer ring, a few of them, so the country
	// has a rhythm the eye can follow back to the plaza.
	if (Posts)
	{
		for (int32 i = 0; i < 12; ++i)
		{
			const double Angle = FMath::DegreesToRadians(i * 30.0);
			const double Radius = 2400.0 * FMath::Pow(ConceptPhi, 1.0 + (i % 3) * 0.5);
			const FVector2D P(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius);
			float Z = 0.f;
			if (!Ground(P.X, P.Y, Z)) { continue; }
			Posts->AddInstance(FTransform(FRotator::ZeroRotator, FVector(P.X, P.Y, Z + 900.f),
				FVector(0.35f, 0.35f, 18.f)));
		}
	}

	UE_LOG(LogVEIL, Warning,
		TEXT("PHI WILDS: %d groves, %d ruins, %d boulders, %d shards, 12 wayposts - instanced, culled at 22-32 km."),
		Trees, Broken, Stones, Spikes);
}
