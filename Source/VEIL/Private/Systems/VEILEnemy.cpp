#include "Systems/VEILEnemy.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Systems/PHI_Sfx.h"
#include "Systems/OMNIQuestSystem.h"
#include "Systems/PHISideQuests.h"
#include "AI/VEILFactionAI.h"
#include "Systems/PHIGear.h"
#include "Systems/PHIMito.h"
#include "Systems/PHICheats.h"
#include "Systems/PHI_NPCProcessor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "VEIL.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSingleNodeInstance.h"
#include "Components/CapsuleComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "EngineUtils.h"
#include "Animation/VEILLocomotionAnimInstance.h"

AVEILEnemy::AVEILEnemy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCapsuleComponent()->InitCapsuleSize(34.f, 88.f);
	GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
	GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MinionMesh(TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Meshes/Minion_Lane_Melee_Dawn.Minion_Lane_Melee_Dawn"));
	if (MinionMesh.Succeeded() && GetMesh())
	{
		GetMesh()->SetSkeletalMeshAsset(MinionMesh.Object);
		// Bone-driven body: keep it out of FMorphVertexBufferPool (assert :336).
		GetMesh()->bDisableMorphTarget = true;
	}
	static ConstructorHelpers::FObjectFinder<UAnimSequence> MinionIdle(TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/NonCombat_Idle.NonCombat_Idle"));
	IdleSeq = MinionIdle.Succeeded() ? MinionIdle.Object : nullptr;
	static ConstructorHelpers::FObjectFinder<UAnimSequence> MinionJog(TEXT("/Game/ParagonMinions/Characters/Minions/Down_Minions/Animations/Melee/NonCombat_JogFwd.NonCombat_JogFwd"));
	JogSeq = MinionJog.Succeeded() ? MinionJog.Object : nullptr;
	Sight = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PhiSight"));
	Brain = CreateDefaultSubobject<UPHI_NPCProcessor>(TEXT("PhiBrain"));
	if (UAISenseConfig_Sight* SightCfg = ObjectInitializer.CreateDefaultSubobject<UAISenseConfig_Sight>(this, TEXT("PhiSightCfg")))
	{
		SightCfg->SightRadius = 2200.f;
		SightCfg->LoseSightRadius = 2600.f;
		SightCfg->PeripheralVisionAngleDegrees = 90.f;
		SightCfg->DetectionByAffiliation.bDetectEnemies = true;
		SightCfg->DetectionByAffiliation.bDetectNeutrals = true;
		SightCfg->DetectionByAffiliation.bDetectFriendlies = true;
		Sight->ConfigureSense(*SightCfg);
		Sight->SetDominantSense(SightCfg->GetSenseImplementation());
		SightConfig = SightCfg; // kept so the sight radius can breathe with the rung
	}
	Sight->OnTargetPerceptionUpdated.AddDynamic(this, &AVEILEnemy::OnTargetSensed);
	// Real locomotion, not a clip swap: both clips play and blend by speed. A
	// single-node instance can only ever show one, which is what read as stiff.
	GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	GetMesh()->SetAnimInstanceClass(UVEILLocomotionAnimInstance::StaticClass());
}

void AVEILEnemy::OnTargetSensed(AActor* Seen, FAIStimulus Stimulus)
{
	if (!Seen || Seen == this) { return; }
	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (Seen != Player) { return; }
	if (Stimulus.WasSuccessfullySensed())
	{
		LastSeenTime = GetWorld()->GetTimeSeconds();
		AlertNearby();
	}
}

bool AVEILEnemy::HasClearSight(const APawn* Target) const
{
	if (!Target || !GetWorld()) { return false; }
	FCollisionQueryParams Params(SCENE_QUERY_STAT(PhiEnemySight), false, this);
	Params.AddIgnoredActor(Target);
	FHitResult Hit;
	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
		Hit, GetActorLocation() + FVector(0.f, 0.f, 70.f),
		Target->GetActorLocation() + FVector(0.f, 0.f, 60.f),
		ECC_Visibility, Params);
	return !bBlocked;
}

