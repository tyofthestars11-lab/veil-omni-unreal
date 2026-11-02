// VEIL: THE LAST SIGNAL - faction AI implementation.
#include "AI/VEILFactionAI.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"
#include "Navigation/PathFollowingComponent.h"
#include "Systems/PHILivingWorld.h"
#include "Systems/PHICheats.h"
#include "Systems/VEILTime.h"
#include "VEIL.h"
#include "Systems/OMNIQuestSystem.h"
#include "Systems/PHIMito.h"
#include "Systems/VEILProgression.h"
#include "Animation/VEILLocomotionAnimInstance.h"
#include "Systems/VEILLootPickup.h"
#include "Systems/VEILBurstFX.h"
#include "Engine/Engine.h"

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------
// A tint that is legal on ANY thread.
//
// Actor constructors run on the ASYNC LOADING thread when the actor is loaded out of
// a package, and creating a dynamic material instance plus a vector write there
// reaches the renderer from a thread that does not own it. The packaged build asserts
// exactly there (measured stack: AOMNICache ctor -> Tint -> SetVectorParameterValue ->
// FRendererModule::InvalidatePathTracedOutput). Runtime spawns are already on the game
// thread; level-loaded agents are not. So the swap is applied immediately on the game
// thread and queued for it otherwise - same colours, correct thread.
#include "Async/Async.h"

static void PHIApplyTint(UStaticMeshComponent* Comp, UMaterialInterface* Base,
	const FLinearColor& Color, float Emissive)
{
	if (!Comp || !Base) return;
	if (UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(Base, Comp))
	{
		MID->SetVectorParameterValue(TEXT("Color"), Color);
		MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
		if (Emissive > 0.f)
		{
			MID->SetVectorParameterValue(TEXT("EmissiveColor"), Color * Emissive);
		}
		Comp->SetMaterial(0, MID);
	}
}

static void PHITintComponent(UStaticMeshComponent* Comp, const FLinearColor& Color, float Emissive)
{
	if (!Comp) return;
	UMaterialInterface* Base = Comp->GetMaterial(0);
	if (!Base) return;
	if (IsInGameThread())
	{
		PHIApplyTint(Comp, Base, Color, Emissive);
		return;
	}
	// The component holds the material, so a raw pointer stays valid across the queue.
	TWeakObjectPtr<UStaticMeshComponent> Weak(Comp);
	AsyncTask(ENamedThreads::GameThread, [Weak, Base, Color, Emissive]()
	{
		UStaticMeshComponent* C = Weak.Get();
		if (C) { PHIApplyTint(C, Base, Color, Emissive); }
	});
}

// Agent
// ---------------------------------------------------------------------------
AVEILFactionAgent::AVEILFactionAgent()
{
	PrimaryActorTick.bCanEverTick = true;
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	GetCharacterMovement()->MaxWalkSpeed = 380.f;

	// Visible machine body from engine shapes - same law as the runner:
	// no mesh assets ship, and an invisible agent is not a machine.
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
			TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
			TEXT("/Engine/BasicShapes/Cube.Cube"));

		UStaticMeshComponent* Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MachineBody"));
		Body->SetupAttachment(GetCapsuleComponent());
		Body->SetRelativeLocation(FVector(0.f, 0.f, -10.f));
		Body->SetRelativeScale3D(FVector(0.72f, 0.72f, 1.55f));
		Body->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (CylinderMesh.Succeeded()) Body->SetStaticMesh(CylinderMesh.Object);
		PHITintComponent(Body, FLinearColor(0.14f, 0.10f, 0.20f), 0.f);

		UStaticMeshComponent* Eye = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MachineEye"));
		Eye->SetupAttachment(GetCapsuleComponent());
		Eye->SetRelativeLocation(FVector(22.f, 0.f, 60.f));
		Eye->SetRelativeScale3D(FVector(0.10f, 0.30f, 0.10f));
		Eye->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		if (CubeMesh.Succeeded()) Eye->SetStaticMesh(CubeMesh.Object);
		PHITintComponent(Eye, FLinearColor(0.85f, 0.2f, 1.f), 3.f);
	}

	Health = MaxHealth;
}

