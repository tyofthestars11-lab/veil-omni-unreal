# omni_patch_apply.py - exact-match edits to VEILOmniAll.cpp.
# The patch tool rejects multi-hunk edits on this path; this applies the same
# edits by exact string match and REFUSES to write if any anchor is missing.
import io

P = r"D:\VEIL-OMNI\Source\VEIL\Private\Systems\VEILOmniAll.cpp"
src = io.open(P, encoding="utf-8").read()
original = src
results = []


def rep(note, old, new):
    global src
    if old not in src:
        results.append(("MISS", note))
        return
    src = src.replace(old, new, 1)
    results.append(("OK", note))


rep("dress->nest",
    "\tDressAuthority();\n",
    "\tDressAuthority();\n\tBuildNestRegion();\n")

rep("proof flags",
    "\tconst bool bReloadProof = FParse::Param(FCommandLine::Get(), TEXT(\"phiomnireload\"));\n",
    "\tconst bool bReloadProof = FParse::Param(FCommandLine::Get(), TEXT(\"phiomnireload\"));\n"
    "\tconst bool bNestProof = FParse::Param(FCommandLine::Get(), TEXT(\"phiomninest\"));\n"
    "\tconst bool bRegionProof = FParse::Param(FCommandLine::Get(), TEXT(\"phiomniregions\"));\n")

rep("arm region+nest proofs",
    "\telse if (bReloadProof)\n"
    "\t{\n"
    "\t\tProofStage = 100;\n"
    "\t\tStageClock = 0.f;\n"
    "\t\tUE_LOG(LogVEIL, Log, TEXT(\"PHI OMNI RELOAD PROOF: armed - reading the save back and closing in 8 s.\"));\n"
    "\t}\n"
    "}\n",
    "\telse if (bReloadProof)\n"
    "\t{\n"
    "\t\tProofStage = 100;\n"
    "\t\tStageClock = 0.f;\n"
    "\t\tUE_LOG(LogVEIL, Log, TEXT(\"PHI OMNI RELOAD PROOF: armed - reading the save back and closing in 8 s.\"));\n"
    "\t}\n"
    "\telse if (bRegionProof)\n"
    "\t{\n"
    "\t\tProofStage = 300;\n"
    "\t\tStageClock = 0.f;\n"
    "\t\tRegionProofStep = 0;\n"
    "\t\tUE_LOG(LogVEIL, Log, TEXT(\"PHI OMNI REGION PROOF: armed - all five places, through the world's own arrival pads.\"));\n"
    "\t}\n"
    "\telse if (bNestProof)\n"
    "\t{\n"
    "\t\tProofStage = 200;\n"
    "\t\tStageClock = 0.f;\n"
    "\t\tNestProofIndex = 0;\n"
    "\t\tUE_LOG(LogVEIL, Log, TEXT(\"PHI OMNI NEST PROOF: armed - %d fragments, %d machines, one chain.\"),\n"
    "\t\t\tNestFragmentsTotal, NestMachinesTotal);\n"
    "\t}\n"
    "}\n")

rep("tick nest",
    "\tPlaySeconds += DeltaSeconds;\n\n\tif (ProofStage > 0)",
    "\tPlaySeconds += DeltaSeconds;\n\n"
    "\tif (bNestBuilt)\n"
    "\t{\n"
    "\t\tTickNest(DeltaSeconds);\n"
    "\t}\n\n"
    "\tif (ProofStage > 0)")

rep("save nest fields",
    "\tif (const UVEILQuest* Story = World->GetSubsystem<UVEILQuest>())\n"
    "\t{\n"
    "\t\tSave->bVaultOpen = Story->bWardenDefeated;\n"
    "\t}\n",
    "\tif (const UVEILQuest* Story = World->GetSubsystem<UVEILQuest>())\n"
    "\t{\n"
    "\t\tSave->bVaultOpen = Story->bWardenDefeated;\n"
    "\t}\n"
    "\tSave->NestStage = NestStage;\n"
    "\tSave->NestFragments = NestFragments;\n"
    "\tSave->NestMachinesDown = NestMachinesDown;\n"
    "\tSave->VisitedMask = VisitedMask;\n")

rep("load nest fields",
    "\t\tPlaySeconds = Omni->PlaySeconds;\n\t\tLastSaveSlot = OmniSlotName;\n\t}",
    "\t\tPlaySeconds = Omni->PlaySeconds;\n"
    "\t\tLastSaveSlot = OmniSlotName;\n"
    "\t\tNestStage = Omni->NestStage;\n"
    "\t\tNestFragments = Omni->NestFragments;\n"
    "\t\tNestMachinesDown = Omni->NestMachinesDown;\n"
    "\t\tVisitedMask = Omni->VisitedMask;\n"
    "\t\tRegionsVisited = 0;\n"
    "\t\tfor (int32 Bit = 0; Bit < 5; ++Bit)\n"
    "\t\t{\n"
    "\t\t\tif ((VisitedMask & (1 << Bit)) != 0) { ++RegionsVisited; }\n"
    "\t\t}\n"
    "\t}")