void AVEILEnemy::AlertNearby()
{
	UWorld* World = GetWorld();
	if (!World) { return; }
	const float Now = World->GetTimeSeconds();
	int32 Woken = 0;
	for (TActorIterator<AVEILEnemy> It(World); It; ++It)
	{
		AVEILEnemy* Other = *It;
		if (!Other || Other == this) { continue; }
		if (FVector::Dist(Other->GetActorLocation(), GetActorLocation()) > 1800.f) { continue; }
		if (Other->LastSeenTime >= Now - 1.f) { continue; }
		Other->LastSeenTime = Now; // the friend now knows where the runner is
		++Woken;
	}
	if (Woken > 0)
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI ALERT: %s calls %d machine(s) to the runner."),
			*GetActorNameOrLabel(), Woken);
	}
}

void AVEILEnemy::BeginPlay()
{
	Super::BeginPlay();
	// Hand the clips to the blended instance that the constructor installed.
	if (GetMesh())
	{
		LocoAnim = Cast<UVEILLocomotionAnimInstance>(GetMesh()->GetAnimInstance());
		if (UVEILLocomotionAnimInstance* Loco = LocoAnim.Get())
		{
			Loco->IdleSequence = IdleSeq;
			Loco->JogSequence = JogSeq;
			Loco->JogSpeed = LiveMoveSpeed;
		}
	}
	const FString L = GetActorNameOrLabel();
	HomeLoc = GetActorLocation();
	if (L.StartsWith(TEXT("Foe_S"))) { ConfigureForKind(EVEILEnemyKind::Soldier); }
	else if (L.StartsWith(TEXT("Foe_D"))) { ConfigureForKind(EVEILEnemyKind::Drone); }

	// The brain is this machine's own: seeded from its name, so the same name is
	// always the same creature, and its decision is carried as a phi numeral.
	if (Brain)
	{
		Brain->Seed(GetFName(), static_cast<int32>(Kind));
	}
}

void AVEILEnemy::ConfigureForKind(EVEILEnemyKind InKind)
{
	Kind = InKind;
	UCharacterMovementComponent* M = GetCharacterMovement();
	switch (Kind)
	{
	case EVEILEnemyKind::Drone:
		MaxHealth = 60.f; BaseDamage = 8.f; BaseSpeed = 420.f;
		M->SetMovementMode(MOVE_Flying); // drones fly straight at her
		SetActorScale3D(FVector(0.85f));
		break;
	case EVEILEnemyKind::Soldier:
		MaxHealth = 140.f; BaseDamage = 14.f; BaseSpeed = 340.f;
		break;
	case EVEILEnemyKind::WardenPrime:
		MaxHealth = 2500.f; BaseDamage = 35.f; BaseSpeed = 300.f;
		BaseReach = 300.f; BaseCooldown = 1.6f;
		SetActorScale3D(FVector(2.2f));
		break;
	}
	// The kind only sets the seed. From here the rung decides what the machine is.
	PhiUpdate(0.f);
	M->MaxWalkSpeed = LiveMoveSpeed;
	M->MaxFlySpeed = LiveMoveSpeed;
	Health = MaxHealth;
	UE_LOG(LogVEIL, Log,
		TEXT("PHI AI SEED %s kind=%d base speed=%.1f dmg=%.1f cd=%.2f reach=%.1f sight=%.1f -> rungs %.4f/%.4f/%.4f/%.4f/%.4f"),
		*GetActorNameOrLabel(), (int32)Kind, BaseSpeed, BaseDamage, BaseCooldown, BaseReach, BaseSightRadius,
		RungSpeed, RungDamage, RungCooldown, RungReach, RungSightRadius);
}

float AVEILEnemy::PhiBreath(float InBaseValue, float PhaseOffset, float AmplitudeScale) const
{
	// Rung of the seed, walked by a phi-clock wave. Amplitude is phi^-4 rungs
	// (~0.1459), so a quantity breathes about +/-15.7% around where it was given
	// to sit - never a typed swing, always the same rung space as the ledger.
	// One clock for every machine in the game - FPhiDynamic, not a second copy of the law.
	return FPhiDynamic::Rung(InBaseValue, PhiPhase, PhaseOffset, AmplitudeScale);
}