void AVEILFactionAgent::BeginPlay()
{
	Super::BeginPlay();
	BindStoneBody();
	Health = MaxHealth;
	HomeLocation = GetActorLocation();
	// Clips for the blended graph, from the body's own pack (Polyart locomotion).
	if (UVEILLocomotionAnimInstance* Loco = Cast<UVEILLocomotionAnimInstance>(GetMesh() ? GetMesh()->GetAnimInstance() : nullptr))
	{
		Loco->IdleSequence = LoadObject<UAnimSequence>(nullptr,
			TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/Animations/Locomotion/Male/MM_Idle.MM_Idle"));
		Loco->JogSequence = LoadObject<UAnimSequence>(nullptr,
			TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/Animations/Locomotion/Shared/MM_Run.MM_Run"));
		Loco->JogSpeed = 380.f;
	}
	// Each machine takes its seat's own yaw as its orbit phase, so six guards
	// on one ring never walk the same arc at the same time.
	OrbitOffsetDeg = GetActorRotation().Yaw;
	OrbitPhase = (float)((int32)FMath::Abs(HomeLocation.X + HomeLocation.Y) % 13) / 13.f;

	if (UVEILFactionManager* Manager = GetWorld()->GetSubsystem<UVEILFactionManager>())
	{
		Manager->RegisterAgent(this);
	}

	// Perception on a light timer keeps 20-40 agents per district cheap.
	GetWorldTimerManager().SetTimer(PerceiveTimer, this, &AVEILFactionAgent::Perceive, 0.25f, true);
}

void AVEILFactionAgent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (UVEILFactionManager* Manager = GetWorld()->GetSubsystem<UVEILFactionManager>())
	{
		Manager->UnregisterAgent(this);
	}
	Super::EndPlay(Reason);
}

void AVEILFactionAgent::SetState(EVEILAgentState NewState)
{
	if (State == NewState || State == EVEILAgentState::Dead) return;
	State = NewState;
	OnStateChanged.Broadcast(this, NewState);
}

bool AVEILFactionAgent::CanSeePlayer() const
{
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player) return false;
	const FVector ToPlayer = Player->GetActorLocation() - GetActorLocation();
	// Off post, attention is elsewhere: the machine still watches its ground, but
	// it has to be a phi closer before it makes the runner a target.
	const UPHILivingWorld* Living = GetWorld() ? GetWorld()->GetSubsystem<UPHILivingWorld>() : nullptr;
	const float EffectiveSight = (Living && Living->IsOffDuty()) ? SightRange * UPHIMito::PhiPow(-1.f) : SightRange;
	if (ToPlayer.Size() > EffectiveSight) return false;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(const_cast<AVEILFactionAgent*>(this));
	const FVector Start = GetActorLocation() + FVector(0.f, 0.f, 60.f);
	const FVector End = Player->GetActorLocation() + FVector(0.f, 0.f, 60.f);
	const bool bBlocked = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params)
		&& Hit.GetActor() != Player;
	return !bBlocked;
}

void AVEILFactionAgent::Perceive()
{
	if (!IsAlive()) return;
	APawn* Player = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (!Player) return;

	if (Health <= MaxHealth * RetreatHealthFraction && Faction != EVEILFaction::Void)
	{
		SetState(EVEILAgentState::Retreat);
		return;
	}

	if (CanSeePlayer())
	{
		// REPUTATION IS BEHAVIOUR, not a number on a menu screen. A faction that
		// trusts the runner does not shoot them, which is what makes a kind word
		// to the Watch worth more than a dialogue flag.
		const UVEILFactionManager* Manager = GetWorld() ? GetWorld()->GetSubsystem<UVEILFactionManager>() : nullptr;
		if (Manager && Manager->IsFriendlyToRunner(Faction))
		{
			TargetActor = nullptr;
			if (State != EVEILAgentState::Patrol) { SetState(EVEILAgentState::Patrol); }
			return;
		}

		TargetActor = Player;
		const float Dist = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
		SetState(Dist <= AttackRange ? EVEILAgentState::Combat : EVEILAgentState::Hunt);
	}
	else if (State == EVEILAgentState::Combat || State == EVEILAgentState::Hunt)
	{
		SetState(EVEILAgentState::Alert);
		FTimerHandle Timer;
		GetWorldTimerManager().SetTimer(Timer, [this]()
		{
			if (State == EVEILAgentState::Alert) SetState(EVEILAgentState::Patrol);
		}, 3.f, false);
	}
}

