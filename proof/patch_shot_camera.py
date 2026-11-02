# VEIL - PROOF CAMERA + EYE PROBE PATCH
#
# Two defects the last Play frame proved:
#   1. -phishot fires from the runner's camera at the seat's -17 deg pitch, and at the plaza
#      centre that frames three metres of pavement. The shot is levelled to the horizon it is
#      meant to show, and says so in the log.
#   2. VEIL SHOT EYES traced from the body's own origin without ignoring the body, so the first
#      hit was always the runner at 0 cm - the probe reported "floor Actor_0 at 0 cm" and could
#      not name the wall in front of the camera. The traces now ignore the body.
import io
import sys

PATH = r"D:\VEIL-OMNI\Source\VEIL\Private\Game\VEILGameMode.cpp"

OLD_SHOT = (
    "\t\t\tGetWorldTimerManager().SetTimer(ShotTimer, [this]()\r\n"
    "\t\t\t{\r\n"
    "\t\t\t\tif (GEngine)\r\n"
    "\t\t\t\t{\r\n"
    "\t\t\t\t\tGEngine->Exec(GetWorld(), TEXT(\"HighResShot 1280x720\"));\r\n"
)

NEW_SHOT = (
    "\t\t\tGetWorldTimerManager().SetTimer(ShotTimer, [this]()\r\n"
    "\t\t\t{\r\n"
    "\t\t\t\tif (GEngine)\r\n"
    "\t\t\t\t{\r\n"
    "\t\t\t\t\t// LEVEL THE PROOF CAMERA BEFORE IT FIRES.\r\n"
    "\t\t\t\t\t//\r\n"
    "\t\t\t\t\t// The seat is aimed down (-17.0324 deg, the golden pitch) at the Keeper's\r\n"
    "\t\t\t\t\t// column, and from the plaza centre that put three metres of paving in the\r\n"
    "\t\t\t\t\t// frame and the world along its top edge. A proof photo of the floor is not\r\n"
    "\t\t\t\t\t// a proof of the world. The shot is taken level, so the buildings, the\r\n"
    "\t\t\t\t\t// columns and the skyline are what the camera actually looks at.\r\n"
    "\t\t\t\t\tif (APlayerController* ShotPC = UGameplayStatics::GetPlayerController(GetWorld(), 0))\r\n"
    "\t\t\t\t\t{\r\n"
    "\t\t\t\t\t\tconst FRotator Level(0.f, ShotPC->GetControlRotation().Yaw, 0.f);\r\n"
    "\t\t\t\t\t\tShotPC->SetControlRotation(Level);\r\n"
    "\t\t\t\t\t\tUE_LOG(LogVEIL, Warning, TEXT(\"VEIL SHOT LEVEL: frame taken at yaw %.0f, pitch 0 - horizon, not paving.\"),\r\n"
    "\t\t\t\t\t\t\tLevel.Yaw);\r\n"
    "\t\t\t\t\t}\r\n"
    "\t\t\t\t\tGEngine->Exec(GetWorld(), TEXT(\"HighResShot 1280x720\"));\r\n"
)

OLD_EYES = (
    "\t\t\t\t\t\tconst FVector Fwd = ShotPC->GetControlRotation().Vector();\r\n"
    "\t\t\t\t\t\tFHitResult Down, Ahead;\r\n"
    "\t\t\t\t\t\tShotWorld->LineTraceSingleByChannel(Down, Eye,\r\n"
    "\t\t\t\t\t\t\tEye - FVector(0.f, 0.f, 20000.f), ECC_Visibility);\r\n"
    "\t\t\t\t\t\tShotWorld->LineTraceSingleByChannel(Ahead, Eye,\r\n"
    "\t\t\t\t\t\t\tEye + Fwd * 3000.f, ECC_Visibility);\r\n"
)

NEW_EYES = (
    "\t\t\t\t\t\tconst FVector Fwd = ShotPC->GetControlRotation().Vector();\r\n"
    "\t\t\t\t\t\tFHitResult Down, Ahead;\r\n"
    "\t\t\t\t\t\t// NOT THE RUNNER'S OWN CAPSULE. The eye sits at the body's origin, so an\r\n"
    "\t\t\t\t\t\t// unignored trace answered \"floor Actor_0 at 0 cm; ahead Actor_0 at 0 cm\"\r\n"
    "\t\t\t\t\t\t// every single frame - a probe reporting itself instead of the room.\r\n"
    "\t\t\t\t\t\tFCollisionQueryParams EyeParams(SCENE_QUERY_STAT(VeilShotEyes), false);\r\n"
    "\t\t\t\t\t\tif (Body) { EyeParams.AddIgnoredActor(Body); }\r\n"
    "\t\t\t\t\t\tShotWorld->LineTraceSingleByChannel(Down, Eye,\r\n"
    "\t\t\t\t\t\t\tEye - FVector(0.f, 0.f, 20000.f), ECC_Visibility, EyeParams);\r\n"
    "\t\t\t\t\t\tShotWorld->LineTraceSingleByChannel(Ahead, Eye,\r\n"
    "\t\t\t\t\t\t\tEye + Fwd * 3000.f, ECC_Visibility, EyeParams);\r\n"
)


def patch(src, old, new, label):
    if new in src:
        print("PATCH {}: already applied".format(label))
        return src, True
    if old not in src:
        print("PATCH {}: anchor not found".format(label))
        return src, False
    print("PATCH {}: applied".format(label))
    return src.replace(old, new, 1), True


def main():
    with io.open(PATH, "r", encoding="utf-8", newline="") as f:
        src = f.read()
    before = len(src)

    src, ok1 = patch(src, OLD_SHOT, NEW_SHOT, "PROOF-CAMERA")
    src, ok2 = patch(src, OLD_EYES, NEW_EYES, "EYE-PROBE")

    if not (ok1 and ok2):
        return 1

    with io.open(PATH, "w", encoding="utf-8", newline="") as f:
        f.write(src)
    print("WROTE {} -> {} bytes".format(before, len(src)))
    return 0


sys.exit(main())
