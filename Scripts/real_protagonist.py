"""The Runner becomes a real character; the ledger becomes legible."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


BODY = "/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody"
IDLE = "/Game/Polyart/SharedResources/Characters/AnimationStuff/Animations/Locomotion/Male/MM_Idle.MM_Idle"

# ---------- header ----------
h = P + "VeilRunner.h"
s = rd(h)
if "Avatar" not in s:
    s = s.replace("class UStaticMeshComponent;",
                  "class UStaticMeshComponent;\nclass USkeletalMeshComponent;\nclass UAnimSequence;", 1)
    a = "\tUPROPERTY(VisibleAnywhere) UStaticMeshComponent* Head;"
    if s.count(a) != 1:
        raise SystemExit("MISS Head member")
    s = s.replace(a, a + "\n\t/** The Runner's real body: a skinned character, not two primitives. */\n"
                         "\tUPROPERTY(VisibleAnywhere) USkeletalMeshComponent* Avatar;\n"
                         "\tUPROPERTY() UAnimSequence* IdleAnim = nullptr;", 1)
    wr(h, s)
    print("runner header: skinned avatar")

# ---------- constructor: real mesh, primitives retired ----------
c = P + "VeilRunner.cpp"
s = rd(c)
if "USkeletalMeshComponent>(TEXT(\"Avatar\"))" not in s:
    a = "\tBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT(\"Boom\"));"
    if s.count(a) != 1:
        raise SystemExit("MISS Boom")
    block = ('''\t// The body the world sees: the same people the NPCs are drawn from.
\tstatic ConstructorHelpers::FObjectFinder<USkeletalMesh> RunnerMesh(TEXT("''' + BODY + '''"));
\tAvatar = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Avatar"));
\tAvatar->SetupAttachment(RootComponent);
\tAvatar->SetCollisionEnabled(ECollisionEnabled::NoCollision);
\tif (RunnerMesh.Succeeded())
\t{
\t\tAvatar->SetSkeletalMesh(RunnerMesh.Object);
\t}
\tAvatar->SetRelativeRotation(FRotator(0.0, -90.0, 0.0));
\tAvatar->SetRelativeLocation(FVector(0.0, 0.0, -90.0));

\tif (Body)
\t{
\t\tBody->SetVisibility(false);
\t}
\tif (Head)
\t{
\t\tHead->SetVisibility(false);
\t}

''')
    s = s.replace(a, block + a, 1)
    if '#include "Components/SkeletalMeshComponent.h"' not in s:
        s = s.replace('#include "VeilRunner.h"',
                      '#include "VeilRunner.h"\n#include "Components/SkeletalMeshComponent.h"\n#include "Animation/AnimSequence.h"', 1)
    wr(c, s)
    print("runner: skinned avatar created, primitives hidden")

# ---------- begin play: scale to Bible height, breathe ----------
s = rd(c)
a = "\tSuper::BeginPlay();"
if "Avatar->GetSkeletalMeshAsset()" not in s:
    block = '''
	// Height is phi: Reach(0.5) stands the Runner at 206 cm.
	if (Avatar)
	{
		if (USkeletalMesh* Mesh = Avatar->GetSkeletalMeshAsset())
		{
			const FBoxSphereBounds B = Mesh->GetBounds();
			const double RawH = FMath::Max(B.BoxExtent.Z * 2.0, 1.0);
			Avatar->SetRelativeScale3D(FVector(PHI_Constants::Reach(0.5) / RawH));
		}
		IdleAnim = LoadObject<UAnimSequence>(nullptr, TEXT("''' + IDLE + '''"));
		if (IdleAnim)
		{
			Avatar->PlayAnimation(IdleAnim, true);
			UE_LOG(LogTemp, Display, TEXT("VEIL RUNNER: skinned body with idle animation"));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("VEIL RUNNER: body loaded, idle animation missing"));
		}
	}
'''
    if s.count(a) < 1:
        raise SystemExit("MISS BeginPlay super")
    s = s.replace(a, a + block, 1)
    wr(c, s)
    print("runner: phi height and idle animation")

# ---------- HUD ledger legible ----------
d = P + "VeilHud.cpp"
s = rd(d)
old = "static_cast<float>(PHI_Constants::PhiPow(-0.598))"
if old in s:
    s = s.replace(old, "static_cast<float>(PHI_Constants::PhiPow(0.35))", 1)
    wr(d, s)
    print("hud: ledger readable")
else:
    print("hud: ledger scale anchor changed")
print("real protagonist pass done")
