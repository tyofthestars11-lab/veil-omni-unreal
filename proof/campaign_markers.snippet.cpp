void UVEILCampaignSubsystem::NoteVisited(int32 Index)
{
	if (!Activities.IsValidIndex(Index)) { return; }
	FVEILActivity& A = Activities[Index];
	if (A.bDone) { return; }
	A.bVisited = true;
	UE_LOG(LogVEIL, Log, TEXT("PHI ACTIVITY: runner reached %d - %s (%d of %d counted)."),
		Index, *A.Title, CountOf(A.Kind), A.Required);
	Scan();
}

FVector UVEILCampaignSubsystem::AnchorFor(int32 Region, int32 Slot) const
{
	// The five anchors are the world's own addresses, not invented ones: the plaza
	// the runner wakes on, the Seer's shelf, the Spire's beacon, the Cartographer's
	// survey ground, and the cache at the far edge.
	static const FVector Anchors[5] =
	{
		FVector(0.f, 0.f, 140.f),
		FVector(-450.f, 780.f, 140.f),
		FVector(2800.f, 500.f, 140.f),
		FVector(-4150.f, -780.f, 140.f),
		FVector(0.f, -6200.f, 140.f)
	};
	const int32 R = FMath::Clamp(Region, 0, 4);
	const double Angle = FMath::DegreesToRadians(Slot * 137.50776405003785);
	const double Radius = 700.0 + 210.0 * Slot;
	FVector P = Anchors[R];
	P.X += FMath::Cos(Angle) * Radius;
	P.Y += FMath::Sin(Angle) * Radius;
	return P;
}

void UVEILCampaignSubsystem::SpawnMarkers()
{
	UWorld* World = GetWorld();
	if (!World || Markers.Num() > 0) { return; }

	Markers.Reserve(Activities.Num());
	int32 Placed = 0;
	for (int32 i = 0; i < Activities.Num(); ++i)
	{
		const FVEILActivity& A = Activities[i];
		FVector At = AnchorFor(A.Region, i / 5);

		// Find the ground under the marker instead of trusting a constant height: the
		// district is not flat, and a marker inside a slab is not a place.
		FHitResult Floor;
		const FVector From(At.X, At.Y, 30000.f);
		const FVector To(At.X, At.Y, -4000.f);
		if (World->LineTraceSingleByChannel(Floor, From, To, ECC_Visibility, FCollisionQueryParams::DefaultQueryParam))
		{
			At.Z = Floor.ImpactPoint.Z + 160.f;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AVEILActivityMarker* Marker = World->SpawnActor<AVEILActivityMarker>(
			AVEILActivityMarker::StaticClass(), At, FRotator::ZeroRotator, Params);
		if (!Marker) { continue; }
		Marker->Bind(this, i, A.Title);
		Markers.Add(Marker);
		++Placed;
	}
	UE_LOG(LogVEIL, Warning, TEXT("PHI CAMPAIGN: %d activity markers placed across five regions."), Placed);
}