void AVEILEnemy::PhiUpdate(float DeltaSeconds)
{
	// Accumulated from dt, so the phase is continuous and never drifts.
	PhiPhase += DeltaSeconds / UPHIMito::Phi;

	// Each stat rides its own offset on the same clock, so speed, damage and
	// reach do not rise and fall together as a single frozen pattern.
	const float Off2 = UPHIMito::PhiPow(-1.f);
	const float Off3 = UPHIMito::PhiPow(-2.f);
	const float Off4 = UPHIMito::PhiPow(-3.f);

	RungSpeed       = PhiBreath(BaseSpeed,        0.f,  1.f);
	RungDamage      = PhiBreath(BaseDamage,       Off2, 0.75f);
	RungCooldown    = PhiBreath(BaseCooldown,     Off3, 0.5f);
	RungReach       = PhiBreath(BaseReach,        Off4, 0.5f);
	RungSightRadius = PhiBreath(BaseSightRadius,  Off2 + Off3, 0.75f);

	// Back to quantities: the machine's actual numbers this update.
	LiveMoveSpeed      = UPHIMito::PhiPow(RungSpeed);
	LiveAttackDamage   = UPHIMito::PhiPow(RungDamage);
	LiveAttackCooldown = FMath::Max(UPHIMito::PhiPow(RungCooldown), 0.15f);
	LiveAttackRange    = UPHIMito::PhiPow(RungReach);
	LiveSightRange     = UPHIMito::PhiPow(RungSightRadius);

	// Round-trip: quantity -> rung -> quantity. This is the check, not a claim.
	PhiRoundTripError = FMath::Abs(UPHIMito::PhiPow(UPHIMito::Rung(LiveMoveSpeed)) - LiveMoveSpeed);

	if (UCharacterMovementComponent* M = GetCharacterMovement())
	{
		M->MaxWalkSpeed = LiveMoveSpeed;
		M->MaxFlySpeed = LiveMoveSpeed;
	}

	if (SightConfig)
	{
		SightConfig->SightRadius = LiveSightRange;
		SightConfig->LoseSightRadius = LiveSightRange * UPHIMito::Phi;
	}

	// Proof in the log: rungs, the quantities they resolve to, and the round-trip.
	// GATED BY THE VALUE, NOT BY THE CLOCK. A timer still wrote a full line for
	// every one of these machines every five seconds, so a live district turned the
	// log into a wall of identical numbers. The line is only worth writing when the
	// reach has actually moved by more than 1/phi^5 (0.0902) - smaller than that is
	// the same machine standing still, and it says nothing a reader needs.
	// The gate is RELATIVE, which is the whole point: 1/phi^5 of the quantity itself.
	// An absolute 0.09 against a reach of 184 is smaller than the frame-to-frame
	// jitter, so the first version of this gate still wrote 19,670 lines in thirty
	// seconds - measured, not guessed. Scaling it makes it a real tolerance: at reach
	// 184 the line is only worth writing when the reach has moved by ~16.6.
	const float PhiGate = 0.0901699f * FMath::Max(1.f, FMath::Abs(LiveAttackRange));
	if (PhiLastLogReach < 0.f || FMath::Abs(LiveAttackRange - PhiLastLogReach) > PhiGate)
	{
		PhiLastLogReach = LiveAttackRange;
		UE_LOG(LogVEIL, Log,
			TEXT("PHI AI LIVE %s phase=%.4f rungs(spd %.4f dmg %.4f cd %.4f reach %.4f sight %.4f) -> spd=%.1f dmg=%.2f cd=%.2f reach=%.1f sight=%.1f round-trip=%.6f"),
			*GetActorNameOrLabel(), PhiPhase, RungSpeed, RungDamage, RungCooldown, RungReach, RungSightRadius,
			LiveMoveSpeed, LiveAttackDamage, LiveAttackCooldown, LiveAttackRange, LiveSightRange, PhiRoundTripError);
	}
}

void AVEILEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bDead) { return; }

	// GODMODE PAGE: a frozen machine holds everything - no sense, no step, no cut.
	if (UPHICheats::AreEnemiesFrozen(GetWorld())) { return; }

	// Nothing about this machine is frozen: every stat it uses below is re-derived
	// from its rung on the phi clock before the first decision of this frame.
	PhiUpdate(DeltaSeconds);

	APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
	if (!Player) { return; }

	// THE PHINARY READING. Who this machine is, how its clock sits and what it
	// wants are one numeral the brain carries; the distance and its own blood are
	// the only inputs. Nothing here is a boolean choice.
	if (Brain)
	{
		const float Dist = (float)FVector::Dist(GetActorLocation(), Player->GetActorLocation());
		const float Threat01 = FMath::Clamp(1.f - Dist / FMath::Max(1.f, LiveSightRange), 0.f, 1.f);
		const float Health01 = MaxHealth > 0.f ? FMath::Clamp(Health / MaxHealth, 0.f, 1.f) : 1.f;
		Brain->ChooseIntent(Threat01, Health01, Dist < LiveSightRange);
	}

	// ONE GATE, TWO REASONS TO HOLD: the window a return buys, and the camp's own
	// inner ring. A machine lets the runner go while its window is open, and it
	// never strikes while he stands inside 950 cm of the seat - the door is a
	// seat, not a kill box. Either way it holds station at home, no grudge held.
	const bool bSanctuary = UPHIMito::IsInsideSanctuary(Player->GetActorLocation());
	if (bSanctuary || (GetWorld() && GetWorld()->GetTimeSeconds() < StandDownUntil))
	{
		if (FVector::Dist(GetActorLocation(), HomeLoc) > 300.f)
		{
			if (AAIController* Home = Cast<AAIController>(GetController()))
			{
				Home->MoveToLocation(HomeLoc, 100.f);
			}
		}
		return;
	}

	const FVector To = Player->GetActorLocation() - GetActorLocation();
	const float Dist = To.Size();
	// Real sight: the perception component lives on this pawn, so a controller-owned
	// perception never fired and every machine was blind past 500 cm. Distance plus
	// line of sight is the machine's own eyes, and it works whatever owns the brain.
	const bool bInSight = Dist < LiveSightRange && HasClearSight(Player);
	if (bInSight)
	{
		LastSeenTime = GetWorld()->GetTimeSeconds();
		if (!bCalledFriends)
		{
			bCalledFriends = true;
			UE_LOG(LogVEIL, Log, TEXT("PHI ALERT: %s spots the runner at %.0f cm."),
				*GetActorNameOrLabel(), Dist);
			AlertNearby();
		}
	}
	const float SinceSeen = GetWorld()->GetTimeSeconds() - LastSeenTime;
	const bool bSeen = SinceSeen < 6.f;
	const bool bHeard = Dist < 500.f; // close range counts as heard
	if ((!bSeen && !bHeard) || Dist > LiveSightRange)
	{
		if (FVector::Dist(GetActorLocation(), HomeLoc) > 300.f)
		{
			if (AAIController* HC = Cast<AAIController>(GetController())) { HC->MoveToLocation(HomeLoc, 100.f); }
		}
		return;
	}

	if (Dist > LiveAttackRange * UPHIMito::PhiPow(-0.5f))
	{
		if (Kind == EVEILEnemyKind::Drone)
		{
			// Golden-angle weave: the approach spirals on 137.507764 deg, so two raids
			// never trace the same line into her.
			const float Sweep = FMath::Sin(PhiPhase) * UPHIMito::GoldenAngleDeg;
			const FVector Weave = To.GetSafeNormal().RotateAngleAxis(Sweep, FVector::UpVector);
			AddMovementInput(Weave, 1.f);
		}
		else
		{
			RepathTimer -= DeltaSeconds;
			if (RepathTimer <= 0.f)
			{
				// Repath cadence is the live cooldown one rung down (phi^-1 of it), not 0.4 typed.
				RepathTimer = LiveAttackCooldown * UPHIMito::PhiPow(-1.f);
				if (AAIController* C = Cast<AAIController>(GetController()))
				{
					C->MoveToActor(Player, LiveAttackRange * UPHIMito::PhiPow(-1.f)); // follows the NavMesh
				}
			}
		}
	}

	AttackTimer -= DeltaSeconds;
	// The blended instance reads velocity itself; all it needs from here is the
	// machine's LIVE speed, so its stride stays matched to a phi-breathed walk.
	if (UVEILLocomotionAnimInstance* Loco = LocoAnim.Get())
	{
		Loco->JogSpeed = LiveMoveSpeed;
	}
	if (Dist <= LiveAttackRange && AttackTimer <= 0.f)
	{
		AttackTimer = LiveAttackCooldown;
		// NAME THE TARGET AND THE RESULT. Three blows landed here in a 130 s run and
		// AVEILCharacter::TakeDamage was never entered, so the target has to say who
		// it actually is and what the engine gave back.
		const float Dealt = UGameplayStatics::ApplyDamage(Player, LiveAttackDamage, GetController(), this, nullptr);
		if (AttackLogs < 4)
		{
			++AttackLogs;
			UE_LOG(LogTemp, Display, TEXT("PHI FOE ATTACK kind=%d dmg=%.0f dist=%.0f target=%s class=%s applied=%.2f"),
				(int32)Kind, LiveAttackDamage, Dist,
				Player ? *Player->GetName() : TEXT("(null)"),
				Player ? *Player->GetClass()->GetName() : TEXT("(null)"),
				Dealt);
		}
	}
}

