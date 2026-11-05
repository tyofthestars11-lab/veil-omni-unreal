"""The record needs a camera of its own, not one inside the protagonist."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


h = P + "VeilGameMode.h"
s = rd(h)
if "ShotCamera" not in s:
    a = "\tFTimerHandle ShotSettleTimer;"
    if s.count(a) != 1:
        raise SystemExit("MISS shot timer decl")
    s = s.replace(a, a + "\n\tUPROPERTY() class ACameraActor* ShotCamera = nullptr;", 1)
    wr(h, s)
    print("header: record camera")

g = P + "VeilGameMode.cpp"
s = rd(g)
start = s.find("void AVeilGameMode::TakeShot()")
if start < 0:
    raise SystemExit("MISS TakeShot")
i = s.find("{", start)
depth = 0
end = -1
for j in range(i, len(s)):
    if s[j] == "{":
        depth += 1
    elif s[j] == "}":
        depth -= 1
        if depth == 0:
            end = j + 1
            break
if end < 0:
    raise SystemExit("MISS TakeShot end")

new = '''void AVeilGameMode::TakeShot()
{
	static const TCHAR* Names[5] = {
		TEXT("VEIL_1_Hub"), TEXT("VEIL_2_Spire"), TEXT("VEIL_3_Nest"),
		TEXT("VEIL_4_Omni"), TEXT("VEIL_5_Deep")
	};
	static const FVector Anchor[5] = {
		FVector(0.0, 0.0, 0.0),
		FVector(0.0, 3500.0, 500.0),
		FVector(-450.0, 780.0, 0.0),
		FVector(2800.0, 500.0, 0.0),
		FVector(0.0, -5000.0, 0.0)
	};
	static const double Bearing[5] = { 45.0, 90.0, 120.0, 10.0, 270.0 };
	static const double Pitch[5] = { -8.0, -6.0, -7.0, -7.0, -14.0 };

	const int32 Idx = FMath::Clamp(ShotIdx, 0, 4);

	// Motion blur turns a teleported camera into a smear of colour.
	if (GEngine)
	{
		GEngine->Exec(GetWorld(), TEXT("r.MotionBlurQuality 0"));
		GEngine->Exec(GetWorld(), TEXT("r.DefaultFeature.MotionBlur 0"));
	}

	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC)
	{
		UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD SHOT %d/5: %s (no controller)"), Idx + 1, Names[Idx]);
	}
	else
	{
		// Ground under the Bible seat, then stand the Runner on it and look at the place.
		const double Span = PHI_Constants::Reach(9.0);
		FHitResult Hit;
		const FVector From(Anchor[Idx].X, Anchor[Idx].Y, Anchor[Idx].Z + Span);
		const FVector To(Anchor[Idx].X, Anchor[Idx].Y, Anchor[Idx].Z - Span);
		double GroundZ = GHubGroundZ;
		if (GetWorld()->LineTraceSingleByChannel(Hit, From, To, ECC_WorldStatic))
		{
			GroundZ = Hit.ImpactPoint.Z;
		}

		const double YawRad = FMath::DegreesToRadians(Bearing[Idx]);
		const double Back = PHI_Constants::Reach(2.0);
		const FVector Stand(Anchor[Idx].X, Anchor[Idx].Y, GroundZ + PHI_Constants::Reach(0.983));
		const FVector CamAt(Anchor[Idx].X - FMath::Cos(YawRad) * Back,
			Anchor[Idx].Y - FMath::Sin(YawRad) * Back,
			GroundZ + PHI_Constants::Reach(1.6));

		if (APawn* Pawn = PC->GetPawn())
		{
			Pawn->SetActorLocation(Stand);
		}
		PC->SetControlRotation(FRotator(static_cast<float>(Pitch[Idx]), static_cast<float>(Bearing[Idx]), 0.f));

		// A camera outside the body: the record must show the world, not the Runner's ribs.
		const FRotator Look = (FVector(Anchor[Idx].X, Anchor[Idx].Y, GroundZ + PHI_Constants::Reach(0.5)) - CamAt).Rotation();
		if (!IsValid(ShotCamera))
		{
			ShotCamera = GetWorld()->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), CamAt, Look);
		}
		else
		{
			ShotCamera->SetActorLocationAndRotation(CamAt, Look);
		}
		if (IsValid(ShotCamera))
		{
			ShotCamera->GetCameraComponent()->SetFieldOfView(static_cast<float>(100.0 / PHI_Constants::Phi));
			PC->SetViewTarget(ShotCamera);
		}
		UE_LOG(LogVEILWorld, Display,
			TEXT("VEIL WORLD SHOT %d/5: %s | stand (%.0f, %.0f, %.0f) ground %.0f | camera %.0f back"),
			Idx + 1, Names[Idx], Stand.X, Stand.Y, Stand.Z, GroundZ, Back);
	}

	ShotName = FString(Names[Idx]) + TEXT(".png");
	GetWorldTimerManager().SetTimer(ShotSettleTimer, this, &AVeilGameMode::RequestShot, 0.6f, false);

	++ShotIdx;
	if (ShotIdx < 5)
	{
		GetWorldTimerManager().SetTimer(ShotTimer, this, &AVeilGameMode::TakeShot, 3.f, false);
	}
	else
	{
		GetWorldTimerManager().SetTimer(QuitTimer, this, &AVeilGameMode::Quit, 5.f, false);
	}
}'''
s = s[:start] + new + s[end:]
if '#include "Camera/CameraActor.h"' not in s:
    s = s.replace('#include "EngineUtils.h"', '#include "EngineUtils.h"\n#include "Camera/CameraActor.h"\n#include "Camera/CameraComponent.h"', 1)
wr(g, s)
print("TakeShot: dedicated record camera outside the body")
