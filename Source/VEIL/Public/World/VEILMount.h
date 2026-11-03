#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VEILMount.generated.h"

class UStaticMeshComponent;
class USceneComponent;

/** The four mounts of the blueprint sheet. Each is a body that stands on the ground it was
 *  given, with its own speed; none of them floats and none of them follows anyone. */
UENUM(BlueprintType)
enum class EVEILMountKind : uint8
{
	GoldWing,
	Crystal,
	Market,
	Deep
};

/**
 * A MOUNT, NOT A HOVERBOARD.
 *
 * Built from engine shapes like the rest of the district, standing on a floor trace at its
 * own post. It carries the blueprint's 900 and nothing else: no fly, no float, no follow.
 * Riding is handled by the player controller, which attaches the body under the runner and
 * raises his pace; dismounting puts it back on the stone where he left it.
 */
UCLASS()
class VEIL_API AVEILMount : public AActor
{
	GENERATED_BODY()

public:
	AVEILMount();
	virtual void BeginPlay() override;

	void Configure(EVEILMountKind InKind, const FVector& Post);

	EVEILMountKind GetKind() const { return Kind; }
	float GetMountSpeed() const { return MountSpeed; }
	bool IsRidden() const { return bRidden; }
	void SetRidden(bool b) { bRidden = b; }
	const TCHAR* KindName() const;

private:
	void BuildBody();

	/** Stand the configured body on the floor under its post. */
	void GroundAtPost();
	bool bConfigured = false;

	UPROPERTY() TObjectPtr<USceneComponent> MountRoot;
	UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Parts;

	EVEILMountKind Kind = EVEILMountKind::GoldWing;
	float MountSpeed = 900.f;
	bool bRidden = false;
	FVector HomePost = FVector::ZeroVector;
};
