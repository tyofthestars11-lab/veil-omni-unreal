// VEIL: THE LAST SIGNAL - player character.
// Third-person movement, sprint, jump, dodge, aim + hitscan fire, health,
// interact trace. Enhanced Input throughout (UE 5.8).
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VEILCharacter.generated.h"

class UCameraComponent;
class UStaticMeshComponent;
class USpringArmComponent;
class USkeletalMesh;
class UAnimSequence;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FVEILHealthChanged, float, NewHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FVEILCharacterDied);

UENUM(BlueprintType)
enum class EVEILCombatState : uint8
{
	Exploring,
	Aiming,
	Dodging,
	Dead
};

UCLASS()
class VEIL_API AVEILCharacter : public ACharacter
{
	GENERATED_BODY()

public:

	/** Claim the nearest named seam within arm's reach - the same call the
	 *  interact key makes, so veil.claim can prove what a player can actually do. */
	void ClaimNearestSeam(float ReachCm = 700.f);

	/** Keep trying to claim for a moment. A press that lands a frame before the
	 *  world has spawned its seams is not a wasted press, and a run can exercise
	 *  the same path from the command line. */
	void RequestClaim(float ReachCm = 700.f);

private:
	float ClaimArmUntil = -1.f;
	float ClaimArmReach = 700.f;

public:
	AVEILCharacter();
	/** Incremented as the first act of Tick - the controller reads it
	 *  to prove whether Tick is ever entered. */
	int32 PhiTickEntryCount = 0;

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	/** PUSH TO TALK: B opens ONE's microphone while held, closes it on release. */
	void OneTalkPressed();
	void OneTalkReleased();
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

