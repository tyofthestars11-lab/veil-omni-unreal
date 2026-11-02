# omni_patch3.py - exact-match edits: Spire / Omni / Deep chains and their proof.
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


def cut(note, start_marker, end_marker, new_text):
    """Replace from start_marker up to (not including) end_marker."""
    global src
    i = src.find(start_marker)
    j = src.find(end_marker, i + 1) if i >= 0 else -1
    if i < 0 or j < 0:
        results.append(("MISS", note))
        return
    src = src[:i] + new_text + src[j:]
    results.append(("OK", note))


rep("build chains at boot",
    "\tBuildNestRegion();\n",
    "\tBuildNestRegion();\n"
    "\tEnsureChainArrays();\n"
    "\tBuildRegionChain(2);\n"
    "\tBuildRegionChain(3);\n"
    "\tBuildRegionChain(4);\n")

rep("chains flag",
    "\tconst bool bRegionProof = FParse::Param(FCommandLine::Get(), TEXT(\"phiomniregions\"));\n",
    "\tconst bool bRegionProof = FParse::Param(FCommandLine::Get(), TEXT(\"phiomniregions\"));\n"
    "\tconst bool bChainsProof = FParse::Param(FCommandLine::Get(), TEXT(\"phiomnichains\"));\n")

rep("arm chains proof",
    "\telse if (bNestProof)\n"
    "\t{\n"
    "\t\tProofStage = 200;\n"
    "\t\tStageClock = 0.f;\n"
    "\t\tNestProofIndex = 0;\n"
    "\t\tUE_LOG(LogVEIL, Log, TEXT(\"PHI OMNI NEST PROOF: armed - %d fragments, %d machines, one chain.\"),\n"
    "\t\t\tNestFragmentsTotal, NestMachinesTotal);\n"
    "\t}\n",
    "\telse if (bNestProof)\n"
    "\t{\n"
    "\t\tProofStage = 200;\n"
    "\t\tStageClock = 0.f;\n"
    "\t\tNestProofIndex = 0;\n"
    "\t\tUE_LOG(LogVEIL, Log, TEXT(\"PHI OMNI NEST PROOF: armed - %d fragments, %d machines, one chain.\"),\n"
    "\t\t\tNestFragmentsTotal, NestMachinesTotal);\n"
    "\t}\n"
    "\telse if (bChainsProof)\n"
    "\t{\n"
    "\t\tProofStage = 400;\n"
    "\t\tStageClock = 0.f;\n"
    "\t\tChainProofRegion = 2;\n"
    "\t\tChainProofFragment = 0;\n"
    "\t\tUE_LOG(LogVEIL, Log, TEXT(\"PHI OMNI CHAINS PROOF: armed - Spire, Omni District and Deep in one session.\"));\n"
    "\t}\n")

rep("tick chains",
    "\tif (bNestBuilt)\n\t{\n\t\tTickNest(DeltaSeconds);\n\t}\n",
    "\tif (bNestBuilt)\n\t{\n\t\tTickNest(DeltaSeconds);\n\t}\n"
    "\tif (ChainBuilt.Num() == 5)\n\t{\n\t\tTickChains(DeltaSeconds);\n\t}\n")

rep("save chain state",
    "\tSave->VisitedMask = VisitedMask;\n",
    "\tSave->VisitedMask = VisitedMask;\n"
    "\tSave->ChainStages = ChainStages;\n"
    "\tSave->ChainFragments = ChainFragments;\n")

rep("load chain state",
    "\t\tVisitedMask = Omni->VisitedMask;\n",
    "\t\tVisitedMask = Omni->VisitedMask;\n"
    "\t\tif (Omni->ChainStages.Num() == 5) { ChainStages = Omni->ChainStages; }\n"
    "\t\tif (Omni->ChainFragments.Num() == 5) { ChainFragments = Omni->ChainFragments; }\n")

