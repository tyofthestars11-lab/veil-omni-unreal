// VEIL OMNI - THE LAST SIGNAL - PATH OF LIGHT: one whole thing.
// Bodies for the seven verbs, the authority dressing and the two proof runs.
#include "Systems/VEILOmniAll.h"

#include "Character/VEILCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Systems/OMNIQuestSystem.h"
#include "Systems/VEILInteractable.h"
#include "Systems/VEILQuest.h"
#include "Systems/VEILRegionSubsystem.h"
#include "Systems/VEILLoopActors.h"
#include "Systems/VEILEnemy.h"
#include "Systems/VEILVault.h"
#include "World/VEILWorldBuilder.h"
#include "VEIL.h"

namespace
{
	constexpr double OmniPhi = 1.618033988749895;

	/** The one gold the district is dressed in. Authored for this project, so it
	 *  actually carries the colour; a missing palette must never stop the world. */
	const TCHAR* GoldMaterialPath = TEXT("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold");
	const TCHAR* BlueMaterialPath = TEXT("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue");

	/** Fab geometry, already on disk. Sicka's ancient kit is what the district is
	 *  built from; the same meshes dress the five places here. */
	const TCHAR* ColumnPath = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big.SM_AncientColumn_Big");
	const TCHAR* CliffPath = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Cliff/SM_CliffBig.SM_CliffBig");
	const TCHAR* MonolithPath = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientMonolith.SM_AncientMonolith");
	const TCHAR* DomePath = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_HlafDome.SM_HlafDome");
	const TCHAR* ArchPath = TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientArchColumn.SM_AncientArchColumn");

	const FName OmniDressTag(TEXT("VEILOmniDress"));
	const FName OmniProofTag(TEXT("VEILOmniProof"));
}

const FString UVEILOmniAll::OmniSlotName = TEXT("VEILOMNI_Omni0");

// ---------------------------------------------------------------------------
// Boot
// ---------------------------------------------------------------------------
void UVEILOmniAll::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	bProofRun = FParse::Param(FCommandLine::Get(), TEXT("phiomni"));
	const bool bReloadProof = FParse::Param(FCommandLine::Get(), TEXT("phiomnireload"));
	const bool bNestProof = FParse::Param(FCommandLine::Get(), TEXT("phiomninest"));
	const bool bRegionProof = FParse::Param(FCommandLine::Get(), TEXT("phiomniregions"));
	const bool bChainsProof = FParse::Param(FCommandLine::Get(), TEXT("phiomnichains"));

	// THE BOOK IS OPENED BEFORE THE WORLD IS DRESSED, so the HUD's first frame
	// already shows the run that was saved, not a zero that then jumps.
	bLoadResult = LoadGame();

	DressAuthority();
	BuildNestRegion();
	EnsureChainArrays();
	BuildRegionChain(2);
	BuildRegionChain(3);
	BuildRegionChain(4);

	const EVEILRegion R = InWorld.GetSubsystem<UVEILRegionSubsystem>()
		? InWorld.GetSubsystem<UVEILRegionSubsystem>()->RegionAt(GetRunner() ? GetRunner()->GetActorLocation() : FVector::ZeroVector)
		: EVEILRegion::Hub;
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI ONLINE: region=%s beacons=%d/%d play=%.1fs deaths=%d respawns=%d saved=%d slot=%s"),
		*UEnum::GetValueAsString(R), BeaconsClaimed, BeaconsTotal, PlaySeconds, Deaths, Respawns,
		bLoadResult ? 1 : 0, *OmniSlotName);

	if (bProofRun)
	{
		ProofStage = 1;
		StageClock = 0.f;
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI PROOF: armed - walk to the Keeper at %.0f,%.0f, claim 3 beacons, fight, die, respawn, save, quit."),
			VEILOmniAuthority::HubX, VEILOmniAuthority::HubY);
	}
	else if (bReloadProof)
	{
		ProofStage = 100;
		StageClock = 0.f;
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI RELOAD PROOF: armed - reading the save back and closing in 8 s."));
	}
	else if (bRegionProof)
	{
		ProofStage = 300;
		StageClock = 0.f;
		RegionProofStep = 0;
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI REGION PROOF: armed - all five places, through the world's own arrival pads."));
	}
	else if (bNestProof)
	{
		ProofStage = 200;
		StageClock = 0.f;
		NestProofIndex = 0;
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI NEST PROOF: armed - %d fragments, %d machines, one chain."),
			NestFragmentsTotal, NestMachinesTotal);
	}
	else if (bChainsProof)
	{
		ProofStage = 400;
		StageClock = 0.f;
		ChainProofRegion = 2;
		ChainProofFragment = 0;
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI CHAINS PROOF: armed - Spire, Omni District and Deep in one session."));
	}
}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------
void UVEILOmniAll::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	PlaySeconds += DeltaSeconds;

	if (bNestBuilt)
	{
		TickNest(DeltaSeconds);
	}
	if (ChainBuilt.Num() == 5)
	{
		TickChains(DeltaSeconds);
	}

	if (ProofStage > 0)
	{
		TickProof(DeltaSeconds);
	}
}

