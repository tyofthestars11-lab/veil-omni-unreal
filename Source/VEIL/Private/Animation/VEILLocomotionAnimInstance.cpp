#include "Animation/VEILLocomotionAnimInstance.h"

#include "AnimationRuntime.h"
#include "Systems/PHIMito.h"
#include "VEIL.h"

void FVEILLocomotionProxy::Initialize(UAnimInstance* InAnimInstance)
{
	FAnimInstanceProxy::Initialize(InAnimInstance);
	// Both players are initialised once and never torn down: the blend is the
	// transition, not a clip swap.
	FAnimationInitializeContext InitContext(this);
	IdleNode.Initialize_AnyThread(InitContext);
	JogNode.Initialize_AnyThread(InitContext);
}

void FVEILLocomotionProxy::CacheBones()
{
	FAnimInstanceProxy::CacheBones();
	FAnimationCacheBonesContext CacheContext(this);
	IdleNode.CacheBones_AnyThread(CacheContext);
	JogNode.CacheBones_AnyThread(CacheContext);
}

void FVEILLocomotionProxy::UpdateAnimationNode(const FAnimationUpdateContext& InContext)
{
	IdleNode.Update_AnyThread(InContext);
	JogNode.Update_AnyThread(InContext);
}

bool FVEILLocomotionProxy::Evaluate(FPoseContext& Output)
{
	FPoseContext IdlePose(Output);
	FPoseContext JogPose(Output);

	IdleNode.Evaluate_AnyThread(IdlePose);
	JogNode.Evaluate_AnyThread(JogPose);

	// Start from the idle pose, then lerp toward the jog pose by the live blend.
	// LerpPoses handles pose AND curve, so montage/curve data blends with it.
	Output.Pose.CopyBonesFrom(IdlePose.Pose);
	Output.Curve.CopyFrom(IdlePose.Curve);
	FAnimationRuntime::LerpPoses(Output.Pose, JogPose.Pose, Output.Curve, JogPose.Curve, BlendAlpha);
	return true;
}

FAnimInstanceProxy* UVEILLocomotionAnimInstance::CreateAnimInstanceProxy()
{
	return new FVEILLocomotionProxy(this);
}

void UVEILLocomotionAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	// GetProxyOnGameThread<T>() returns T& , not T* .
	FVEILLocomotionProxy& Proxy = GetProxyOnGameThread<FVEILLocomotionProxy>();
	Proxy.IdleNode.SetSequence(IdleSequence);
	Proxy.JogNode.SetSequence(JogSequence);
}

void UVEILLocomotionAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	FVEILLocomotionProxy& Proxy = GetProxyOnGameThread<FVEILLocomotionProxy>();

	// The body can be assigned its clips after the instance exists (the machine
	// finds them from its own mesh), so pick up any late assignment.
	if (Proxy.IdleNode.GetSequence() != IdleSequence)
	{
		Proxy.IdleNode.SetSequence(IdleSequence);
	}
	if (Proxy.JogNode.GetSequence() != JogSequence)
	{
		Proxy.JogNode.SetSequence(JogSequence);
	}

	const APawn* Owner = TryGetPawnOwner();
	const float Speed = Owner ? (float)Owner->GetVelocity().Size2D() : 0.f;
	const float SafeJog = FMath::Max(JogSpeed, 1.f);
	const float Target = JogSequence ? FMath::Clamp(Speed / SafeJog, 0.f, 1.f) : 0.f;

	// A stop is not a snap. The blend walks toward the target at phi per second,
	// so starting and stopping both read as weight moving rather than a cut.
	BlendAlpha = FMath::FInterpTo(BlendAlpha, Target, DeltaSeconds, UPHIMito::Phi);
	Proxy.BlendAlpha = BlendAlpha;

	// Stride match: legs turn over at the speed the body actually travels, so a
	// chasing machine is not skating with a fixed 1.0 play rate.
	StridePlayRate = FMath::Clamp(Speed / SafeJog, 0.35f, 2.0f);
	Proxy.JogNode.SetPlayRate(StridePlayRate);

	// Proof, once per body: a real blend crossed the halfway point, meaning the
	// pose is a lerp of both clips rather than a swapped asset.
	if (!bBlendLogged && BlendAlpha > 0.5f)
	{
		bBlendLogged = true;
		const AActor* Body = GetOwningActor();
		UE_LOG(LogVEIL, Log,
			TEXT("PHI LOCO BLEND: %s crossed to jog at blend=%.3f speed=%.0f stride=%.3f (idle+jog lerped)."),
			Body ? *Body->GetName() : TEXT("(body)"), BlendAlpha, Speed, StridePlayRate);
	}
}