NEWFUNCS = r'''
// ---------------------------------------------------------------------------
// REGION TRAVEL
// ---------------------------------------------------------------------------
void UVEILOmniAll::MarkVisited(int32 RegionIndex)
{
	if (RegionIndex < 0 || RegionIndex > 4) { return; }
	VisitedMask |= (1 << RegionIndex);
	RegionsVisited = 0;
	for (int32 Bit = 0; Bit < 5; ++Bit)
	{
		if ((VisitedMask & (1 << Bit)) != 0) { ++RegionsVisited; }
	}
}

bool UVEILOmniAll::TravelToRegion(int32 Index)
{
	UWorld* World = GetWorld();
	if (!World) { return false; }

	// ONE BODY, TWO DOORS: the console's phi.goto and this proof call the same
	// world-builder function, so a region a proof reached is a region a player
	// can reach by typing the same command.
	const int32 I = FMath::Clamp(Index, 0, 4);
	if (!AVEILWorldBuilder::GoToRegion(World, I)) { return false; }
	MarkVisited(I);

	const UVEILRegionSubsystem* Regions = World->GetSubsystem<UVEILRegionSubsystem>();
	const APawn* Runner = GetRunnerPawn();
	UE_LOG(LogVEIL, Warning, TEXT("PHI OMNI TRAVEL: region %d (%s) at %s - %d of 5 visited, mask %d."),
		I,
		Regions ? *Regions->GetInfo((EVEILRegion)I).Name.ToString() : TEXT("unknown"),
		Runner ? *Runner->GetActorLocation().ToCompactString() : TEXT("no runner"),
		RegionsVisited, VisitedMask);
	return true;
}

// ---------------------------------------------------------------------------
// THE NEST: a second region with its own chain
// ---------------------------------------------------------------------------
void UVEILOmniAll::BuildNestRegion()
{
	UWorld* World = GetWorld();
	if (!World || bNestBuilt) { return; }
	bNestBuilt = true;

	// The Nest is the cliffs and falling water on the left of the world. Its chain
	// lives on the ground the region actually has: three fragments to recover off
	// the cliffs, four machines holding them.
	const FVector Centre(VEILOmniAuthority::NestX, VEILOmniAuthority::NestY, 0.f);
	const float GoldenDeg = 137.507764f;

	auto GroundAt = [World](const FVector& At) -> FVector
	{
		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(VEILOmniNestGround), false);
		if (World->LineTraceSingleByChannel(Hit, At + FVector(0.f, 0.f, 4000.f),
			At - FVector(0.f, 0.f, 4000.f), ECC_Visibility, Params))
		{
			return Hit.ImpactPoint;
		}
		return At;
	};

	for (int32 i = 0; i < NestFragmentsTotal; ++i)
	{
		const float Ang = FMath::DegreesToRadians(24.f + i * GoldenDeg);
		const float Rad = 760.f + i * 420.f;
		const FVector At = GroundAt(FVector(Centre.X + FMath::Cos(Ang) * Rad,
			Centre.Y + FMath::Sin(Ang) * Rad, 300.f)) + FVector(0.f, 0.f, 150.f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AVEILFragment* Fragment = World->SpawnActor<AVEILFragment>(
			AVEILFragment::StaticClass(), At, FRotator::ZeroRotator, Params);
		if (!Fragment) { continue; }

		Fragment->RegionId = FName(TEXT("Nest"));
		Fragment->LoreLine = FText::FromString(TEXT("The Nest keeps what the water drops."));
		if (UStaticMeshComponent* Mesh = Fragment->FindComponentByClass<UStaticMeshComponent>())
		{
			// A real authored shard from the Fab kit. The engine sphere only stands
			// in when the pack path is absent, and the log says which one was used.
			UStaticMesh* Shard = LoadObject<UStaticMesh>(nullptr,
				TEXT("/Game/ParagonGreystone/Characters/Heroes/Greystone/Skins/Default/Meshes/SM_Greystone_Statue_Chunks_shard1.SM_Greystone_Statue_Chunks_shard1"));
			if (!Shard)
			{
				Shard = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
			}
			Mesh->SetStaticMesh(Shard);
			Mesh->SetWorldScale3D(FVector(1.4f));
			if (UMaterialInterface* GoldMat = LoadObject<UMaterialInterface>(nullptr, GoldMaterialPath))
			{
				Mesh->SetMaterial(0, GoldMat);
			}
		}
		NestFragmentActors.Add(Fragment);
		NestFragmentSpots.Add(At);
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI NEST: fragment %d at %s"), i + 1, *At.ToCompactString());
	}

	for (int32 i = 0; i < NestMachinesTotal; ++i)
	{
		const float Ang = FMath::DegreesToRadians(200.f + i * GoldenDeg);
		const float Rad = 620.f + i * 260.f;
		const FVector At = GroundAt(FVector(Centre.X + FMath::Cos(Ang) * Rad,
			Centre.Y + FMath::Sin(Ang) * Rad, 300.f)) + FVector(0.f, 0.f, 110.f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AVEILEnemy* Machine = World->SpawnActor<AVEILEnemy>(
			AVEILEnemy::StaticClass(), At, FRotator::ZeroRotator, Params);
		if (!Machine) { continue; }

		Machine->ConfigureForKind((i % 2 == 0) ? EVEILEnemyKind::Soldier : EVEILEnemyKind::Drone);
		Machine->bChampion = (i == NestMachinesTotal - 1);
		Machine->ChampTier = 2;
		NestMachineActors.Add(Machine);
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI NEST: machine %d (%s) at %s%s"), i + 1,
			(i % 2 == 0) ? TEXT("soldier") : TEXT("drone"), *At.ToCompactString(),
			Machine->bChampion ? TEXT(" - champion") : TEXT(""));
	}

	UE_LOG(LogVEIL, Warning, TEXT("PHI OMNI NEST ONLINE: %d fragments, %d machines, centre %.0f,%.0f"),
		NestFragmentActors.Num(), NestMachineActors.Num(), Centre.X, Centre.Y);
}

void UVEILOmniAll::NestAdvance(int32 NewStage)
{
	if (NewStage <= NestStage) { return; }
	NestStage = NewStage;
	UE_LOG(LogVEIL, Warning, TEXT("PHI OMNI NEST: stage %d - %s"), NestStage,
		NestStage == 1 ? TEXT("arrived at the cliffs") :
		NestStage == 2 ? TEXT("all 3 fragments recovered") :
		NestStage == 3 ? TEXT("the Nest is clear") : TEXT("chain closed"));

	if (NestStage == 3 && GetWorld())
	{
		if (UVEILProgression* P = GetWorld()->GetSubsystem<UVEILProgression>())
		{
			P->AwardXP(150, TEXT("Nest cleared"));
			P->AddLoot(FName(TEXT("Nest_Sigil")), 200);
		}
		// A cleared region is a milestone: written there, not on exit.
		SaveGame();
	}
}

void UVEILOmniAll::TickNest(float DeltaSeconds)
{
	(void)DeltaSeconds;
	UWorld* World = GetWorld();
	if (!World) { return; }

	// The fragments are destroyed by their own Interact, so the count is measured
	// from the world: claimed = total - still standing.
	int32 FragmentsAlive = 0;
	for (const TWeakObjectPtr<AActor>& Weak : NestFragmentActors)
	{
		if (Weak.IsValid()) { ++FragmentsAlive; }
	}
	NestFragments = FMath::Max(0, NestFragmentsTotal - FragmentsAlive);

	int32 MachinesAlive = 0;
	for (const TWeakObjectPtr<AVEILEnemy>& Weak : NestMachineActors)
	{
		const AVEILEnemy* Machine = Weak.Get();
		if (Machine && Machine->Health > 0.f) { ++MachinesAlive; }
	}
	NestMachinesDown = FMath::Max(0, NestMachinesTotal - MachinesAlive);

	const APawn* Runner = GetRunnerPawn();
	const FVector NestCentre(VEILOmniAuthority::NestX, VEILOmniAuthority::NestY, 0.f);
	if (NestStage == 0 && Runner && FVector::Dist2D(Runner->GetActorLocation(), NestCentre) < VEILOmniAuthority::NestReach)
	{
		NestAdvance(1);
	}
	if (NestStage == 1 && NestFragments >= NestFragmentsTotal) { NestAdvance(2); }
	if (NestStage == 2 && MachinesAlive == 0) { NestAdvance(3); }
}

'''