AVEILCharacter* UVEILOmniAll::GetRunner() const
{
	return Cast<AVEILCharacter>(UGameplayStatics::GetPlayerPawn(this, 0));
}

APawn* UVEILOmniAll::GetRunnerPawn() const
{
	return GetRunner();
}

// ---------------------------------------------------------------------------
// SAVE / LOAD
// ---------------------------------------------------------------------------
bool UVEILOmniAll::SlotHasSave() const
{
	return UGameplayStatics::DoesSaveGameExist(OmniSlotName, SaveFileIndex);
}

bool UVEILOmniAll::SaveGame()
{
	UWorld* World = GetWorld();
	if (!World) { return false; }

	// A passage world is not the game. The engine boots through one on the way
	// to the district, and writing there would overwrite a real run with zeros.
	const FString MapName = World->GetMapName();
	if (MapName.Contains(TEXT("Entry")) || MapName.Contains(TEXT("Untitled")))
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI SAVE: refused in passage world %s"), *MapName);
		return false;
	}

	UVEILOmniSave* Save = Cast<UVEILOmniSave>(
		UGameplayStatics::CreateSaveGameObject(UVEILOmniSave::StaticClass()));
	if (!Save) { return false; }

	// The progression book is the single source of truth for XP, gold and loot.
	// This save copies it; it never keeps a second counter that could drift.
	if (UVEILProgression* P = World->GetSubsystem<UVEILProgression>())
	{
		Save->Level = P->Level;
		Save->XP = P->XP;
		Save->Gold = P->Gold;
		Save->Fragments = P->Fragments;
		Save->Inventory = P->Inventory;
	}
	if (const AVEILCharacter* Runner = GetRunner())
	{
		Save->PlayerLocation = Runner->GetActorLocation();
		Save->Health = Runner->Health;
		Save->Respawns = Runner->GetRespawnCount();
	}
	// The quest system owns the beacon list and the HUD draws its length, so that
	// length is what gets written - not a second counter the player never sees.
	if (const UOMNIQuestSystem* QuestBeacons = World->GetSubsystem<UOMNIQuestSystem>())
	{
		BeaconsClaimed = QuestBeacons->ClaimedBeacons.Num();
	}
	Save->BeaconsClaimed = BeaconsClaimed;
	Save->Deaths = Deaths;
	Save->PlaySeconds = PlaySeconds;
	Save->CurrentMap = FName(*MapName);
	if (const UVEILRegionSubsystem* Regions = World->GetSubsystem<UVEILRegionSubsystem>())
	{
		Save->Region = (int32)Regions->Current;
	}
	if (const UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>())
	{
		Save->QuestStage = (int32)Quest->Stage;
	}
	if (const UVEILQuest* Story = World->GetSubsystem<UVEILQuest>())
	{
		Save->bVaultOpen = Story->bWardenDefeated;
	}
	Save->NestStage = NestStage;
	Save->NestFragments = NestFragments;
	Save->NestMachinesDown = NestMachinesDown;
	Save->VisitedMask = VisitedMask;
	Save->ChainStages = ChainStages;
	Save->ChainFragments = ChainFragments;

	// Two writes, one door: the district's own slot keeps any older reader
	// working, and the omni slot carries the fields the base class cannot.
	bool bProgressionSaved = false;
	if (UVEILProgression* P = World->GetSubsystem<UVEILProgression>())
	{
		bProgressionSaved = P->SaveGame();
	}
	const bool bOmniSaved = UGameplayStatics::SaveGameToSlot(Save, OmniSlotName, SaveFileIndex);

	bSaveResult = bOmniSaved && bProgressionSaved;
	LastSaveSlot = OmniSlotName;
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI OMNI SAVE RESULT %s slot=%s map=%s level=%d gold=%d beacons=%d deaths=%d respawns=%d play=%.1fs health=%.1f"),
		bSaveResult ? TEXT("True") : TEXT("False"), *OmniSlotName, *MapName,
		Save->Level, Save->Gold, Save->BeaconsClaimed, Save->Deaths, Save->Respawns,
		Save->PlaySeconds, Save->Health);
	return bSaveResult;
}