# --- generic shard claim replaces the Nest-only block ---
cut("generic shard claim",
    "\t// 1b. THE NEST'S OWN SHARDS",
    "\t// 2. The nearest thing that answers the interact interface.",
    "\t// 1b. A REGION SHARD, CLAIMED BY IDENTITY. The generic nearest-actor search\n"
    "\t// picked whichever interactable stood closest, so a beacon or a forge inside\n"
    "\t// the reach could take the claim and the shard stayed standing. Every chain's\n"
    "\t// shard is now claimed as a shard of that region, and counted when it is.\n"
    "\t{\n"
    "\t\tstatic const TCHAR* ShardRegionIds[5] = { TEXT(\"Hub\"), TEXT(\"Nest\"), TEXT(\"Spire\"), TEXT(\"Omni\"), TEXT(\"Deep\") };\n"
    "\t\tAVEILFragment* NearestShard = nullptr;\n"
    "\t\tint32 ShardRegion = INDEX_NONE;\n"
    "\t\tfloat ShardDist = VEILOmniAuthority::SpeakerReach;\n"
    "\t\tfor (TActorIterator<AVEILFragment> It(World); It; ++It)\n"
    "\t\t{\n"
    "\t\t\tAVEILFragment* Shard = *It;\n"
    "\t\t\tif (!Shard) { continue; }\n"
    "\t\t\tint32 ShardIndex = INDEX_NONE;\n"
    "\t\t\tfor (int32 R = 1; R < 5; ++R)\n"
    "\t\t\t{\n"
    "\t\t\t\tif (Shard->RegionId == FName(ShardRegionIds[R])) { ShardIndex = R; break; }\n"
    "\t\t\t}\n"
    "\t\t\tif (ShardIndex == INDEX_NONE) { continue; }\n"
    "\t\t\tconst float D = FVector::Dist(At, Shard->GetActorLocation());\n"
    "\t\t\tif (D < ShardDist) { ShardDist = D; NearestShard = Shard; ShardRegion = ShardIndex; }\n"
    "\t\t}\n"
    "\t\tif (NearestShard)\n"
    "\t\t{\n"
    "\t\t\tIVEILVaultInteractable::Execute_Interact(NearestShard, Runner);\n"
    "\t\t\tif (ShardRegion == 1)\n"
    "\t\t\t{\n"
    "\t\t\t\tNestFragments = FMath::Min(NestFragmentsTotal, NestFragments + 1);\n"
    "\t\t\t\tUE_LOG(LogVEIL, Warning, TEXT(\"PHI OMNI INTERACT: Nest shard claimed at %.0f cm - %d of %d.\"),\n"
    "\t\t\t\t\tShardDist, NestFragments, NestFragmentsTotal);\n"
    "\t\t\t}\n"
    "\t\t\telse if (ChainFragments.Num() == 5)\n"
    "\t\t\t{\n"
    "\t\t\t\tChainFragments[ShardRegion] = FMath::Min(3, ChainFragments[ShardRegion] + 1);\n"
    "\t\t\t\tUE_LOG(LogVEIL, Warning, TEXT(\"PHI OMNI INTERACT: %s shard claimed at %.0f cm - %d of 3.\"),\n"
    "\t\t\t\t\tShardRegionIds[ShardRegion], ShardDist, ChainFragments[ShardRegion]);\n"
    "\t\t\t}\n"
    "\t\t}\n"
    "\t}\n\n")