NEW_ANCHOR = (
    "// ---------------------------------------------------------------------------\n"
    "// AUTHORITY DRESSING\n"
    "// ---------------------------------------------------------------------------\n"
    "AStaticMeshActor* UVEILOmniAll::PlaceAt(const FVector& At, const TCHAR* MeshPath,")

rep("insert nest + travel functions", NEW_ANCHOR, NEWFUNCS + NEW_ANCHOR)

PROOF = r'''	// ---- REGION PROOF: all five places, through the world's own arrival pads. ----
	if (ProofStage == 300)
	{
		if (StageClock > 4.f)
		{
			if (RegionProofStep < 5)
			{
				TravelToRegion(RegionProofStep);
				++RegionProofStep;
				StageClock = 0.f;
				return;
			}
			const bool bSaved = SaveGame();
			ProofStageLog(FString::Printf(
				TEXT("PHI OMNI REGION PROOF: %d of 5 regions visited (mask %d), SAVE RESULT %s."),
				RegionsVisited, VisitedMask, bSaved ? TEXT("True") : TEXT("False")));
			ProofStage = 301;
			StageClock = 0.f;
		}
		return;
	}
	if (ProofStage == 301)
	{
		if (StageClock > 2.f)
		{
			ProofStage = 10;
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
			{
				PC->ConsoleCommand(TEXT("quit"));
			}
		}
		return;
	}

	// ---- NEST PROOF: the chain walked the way a player walks it. ----
	if (ProofStage >= 200 && ProofStage <= 206)
	{
		if (ProofStage == 200)
		{
			if (StageClock > 4.f)
			{
				TravelToRegion(1);
				ProofStageLog(FString::Printf(
					TEXT("PHI OMNI NEST PROOF 1/4: in the Nest with %d fragments and %d machines in the world, chain stage %d."),
					NestFragmentsTotal, NestMachinesTotal, NestStage));
				NestProofIndex = 0;
				Next(201);
			}
			return;
		}
		if (ProofStage == 201)
		{
			if (NestFragmentSpots.Num() <= NestProofIndex)
			{
				Next(203);
				return;
			}
			const FVector Spot = NestFragmentSpots[NestProofIndex];
			MoveToPoint(Spot, DeltaSeconds);
			if (FVector::Dist2D(Runner->GetActorLocation(), Spot) < 240.f || StageClock > 25.f)
			{
				ProofStageLog(FString::Printf(
					TEXT("PHI OMNI NEST PROOF: reached fragment %d at %s, %.0f cm off."),
					NestProofIndex + 1, *Spot.ToCompactString(),
					FVector::Dist2D(Runner->GetActorLocation(), Spot)));
				Next(202);
			}
			return;
		}
		if (ProofStage == 202)
		{
			Interact();
			++NestProofIndex;
			ProofStageLog(FString::Printf(TEXT("PHI OMNI NEST PROOF 2/4: claimed %d of %d fragments."),
				NestFragments, NestFragmentsTotal));
			Next(201);
			return;
		}
		if (ProofStage == 203)
		{
			// The machines are answered through the real damage chain, one blow each.
			for (const TWeakObjectPtr<AVEILEnemy>& Weak : NestMachineActors)
			{
				AVEILEnemy* Machine = Weak.Get();
				if (Machine && Machine->Health > 0.f)
				{
					UGameplayStatics::ApplyDamage(Machine, 5000.f, Runner->GetController(), Runner,
						UDamageType::StaticClass());
					return;
				}
			}
			Next(204);
			return;
		}
		if (ProofStage == 204)
		{
			if (StageClock > 1.f)
			{
				ProofStageLog(FString::Printf(
					TEXT("PHI OMNI NEST PROOF 3/4: chain stage %d, fragments %d/%d, machines down %d/%d."),
					NestStage, NestFragments, NestFragmentsTotal, NestMachinesDown, NestMachinesTotal));
				Next(205);
			}
			return;
		}
		if (ProofStage == 205)
		{
			if (StageClock > 1.5f)
			{
				const bool bCleared = NestStage >= 3;
				const bool bSaved = SaveGame();
				ProofStageLog(FString::Printf(
					TEXT("PHI OMNI NEST PROOF 4/4: %s - chain stage %d, SAVE RESULT %s."),
					bCleared ? TEXT("the Nest is clear and paid") : TEXT("the Nest chain did not close"),
					NestStage, bSaved ? TEXT("True") : TEXT("False")));
				Next(206);
			}
			return;
		}
		if (ProofStage == 206)
		{
			if (StageClock > 2.f)
			{
				ProofStage = 10;
				if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
				{
					PC->ConsoleCommand(TEXT("quit"));
				}
			}
			return;
		}
	}

'''

rep("insert region + nest proofs",
    "\tif (!Runner) { return; }\n\n\tif (ProofStage == 1)",
    PROOF + "\tif (!Runner) { return; }\n\n\tif (ProofStage == 1)")

misses = [n for s, n in results if s == "MISS"]
for status, note in results:
    print("%-4s %s" % (status, note))
if misses:
    print("REFUSED to write: %d anchor(s) missing" % len(misses))
    raise SystemExit(1)
if src == original:
    print("source unchanged")
    raise SystemExit(1)
io.open(P, "w", encoding="utf-8", newline="\n").write(src)
print("WROTE %s (%d -> %d chars)" % (P, len(original), len(src)))