	// ---- Health ----
	UPROPERTY(BlueprintAssignable, Category = "VEIL|Health")
	FVEILHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "VEIL|Health")
	FVEILCharacterDied OnDied;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Health")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Health")
	float Health = 100.f;

	UFUNCTION(BlueprintCallable, Category = "VEIL|Health")
	void Heal(float Amount);

	/**
	 * A collected drop pays in power, not just coin: it closes wounds and raises
	 * the runner's damage by a phi slice of his base, tier by tier, capped at
	 * phi^4 of the base so a long run cannot turn the lance into a cheat.
	 */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Loot")
	void ApplyDrop(int32 Tier);

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Loot")
	float DamageBonus = 0.f;

	UFUNCTION(BlueprintPure, Category = "VEIL|Health")
	bool IsAlive() const { return Health > 0.f; }

	/** INFINITE RESPAWN. Death is a set-back, never an ending: after one phi
	 *  second the runner stands again at his anchored seat with full health.
	 *  There is no death budget, no life counter and no cap - the world always
	 *  gets him back, however many times he falls. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Health")
	void RespawnNow();

	UFUNCTION(BlueprintPure, Category = "VEIL|Health")
	int32 GetRespawnCount() const { return RespawnCount; }

	/** How many times the runner has come back. Uncapped by design. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Health")
	int32 RespawnCount = 0;

	/** Seconds spent on the floor since the last death. */
	float DeathClock = 0.f;

	/** Eased move axis: raw stick input snapped, this carries it in and out. */
	FVector2D MoveAxis = FVector2D::ZeroVector;

	/** ATTACK TOKENS. Reverse-engineered from God of War's aggression token pool
	 *  (Mihir Sheth, Santa Monica Studio, GDC 2019): a fixed pool of attackers,
	 *  scored periodically, and everyone without a token circles and waits. Here
	 *  the pool is enforced on the receiving end, because that is one gate every
	 *  attacker must pass - so three machines can surround the runner while only
	 *  AttackersAllowed of them may land a blow in the same window. */
	int32 AttackersAllowed = 2;
	TArray<float> RecentHitTimes;
	float TokenWindow = 1.f / 1.618033988749895f;   // phi^-1 = 0.618 s
	bool bTokenHoldAnnounced = false;

	/** REWARDING, NOT PUNISHING. The runner cannot be hurt for phi^5 = 11.0902
	 *  seconds from spawn, and for phi^3 = 4.2361 seconds after every respawn.
	 *  The camp still stands and still hunts - it simply does not execute a
	 *  player who has not been given a moment to move yet. */
	float GraceClock = 0.f;
	float GraceWindow = 0.f;
	bool bGraceAnnounced = false;

	// ---- God Mode: on the menus, on the console, in the damage gate ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|GodMode")
	bool bGodMode = false;

	UFUNCTION(Exec, BlueprintCallable, Category = "VEIL|GodMode")
	void ToggleGodMode();

	UFUNCTION(BlueprintPure, Category = "VEIL|GodMode")
	bool IsGodMode() const { return bGodMode; }

	// ---- Combat ----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Combat")
	EVEILCombatState CombatState = EVEILCombatState::Exploring;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Combat")
	float WeaponDamage = 25.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Combat")
	float WeaponRange = 6000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Combat")
	float FireCooldown = 0.22f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Combat")
	float DodgeDistance = 550.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Combat")
	float DodgeCooldown = 0.9f;

	// ---- Swing + Lance Bolt (VEIL OMNI whole, 2026-10-31) ----
	// Fire() is the lance bolt (hitscan, WeaponDamage, WeaponRange).
	// Swing() is the close melee: Echo Blade / Signal Lance swing,
	// short arc in front of the runner, no projectile asset needed.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Combat")
	float SwingDamage = 35.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Combat")
	float SwingRange = 260.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Combat")
	float SwingCooldown = 0.45f;

	UFUNCTION(BlueprintCallable, Category = "VEIL|Combat")
	void Swing();

	/** The whole strike gesture - the swing and the observation of the field it
	 *  carries - reachable from outside the key, so proof runs and any future
	 *  input source go through one handler instead of a copy of it. */
	UFUNCTION(BlueprintCallable, Category = "VEIL|Combat")
	void Strike();

	UFUNCTION(BlueprintCallable, Category = "VEIL|Combat")
	void LanceBolt();

	// ---- MetaHuman body (VEIL OMNI, 2026-10-31) ----
	// This is the PLAYER BODY assignment path, not a clothing path.
	// Assign in BP_PlayerCharacter ONLY an assembled MetaHuman body
	// mesh, e.g. a mesh under /Game/MetaHumans/<Name>/Body/.
	// Buff_Black / Buff_Blue / Buff_* are Skeletal Clothing in the
	// MetaHuman Manager (photo 210), NOT a body: assigning a Buff
	// here is not a player character, and a Buff that is still
	// "Asset has not been verified" (0 B) is not a playable body.
	// If this is left empty, AssignMetaHumanBody() does nothing and
	// the Countess mesh (constructor) / primitive runner (fallback)
	// stands, exactly as before. Nothing here verifies, packages,
	// downloads, or assembles a MetaHuman on its own.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|MetaHuman")
	TObjectPtr<USkeletalMesh> MetaHumanBodyMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|MetaHuman")
	TSubclassOf<UAnimInstance> MetaHumanAnimBlueprintClass;

	UFUNCTION(BlueprintCallable, Category = "VEIL|MetaHuman")
	void AssignMetaHumanBody();

	// ---- Movement tuning ----
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Movement")
	float WalkSpeed = 450.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Movement")
	float SprintSpeed = 750.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Movement")
	float AimWalkSpeed = 220.f;

	// ------------------------------------------------------------------
	// FLIGHT: its own vertical axis.
	//
	// The bug this is built around: flight that climbed by pitching the camera
	// means looking up flies you up, so you cannot look down while you climb.
	// Here the climb is ClimbAxis * FlyClimbSpeed applied to Velocity.Z - the
	// camera keeps looking wherever the runner looks, and the body goes where the
	// vertical input says. Hover means ClimbAxis 0 holds altitude, not sinks.
	// ------------------------------------------------------------------
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Flight")
	bool bFlying = false;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Flight")
	bool bNoclip = false;

	/** -1 dive, 0 hover, +1 climb. Set by keys or by phi.climb. */
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Flight")
	float ClimbAxis = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Flight")
	float FlyCruiseSpeed = 900.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Flight")
	float FlyFastSpeed = 1700.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Flight")
	float FlyClimbSpeed = 800.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Flight")
	float FlyVerticalAccel = 2400.f;

	/** Fall speed cap, cm/s. A long drop lands soft; it never runs away. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Flight")
	float MaxFallSpeed = 2400.f;

	/** ATP: the +1 fuel. Sprint, climb and flight spend it; rest returns it;
	 *  GODMODE holds it full. */
	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Flight")
	float ATP = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Flight")
	float MaxATP = 100.f;

	UFUNCTION(Exec, BlueprintCallable, Category = "VEIL|Flight")
	void ToggleFlight();

	UFUNCTION(Exec, BlueprintCallable, Category = "VEIL|Flight")
	void ToggleNoclip();

	UFUNCTION(BlueprintCallable, Category = "VEIL|Flight")
	void SetClimbAxis(float InAxis);

	UFUNCTION(BlueprintPure, Category = "VEIL|Flight")
	bool IsFlyingNow() const { return bFlying; }

	UFUNCTION(BlueprintPure, Category = "VEIL|Flight")
	float GetATP() const { return ATP; }

	/** Landing: soft touch-down in flight, and fall damage in clean play only. */
	virtual void Landed(const FHitResult& Hit) override;