bool UVEILOmniAll::LoadGame()
{
	UWorld* World = GetWorld();
	if (!World) { return false; }

	if (!SlotHasSave())
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI LOAD: no save in %s - fresh run, not an error."), *OmniSlotName);
		bLoadResult = false;
		return false;
	}

	// A base-class save still loads here: UVEILOmniSave derives from UVEILSaveGame,
	// and a save written as the base is read back as the base and upgraded in place.
	UVEILSaveGame* Loaded = Cast<UVEILSaveGame>(UGameplayStatics::LoadGameFromSlot(OmniSlotName, LoadFileIndex));
	if (!Loaded)
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI OMNI LOAD: slot %s exists but would not open."), *OmniSlotName);
		bLoadResult = false;
		return false;
	}

	if (UVEILProgression* P = World->GetSubsystem<UVEILProgression>())
	{
		P->Level = Loaded->Level;
		P->XP = Loaded->XP;
		P->Gold = Loaded->Gold;
		P->Fragments = Loaded->Fragments;
		P->Inventory = Loaded->Inventory;
	}

	// THE HUD DRAWS THE QUEST SYSTEM. This is the M1 defect: the save held level
	// 4 / gold 200 while the panel drew LV 1 / GOLD 0 / BEACONS 0, because the
	// load landed in the progression book and nothing told the book the panel
	// reads. One file, one live copy, one line joining them.
	if (UOMNIQuestSystem* QuestBook = World->GetSubsystem<UOMNIQuestSystem>())
	{
		QuestBook->Level = Loaded->Level;
		QuestBook->XP = Loaded->XP;
		QuestBook->Gold = Loaded->Gold;
		QuestBook->Fragments = Loaded->Fragments;
		if (const UVEILOmniSave* OmniBook = Cast<UVEILOmniSave>(Loaded))
		{
			QuestBook->Stage = (EOMNIQuestStage)OmniBook->QuestStage;
			QuestBook->RestoreBeacons(OmniBook->BeaconsClaimed);
		}
	}

	if (const UVEILOmniSave* Omni = Cast<UVEILOmniSave>(Loaded))
	{
		BeaconsClaimed = Omni->BeaconsClaimed;
		Deaths = Omni->Deaths;
		Respawns = Omni->Respawns;
		PlaySeconds = Omni->PlaySeconds;
		LastSaveSlot = OmniSlotName;
		NestStage = Omni->NestStage;
		NestFragments = Omni->NestFragments;
		NestMachinesDown = Omni->NestMachinesDown;
		VisitedMask = Omni->VisitedMask;
		if (Omni->ChainStages.Num() == 5) { ChainStages = Omni->ChainStages; }
		if (Omni->ChainFragments.Num() == 5) { ChainFragments = Omni->ChainFragments; }
		RegionsVisited = 0;
		for (int32 Bit = 0; Bit < 5; ++Bit)
		{
			if ((VisitedMask & (1 << Bit)) != 0) { ++RegionsVisited; }
		}
	}

	bLoadResult = true;
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI OMNI LOAD RESULT True loaded=1 slot=%s level=%d xp=%d gold=%d fragments=%d beacons=%d deaths=%d respawns=%d play=%.1fs seat=%s"),
		*OmniSlotName, Loaded->Level, Loaded->XP, Loaded->Gold, Loaded->Fragments,
		BeaconsClaimed, Deaths, Respawns, PlaySeconds,
		*Loaded->PlayerLocation.ToCompactString());
	return true;
}

// ---------------------------------------------------------------------------
// MOVE
// ---------------------------------------------------------------------------
void UVEILOmniAll::MoveAxis(const FVector& WorldDir, float Scale)
{
	AVEILCharacter* Runner = GetRunner();
	if (!Runner || !Runner->IsAlive() || FMath::IsNearlyZero(Scale)) { return; }
	Runner->AddMovementInput(WorldDir.GetSafeNormal2D(), FMath::Clamp(Scale, -1.f, 1.f));
}

void UVEILOmniAll::Move(float ForwardAxis, float RightAxis, float ClimbAxis, float DeltaSeconds)
{
	AVEILCharacter* Runner = GetRunner();
	if (!Runner) { return; }

	// The axes are the camera's, not the world's: forward is where the runner is
	// looking, exactly as the stick does it.
	const FRotator YawOnly(0.f, Runner->GetControlRotation().Yaw, 0.f);
	const FVector Forward = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawOnly).GetUnitAxis(EAxis::Y);

	const FVector Before = Runner->GetActorLocation();
	MoveAxis(Forward, ForwardAxis);
	MoveAxis(Right, RightAxis);

	// Flight has its own vertical axis, so looking down never flies the runner
	// down: the climb is the climb. On foot the axis is ignored, not applied.
	if (Runner->IsFlyingNow() || !FMath::IsNearlyZero(ClimbAxis))
	{
		Runner->SetClimbAxis(ClimbAxis);
	}

	DistanceWalked += FVector::Dist2D(Before, Runner->GetActorLocation());
	(void)DeltaSeconds;
}

