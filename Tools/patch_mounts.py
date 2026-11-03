"""Wire the mounts into the game: spawn four on the Hub ring, ride with [V]."""
import io, sys

ROOT = "D:/VEIL-OMNI/"
fails = []


def rd(p):
    s = io.open(ROOT + p, "r", encoding="utf-8", newline="").read()
    return s.replace("\r\n", "\n"), ("\r\n" in s)


def wr(p, t, crlf):
    io.open(ROOT + p, "w", encoding="utf-8", newline="").write(
        t.replace("\n", "\r\n") if crlf else t)


def ins(t, old, new, label):
    if t.count(old) != 1:
        fails.append("%s=%d" % (label, t.count(old)))
        return t
    return t.replace(old, new, 1)


SPAWN = '''\t// FOUR MOUNTS, ONE PER PEOPLE, ON THE HUB RING. The blueprint's BP_Mount has four
\t// varieties, 900 speed and no float; they stand on the ring by the golden angle and
\t// each finds its own floor, so they are grounded rather than placed at a guessed height.
\t{
\t\tFTimerHandle MountHandle;
\t\tGetWorldTimerManager().SetTimer(MountHandle, [this]()
\t\t{
\t\t\tfor (int32 i = 0; i < 4; ++i)
\t\t\t{
\t\t\t\tconst float A = FMath::DegreesToRadians(137.5077640500378f * (float)(i + 1));
\t\t\t\tconst FVector Post(FMath::Cos(A) * 1000.f, FMath::Sin(A) * 1000.f, 200.f);
\t\t\t\tFActorSpawnParameters P;
\t\t\t\tP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
\t\t\t\tif (AVEILMount* M = GetWorld()->SpawnActor<AVEILMount>(AVEILMount::StaticClass(), Post,
\t\t\t\t\tFRotator(0.f, FMath::RadiansToDegrees(A), 0.f), P))
\t\t\t\t{
\t\t\t\t\tM->Configure((EVEILMountKind)i, Post);
\t\t\t\t}
\t\t\t}
\t\t}, 4.f, false);
\t}

'''

RIDE = '''void AVEILPlayerController::ToggleMount()
{
\tAVEILCharacter* Runner = Cast<AVEILCharacter>(GetPawn());
\tif (!Runner || !GetWorld()) { return; }

\tAVEILMount* Best = nullptr;
\tfloat BestD = 900.f;
\tfor (TActorIterator<AVEILMount> It(GetWorld()); It; ++It)
\t{
\t\tconst float D = FVector::Dist2D(It->GetActorLocation(), Runner->GetActorLocation());
\t\tif (D < BestD) { BestD = D; Best = *It; }
\t}
\tif (!Best)
\t{
\t\tUE_LOG(LogVEIL, Warning, TEXT("PHI MOUNT: nothing within 9 m to ride."));
\t\treturn;
\t}

\tif (Best->IsRidden())
\t{
\t\t// STEPPING OFF PUTS THE BODY BACK ON THE STONE, NOT IN THE AIR.
\t\tBest->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
\t\tconst FVector Back = Best->GetActorLocation();
\t\tBest->SetActorLocation(FVector(Back.X, Back.Y, Back.Z - 96.f));
\t\tBest->SetRidden(false);
\t\tRunner->WalkSpeed = 439.f;
\t\tRunner->SprintSpeed = 750.f;
\t\tif (UCharacterMovementComponent* Move = Runner->GetCharacterMovement()) { Move->MaxWalkSpeed = 439.f; }
\t\tUE_LOG(LogVEIL, Warning, TEXT("PHI MOUNT: stepped off the %s. The runner walks at %.0f again."),
\t\t\tBest->KindName(), Runner->WalkSpeed);
\t\treturn;
\t}

\tBest->AttachToActor(Runner, FAttachmentTransformRules::KeepRelativeTransform);
\tBest->SetActorRelativeLocation(FVector(0.f, 0.f, -96.f));
\tBest->SetRidden(true);
\tRunner->WalkSpeed = Best->GetMountSpeed();
\tRunner->SprintSpeed = Best->GetMountSpeed();
\tif (UCharacterMovementComponent* Move = Runner->GetCharacterMovement())
\t{
\t\tMove->MaxWalkSpeed = Best->GetMountSpeed();
\t}
\tUE_LOG(LogVEIL, Warning, TEXT("PHI MOUNT: riding the %s at %.0f, grounded."),
\t\tBest->KindName(), Best->GetMountSpeed());
}

'''

GM = "Source/VEIL/Private/Game/VEILGameMode.cpp"
HDR = "Source/VEIL/Public/Game/VEILGameMode.h"

t, c = rd(GM)
if '#include "World/VEILMount.h"' not in t:
    t = ins(t, '#include "World/VEILKitMesh.h"\n',
            '#include "World/VEILKitMesh.h"\n#include "World/VEILMount.h"\n', "gm include")
t = ins(t, "\t// OPEN THE SKY BEFORE ANYONE IS SEATED.\n", SPAWN + "\t// OPEN THE SKY BEFORE ANYONE IS SEATED.\n", "gm spawn")
t = ins(t, "void AVEILPlayerController::SetupInputComponent()\n", RIDE + "void AVEILPlayerController::SetupInputComponent()\n", "pc ride")
t = ins(t, "\tif (K == EKeys::F1)\n",
        '''\t// [V] RIDES. A mount is a body at a post, not a vehicle system: the nearest one within
\t// nine metres is saddled, the runner's pace becomes its 900, and [V] again steps off.
\tif (K == EKeys::V && !bTypingToONE)
\t{
\t\tToggleMount();
\t\treturn true;
\t}

\tif (K == EKeys::F1)
''', "pc key")
wr(GM, t, c)

t, c = rd(HDR)
t = ins(t, "\tvoid TogglePauseMenu();\n",
        "\tvoid TogglePauseMenu();\n\n\t/** [V] rides the nearest mount (900, grounded) or steps off it. */\n\tvoid ToggleMount();\n",
        "pc decl")
wr(HDR, t, c)

if fails:
    print("FAILS:", fails)
    sys.exit(1)
print("mounts wired")