void AVEILFactionAgent::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!IsAlive()) return;

	// GODMODE PAGE: frozen means frozen - the agent keeps its post and its
	// memories, it simply stops living for as long as the switch is on.
	if (UPHICheats::AreEnemiesFrozen(GetWorld())) return;

	// POWER. Nobody hands this machine anything: it lives through a day on the
	// same clock as the world and comes back carrying more of it. This is what
	// "they are their own beings" means mechanically - the district gets harder
	// while the runner is standing still.
	if (const UVEILTime* Clock = GetWorld() ? GetWorld()->GetSubsystem<UVEILTime>() : nullptr)
	{
		if (Clock->GetDay() != AgentDay)
		{
			AgentDay = Clock->GetDay();
			const float Before = PowerRung;
			PowerRung = FMath::Min(4.f, PowerRung + 1.f / 4.2360679f);   // 1/phi^3 of a rung per day
			WeaponDamage *= UPHIMito::PhiPow(PowerRung - Before);
			MaxHealth *= UPHIMito::PhiPow((PowerRung - Before) * 0.5f);
			UE_LOG(LogVEIL, Log, TEXT("PHI POWER: %s has lived to day %d - rung %.4f, damage %.2f, health %.0f."),
				*GetActorNameOrLabel(), AgentDay, PowerRung, WeaponDamage, MaxHealth);
		}
	}

	// One lap per phi^4 = 6.8541 s: a guard's walk around a 432.6 cm ring at
	// roughly its own pace. Accumulated, so the phase can never drift.
	OrbitPhase += DeltaSeconds / UPHIMito::PhiPow(4.f);

	// STAND DOWN / SANCTUARY: the window a return buys, and the camp's own inner
	// ring. No target, no shot while either holds, and the machine walks back to
	// the ring it was seated on instead of standing over the seat.
	const APawn* Runner = GetWorld() ? UGameplayStatics::GetPlayerPawn(GetWorld(), 0) : nullptr;
	const bool bSanctuary = Runner && UPHIMito::IsInsideSanctuary(Runner->GetActorLocation());
	if (bSanctuary || (GetWorld() && GetWorld()->GetTimeSeconds() < StandDownUntil))
	{
		// HOLDING STATION MEANS STANDING ON IT. The guard used to walk a ring the whole
		// time it was on duty, so no agent in the district ever stopped moving.
		HoldPost();
		return;
	}

	switch (State)
	{
	case EVEILAgentState::Patrol:  ActPatrol(); break;
	case EVEILAgentState::Hunt:    ActHunt(); break;
	case EVEILAgentState::Combat:  ActCombat(DeltaSeconds); break;
	case EVEILAgentState::Defend:  ActDefendOrRetreat(false); break;
	case EVEILAgentState::Retreat: ActDefendOrRetreat(true); break;
	default: break;
	}
}

void AVEILFactionAgent::StandDown(float Seconds)
{
	if (!GetWorld() || Seconds <= 0.f) { return; }
	StandDownUntil = FMath::Max(StandDownUntil, (float)GetWorld()->GetTimeSeconds() + Seconds);
	// Forget him outright: no held target, no fire timer already running.
	TargetActor = nullptr;
	LastFireTime = (float)GetWorld()->GetTimeSeconds();
	SetState(EVEILAgentState::Patrol);
	UE_LOG(LogVEIL, Log, TEXT("PHI STAND DOWN: %s lets the returned runner go for %.2fs."),
		*GetActorNameOrLabel(), Seconds);
}

void AVEILFactionAgent::OrbitHome()
{
	// Radius is the fighting distance one rung down, not a typed number:
	// 700 / phi = 432.6 cm, so a guard circles at the edge of its own reach.
	// ROUTINE: on post that is its fighting ring. Off post - the quiet beats of
	// the Fibonacci day - the machine widens the same ring by phi and walks it
	// anyway, because a post is a place somebody keeps, not a statue's plinth.
	const UPHILivingWorld* Living = GetWorld() ? GetWorld()->GetSubsystem<UPHILivingWorld>() : nullptr;
	const bool bOffDuty = Living && Living->IsOffDuty();
	const float Radius = AttackRange * UPHIMito::PhiPow(-1.f) * (bOffDuty ? UPHIMito::PhiPow(1.f) : 1.f);
	const float AngleDeg = OrbitOffsetDeg + OrbitPhase * 360.f;
	const float Angle = FMath::DegreesToRadians(AngleDeg);
	const FVector Point = HomeLocation
		+ FVector(FMath::Cos(Angle) * Radius, FMath::Sin(Angle) * Radius, 0.f);
	MoveToward(Point, 60.f);

	// Proof that the ring is walked, not claimed: one line per 5s, six per agent.
	if (GetWorld() && OrbitLogs < 6 && GetWorld()->GetTimeSeconds() - LastOrbitLog > 5.f)
	{
		LastOrbitLog = (float)GetWorld()->GetTimeSeconds();
		++OrbitLogs;
		UE_LOG(LogVEIL, Log, TEXT("PHI ORBIT: %s at %s walking its %s ring (home %s, radius %.1f, phase %.3f, power %.4f)."),
			*GetActorNameOrLabel(), *GetActorLocation().ToCompactString(),
			bOffDuty ? TEXT("REST") : TEXT("POST"),
			*HomeLocation.ToCompactString(), Radius, OrbitPhase, PowerRung);
	}
}

