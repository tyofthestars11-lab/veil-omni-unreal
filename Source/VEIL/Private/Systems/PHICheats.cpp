#include "Systems/PHICheats.h"

#include "AI/VEILCompanion.h"
#include "AI/VEILFactionAI.h"
#include "Character/VEILCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Systems/OMNIQuestSystem.h"
#include "Systems/PHICompanionParty.h"
#include "Systems/VEILEnemy.h"
#include "Systems/PHIWeather.h"
#include "Systems/VEILProgression.h"
#include "World/OMNIQuestContent.h"
#include "VEIL.h"

bool UPHICheats::AreEnemiesFrozen(const UWorld* World)
{
	if (!World) { return false; }
	const UPHICheats* Cheats = World->GetSubsystem<UPHICheats>();
	return Cheats && Cheats->bFreezeEnemies;
}

AVEILCharacter* UPHICheats::Runner() const
{
	const UWorld* World = GetWorld();
	if (!World) { return nullptr; }
	return Cast<AVEILCharacter>(UGameplayStatics::GetPlayerPawn(World, 0));
}

FString UPHICheats::LabelOf(int32 Row) const
{
	switch ((ERow)Row)
	{
	case ERow::GodMode:            return TEXT("GODMODE  -  no damage, no death, no fall");
	case ERow::InfiniteHealth:     return TEXT("INFINITE HEALTH  -  refilled every frame");
	case ERow::Fly:                return TEXT("FLY  -  off the ground, anywhere");
	case ERow::Noclip:             return TEXT("NOCLIP  -  through walls");
	case ERow::DamageX2:           return TEXT("DAMAGE x2");
	case ERow::DamageX5:           return TEXT("DAMAGE x5");
	case ERow::DamageX10:          return TEXT("DAMAGE x10");
	case ERow::OneHitKill:         return TEXT("ONE-HIT KILL");
	case ERow::FreezeEnemies:      return TEXT("FREEZE THE MACHINES");
	case ERow::InfiniteResources:  return TEXT("INFINITE GOLD / SHARDS / FRAGMENTS");
	case ERow::SuperJump:          return TEXT("SUPER JUMP + SURGE SPEED");
	case ERow::TeleportKeeper:     return TEXT("TELEPORT  -  the Keeper");
	case ERow::TeleportSeer:       return TEXT("TELEPORT  -  the Seer");
	case ERow::TeleportCache:      return TEXT("TELEPORT  -  the Cache");
	case ERow::MarkSpot:           return TEXT("MARK THIS SPOT");
	case ERow::ReturnToMark:       return TEXT("RETURN TO MARK");
	case ERow::AllOff:             return TEXT("GODMODE: ALL OFF  -  a clean run");
	case ERow::KillAllNearby:      return TEXT("KILL EVERY MACHINE IN REACH");
	case ERow::MaxReputation:      return TEXT("ALL FACTIONS FRIENDLY  -  rep +60");
	case ERow::ZeroReputation:     return TEXT("ALL FACTIONS HUNT  -  rep -60");
	case ERow::TeleportCartographer: return TEXT("TELEPORT  -  the Cartographer");
	case ERow::TimeSkip:           return TEXT("SKY CLOCK  -  +1 hour");
	case ERow::WeatherShift:       return TEXT("WEATHER  -  next mood now");
	case ERow::SpawnAlly:          return TEXT("MUSTER AN ALLY");
	case ERow::SpawnEnemy:         return TEXT("SPAWN A HOSTILE");
	case ERow::LevelUp:            return TEXT("LEVEL UP  -  a full rung of XP");
	case ERow::RestoreAll:         return TEXT("RESTORE EVERYTHING  -  health to full");
	case ERow::SaveNow:            return TEXT("SAVE NOW  -  anywhere, any time");
	default:                       return TEXT("?");
	}
}

