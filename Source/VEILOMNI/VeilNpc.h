#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VeilNpc.generated.h"

class USkeletalMeshComponent;
class UAnimSequence;

/**
 * One body of the world. Its people decides body, skin, height, pace and duty.
 * It stands on the traced ground under its own feet, and the mind that owns it
 * drives it through Perceive.
 */
UCLASS()
class VEILOMNI_API AVeilNpc : public APawn
{
	GENERATED_BODY()
public:
	AVeilNpc();
	virtual void Tick(float DeltaSeconds) override;

	/** Seat one body of the given people. Race index is into VeilRaces. */
	void Seat(const FVector& Home, int32 RaceIndex, int32 SeatIndex, double WalkRadius);

	/** Called by its mind every half second with what it can see. */
	void Perceive(APawn* Player, double Dist, int32 State);

	int32 GetRace() const { return RaceIdx; }
	const TCHAR* GetRaceName() const;

private:
	void Retarget();
	double GroundZ(double X, double Y, double FromZ) const;

	UPROPERTY() USkeletalMeshComponent* Body = nullptr;
	UPROPERTY() UAnimSequence* WalkAnim = nullptr;
	UPROPERTY() UAnimSequence* IdleAnim = nullptr;

	FVector HomeLoc = FVector::ZeroVector;
	FVector Target = FVector::ZeroVector;
	int32 SeatIdx = 0;
	int32 Step = 0;
	int32 RaceIdx = 0;
	double Radius = 400.0;
	double Speed = 120.0;
	float Wait = 0.f;
	float Phase = 0.f;
	bool bWalking = false;
	bool bHunter = false;
	float HitCooldown = 0.f;
};