void UVEILOmniAll::MoveToPoint(const FVector& Target, float DeltaSeconds)
{
	AVEILCharacter* Runner = GetRunner();
	if (!Runner) { return; }

	const FVector To = Target - Runner->GetActorLocation();
	if (To.SizeSquared2D() < FMath::Square(60.f)) { return; }

	// Face the errand and walk it. Movement follows the CONTROL rotation, so the
	// controller is what turns - turning the mesh alone would walk him sideways.
	if (AController* C = Runner->GetController())
	{
		C->SetControlRotation(FRotator(0.f, To.Rotation().Yaw, 0.f));
	}

	// A wall is answered the way a player answers it: jump it, then step around
	// it. Standing still until a timeout proves nothing about the game.
	const FVector Now = Runner->GetActorLocation();
	if (FVector::Dist2D(Now, StallFrom) > 60.f)
	{
		StallFrom = Now;
		StallClock = 0.f;
		DetourClock = 0.f;
	}
	else
	{
		StallClock += DeltaSeconds;
		if (StallClock > 1.2f)
		{
			Runner->JumpFromInput();
			DetourSign = -DetourSign;
			DetourClock = 1.5f;
			// Name the wall. A walk that stopped short is a fact; WHICH actor stopped
			// it is the thing that makes it fixable.
			if (UWorld* World = GetWorld())
			{
				FHitResult Hit;
				FCollisionQueryParams Params(SCENE_QUERY_STAT(VEILOmniWalkProbe), false, Runner);
				const FVector Probe = Now + FVector(0.f, 0.f, 60.f);
				if (World->LineTraceSingleByChannel(Hit, Probe, Probe + To.GetSafeNormal2D() * 400.f, ECC_Visibility, Params))
				{
					UE_LOG(LogVEIL, Warning, TEXT("PHI OMNI WALK: blocked by %s (%s) %.0f cm ahead at %s"),
						Hit.GetActor() ? *Hit.GetActor()->GetName() : TEXT("(world)"),
						Hit.GetActor() ? *Hit.GetActor()->GetClass()->GetName() : TEXT("none"),
						Hit.Distance, *Hit.ImpactPoint.ToCompactString());
				}
			}
			StallClock = 0.f;
		}
	}

	// The detour owns the body while it runs: forward plus a steady sideways
	// push walks him around the corner instead of into it.
	if (DetourClock > 0.f)
	{
		DetourClock -= DeltaSeconds;
		Move(0.75f, (float)DetourSign * 0.75f, 0.f, DeltaSeconds);
		return;
	}

	Move(1.f, 0.f, 0.f, DeltaSeconds);
}

// ---------------------------------------------------------------------------
// ATTACK / INTERACT
// ---------------------------------------------------------------------------
void UVEILOmniAll::Attack(bool bRanged)
{
	AVEILCharacter* Runner = GetRunner();
	if (!Runner || !Runner->IsAlive()) { return; }

	if (bRanged)
	{
		Runner->LanceBolt();
	}
	else
	{
		Runner->Strike();
	}
	++Attacks;
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI ATTACK: %s #%d at %s"),
		bRanged ? TEXT("lance bolt") : TEXT("swing"), Attacks,
		*Runner->GetActorLocation().ToCompactString());
}

void UVEILOmniAll::ClaimBeaconAt(int32 Index)
{
	UWorld* World = GetWorld();
	UOMNIQuestSystem* Quest = World ? World->GetSubsystem<UOMNIQuestSystem>() : nullptr;
	if (!Quest) { return; }

	Quest->ClaimBeacon(Index);
	BeaconsClaimed = Quest->ClaimedBeacons.Num();
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI BEACON: index %d -> %d of %d forged."),
		Index, BeaconsClaimed, BeaconsTotal);
}