CHAINFUNCS = r'''
// ---------------------------------------------------------------------------
// SPIRE / OMNI DISTRICT / DEEP: one chain per region, one set of rules
// ---------------------------------------------------------------------------
void UVEILOmniAll::EnsureChainArrays()
{
	if (ChainBuilt.Num() == 5) { return; }
	ChainStages.SetNum(5);
	ChainFragments.SetNum(5);
	ChainMachinesDown.SetNum(5);
	ChainMachineTotal.SetNum(5);
	ChainBuilt.SetNum(5);
	ChainFragmentActors.SetNum(5);
	ChainMachineActors.SetNum(5);
	ChainFragmentSpots.SetNum(5);
}

void UVEILOmniAll::BuildRegionChain(int32 RegionIndex)
{
	EnsureChainArrays();
	if (RegionIndex < 2 || RegionIndex > 4 || ChainBuilt[RegionIndex]) { return; }
	ChainBuilt[RegionIndex] = true;

	static const float CentreX[5] = { VEILOmniAuthority::HubX, VEILOmniAuthority::NestX,
		VEILOmniAuthority::SpireX, VEILOmniAuthority::OmniX, VEILOmniAuthority::DeepX };
	static const float CentreY[5] = { VEILOmniAuthority::HubY, VEILOmniAuthority::NestY,
		VEILOmniAuthority::SpireY, VEILOmniAuthority::OmniY, VEILOmniAuthority::DeepY };
	static const TCHAR* RegionIds[5] = { TEXT("Hub"), TEXT("Nest"), TEXT("Spire"), TEXT("Omni"), TEXT("Deep") };
	static const int32 MachineCounts[5] = { 0, 4, 4, 4, 3 };
	static const TCHAR* ShardLines[5] = {
		TEXT("The market keeps no secrets."),
		TEXT("The Nest keeps what the water drops."),
		TEXT("The Spire records every height a signal ever reached."),
		TEXT("The district sells the signal back to itself."),
		TEXT("The Deep remembers the machine that made the world.") };

	UWorld* World = GetWorld();
	if (!World) { return; }

	const FVector Centre(CentreX[RegionIndex], CentreY[RegionIndex], 0.f);
	const float GoldenDeg = 137.507764f;

	// Street level, not rooftops: a trace from 800 cm above the walking line finds
	// the ground the runner is on, never a dome or a Vault roof above it.
	auto GroundAt = [World](const FVector& At) -> FVector
	{
		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(VEILOmniChainGround), false);
		if (World->LineTraceSingleByChannel(Hit, At + FVector(0.f, 0.f, 800.f),
			At - FVector(0.f, 0.f, 2200.f), ECC_Visibility, Params))
		{
			return Hit.ImpactPoint;
		}
		return At;
	};

	for (int32 i = 0; i < 3; ++i)
	{
		const float Ang = FMath::DegreesToRadians(28.f + i * GoldenDeg);
		const float Rad = 900.f + i * 420.f;
		const FVector At = GroundAt(FVector(Centre.X + FMath::Cos(Ang) * Rad,
			Centre.Y + FMath::Sin(Ang) * Rad, 320.f)) + FVector(0.f, 0.f, 140.f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AVEILFragment* Shard = World->SpawnActor<AVEILFragment>(
			AVEILFragment::StaticClass(), At, FRotator::ZeroRotator, Params);
		if (!Shard) { continue; }

		Shard->RegionId = FName(RegionIds[RegionIndex]);
		Shard->LoreLine = FText::FromString(ShardLines[RegionIndex]);
		if (UStaticMeshComponent* Mesh = Shard->FindComponentByClass<UStaticMeshComponent>())
		{
			UStaticMesh* ShardMesh = LoadObject<UStaticMesh>(nullptr,
				TEXT("/Game/ParagonGreystone/Characters/Heroes/Greystone/Skins/Default/Meshes/SM_Greystone_Statue_Chunks_shard1.SM_Greystone_Statue_Chunks_shard1"));
			if (!ShardMesh)
			{
				ShardMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
			}
			Mesh->SetStaticMesh(ShardMesh);
			Mesh->SetWorldScale3D(FVector(1.4f));
			if (UMaterialInterface* GoldMat = LoadObject<UMaterialInterface>(nullptr, GoldMaterialPath))
			{
				Mesh->SetMaterial(0, GoldMat);
			}
		}
		ChainFragmentActors[RegionIndex].Add(Shard);
		ChainFragmentSpots[RegionIndex].Add(At);
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI CHAIN %s: shard %d at %s"),
			RegionIds[RegionIndex], i + 1, *At.ToCompactString());
	}

	ChainMachineTotal[RegionIndex] = MachineCounts[RegionIndex];
	for (int32 i = 0; i < MachineCounts[RegionIndex]; ++i)
	{
		const float Ang = FMath::DegreesToRadians(210.f + i * GoldenDeg);
		const float Rad = 640.f + i * 300.f;
		const FVector At = GroundAt(FVector(Centre.X + FMath::Cos(Ang) * Rad,
			Centre.Y + FMath::Sin(Ang) * Rad, 320.f)) + FVector(0.f, 0.f, 110.f);

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		AVEILEnemy* Machine = World->SpawnActor<AVEILEnemy>(
			AVEILEnemy::StaticClass(), At, FRotator::ZeroRotator, Params);
		if (!Machine) { continue; }

		Machine->ConfigureForKind((i % 2 == 0) ? EVEILEnemyKind::Soldier : EVEILEnemyKind::Drone);
		Machine->bChampion = (i == MachineCounts[RegionIndex] - 1);
		Machine->ChampTier = 2 + (RegionIndex - 2);
		ChainMachineActors[RegionIndex].Add(Machine);
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI CHAIN %s: machine %d at %s%s"),
			RegionIds[RegionIndex], i + 1, *At.ToCompactString(),
			Machine->bChampion ? TEXT(" - champion") : TEXT(""));
	}

	UE_LOG(LogVEIL, Warning, TEXT("PHI OMNI CHAIN %s ONLINE: 3 shards, %d machines, centre %.0f,%.0f"),
		RegionIds[RegionIndex], MachineCounts[RegionIndex], Centre.X, Centre.Y);
}

void UVEILOmniAll::AdvanceChain(int32 RegionIndex, int32 NewStage)
{
	if (!ChainStages.IsValidIndex(RegionIndex) || NewStage <= ChainStages[RegionIndex]) { return; }
	ChainStages[RegionIndex] = NewStage;

	static const TCHAR* RegionIds[5] = { TEXT("Hub"), TEXT("Nest"), TEXT("Spire"), TEXT("Omni"), TEXT("Deep") };
	static const int32 RewardXP[5] = { 0, 150, 180, 220, 300 };
	static const int32 RewardGold[5] = { 0, 200, 250, 300, 500 };
	static const TCHAR* LootIds[5] = { TEXT("Hub_Token"), TEXT("Nest_Sigil"), TEXT("Spire_Key"), TEXT("Omni_Cred"), TEXT("Deep_Core") };

	UE_LOG(LogVEIL, Warning, TEXT("PHI OMNI CHAIN %s: stage %d - %s"), RegionIds[RegionIndex], NewStage,
		NewStage == 1 ? TEXT("arrived") :
		NewStage == 2 ? TEXT("all 3 shards recovered") :
		NewStage == 3 ? TEXT("clear and paid") : TEXT("closed"));

	if (NewStage >= 3 && GetWorld())
	{
		if (UVEILProgression* P = GetWorld()->GetSubsystem<UVEILProgression>())
		{
			P->AwardXP(RewardXP[RegionIndex], FString::Printf(TEXT("%s cleared"), RegionIds[RegionIndex]));
			P->AddLoot(FName(LootIds[RegionIndex]), RewardGold[RegionIndex]);
		}
		SaveGame();
	}
}

void UVEILOmniAll::TickChains(float DeltaSeconds)
{
	(void)DeltaSeconds;
	UWorld* World = GetWorld();
	if (!World) { return; }

	for (int32 R = 2; R <= 4; ++R)
	{
		if (!ChainBuilt.IsValidIndex(R) || !ChainBuilt[R]) { continue; }

		int32 ShardsAlive = 0;
		for (const TWeakObjectPtr<AActor>& Weak : ChainFragmentActors[R])
		{
			if (Weak.IsValid()) { ++ShardsAlive; }
		}
		ChainFragments[R] = FMath::Max(ChainFragments[R], 3 - ShardsAlive);

		int32 MachinesAlive = 0;
		for (const TWeakObjectPtr<AVEILEnemy>& Weak : ChainMachineActors[R])
		{
			const AVEILEnemy* Machine = Weak.Get();
			if (Machine && Machine->Health > 0.f) { ++MachinesAlive; }
		}
		ChainMachinesDown[R] = FMath::Max(0, ChainMachineTotal[R] - MachinesAlive);

		if (ChainStages[R] == 0 && ChainFragments[R] > 0) { AdvanceChain(R, 1); }
		if (ChainStages[R] == 1 && ChainFragments[R] >= 3) { AdvanceChain(R, 2); }
		if (ChainStages[R] == 2 && MachinesAlive == 0) { AdvanceChain(R, 3); }
	}
}

'''

