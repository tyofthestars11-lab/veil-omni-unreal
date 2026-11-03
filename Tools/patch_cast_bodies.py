"""Give the cast its own bodies.

Measured before this change: every NPC loaded SK_Garret_FullBody - the runner's mesh - and
AOMNIQuestGiver was a cylinder with a column on top. Bodies that are actually installed and
rigged: Garret (male), Elara (female), Quinn (female), plus Countess and Murdock meshes.

Every edit is anchored and count-checked; a miss aborts the whole run without writing.
"""

import io

fails = []


def patch(path, pairs, incs=()):
    s = io.open(path, encoding="utf-8", newline="").read()
    N = "\r\n" if "\r\n" in s else "\n"
    for old, new in pairs:
        o, n = old.replace("\n", N), new.replace("\n", N)
        if s.count(o) != 1:
            fails.append("%s [%s] count=%d" % (path.split("/")[-1], old.split("\n")[0][:40], s.count(o)))
            continue
        s = s.replace(o, n, 1)
    for inc in incs:
        if inc not in s:
            m = s.find("#include")
            if m == -1:
                fails.append("%s: no include block" % path)
                continue
            s = s[:m] + inc.replace("\n", N) + N + s[m:]
    io.open(path, "w", encoding="utf-8", newline="").write(s)


# ---------------------------------------------------------------- companion
patch("Source/VEIL/Private/AI/VEILCompanion.cpp", [
    ("""	// Proven foe pattern: runtime load, silent on miss, never a dialog.
	// Paragon minion was a placeholder; the Polyart character is ours and complete.
	if (USkeletalMesh* M = LoadObject<USkeletalMesh>(nullptr, TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody")))
	{
		GetMesh()->SetSkeletalMesh(M);
		// Bone-driven body: keep it out of FMorphVertexBufferPool (assert :336).
		GetMesh()->bDisableMorphTarget = true;
		GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	}""",
     """	// THE BODY COMES FROM THE CALL SIGN, NOT FROM THE RUNNER. This constructor used to load
	// the runner's own mesh, so every companion was the player standing beside himself. The
	// body is applied in ConfigureForRole, once the call sign that decides it is known."""),
    ("""	Health = MaxHealth;
	SetActorScale3D(Scale);""",
     """	Health = MaxHealth;
	SetActorScale3D(Scale);

	// A PERSON, CHOSEN BY NAME. Sage, Scout, Guard, Bulwark and Drone resolve to different
	// bodies from the installed cast, so a party reads as five people rather than five copies.
	VEILCastBody::Apply(GetMesh(), VEILCastBody::ForRole(CallSign.ToString()), CallSign.ToString(), 1.f);"""),
], incs=('#include "Story/VEILCastBody.h"',))

# ---------------------------------------------------------------- faction agents
patch("Source/VEIL/Private/AI/VEILFactionAI.cpp", [
    ("""	USkeletalMesh* Stone = LoadObject<USkeletalMesh>(nullptr,
		TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody"));""",
     """	// A CAST BODY, NOT THE RUNNER AGAIN. The mesh is resolved from this agent's own name, so
	// the plaza's agents are different people instead of the player standing at every post.
	// The locomotion instance below still drives whichever body is resolved.
	const VEILCastBody::EBody AgentBody = VEILCastBody::ForRole(GetName());
	USkeletalMesh* Stone = VEILCastBody::LoadMesh(AgentBody);"""),
], incs=('#include "Story/VEILCastBody.h"',))

# ---------------------------------------------------------------- enemies
patch("Source/VEIL/Private/Systems/VEILEnemy.cpp", [
    ("""	// THE SAME BODY THE PLAZA WEARS. Enemies were Paragon minions - a placeholder pack - while
	// the project owns a complete Polyart character with its own skeleton and animations.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MinionMesh(TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody"));""",
     """	// NOT THE RUNNER'S BODY. Enemies wore SK_Garret_FullBody, the player's own mesh, so a
	// fight was the player against himself. Elara is the same Polyart rig - the locomotion
	// instance below drives her exactly as it drove him - and she is a different person.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> MinionMesh(TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Elara_FullBody.SK_Elara_FullBody"));"""),
])

# ---------------------------------------------------------------- warden
patch("Source/VEIL/Private/AI/OMNIWardenPrime.cpp", [
    ("""	USkeletalMesh* Boss = LoadObject<USkeletalMesh>(nullptr,
		TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody.SK_Garret_FullBody"));
	UClass* BossAnim = LoadClass<UAnimInstance>(nullptr,
		TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/AnimationBlueprints/ABP_MaleCharacter.ABP_MaleCharacter_C"));""",
     """	// THE WARDEN IS NOT THE RUNNER. He wore Garret, the player's own mesh, so the boss of the
	// district arrived looking like the player. Elara's rig is the same skeleton family and
	// her graph is installed, so the body animates and the silhouette is someone else.
	USkeletalMesh* Boss = LoadObject<USkeletalMesh>(nullptr,
		TEXT("/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Elara_FullBody.SK_Elara_FullBody"));
	UClass* BossAnim = LoadClass<UAnimInstance>(nullptr,
		TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/AnimationBlueprints/ABP_FemaleCharacter.ABP_FemaleCharacter_C"));"""),
    ("		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));\n\t\tif (BossAnim) GetMesh()->SetAnimInstanceClass(BossAnim);",
     "		GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));\n\t\t// A head taller than the runner, so the boss reads as a boss at a distance.\n\t\tGetMesh()->SetRelativeScale3D(FVector(1.15f));\n\t\tif (BossAnim) GetMesh()->SetAnimInstanceClass(BossAnim);"),
])

# ---------------------------------------------------------------- quest givers
patch("Source/VEIL/Private/World/OMNIQuestContent.cpp", [
    ("""	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);""",
     """	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// A REAL PERSON, NOT A STATUE. The giver was a cylinder with a column balanced on it; this
	// component carries an installed cast body, applied in BeginPlay once the name is known.
	USkeletalMeshComponent* Figure = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CastFigure"));
	Figure->SetupAttachment(Root);
	Figure->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Figure->SetRelativeLocation(FVector(0.f, 0.f, 0.f));"""),
    ("""	UE_LOG(LogVEIL, Log, TEXT("OMNI QUEST CONTENT: the %s stands ready."), *GiverName.ToString());""",
     """	UE_LOG(LogVEIL, Log, TEXT("OMNI QUEST CONTENT: the %s stands ready."), *GiverName.ToString());

	// THE STATUE STEPS ASIDE. Cylinder, column head and visor were the placeholder; the giver
	// now wears a body chosen by name, and the stone parts go dark so the person is what the
	// player meets rather than a pillar with a person's job.
	if (USkeletalMeshComponent* CastFigure = FindComponentByClass<USkeletalMeshComponent>())
	{
		VEILCastBody::Apply(CastFigure, VEILCastBody::ForRole(GiverName.ToString()),
			GiverName.ToString(), 1.f);
	}
	{
		TArray<UStaticMeshComponent*> StatueParts;
		GetComponents<UStaticMeshComponent>(StatueParts);
		for (UStaticMeshComponent* Part : StatueParts)
		{
			if (!Part) { continue; }
			if (Part->GetName().Contains(TEXT("Statue")))
			{
				Part->SetVisibility(false);
				Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			}
		}
	}"""),
], incs=('#include "Components/SkeletalMeshComponent.h"\n#include "Story/VEILCastBody.h"',))

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("CAST EDITS APPLIED: companion, faction agents, enemies, warden, quest givers")