void UVEILOmniAll::Interact()
{
	UWorld* World = GetWorld();
	AVEILCharacter* Runner = GetRunner();
	if (!World || !Runner || !Runner->IsAlive()) { return; }
	const FVector At = Runner->GetActorLocation();
	++Interactions;

	// 1. A beacon underfoot is forged by standing at it. Same reach and same
	//    index claim the player's own key uses - not a second switch.
	if (UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>())
	{
		Quest->ClaimBeaconNear(At, VEILOmniAuthority::BeaconReach);
		BeaconsClaimed = Quest->ClaimedBeacons.Num();
	}

	// 1b. A REGION SHARD, CLAIMED BY IDENTITY. The generic nearest-actor search
	// picked whichever interactable stood closest, so a beacon or a forge inside
	// the reach could take the claim and the shard stayed standing. Every chain's
	// shard is now claimed as a shard of that region, and counted when it is.
	{
		static const TCHAR* ShardRegionIds[5] = { TEXT("Hub"), TEXT("Nest"), TEXT("Spire"), TEXT("Omni"), TEXT("Deep") };
		AVEILFragment* NearestShard = nullptr;
		int32 ShardRegion = INDEX_NONE;
		float ShardDist = VEILOmniAuthority::SpeakerReach;
		for (TActorIterator<AVEILFragment> It(World); It; ++It)
		{
			AVEILFragment* Shard = *It;
			if (!Shard) { continue; }
			int32 ShardIndex = INDEX_NONE;
			for (int32 R = 1; R < 5; ++R)
			{
				if (Shard->RegionId == FName(ShardRegionIds[R])) { ShardIndex = R; break; }
			}
			if (ShardIndex == INDEX_NONE) { continue; }
			const float D = FVector::Dist(At, Shard->GetActorLocation());
			if (D < ShardDist) { ShardDist = D; NearestShard = Shard; ShardRegion = ShardIndex; }
		}
		if (NearestShard)
		{
			IVEILVaultInteractable::Execute_Interact(NearestShard, Runner);
			if (ShardRegion == 1)
			{
				NestFragments = FMath::Min(NestFragmentsTotal, NestFragments + 1);
				UE_LOG(LogVEIL, Warning, TEXT("PHI OMNI INTERACT: Nest shard claimed at %.0f cm - %d of %d."),
					ShardDist, NestFragments, NestFragmentsTotal);
			}
			else if (ChainFragments.Num() == 5)
			{
				ChainFragments[ShardRegion] = FMath::Min(3, ChainFragments[ShardRegion] + 1);
				UE_LOG(LogVEIL, Warning, TEXT("PHI OMNI INTERACT: %s shard claimed at %.0f cm - %d of 3."),
					ShardRegionIds[ShardRegion], ShardDist, ChainFragments[ShardRegion]);
			}
		}
	}

	// 2. The nearest thing that answers the interact interface.
	AActor* Nearest = nullptr;
	float NearestD = VEILOmniAuthority::SpeakerReach;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || Actor == Runner) { continue; }
		if (!Actor->GetClass()->ImplementsInterface(UVEILVaultInteractable::StaticClass())) { continue; }
		if (Actor->IsA(AVEILFragment::StaticClass())) { continue; } // claimed above, by identity
		const float D = FVector::Dist(At, Actor->GetActorLocation());
		if (D < NearestD) { NearestD = D; Nearest = Actor; }
	}
	if (Nearest)
	{
		IVEILVaultInteractable::Execute_Interact(Nearest, Runner);
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI INTERACT: %s at %.0f cm"), *Nearest->GetName(), NearestD);
	}

	// 3. A speaker in reach opens the story, through the quest systems' own TalkTo.
	const FVector Speakers[] = {
		FVector(VEILOmniAuthority::HubX, VEILOmniAuthority::HubY, 0.f),
		FVector(VEILOmniAuthority::NestX, VEILOmniAuthority::NestY, 0.f),
		FVector(-4150.f, -780.f, 0.f)
	};
	const FName SpeakerNames[] = { FName(TEXT("Keeper")), FName(TEXT("Seer")), FName(TEXT("Cartographer")) };
	for (int32 i = 0; i < UE_ARRAY_COUNT(Speakers); ++i)
	{
		if (FVector::Dist2D(At, Speakers[i]) > VEILOmniAuthority::SpeakerReach) { continue; }
		if (UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>())
		{
			Quest->TalkTo(SpeakerNames[i]);
		}
		if (UVEILQuest* Story = World->GetSubsystem<UVEILQuest>())
		{
			Story->TalkTo(SpeakerNames[i]);
		}
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI INTERACT: spoke with %s at %.0f,%.0f"),
			*SpeakerNames[i].ToString(), Speakers[i].X, Speakers[i].Y);
		break;
	}
}

// ---------------------------------------------------------------------------
// DIE / RESPAWN
// ---------------------------------------------------------------------------
void UVEILOmniAll::Die()
{
	AVEILCharacter* Runner = GetRunner();
	if (!Runner) { return; }
	if (!Runner->IsAlive())
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI OMNI DEATH: the runner is already down - no second death counted."));
		return;
	}

	// A death goes through the same gate a machine's blow goes through. Grace
	// still protects the runner, so a death that is refused says so instead of
	// being faked: the proof repeats the blow until the world lets one land.
	const float Blow = Runner->MaxHealth * (float)FMath::Pow(OmniPhi, 2.0);
	const float Applied = UGameplayStatics::ApplyDamage(Runner, Blow, Runner->GetController(),
		Runner, UDamageType::StaticClass());

	if (!Runner->IsAlive())
	{
		++Deaths;
		UE_LOG(LogVEIL, Warning,
			TEXT("PHI OMNI DEATH: the runner went down (blow %.1f applied %.1f) at %s - death %d, respawns so far %d."),
			Blow, Applied, *Runner->GetActorLocation().ToCompactString(), Deaths, Runner->GetRespawnCount());
	}
	else if (Applied <= 0.f)
	{
		UE_LOG(LogVEIL, Log,
			TEXT("PHI OMNI DEATH: the blow was held (grace %.2f of %.2f s, health %.1f) - the protected window is real."),
			Runner->GraceClock, Runner->GraceWindow, Runner->Health);
	}
}

