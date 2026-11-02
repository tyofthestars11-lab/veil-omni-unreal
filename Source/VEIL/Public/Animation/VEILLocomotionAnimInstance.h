#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "VEILLocomotionAnimInstance.generated.h"

class UAnimSequence;

/**
 * Two sequences played at the SAME time and blended by speed: idle <-> jog.
 *
 * Why this exists: the machines used UAnimSingleNodeInstance + PlayAnimation(),
 * which can only ever show one clip at a time. Swapping clips on a velocity
 * test is what makes a body read as a paper cut-out - no blend into the jog, no
 * blend out of it, and legs turning over at a fixed rate under a body that
 * moves at its own phi-breathed speed (visible foot-skate).
 *
 * Here both clips run continuously, the pose is a real lerp between them
 * (FAnimationRuntime::LerpPoses), the blend is smoothed on a phi clock, and the
 * jog's play rate is scaled to actual speed so the stride matches the ground.
 */
struct FVEILLocomotionProxy : public FAnimInstanceProxy
{
	FVEILLocomotionProxy() {}
	FVEILLocomotionProxy(UAnimInstance* InInstance) : FAnimInstanceProxy(InInstance) {}

	FAnimNode_SequencePlayer_Standalone IdleNode;
	FAnimNode_SequencePlayer_Standalone JogNode;
	/** 0 = idle only, 1 = jog only. Game thread writes, anim thread reads. */
	float BlendAlpha = 0.f;

	// UE 5.8's proxy surface (AnimInstanceProxy.h): Initialize / UpdateAnimationNode /
	// CacheBones / Evaluate. The *_AnyThread names belong to graph NODES, not the proxy.
	virtual void Initialize(UAnimInstance* InAnimInstance) override;
	virtual void CacheBones() override;
	virtual void UpdateAnimationNode(const FAnimationUpdateContext& InContext) override;
	virtual bool Evaluate(FPoseContext& Output) override;
};

UCLASS()
class VEIL_API UVEILLocomotionAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Locomotion")
	UAnimSequence* IdleSequence = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Locomotion")
	UAnimSequence* JogSequence = nullptr;

	/** Speed at which the jog clip's stride is natural; play rate scales off it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "VEIL|Locomotion")
	float JogSpeed = 380.f;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Locomotion")
	float BlendAlpha = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "VEIL|Locomotion")
	float StridePlayRate = 1.f;

private:
	/** Proof: the first time this body crosses into the jog, say so once. */
	bool bBlendLogged = false;

protected:
	virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
};