void AVEILFactionAgent::HoldPost()
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		AIC->StopMovement();
	}
	// Face the plaza, not the nearest wall: the runner approaches from the centre, and a
	// guard that watches the square reads as posted rather than switched off.
	const FVector ToPlaza = FVector::ZeroVector - GetActorLocation();
	if (ToPlaza.SizeSquared2D() > 100.f)
	{
		SetActorRotation(FRotator(0.f, ToPlaza.Rotation().Yaw, 0.f));
	}
	// One line per post, once: proof that the ring is gone rather than merely quiet.
	if (!bLoggedPost)
	{
		bLoggedPost = true;
		UE_LOG(LogVEIL, Log, TEXT("PHI POST: %s holds its post at %s, watching the plaza."),
			*GetActorNameOrLabel(), *GetActorLocation().ToCompactString());
	}
}

void AVEILFactionAgent::ActPatrol()
{
	AAIController* AIC = Cast<AAIController>(GetController());
	if (!AIC) return;
	// NO RING WHEN THERE IS NOBODY TO WALK TO. This used to call OrbitHome(), which put
	// every agent in the district on a circle around its own home point, forever - the
	// thing that reads as NPCs running in circles. A post is a place somebody keeps: the
	// agent stands on it and watches the plaza, and only moves when it is hunting.
	if (PatrolPoints.Num() == 0)
	{
		HoldPost();
		return;
	}
	if (GetVelocity().SizeSquared2D() > 100.f) return; // still travelling to the current point (UE 5.8: EPathFollowingStatus has no 'Moving' member)

	AActor* Point = PatrolPoints[PatrolIndex % PatrolPoints.Num()];
	if (Point)
	{
		MoveToward(Point->GetActorLocation(), 80.f);
		PatrolIndex++;
	}
}

void AVEILFactionAgent::ActHunt()
{
	AAIController* AIC = Cast<AAIController>(GetController());
	if (!AIC || !TargetActor.IsValid()) return;
	MoveToward(TargetActor->GetActorLocation(), AttackRange * 0.7f);
}