void UVEILOmniAll::Respawn()
{
	AVEILCharacter* Runner = GetRunner();
	if (!Runner) { return; }

	Runner->RespawnNow();
	Respawns = Runner->GetRespawnCount();
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI OMNI RESPAWN: standing again at %s with %.0f health, grace %.2f s - return %d, infinite, no cap."),
		*Runner->GetActorLocation().ToCompactString(), Runner->Health,
		FMath::Max(0.f, Runner->GraceWindow - Runner->GraceClock), Respawns);
}


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
			// This path never existed in Content: it logged "Failed to find object" on every boot
			// and fell through to the kit anyway. It loads the kit directly now.
			UStaticMesh* Shard = LoadObject<UStaticMesh>(nullptr,
				TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big.SM_AncientColumn_Big"));
			if (!Shard)
			{
				Shard = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big.SM_AncientColumn_Big"));
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
				TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big.SM_AncientColumn_Big"));
			if (!ShardMesh)
			{
				ShardMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Props/SM_AncientColumn_Big.SM_AncientColumn_Big"));
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
	// The measured count and the claimed count only ever rise: a shard that was
	// claimed and is still pending kill must not drop the chain back a step.
	NestFragments = FMath::Max(NestFragments, NestFragmentsTotal - FragmentsAlive);

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

// ---------------------------------------------------------------------------
// AUTHORITY DRESSING
// ---------------------------------------------------------------------------
AStaticMeshActor* UVEILOmniAll::PlaceAt(const FVector& At, const TCHAR* MeshPath,
	const FVector& TargetSizeCm, bool bGold, float YawDegrees)
{
	UWorld* World = GetWorld();
	UStaticMesh* Mesh = LoadObject<UStaticMesh>(nullptr, MeshPath);
	if (!World || !Mesh)
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI OMNI FAB: %s missing - nothing placed at %.0f,%.0f."),
			MeshPath, At.X, At.Y);
		return nullptr;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(
		AStaticMeshActor::StaticClass(), At, FRotator(0.f, YawDegrees, 0.f), Params);
	if (!Actor) { return nullptr; }

	UStaticMeshComponent* Comp = Actor->GetStaticMeshComponent();
	if (!Comp) { return nullptr; }
	Comp->SetMobility(EComponentMobility::Movable);
	Comp->SetStaticMesh(Mesh);

	// Authored meshes are not unit cubes: the placement is normalised to the size
	// the region asks for, so the phi layout reads the same with any source mesh.
	const float HalfExtent = Mesh->GetBounds().BoxExtent.GetMax();
	const float WantHalf = TargetSizeCm.GetMax() * 0.5f;
	const float Scale = (HalfExtent > 1.f && WantHalf > 1.f) ? WantHalf / HalfExtent : 1.f;
	Actor->SetActorScale3D(FVector(FMath::Clamp(Scale, 0.01f, 200.f)));

	if (UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, bGold ? GoldMaterialPath : BlueMaterialPath))
	{
		if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, Comp))
		{
			MID->SetVectorParameterValue(TEXT("Color"), bGold ? FLinearColor(1.f, 0.77f, 0.23f) : FLinearColor(0.35f, 0.66f, 1.f));
			MID->SetVectorParameterValue(TEXT("BaseColor"), bGold ? FLinearColor(1.f, 0.77f, 0.23f) : FLinearColor(0.35f, 0.66f, 1.f));
			Comp->SetMaterial(0, MID);
		}
		else
		{
			Comp->SetMaterial(0, Base);
		}
	}

	Actor->Tags.Add(OmniDressTag);
	++DressCount;
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI FAB: %s at (%.0f, %.0f, %.0f) yaw %.0f size %.0f cm (%s palette)"),
		*Mesh->GetName(), At.X, At.Y, At.Z, YawDegrees, TargetSizeCm.GetMax(),
		bGold ? TEXT("gold") : TEXT("deep blue"));
	return Actor;
}