bool UPHICheats::IsOn(int32 Row) const
{
	switch ((ERow)Row)
	{
	case ERow::GodMode:           return bGodMode;
	case ERow::InfiniteHealth:    return bInfiniteHealth;
	case ERow::Fly:               return bFly;
	case ERow::Noclip:            return bNoclip;
	case ERow::DamageX2:          return DamageTier == 2;
	case ERow::DamageX5:          return DamageTier == 5;
	case ERow::DamageX10:         return DamageTier == 10;
	case ERow::OneHitKill:        return bOneHitKill;
	case ERow::FreezeEnemies:     return bFreezeEnemies;
	case ERow::InfiniteResources: return bInfiniteResources;
	case ERow::SuperJump:         return bSuperJump;
	case ERow::ReturnToMark:      return bHasMark;
	case ERow::MaxReputation:     return ReputationHand == 1;
	case ERow::ZeroReputation:    return ReputationHand == -1;
	default:                      return false;
	}
}

FString UPHICheats::ValueOf(int32 Row) const
{
	switch ((ERow)Row)
	{
	case ERow::TeleportKeeper:
	case ERow::TeleportSeer:
	case ERow::TeleportCache:
	case ERow::TeleportCartographer:
	case ERow::MarkSpot:
	case ERow::ReturnToMark:
	case ERow::AllOff:
	case ERow::KillAllNearby:
	case ERow::MaxReputation:
	case ERow::ZeroReputation:
	case ERow::TimeSkip:
	case ERow::WeatherShift:
	case ERow::SpawnAlly:
	case ERow::SpawnEnemy:
	case ERow::LevelUp:
	case ERow::RestoreAll:
	case ERow::SaveNow:
		return FString();
	default:
		return IsOn(Row) ? TEXT("ON") : TEXT("OFF");
	}
}

bool UPHICheats::AnyOn() const
{
	return bGodMode || bInfiniteHealth || bFly || bNoclip || bOneHitKill
		|| bFreezeEnemies || bInfiniteResources || bSuperJump || DamageTier != 1;
}