void AVEILFactionAgent::ActCombat(float DeltaSeconds)
{
	// PLAYER-FIRST, every tick: if the player is visible, the mob's
	// target IS the player, even if another mob hit it last frame.
	// This is the snap-back that stops mobs chasing each other.
	if (CanSeePlayer())
	{
		if (APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
		{
			TargetActor = PlayerPawn;
		}
	}
	if (!TargetActor.IsValid()) { SetState(EVEILAgentState::Patrol); return; }
	AAIController* AIC = Cast<AAIController>(GetController());
	AActor* Target = TargetActor.Get();
	const float Dist = FVector::Dist(GetActorLocation(), Target->GetActorLocation());

	// Face the target; keep at firing distance.
	FVector ToTarget = Target->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0.f;
	if (!ToTarget.IsNearlyZero())
	{
		SetActorRotation(FRotationMatrix::MakeFromX(ToTarget).Rotator());
	}
	if (AIC && Dist > AttackRange * 0.8f)
	{
		MoveToward(Target->GetActorLocation(), AttackRange * 0.7f);
	}

	const float Now = GetWorld()->TimeSeconds;
	if (Dist <= AttackRange && Now - LastFireTime >= FireInterval)
	{
		LastFireTime = Now;
		if (APawn* TargetPawn = Cast<APawn>(Target))
		{
			UGameplayStatics::ApplyDamage(TargetPawn, WeaponDamage, GetController(), this, UDamageType::StaticClass());
		}
	}
}

void AVEILFactionAgent::ActDefendOrRetreat(bool bRetreat)
{
	AAIController* AIC = Cast<AAIController>(GetController());
	if (!AIC) return;
	if (FVector::Dist(GetActorLocation(), HomeLocation) > 150.f)
	{
		MoveToward(HomeLocation, 100.f);
	}
	if (!bRetreat && CanSeePlayer())
	{
		SetState(EVEILAgentState::Combat);
	}
	if (bRetreat && Health > MaxHealth * (RetreatHealthFraction + 0.25f))
	{
		SetState(EVEILAgentState::Patrol);
	}
}


void AVEILFactionAgent::BindStoneBody()
{
	// The stone body (job 126's triangle): Greystone is the only hero
	// tree besides the Countess complete in BOTH projects - mesh,
	// Greystone_Skeleton, Greystone_AnimBlueprint, full Jog library.
	// The Warden's own line names the material: nine hundred
	// heartbeats of STONE. If the bind fails, the machine body stands
	// and the log says so - an invisible agent is not an agent.
	// THE ASSET'S REAL NAME. The mesh in Content/ParagonGreystone/.../Meshes is
	// Greystone.uasset - the asset is "Greystone", not "SM_Greystone" or
	// "SK_Greystone". Two loads on the wrong paths were failing every run and
	// printing "Failed to find object 'SkeletalMesh'" into the log, which is the
	// sort of warning that gets ignored until it hides a real one.
	// PARAGON IS A PLACEHOLDER. The agents wore a Paragon hero's statue mesh because the
	// Greystone skeletal mesh was never in Content at all - both loads failed every boot and
	// the fallback carried the plaza. The body is now the Polyart character this project
	// OWNS, first choice rather than fallback, driven by its own locomotion graph.
	USkeletalMesh* Stone = LoadObject<USkeletalMesh>(nullptr,
		TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody"));
	UClass* StoneAnim = LoadClass<UAnimInstance>(nullptr,
		TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/AnimationBlueprints/ABP_MaleCharacter.ABP_MaleCharacter_C"));
	// Greystone is NOT whole in this project: Content/ParagonGreystone holds
	// its animations, textures and AnimBlueprint but never its skeletal mesh,
	// so both loads above come back empty. The machines then wear a real
	// animated body from a pack that IS whole in Content, and the same law
	// holds as for the stone: a body that stands, never a bare primitive.
	if (!Stone)
	{
		Stone = LoadObject<USkeletalMesh>(nullptr,
			TEXT("/Game/Polyart/SharedResources/Characters/Meshes/SkeletalMeshes/SK_Quinn_Simple.SK_Quinn_Simple"));
		if (!Stone)
		{
			// Last stand: the MetaHuman body, which is also ours and also in Content.
			Stone = LoadObject<USkeletalMesh>(nullptr,
				TEXT("/Game/MetaHumans/MH_Runner_Built/MH_Runner/Body/SKM_MH_Runner_BodyMesh.SKM_MH_Runner_BodyMesh"));
		}
		if (Stone)
		{
			// The machine now wears a Polyart body, and the Greystone graph is
			// built for the Greystone skeleton - it would hold this mesh in bind
			// pose. The Polyart locomotion graph drives the body it belongs to.
			if (UClass* PolyArtAnim = LoadClass<UAnimInstance>(nullptr,
				TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/AnimationBlueprints/ABP_MaleCharacter.ABP_MaleCharacter_C")))
			{
				StoneAnim = PolyArtAnim;
			}
		}
	}
	if (Stone)
	{
		GetMesh()->SetSkeletalMeshAsset(Stone);
		// Bone-driven body: keep it out of FMorphVertexBufferPool (assert :336).
		GetMesh()->bDisableMorphTarget = true;
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		// BLENDED LOCOMOTION, not a dropped-in graph. The Polyart AnimBlueprint is
		// fed by its own character Blueprint's speed variable, and an AI pawn never
		// writes it - so the body stayed in idle while the capsule slid along the
		// navmesh. That is the paper cut-out look. This instance reads the pawn's
		// own velocity, lerps idle into run, and matches stride to speed.
		GetMesh()->SetAnimationMode(EAnimationMode::AnimationBlueprint);
		GetMesh()->SetAnimInstanceClass(UVEILLocomotionAnimInstance::StaticClass());
		for (UActorComponent* Comp : GetComponents())
		{
			if (Comp && (Comp->GetName() == TEXT("MachineBody") || Comp->GetName() == TEXT("MachineEye")))
			{
				if (UStaticMeshComponent* SMC = Cast<UStaticMeshComponent>(Comp))
				{
					SMC->SetVisibility(false);
				}
			}
		}
		UE_LOG(LogVEIL, Log, TEXT("PHI FACTION BODY: %s stands (mesh %s, anim %s)."),
			*Stone->GetName(), *Stone->GetName(),
			StoneAnim ? *StoneAnim->GetName() : TEXT("unassigned"));
	}
	else
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI FACTION BODY FALLBACK: no character mesh resolved - machine body stands."));
	}
}

void AVEILFactionAgent::MoveToward(const FVector& Dest, float AcceptRadius)
{
	if (AAIController* AIC = Cast<AAIController>(GetController()))
	{
		// UE 5.8 header evidence (PathFollowingComponent.h, grepped by job
		// 146): EPathFollowingRequestResult::Type = { Failed, AlreadyAtGoal,
		// RequestSuccessful }. AIController.h only forward-declares the enum,
		// so this file includes PathFollowingComponent.h directly - without
		// it, no enumerator resolves (error C2039, jobs 142 and 146).
		if (AIC->MoveToLocation(Dest, AcceptRadius) != EPathFollowingRequestResult::Failed)
		{
			return;
		}
	}
	// Direct steer: no nav path - walk the line, face the line.
	// (The raycast-nav kernel in house clothes; runs per-Tick via
	// the Act functions, so the steering sustains.)
	FVector To = Dest - GetActorLocation();
	To.Z = 0.f;
	if (To.Size() > AcceptRadius)
	{
		To.Normalize();
		AddMovementInput(To, 1.f);
		SetActorRotation(FRotationMatrix::MakeFromX(To).Rotator());
	}
}

float AVEILFactionAgent::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (!IsAlive()) return 0.f;
	const float Applied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	Health = FMath::Clamp(Health - Applied, 0.f, MaxHealth);

	// PLAYER-FIRST FIX (Tyree: "mobs keep chasing each other"):
	// The old code did TargetActor = DamageCauser unconditionally, so
	// a mob hit by another mob retargeted to that mob and chased it.
	// New rule: damage NEVER retargets to a non-player causer.
	// - Player visible: target the player, enter Combat.
	// - Player not visible: stay Alert (do not chase the causer).
	// - Causer IS the player pawn: target the player, enter Combat.
	// Faction-vs-faction damage still reduces Health; it just does
	// not steer the mob. Hostility matrix in IsHostile() is unchanged:
	// Void vs any, Scavenger vs Guardian, Remnant defensive.
	if (DamageCauser)
	{
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
		const bool bCauserIsPlayer = (PlayerPawn && DamageCauser == PlayerPawn);
		if (bCauserIsPlayer || CanSeePlayer())
		{
			if (PlayerPawn) TargetActor = PlayerPawn;
			if (Health > MaxHealth * RetreatHealthFraction || Faction == EVEILFaction::Void)
			{
				SetState(EVEILAgentState::Combat);
			}
		}
		else
		{
			UE_LOG(LogVEIL, Verbose, TEXT("PHI AI PLAYER-FIRST: damage from non-player %s ignored for targeting; player not visible."),
				*DamageCauser->GetName());
			if (State == EVEILAgentState::Patrol) SetState(EVEILAgentState::Alert);
		}
	}

	if (Health <= 0.f)
	{
		AwardKill();
		SetState(EVEILAgentState::Dead);
		DisableInput(nullptr);
		if (AAIController* AIC = Cast<AAIController>(GetController())) AIC->StopMovement();
		GetCharacterMovement()->DisableMovement();
		SetActorEnableCollision(false);
		// Body stays in the world briefly, then cleans up.
		SetLifeSpan(6.f);
	}
	return Applied;
}

void AVEILFactionAgent::AwardKill()
{
	// The camp hears every loss: the district counts its dead, and the Vault's
	// gate reads that count.
	OnAgentDown.Broadcast(this);
	// THE CORE LOOP'S FIRST LINK: the world paid nothing for a kill. Machines
	// could be shot, swung at and killed while the ladder never moved - only the
	// quests gave XP, so the runner had no reason to fight. The pay is a phi
	// slice of the rung the ladder is asking for, computed on the spot and never
	// typed: phi^-2 of the level's own requirement, so phi^2 machine answers
	// earn one level. Gold rides the machine's own health rung: heavier body,
	// heavier purse.
	UWorld* World = GetWorld();
	if (!World) return;
	UOMNIQuestSystem* Quest = World->GetSubsystem<UOMNIQuestSystem>();
	if (!Quest) return;

	const int32 Need = UOMNIQuestSystem::XPNeedForLevel(FMath::Max(1, Quest->Level));
	const float Share = UPHIMito::PhiPow(-2.f);
	const int32 Pay = FMath::Max(1, FMath::RoundToInt(Need * Share));
	Quest->AwardXP(Pay, *FString::Printf(TEXT("%s down"), *GetActorNameOrLabel()));

	const int32 GoldPay = FMath::Max(1, FMath::RoundToInt(
		UPHIMito::PhiPow(UPHIMito::Rung(FMath::Max(1.f, MaxHealth)) * UPHIMito::PhiPow(-1.f)) * UPHIMito::PhiPow(3.f)));
	Quest->Gold += GoldPay;

	// THE DROP ITSELF: a machine leaves something behind in the inventory. The
	// tier is read from its own body rung in phi steps above a light machine -
	// heavier body, better drop - computed here, never a table typed by hand.
	static const TCHAR* DropNames[] = { TEXT("SHARD"), TEXT("CORE"), TEXT("RELIC"), TEXT("SIGNET") };
	const float BodyRung = UPHIMito::Rung(FMath::Max(1.f, MaxHealth));
	const float PhiSteps = (BodyRung - UPHIMito::Rung(60.f)) * UPHIMito::PhiPow(-1.f);
	const int32 Tier = FMath::Clamp(FMath::FloorToInt(PhiSteps) + 1, 0, 3);
	const FName Drop(DropNames[Tier]);

	// THE DROP LANDS IN THE WORLD, NOT IN THE BAG. The kill leaves a lit pickup
	// where the machine fell; it is the runner's only when he walks over it.
	const FVector DropAt = GetActorLocation() + FVector(0.f, 0.f, 60.f);
	FActorSpawnParameters DropParams;
	DropParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// Deferred, so the drop knows what it IS before it begins play: a pickup that
	// announces itself as "None" is a drop nobody can name.
	if (AVEILLootPickup* Pickup = World->SpawnActorDeferred<AVEILLootPickup>(
		AVEILLootPickup::StaticClass(), FTransform(FRotator::ZeroRotator, DropAt),
		nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
	{
		Pickup->ItemName = Drop;
		Pickup->GoldValue = GoldPay;
		Pickup->Tier = Tier;
		Pickup->FinishSpawning(FTransform(FRotator::ZeroRotator, DropAt));
	}

	// XP is the machine's own answer and is earned at the kill; the item is the
	// thing the district makes the runner walk for.
	if (UVEILProgression* Ledger = World->GetSubsystem<UVEILProgression>())
	{
		Ledger->AwardXP(Pay, TEXT("machine down"));
		Ledger->SaveGame();
	}

	// A kill is a flash in the world, not a line in a log.
	AVEILBurstFX::Fire(World, GetActorLocation() + FVector(0.f, 0.f, 60.f), Tier);

	UE_LOG(LogVEIL, Log, TEXT("PHI KILL: %s down -> +%d XP, +%d gold, drop %s (tier %d) | ladder %d XP at level %d (needs %d)."),
		*GetActorNameOrLabel(), Pay, GoldPay, *Drop.ToString(), Tier, Quest->XP, Quest->Level, Need);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor(255, 216, 110),
			FString::Printf(TEXT("+%d XP  +%d GOLD   %s down"), Pay, GoldPay, *GetActorNameOrLabel()));
	}
}

// ---------------------------------------------------------------------------
// Faction Manager
// ---------------------------------------------------------------------------
void UVEILFactionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogVEIL, Log, TEXT("Faction Manager online: Void / Scavenger / Guardian / Remnant."));
}

void UVEILFactionManager::RegisterAgent(AVEILFactionAgent* Agent)
{
	if (Agent) Agents.AddUnique(Agent);
}

void UVEILFactionManager::UnregisterAgent(AVEILFactionAgent* Agent)
{
	Agents.RemoveAll([Agent](const TWeakObjectPtr<AVEILFactionAgent>& A) { return A.Get() == Agent || !A.IsValid(); });
}

void UVEILFactionManager::RegisterTerritory(const FVEILTerritory& Territory)
{
	Territories.Add(Territory);
}

void UVEILFactionManager::AdjustRep(EVEILFaction Faction, float Amount)
{
	float& Rep = Reputation.FindOrAdd(Faction);
	const float Before = Rep;
	Rep = FMath::Clamp(Rep + Amount, -100.f, 100.f);
	UE_LOG(LogVEIL, Log, TEXT("PHI FACTION REP: faction %d %+f -> %f."), (int32)Faction, Amount, Rep);

	// Crossing a line is an event, not a number: say so once, in the log the
	// faction agents read, so the world's change can be traced to the words that
	// caused it.
	const bool bWasFriendly = Before >= 20.f, bIsFriendly = Rep >= 20.f;
	const bool bWasHunting = Before <= -20.f, bIsHunting = Rep <= -20.f;
	if (bWasFriendly != bIsFriendly || bWasHunting != bIsHunting)
	{
		UE_LOG(LogVEIL, Log, TEXT("PHI FACTION REP: faction %d is now %s the runner."), (int32)Faction,
			bIsFriendly ? TEXT("FRIENDLY to") : bIsHunting ? TEXT("HUNTING") : TEXT("neutral toward"));
	}
}

float UVEILFactionManager::GetReputation(EVEILFaction Faction) const
{
	const float* Found = Reputation.Find(Faction);
	return Found ? *Found : 0.f;
}

bool UVEILFactionManager::IsFriendlyToRunner(EVEILFaction Faction) const
{
	return GetReputation(Faction) >= 20.f;
}

bool UVEILFactionManager::IsHuntingRunner(EVEILFaction Faction) const
{
	return GetReputation(Faction) <= -20.f;
}

bool UVEILFactionManager::ClaimTerritory(FName TerritoryName, EVEILFaction NewOwner)
{
	for (FVEILTerritory& T : Territories)
	{
		if (T.Name == TerritoryName)
		{
			const EVEILFaction Old = T.Owner;
			T.Owner = NewOwner;
			OnTerritoryChanged.Broadcast(TerritoryName, Old, NewOwner);
			UE_LOG(LogVEIL, Log, TEXT("Territory %s: %d -> %d"), *TerritoryName.ToString(), (int32)Old, (int32)NewOwner);
			return true;
		}
	}
	return false;
}

EVEILFaction UVEILFactionManager::GetTerritoryOwner(FName TerritoryName) const
{
	for (const FVEILTerritory& T : Territories)
	{
		if (T.Name == TerritoryName) return T.Owner;
	}
	return EVEILFaction::None;
}

float UVEILFactionManager::GetFactionPower(EVEILFaction Faction) const
{
	float Power = 0.f;
	for (const TWeakObjectPtr<AVEILFactionAgent>& A : Agents)
	{
		if (A.IsValid() && A->Faction == Faction && A->IsAlive())
		{
			Power += A->Health / A->MaxHealth;
		}
	}
	return Power;
}

int32 UVEILFactionManager::GetLivingAgentCount(EVEILFaction Faction) const
{
	int32 Count = 0;
	for (const TWeakObjectPtr<AVEILFactionAgent>& A : Agents)
	{
		if (A.IsValid() && A->Faction == Faction && A->IsAlive()) Count++;
	}
	return Count;
}

AVEILFactionAgent* UVEILFactionManager::SpawnReinforcement(EVEILFaction Faction, FVector Location)
{
	if (!ReinforcementClass) return nullptr;
	UWorld* World = GetWorld();
	if (!World) return nullptr;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AVEILFactionAgent* Agent = World->SpawnActor<AVEILFactionAgent>(ReinforcementClass, Location, FRotator::ZeroRotator, Params);
	if (Agent) Agent->Faction = Faction;
	return Agent;
}

bool UVEILFactionManager::IsHostile(EVEILFaction A, EVEILFaction B) const
{
	if (A == B || A == EVEILFaction::None || B == EVEILFaction::None) return false;
	if (A == EVEILFaction::Void || B == EVEILFaction::Void) return true;
	// Scavenger <-> Guardian rivalry.
	if ((A == EVEILFaction::Scavenger && B == EVEILFaction::Guardian) ||
		(A == EVEILFaction::Guardian && B == EVEILFaction::Scavenger)) return true;
	// Remnant defends itself only - handled in agent territory logic.
	return false;
}