void UVEILOmniAll::DressAuthority()
{
	UWorld* World = GetWorld();
	if (!World || bDressPlaced) { return; }

	// Idempotent per world: a second Play pass, or a map that already carries
	// these actors, must not double the dressing.
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		if (*It && It->ActorHasTag(OmniDressTag))
		{
			bDressPlaced = true;
			return;
		}
	}

	const float Spacing = 420.f / (float)OmniPhi;   // phi of the pillar spacing, not a typed grid

	// HUB MARKET: three gold phi pillars in front of the Keeper, stepping back
	// from her platform toward the runner's seat.
	PlaceAt(FVector(VEILOmniAuthority::HubX, VEILOmniAuthority::HubY - 420.f, 60.f), ColumnPath,
		FVector(240.f, 240.f, 700.f), true, 0.f);
	PlaceAt(FVector(VEILOmniAuthority::HubX - Spacing, VEILOmniAuthority::HubY - 420.f - Spacing, 60.f), ColumnPath,
		FVector(200.f, 200.f, 560.f), true, 21.f);
	PlaceAt(FVector(VEILOmniAuthority::HubX + Spacing, VEILOmniAuthority::HubY - 420.f - Spacing, 60.f), ColumnPath,
		FVector(200.f, 200.f, 560.f), true, -21.f);

	// THE NEST: authored cliffs standing on the cliff region, left of the market.
	PlaceAt(FVector(VEILOmniAuthority::NestX, VEILOmniAuthority::NestY, 60.f), CliffPath,
		FVector(1600.f, 1600.f, 1400.f), false, 13.f);
	PlaceAt(FVector(VEILOmniAuthority::NestX - 700.f, VEILOmniAuthority::NestY + 500.f, 60.f), CliffPath,
		FVector(1200.f, 1200.f, 1100.f), false, 144.f);
	PlaceAt(FVector(VEILOmniAuthority::NestX + 700.f, VEILOmniAuthority::NestY + 400.f, 60.f), CliffPath,
		FVector(1200.f, 1200.f, 900.f), false, -144.f);

	// THE SPIRE: the standing stone in the clouds, centre of the world.
	PlaceAt(FVector(VEILOmniAuthority::SpireX, VEILOmniAuthority::SpireY, 120.f), MonolithPath,
		FVector(900.f, 900.f, 3200.f), true, 33.f);

	// OMNI DISTRICT: a real roof over the city, right of the market.
	PlaceAt(FVector(VEILOmniAuthority::OmniX, VEILOmniAuthority::OmniY, 60.f), DomePath,
		FVector(2400.f, 2400.f, 1200.f), false, 0.f);

	// MEGASTURE DEEP: the arch that guards the Vault, grounded at its anchor.
	PlaceAt(FVector(VEILOmniAuthority::DeepX, VEILOmniAuthority::DeepY - 700.f, VEILOmniAuthority::VaultZ + 400.f), ArchPath,
		FVector(1400.f, 1400.f, 1600.f), true, 180.f);

	bDressPlaced = true;
	UE_LOG(LogVEIL, Log, TEXT("PHI OMNI FAB: %d authored pieces standing at the five authority centres."), DressCount);
}

// ---------------------------------------------------------------------------
// PROOF
// ---------------------------------------------------------------------------
void UVEILOmniAll::ProofStageLog(const FString& Line) const
{
	UE_LOG(LogVEIL, Warning, TEXT("%s"), *Line);
}