void UPHICheats::AllOff()
{
	bGodMode = false;
	bInfiniteHealth = false;
	bFly = false;
	bNoclip = false;
	bOneHitKill = false;
	bFreezeEnemies = false;
	bInfiniteResources = false;
	bSuperJump = false;
	DamageTier = 1;

	// Put the runner back exactly as he was, not approximately.
	if (AVEILCharacter* R = Runner())
	{
		if (BaseWeaponDamage > 0.f) { R->WeaponDamage = BaseWeaponDamage; }
		if (BaseSwingDamage > 0.f) { R->SwingDamage = BaseSwingDamage; }
		if (UCharacterMovementComponent* Move = R->GetCharacterMovement())
		{
			if (BaseWalkSpeed > 0.f) { R->WalkSpeed = BaseWalkSpeed; Move->MaxWalkSpeed = BaseWalkSpeed; }
			if (BaseJumpZ > 0.f) { Move->JumpZVelocity = BaseJumpZ; }
			Move->SetWalkableFloorAngle(44.765f);   // the game's own climb limit, back on
			if (Move->MovementMode == MOVE_Flying) { Move->SetMovementMode(MOVE_Walking); }
		}
		R->SetActorEnableCollision(true);
		R->bGodMode = false;
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: ALL OFF - clean holy grail run, nothing carried."));
}

void UPHICheats::ApplyDamageTier(AVEILCharacter* R)
{
	if (!R) { return; }
	if (BaseWeaponDamage <= 0.f) { BaseWeaponDamage = R->WeaponDamage; }
	if (BaseSwingDamage <= 0.f) { BaseSwingDamage = R->SwingDamage; }

	if (bOneHitKill)
	{
		R->WeaponDamage = 100000.f;
		R->SwingDamage = 100000.f;
		return;
	}
	R->WeaponDamage = BaseWeaponDamage * (float)DamageTier;
	R->SwingDamage = BaseSwingDamage * (float)DamageTier;
}

void UPHICheats::TeleportTo(const FString& Why, const FVector& Where)
{
	AVEILCharacter* R = Runner();
	if (!R) { return; }
	R->TeleportTo(Where, R->GetActorRotation(), false, true);
	UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: carried to %s at %s."), *Why, *Where.ToCompactString());
}

void UPHICheats::SetReputation(float Amount, const TCHAR* Why)
{
	UWorld* World = GetWorld();
	UVEILFactionManager* Factions = World ? World->GetSubsystem<UVEILFactionManager>() : nullptr;
	if (!Factions) { return; }
	// One hand, four houses: the same lever the quest system pulls, applied to
	// each faction that actually lives in this world.
	for (EVEILFaction F : { EVEILFaction::Scavenger, EVEILFaction::Guardian, EVEILFaction::Remnant, EVEILFaction::Void })
	{
		Factions->AdjustRep(F, Amount);
	}
	ReputationHand = Amount > 0.f ? 1 : -1;
	UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: %s - every faction moved %+.0f."), Why, Amount);
}

void UPHICheats::KillNearby()
{
	UWorld* World = GetWorld();
	AVEILCharacter* R = Runner();
	if (!World || !R) { return; }

	const FVector At = R->GetActorLocation();
	const float Reach = 5000.f;
	const int32 Before = Kills;

	for (TActorIterator<AVEILEnemy> It(World); It; ++It)
	{
		if (!IsValid(*It)) { continue; }
		if (FVector::Dist(At, It->GetActorLocation()) > Reach) { continue; }
		UGameplayStatics::ApplyDamage(*It, 1.0e6f, R->GetController(), R, UDamageType::StaticClass());
		++Kills;
	}
	for (TActorIterator<AVEILFactionAgent> It(World); It; ++It)
	{
		if (!IsValid(*It) || !It->IsAlive()) { continue; }
		if (FVector::Dist(At, It->GetActorLocation()) > Reach) { continue; }
		UGameplayStatics::ApplyDamage(*It, 1.0e6f, R->GetController(), R, UDamageType::StaticClass());
		++Kills;
	}
	UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: %d machines cleared inside %.0f cm (total %d)."),
		Kills - Before, Reach, Kills);
}

void UPHICheats::AdvanceWorldHour()
{
	if (UWorld* World = GetWorld())
	{
		if (UPHIWeather* Sky = World->GetSubsystem<UPHIWeather>())
		{
			Sky->AdvanceHours(1.f);
			++HoursSkipped;
			UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: sky clock +1 hour (%d skipped)."), HoursSkipped);
		}
	}
}

void UPHICheats::CycleWeather()
{
	if (UWorld* World = GetWorld())
	{
		if (UPHIWeather* Sky = World->GetSubsystem<UPHIWeather>())
		{
			const EPHIWeather Next = (EPHIWeather)(((int32)Sky->GetWeather() + 1) % 4);
			Sky->ForceWeather(Next);
			++WeatherShifts;
			UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: weather handed to %d (%d shifts)."), (int32)Next, WeatherShifts);
		}
	}
}

void UPHICheats::SummonAlly()
{
	UWorld* World = GetWorld();
	if (!World) { return; }
	if (UPHICompanionParty* Party = World->GetSubsystem<UPHICompanionParty>())
	{
		const int32 Slot = Summons % 5;
		if (AVEILCompanion* C = Party->Recruit((uint8)Slot, FName(*FString::Printf(TEXT("PHI ALLY %d"), Summons + 1))))
		{
			++Summons;
			UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: ally %s mustered (%d summoned)."), *C->GetName(), Summons);
		}
	}
}