public:
	/** THE CAMERA RIG - one whole thing, not a patch.
	 *  Rest pitch is the golden pitch -17.0324. Look input swings inside
	 *  +-(90/phi) = +-55.6231 degrees around that law. The arm steps between
	 *  phi rings 420/phi, 420, 420*phi. Aim tightens the field of view to
	 *  90/phi. First person drops the arm to zero and hides the body.
	 *  Called every frame by the player controller, the loop proven to tick. */
	void HoldGoldenCamera(float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category = "VEIL|Camera")
	void AddCameraPitch(float DeltaDegrees);

	UFUNCTION(BlueprintCallable, Category = "VEIL|Camera")
	void StepCameraRing(int32 Direction);

	/** Mouse wheel steps the phi rings: in closes on the runner, out opens wide. */
	void CameraRingIn();
	void CameraRingOut();

	UFUNCTION(BlueprintPure, Category = "VEIL|Camera")
	float GetCameraPitch() const { return CameraPitch; }

	UFUNCTION(BlueprintPure, Category = "VEIL|Camera")
	int32 GetCameraRing() const { return CameraRing; }

	UFUNCTION(BlueprintPure, Category = "VEIL|Camera")
	float GetCameraArmLength() const { return CameraArmLength; }

	// ---- Read by the field manual: the gesture itself, never a guess ----
	UFUNCTION(BlueprintPure, Category = "VEIL|State")
	bool IsSprintingNow() const { return bSprinting; }

	UFUNCTION(BlueprintPure, Category = "VEIL|State")
	bool IsFirstPersonNow() const { return bFirstPerson; }

	float GetLastSwingTime() const { return LastSwingTime; }
	float GetLastFireTime() const { return LastFireTime; }

	/** Jump through the story gate: [SPACE]/[A] cannot leap while someone talks. */
	void JumpFromInput();

	/** True while a conversation owns the input. */
	bool IsStoryHoldingInput() const;

protected:
	virtual void BeginPlay() override;

	// ---- Camera rig state ----
	/** Rest and live pitch of the rig, degrees. Never typed: golden pitch, then
	 *  moved by the look axis inside the phi envelope. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Camera")
	float CameraPitch = -17.0324f;

	/** 0 = 420/phi (close), 1 = 420 (the law), 2 = 420*phi (wide). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Camera")
	int32 CameraRing = 1;

	/** Live arm length, eased toward the ring rather than snapped. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Camera")
	float CameraArmLength = 420.f;

	// ---- Components ----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "VEIL|Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	// ---- Enhanced Input assets (assign in BP_PlayerCharacter) ----
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VEIL|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VEIL|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VEIL|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VEIL|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VEIL|Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VEIL|Input")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VEIL|Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VEIL|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VEIL|Input")
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "VEIL|Input")
	TObjectPtr<UInputAction> SwingAction;

	// ---- Input handlers ----
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartSprint();
	void StopSprint();
	void DoDodge();
	void StartAim();
	void StopAim();
	void Fire();
	void Interact();
	void OnSwingInput();

private:
	/** If no Input assets are assigned in the Blueprint, the game builds its
	 *  own mapping in code: WASD + mouse AND a full Xbox controller map.
	 *  Either way, both control schemes are live from the first frame. */
	void BuildFallbackInput();

	UPROPERTY()
	TObjectPtr<UInputMappingContext> BuiltMappingContext;

	UPROPERTY()
	TArray<TObjectPtr<UInputAction>> BuiltActions;

	bool bSprinting = false;
	bool bAiming = false;
	float LastFireTime = -100.f;
	float LastDodgeTime = -100.f;
	float LastSwingTime = -100.f;

	bool bFirstPerson = false;
	/** True while the camera is pressed inside the body and the body is hidden. */
	bool bBodyHiddenForCamera = false;
	void ToggleCameraMode();
	TObjectPtr<UStaticMeshComponent> RunnerBody;
	TObjectPtr<UStaticMeshComponent> RunnerVisor;

	void ApplySpeed();

	// ---- Flight internals ----
	void ApplyFlight(float DeltaSeconds);
	void UpdateATP(float DeltaSeconds);
	/** Small key handlers: the climb axis is a held axis, not a per-frame read. */
	void FlyUpStart();
	void FlyUpStop();
	void FlyDownStart();
	void FlyDownStop();

	float BaseWeaponDamage = 25.f;
	float BaseSwingDamage = 35.f;

	/**
	 * The Countess tree ships 210 sequences and NO AnimBlueprint, so the runner
	 * stood in a frozen pose: animation was never wired. The body is driven
	 * straight from its own sequences here - idle, jog, air - chosen by the
	 * runner's actual motion and cadenced by his actual speed.
	 */
	void DriveBodyAnimation(float DeltaSeconds);

	UPROPERTY()
	TObjectPtr<UAnimSequence> IdleSequence;
	UPROPERTY()
	TObjectPtr<UAnimSequence> JogSequence;
	UPROPERTY()
	TObjectPtr<UAnimSequence> AirSequence;
	UPROPERTY()
	TObjectPtr<UAnimSequence> PlayingSequence;

	/** Proof harness only: -phimove drives synthetic forward input once. */
	bool bPhiMoveProof = false;
};