void UVEILOmniAll::TickProof(float DeltaSeconds)
{
	UWorld* World = GetWorld();
	AVEILCharacter* Runner = GetRunner();
	if (!World) { return; }
	StageClock += DeltaSeconds;

	auto Next = [this](int32 Stage)
	{
		ProofStage = Stage;
		StageClock = 0.f;
	};

	// ---- RELOAD PROOF: read the save, say what survived, close. ----
	if (ProofStage == 100)
	{
		if (StageClock > 8.f)
		{
			const UVEILProgression* Book = World->GetSubsystem<UVEILProgression>();
			ProofStageLog(FString::Printf(
				TEXT("PHI OMNI RELOAD PROOF: loaded=%d beacons=%d deaths=%d respawns=%d play=%.1fs level=%d gold=%d - session survived reload."),
				bLoadResult ? 1 : 0, BeaconsClaimed, Deaths, Respawns, PlaySeconds,
				Book ? Book->Level : 0, Book ? Book->Gold : 0));
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
			{
				PC->ConsoleCommand(TEXT("quit"));
			}
		}
		return;
	}

	// ---- REGION PROOF: all five places, through the world's own arrival pads. ----
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

	// ---- CHAINS PROOF: Spire, Omni District and Deep, one after another. ----
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

	if (!Runner) { return; }

	if (ProofStage == 1)
	{
		// Let the world boot before touching it: camp, HUD and nav all come up first.
		if (StageClock < 4.f) { return; }
		WalkStart = Runner->GetActorLocation();
		const UVEILProgression* Book = World->GetSubsystem<UVEILProgression>();
		ProofStageLog(FString::Printf(
			TEXT("PHI OMNI PROOF 1/7: world live - runner at %s health %.0f of %.0f, gold %d. Walking to the Keeper at %.0f,%.0f."),
			*WalkStart.ToCompactString(), Runner->Health, Runner->MaxHealth, Book ? Book->Gold : 0,
			VEILOmniAuthority::HubX, VEILOmniAuthority::HubY));
		Next(2);
		return;
	}

	if (ProofStage == 2)
	{
		const FVector Keeper(VEILOmniAuthority::HubX, VEILOmniAuthority::HubY, Runner->GetActorLocation().Z);
		MoveToPoint(Keeper, DeltaSeconds);
		const float D = FVector::Dist2D(Runner->GetActorLocation(), Keeper);
		// Standing AT her is the bar, not "near her": 250 cm is inside arm's reach
		// of the column, so a proof that says "walked to the Keeper" means it.
		if (D < 250.f || StageClock > 30.f)
		{
			ProofStageLog(FString::Printf(
				TEXT("PHI OMNI PROOF 2/7: walked %.0f cm to the Keeper, %.0f cm from her at %s (%s)."),
				FVector::Dist2D(WalkStart, Runner->GetActorLocation()), D,
				*Runner->GetActorLocation().ToCompactString(),
				D < 250.f ? TEXT("arrived on foot") : TEXT("stopped short")));
			Next(3);
		}
		return;
	}

	if (ProofStage == 3)
	{
		Interact();
		UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();
		ProofStageLog(FString::Printf(
			TEXT("PHI OMNI PROOF 3/7: interacted at the Keeper - beacons %d/%d, level %d, gold %d."),
			Quest ? Quest->ClaimedBeacons.Num() : 0, BeaconsTotal,
			Quest ? Quest->Level : 0, Quest ? Quest->Gold : 0));
		Next(4);
		return;
	}

	if (ProofStage == 4)
	{
		// The three beacons, through the same index claim the console and the
		// player's key reach. Beacons 1 and 2 are Seer and Cartographer.
		if (StageClock > 0.5f)
		{
			ClaimBeaconAt(0);
			ClaimBeaconAt(1);
			ClaimBeaconAt(2);
			UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();
			BeaconsClaimed = Quest ? Quest->ClaimedBeacons.Num() : BeaconsClaimed;
			ProofStageLog(FString::Printf(
				TEXT("PHI OMNI PROOF 4/7: beacons claimed %d/%d - KEEPER, SEER, CARTOGRAPHER."),
				BeaconsClaimed, BeaconsTotal));
			Next(5);
		}
		return;
	}

	if (ProofStage == 5)
	{
		// FIGHT: a swing every 0.5 s for two seconds, then the lance bolt.
		if (StageClock > 0.5f && StageClock < 2.5f)
		{
			if (FMath::Fmod(StageClock, 0.5f) < DeltaSeconds)
			{
				Attack(false);
			}
		}
		if (StageClock >= 2.5f)
		{
			Attack(true);
			ProofStageLog(FString::Printf(
				TEXT("PHI OMNI PROOF 5/7: fought - %d strikes landed, health %.0f of %.0f."),
				Attacks, Runner->Health, Runner->MaxHealth));
			Next(6);
		}
		return;
	}

	if (ProofStage == 6)
	{
		// DIE, through the real gate, repeated until one lands. Grace protects the
		// runner for phi^5 at spawn; a proof that skipped it would prove nothing.
		if (Runner->IsAlive() && StageClock - LastBlowClock > 0.75f)
		{
			LastBlowClock = StageClock;
			Die();
		}
		if (!Runner->IsAlive())
		{
			ProofStageLog(FString::Printf(
				TEXT("PHI OMNI PROOF 6/7: the runner is down at %s, deaths %d - returning."),
				*Runner->GetActorLocation().ToCompactString(), Deaths));
			Next(7);
		}
		else if (StageClock > 45.f)
		{
			ProofStageLog(TEXT("PHI OMNI PROOF 6/7: the death gate held for 45 s - that is a finding, not a pass."));
			Next(7);
		}
		return;
	}

	if (ProofStage == 7)
	{
		// The world's own infinite respawn brings him back in phi seconds. The proof
		// waits for that return and only seats him if the world never does.
		if (Runner->IsAlive())
		{
			Respawns = Runner->GetRespawnCount();
			ProofStageLog(FString::Printf(
				TEXT("PHI OMNI PROOF 7/7: the world returned the runner to %s with %.0f health - return %d, no cap."),
				*Runner->GetActorLocation().ToCompactString(), Runner->Health, Respawns));
			Next(8);
			return;
		}
		if (StageClock > 4.f)
		{
			Respawn();
			ProofStageLog(FString::Printf(
				TEXT("PHI OMNI PROOF 7/7: the world had not returned him in 4 s - seated at %s, return %d."),
				*Runner->GetActorLocation().ToCompactString(), Respawns));
			Next(8);
		}
		return;
	}

	if (ProofStage == 8)
	{
		if (StageClock > 1.5f)
		{
			const bool bSaved = SaveGame();
			ProofStageLog(FString::Printf(
				TEXT("PHI OMNI PROOF: SAVE RESULT %s at slot %s - level %d gold %d beacons %d deaths %d respawns %d play %.1f s."),
				bSaved ? TEXT("True") : TEXT("False"), *OmniSlotName,
				World->GetSubsystem<UVEILProgression>() ? World->GetSubsystem<UVEILProgression>()->Level : 0,
				World->GetSubsystem<UVEILProgression>() ? World->GetSubsystem<UVEILProgression>()->Gold : 0,
				BeaconsClaimed, Deaths, Respawns, PlaySeconds));
			Next(9);
		}
		return;
	}

	if (ProofStage == 9)
	{
		if (StageClock > 2.0f)
		{
			ProofStageLog(TEXT("PHI OMNI PROOF: whole session survived - walking, beacons, fight, death, respawn, save. Closing the game."));
			ProofStage = 10;
			StageClock = 0.f;
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
			{
				PC->ConsoleCommand(TEXT("quit"));
			}
		}
	}
}