void UPHICheats::SummonEnemy()
{
	UWorld* World = GetWorld();
	AVEILCharacter* R = Runner();
	if (!World || !R) { return; }
	if (UVEILFactionManager* Factions = World->GetSubsystem<UVEILFactionManager>())
	{
		const FVector At = R->GetActorLocation() + R->GetActorForwardVector() * 700.f + FVector(0.f, 0.f, 60.f);
		if (AVEILFactionAgent* A = Factions->SpawnReinforcement(EVEILFaction::Void, At))
		{
			++Summons;
			UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: hostile %s spawned at %s (%d summoned)."),
				*A->GetName(), *At.ToCompactString(), Summons);
		}
	}
}

void UPHICheats::GrantLevel()
{
	UWorld* World = GetWorld();
	if (!World) { return; }
	if (UVEILProgression* Prog = World->GetSubsystem<UVEILProgression>())
	{
		const int32 Need = UVEILProgression::XPNeedForLevel(Prog->Level);
		Prog->AwardXP(FMath::Max(1, Need), TEXT("GODMODE"));
		++LevelsGiven;
		UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: level rung granted (+%d XP, level %d, %d given)."),
			Need, Prog->Level, LevelsGiven);
	}
}

void UPHICheats::RestoreAll()
{
	if (AVEILCharacter* R = Runner())
	{
		R->Health = R->MaxHealth;
		UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: restore - health back to %.0f/%.0f."), R->Health, R->MaxHealth);
	}
	if (UWorld* World = GetWorld())
	{
		if (UOMNIQuestSystem* Q = World->GetSubsystem<UOMNIQuestSystem>())
		{
			Q->Gold = FMath::Max(Q->Gold, 500);
			Q->Shards = FMath::Max(Q->Shards, 50);
		}
	}
}

void UPHICheats::SaveNow()
{
	if (UWorld* World = GetWorld())
	{
		if (UVEILProgression* Prog = World->GetSubsystem<UVEILProgression>())
		{
			const bool bOk = Prog->SaveGame();
			++Saves;
			UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: save now -> %s (slot %s, %d saves)."),
				bOk ? TEXT("written") : TEXT("refused"), *UVEILProgression::SaveSlotName, Saves);
		}
	}
}