void AVEILEnemy::StandDown(float Seconds)
{
	if (!GetWorld() || Seconds <= 0.f) { return; }
	StandDownUntil = FMath::Max(StandDownUntil, (float)GetWorld()->GetTimeSeconds() + Seconds);
	// Forget him outright: sight memory, the friends this machine called, and
	// any lunge already in flight.
	LastSeenTime = -1000.f;
	bCalledFriends = false;
	AttackTimer = 0.f;
	if (AAIController* C = Cast<AAIController>(GetController())) { C->StopMovement(); }
	UE_LOG(LogVEIL, Log, TEXT("PHI STAND DOWN: %s lets the returned runner go for %.2fs."),
		*GetActorNameOrLabel(), Seconds);
}

float AVEILEnemy::TakeDamage(float Damage, const FDamageEvent& Event, AController* By, AActor* Causer)
{
	const float Applied = Super::TakeDamage(Damage, Event, By, Causer);
	if (bDead || Applied <= 0.f) { return Applied; }
	Health -= Applied;
	if (Health <= 0.f) { Die(); }
	return Applied;
}

void AVEILEnemy::Die()
{
	if (bDead) { return; }
	bDead = true;
	UE_LOG(LogTemp, Display, TEXT("PHI FOE DOWN kind=%d"), (int32)Kind);
	UPHI_Sfx::FoeDown(GetWorld(), GetActorLocation());
	OnEnemyDied.Broadcast(this);
	{ // PHI BIGBANG: every kill feeds the loop - shards, echoes, rep
		if (UWorld* DieW = GetWorld())
		{
			if (UOMNIQuestSystem* DieQ = DieW->GetSubsystem<UOMNIQuestSystem>()) { DieQ->AddShard(); }
			if (UPHISideQuests* DieS = DieW->GetSubsystem<UPHISideQuests>()) { DieS->AddEcho(); }
			if (UVEILFactionManager* DieF = DieW->GetSubsystem<UVEILFactionManager>())
			{
				DieF->AdjustRep(Kind == EVEILEnemyKind::Soldier ? EVEILFaction::Scavenger : EVEILFaction::Void, -1.f);
			if (bChampion)
			{
				if (APhiGear* Drop = DieW->SpawnActorDeferred<APhiGear>(APhiGear::StaticClass(), FTransform(FRotator::ZeroRotator, GetActorLocation())))
				{
					Drop->Stat = (EPHIGearStat)FMath::RandRange(0, 3);
					Drop->Tier = ChampTier;
					Drop->GearName = FName(TEXT("Champion Spoil"));
					Drop->FinishSpawning(FTransform(FRotator::ZeroRotator, GetActorLocation()));
					UE_LOG(LogVEIL, Log, TEXT("PHI EPIC: a champion falls, T%d spoil left behind."), ChampTier);
					UPHI_Sfx::Explode(DieW, GetActorLocation());
				}
			}
			}
		}
	}
	SetLifeSpan(0.2f);
}