rep("insert chain functions",
    "void UVEILOmniAll::NestAdvance(int32 NewStage)",
    CHAINFUNCS + "void UVEILOmniAll::NestAdvance(int32 NewStage)")

CHAINS_PROOF = r'''	// ---- CHAINS PROOF: Spire, Omni District and Deep, one after another. ----
	if (ProofStage >= 400 && ProofStage <= 406)
	{
		static const TCHAR* RegionIds[5] = { TEXT("Hub"), TEXT("Nest"), TEXT("Spire"), TEXT("Omni"), TEXT("Deep") };

		if (ProofStage == 400)
		{
			if (ChainProofRegion > 4)
			{
				Next(405);
				return;
			}
			if (ChainProofRegion == 4) { TravelToRegion(4); }
			else { TravelToRegion(ChainProofRegion); }
			ProofStageLog(FString::Printf(TEXT("PHI OMNI CHAINS PROOF: in %s, chain stage %d, 3 shards to recover."),
				RegionIds[ChainProofRegion], ChainStages[ChainProofRegion]));
			ChainProofFragment = 0;
			Next(401);
			return;
		}
		if (ProofStage == 401)
		{
			const TArray<FVector>& Spots = ChainFragmentSpots[ChainProofRegion];
			if (!Spots.IsValidIndex(ChainProofFragment)) { Next(403); return; }
			const FVector Spot = Spots[ChainProofFragment];
			MoveToPoint(Spot, DeltaSeconds);
			if (FVector::Dist2D(Runner->GetActorLocation(), Spot) < 240.f || StageClock > 25.f)
			{
				Next(402);
			}
			return;
		}
		if (ProofStage == 402)
		{
			Interact();
			++ChainProofFragment;
			Next(401);
			return;
		}
		if (ProofStage == 403)
		{
			for (const TWeakObjectPtr<AVEILEnemy>& Weak : ChainMachineActors[ChainProofRegion])
			{
				AVEILEnemy* Machine = Weak.Get();
				if (Machine && Machine->Health > 0.f)
				{
					UGameplayStatics::ApplyDamage(Machine, 5000.f, Runner->GetController(), Runner,
						UDamageType::StaticClass());
					return;
				}
			}
			Next(404);
			return;
		}
		if (ProofStage == 404)
		{
			ProofStageLog(FString::Printf(
				TEXT("PHI OMNI CHAINS PROOF: %s - stage %d, shards %d/3, machines down %d/%d."),
				RegionIds[ChainProofRegion], ChainStages[ChainProofRegion], ChainFragments[ChainProofRegion],
				ChainMachinesDown[ChainProofRegion], ChainMachineTotal[ChainProofRegion]));
			++ChainProofRegion;
			Next(400);
			return;
		}
		if (ProofStage == 405)
		{
			const bool bSaved = SaveGame();
			ProofStageLog(FString::Printf(
				TEXT("PHI OMNI CHAINS PROOF: Spire %d, Omni %d, Deep %d - SAVE RESULT %s."),
				ChainStages[2], ChainStages[3], ChainStages[4], bSaved ? TEXT("True") : TEXT("False")));
			Next(406);
			return;
		}
		if (ProofStage == 406)
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

rep("insert chains proof",
    "\tif (!Runner) { return; }\n\n\tif (ProofStage == 1)",
    CHAINS_PROOF + "\tif (!Runner) { return; }\n\n\tif (ProofStage == 1)")

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