void UPHICheats::Activate(int32 Row)
{
	const FString Label = LabelOf(Row);

	switch ((ERow)Row)
	{
	case ERow::GodMode:
		bGodMode = !bGodMode;
		if (bGodMode) { bInfiniteHealth = true; }   // godmode implies staying up
		if (AVEILCharacter* R = Runner())
		{
			R->bGodMode = bGodMode;
			if (bGodMode)
			{
				R->Health = R->MaxHealth;
				if (UCharacterMovementComponent* Move = R->GetCharacterMovement())
				{
					Move->SetWalkableFloorAngle(89.f);   // no fall damage, no cliffs
				}
			}
		}
		break;

	case ERow::InfiniteHealth:
		bInfiniteHealth = !bInfiniteHealth;
		if (bInfiniteHealth)
		{
			if (AVEILCharacter* R = Runner()) { R->Health = R->MaxHealth; }
		}
		break;

	case ERow::Fly:
		bFly = !bFly;
		if (AVEILCharacter* R = Runner())
		{
			if (UCharacterMovementComponent* Move = R->GetCharacterMovement())
			{
				Move->SetMovementMode(bFly ? MOVE_Flying : MOVE_Walking);
				R->SetActorEnableCollision(!bNoclip);
			}
		}
		break;

	case ERow::Noclip:
		bNoclip = !bNoclip;
		if (AVEILCharacter* R = Runner())
		{
			R->SetActorEnableCollision(!bNoclip && !bFly);
			if (bNoclip)
			{
				if (UCharacterMovementComponent* Move = R->GetCharacterMovement())
				{
					Move->SetMovementMode(MOVE_Flying);
					bFly = true;
				}
			}
		}
		break;

	case ERow::DamageX2:  DamageTier = (DamageTier == 2) ? 1 : 2;  bOneHitKill = false; ApplyDamageTier(Runner()); break;
	case ERow::DamageX5:  DamageTier = (DamageTier == 5) ? 1 : 5;  bOneHitKill = false; ApplyDamageTier(Runner()); break;
	case ERow::DamageX10: DamageTier = (DamageTier == 10) ? 1 : 10; bOneHitKill = false; ApplyDamageTier(Runner()); break;

	case ERow::OneHitKill:
		bOneHitKill = !bOneHitKill;
		ApplyDamageTier(Runner());
		break;

	case ERow::FreezeEnemies:
		bFreezeEnemies = !bFreezeEnemies;
		break;

	case ERow::InfiniteResources:
		bInfiniteResources = !bInfiniteResources;
		break;

	case ERow::SuperJump:
		bSuperJump = !bSuperJump;
		if (AVEILCharacter* R = Runner())
		{
			if (UCharacterMovementComponent* Move = R->GetCharacterMovement())
			{
				if (BaseJumpZ <= 0.f) { BaseJumpZ = Move->JumpZVelocity; }
				if (BaseWalkSpeed <= 0.f) { BaseWalkSpeed = R->WalkSpeed; }
				Move->JumpZVelocity = bSuperJump ? BaseJumpZ * 2.618f : BaseJumpZ;
				R->WalkSpeed = bSuperJump ? BaseWalkSpeed * 2.f : BaseWalkSpeed;
				Move->MaxWalkSpeed = R->WalkSpeed;
			}
		}
		break;

	case ERow::MaxReputation:  SetReputation(60.f, TEXT("all factions friendly")); break;
	case ERow::ZeroReputation: SetReputation(-60.f, TEXT("all factions hunt")); break;

	case ERow::KillAllNearby:  KillNearby(); break;
	case ERow::TimeSkip:       AdvanceWorldHour(); break;
	case ERow::WeatherShift:   CycleWeather(); break;
	case ERow::SpawnAlly:      SummonAlly(); break;
	case ERow::SpawnEnemy:     SummonEnemy(); break;
	case ERow::LevelUp:        GrantLevel(); break;
	case ERow::RestoreAll:     RestoreAll(); break;
	case ERow::SaveNow:        SaveNow(); break;

	// ---- teleports ----
	case ERow::TeleportKeeper:
	case ERow::TeleportSeer:
	case ERow::TeleportCartographer:
	{
		UWorld* World = GetWorld();
		if (!World) { break; }
		const FString Want = (ERow)Row == ERow::TeleportKeeper ? TEXT("Keeper")
			: (ERow)Row == ERow::TeleportSeer ? TEXT("Seer") : TEXT("Cartographer");

		bool bFound = false;
		for (TActorIterator<AOMNIQuestGiver> It(World); It; ++It)
		{
			if (!IsValid(*It) || !It->GiverName.ToString().Contains(Want)) { continue; }
			TeleportTo(Want, It->GetActorLocation() + FVector(0.f, 0.f, 220.f));
			bFound = true;
			break;
		}
		if (!bFound)
		{
			UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: no %s in this world to travel to."), *Want);
		}
		break;
	}

	case ERow::TeleportCache:
		// The Cache's own seat, constant in this world like the beacons.
		TeleportTo(TEXT("the Cache"), FVector(0.f, -6200.f, 180.f) + FVector(0.f, 0.f, 220.f));
		break;

	case ERow::MarkSpot:
		if (AVEILCharacter* R = Runner())
		{
			MarkLocation = R->GetActorLocation();
			bHasMark = true;
			++Marks;
			UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: mark %d set at %s."), Marks, *MarkLocation.ToCompactString());
		}
		break;

	case ERow::ReturnToMark:
		if (AVEILCharacter* R = Runner(); R && bHasMark)
		{
			R->TeleportTo(MarkLocation, R->GetActorRotation(), false, true);
			UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: returned to mark at %s."), *MarkLocation.ToCompactString());
		}
		break;

	case ERow::AllOff:
		AllOff();
		return;

	default:
		break;
	}

	UE_LOG(LogVEIL, Log, TEXT("PHI CHEATS: %s -> %s."), *Label,
		ValueOf(Row).IsEmpty() ? TEXT("done") : *ValueOf(Row));
}

void UPHICheats::Tick(float DeltaTime)
{
	AVEILCharacter* R = Runner();
	if (!R) { return; }

	// Everything held ON is re-applied every frame, because the world is allowed
	// to fight back: a heal, a drag, a cooldown. A cheat that has to be re-typed
	// is not a cheat.
	if (bGodMode) { R->bGodMode = true; }
	if (bInfiniteHealth) { R->Health = R->MaxHealth; }

	if (bInfiniteResources)
	{
		if (UWorld* World = GetWorld())
		{
			if (UOMNIQuestSystem* Q = World->GetSubsystem<UOMNIQuestSystem>())
			{
				Q->Gold = FMath::Max(Q->Gold, 9999);
				Q->Shards = FMath::Max(Q->Shards, 999);
				Q->Fragments = FMath::Max(Q->Fragments, 10);
			}
		}
	}

	if (bFly)
	{
		// FLIGHT IS ITS OWN HAND. Walking input only pushes along the ground plane,
		// so a trainer that flips the movement mode alone leaves the runner glued to
		// the floor. Here the keys drive the body directly: forward along the view,
		// up on jump, down on crouch - the same shape as every trainer's fly.
		if (UCharacterMovementComponent* Move = R->GetCharacterMovement())
		{
			if (Move->MovementMode != MOVE_Flying) { Move->SetMovementMode(MOVE_Flying); }
			Move->MaxFlySpeed = FMath::Max(Move->MaxFlySpeed, 1400.f);

			if (APlayerController* PC = Cast<APlayerController>(R->GetController()))
			{
				const FRotator Rot = PC->GetControlRotation();
				const FVector Fwd = Rot.Vector();
				const FVector Right = FRotationMatrix(Rot).GetUnitAxis(EAxis::Y);
				if (PC->IsInputKeyDown(EKeys::W)) { R->AddMovementInput(Fwd, 1.f); }
				if (PC->IsInputKeyDown(EKeys::S)) { R->AddMovementInput(Fwd, -1.f); }
				if (PC->IsInputKeyDown(EKeys::D)) { R->AddMovementInput(Right, 1.f); }
				if (PC->IsInputKeyDown(EKeys::A)) { R->AddMovementInput(Right, -1.f); }
				if (PC->IsInputKeyDown(EKeys::SpaceBar) || PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Bottom))
				{
					R->AddMovementInput(FVector::UpVector, 1.f);
				}
				if (PC->IsInputKeyDown(EKeys::LeftControl) || PC->IsInputKeyDown(EKeys::Gamepad_FaceButton_Right))
				{
					R->AddMovementInput(FVector::UpVector, -1.f);
				}
			}
		}
	}
}

FString UPHICheats::StatusLine() const
{
	if (!AnyOn()) { return TEXT("GODMODE  //  ALL OFF  -  a clean run"); }
	FString Out = TEXT("GODMODE  //");
	if (bGodMode) { Out += TEXT(" GOD"); }
	if (bInfiniteHealth) { Out += TEXT(" HEALTH"); }
	if (bFly) { Out += TEXT(" FLY"); }
	if (bNoclip) { Out += TEXT(" NOCLIP"); }
	if (DamageTier != 1) { Out += FString::Printf(TEXT(" DMGx%d"), DamageTier); }
	if (bOneHitKill) { Out += TEXT(" ONE-HIT"); }
	if (bFreezeEnemies) { Out += TEXT(" FROZEN"); }
	if (bInfiniteResources) { Out += TEXT(" RICH"); }
	if (bSuperJump) { Out += TEXT(" JUMP"); }
	return Out;
}

TStatId UPHICheats::GetStatId() const
{
	return TStatId();
}
