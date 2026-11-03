"""Repair the cast-body pass: declaration, include order, and the quest giver body.

Three things went wrong in the first run.
1. VEILCastBody.h never declared LoadMesh, so the resolver's own cpp could not compile.
2. The include was inserted before each file's own header. Unreal requires the module's own
   header first: "Expected VEILCompanion.h to be first header included."
3. The quest-giver anchor matched three constructors, so it aborted without writing.

Everything below is anchored and count-checked; the run fails loudly rather than half-applying.
"""

import io
import re

fails = []
CAST_INC = '#include "Story/VEILCastBody.h"'


def read(p):
    return io.open(p, encoding="utf-8", newline="").read()


def write(p, s):
    io.open(p, "w", encoding="utf-8", newline="").write(s)


# ---------------------------------------------------------------- 1. the declaration
PH = "Source/VEIL/Public/Story/VEILCastBody.h"
h = read(PH)
N = "\r\n" if "\r\n" in h else "\n"
anchor = "\tVEIL_API EBody ForRole(const FString& Role);"
if "LoadMesh" in h:
    print("header: LoadMesh already declared")
elif h.count(anchor) == 1:
    add = (anchor + "\n"
           "\n"
           "\t/** Load just the mesh for a body, with the same Garret fallback. Used where a\n"
           "\t *  pawn already decides its own animation instance. */\n"
           "\tVEIL_API USkeletalMesh* LoadMesh(EBody Body);")
    write(PH, h.replace(anchor, add.replace("\n", N), 1))
    print("header: LoadMesh declared")
else:
    fails.append("header anchor count=%d" % h.count(anchor))

# ---------------------------------------------------------------- 2. include order
OWN = {
    "Source/VEIL/Private/AI/VEILCompanion.cpp": "VEILCompanion.h",
    "Source/VEIL/Private/AI/VEILFactionAI.cpp": "VEILFactionAI.h",
    "Source/VEIL/Private/World/OMNIQuestContent.cpp": "OMNIQuestContent.h",
}
for path, own in OWN.items():
    s = read(path)
    NL = "\r\n" if "\r\n" in s else "\n"
    lines = s.split(NL)
    # drop every stray copy first
    lines = [ln for ln in lines if ln.strip() != CAST_INC]
    idx = None
    for i, ln in enumerate(lines):
        if ln.strip().startswith("#include") and own in ln:
            idx = i
            break
    if idx is None:
        fails.append("%s: own header %s not found" % (path, own))
        continue
    lines.insert(idx + 1, CAST_INC)
    write(path, NL.join(lines))
    print("%s: cast include placed after its own header" % path.split("/")[-1])

# ---------------------------------------------------------------- 3. the quest giver body
P = "Source/VEIL/Private/World/OMNIQuestContent.cpp"
s = read(P)
NL = "\r\n" if "\r\n" in s else "\n"

old = """	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	UStaticMeshComponent* Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatueBody"));"""
new = """	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// A REAL PERSON, NOT A STATUE. The giver was a cylinder with a column balanced on it; this
	// component carries an installed cast body, applied in BeginPlay once the name is known.
	USkeletalMeshComponent* Figure = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CastFigure"));
	Figure->SetupAttachment(Root);
	Figure->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Figure->SetRelativeLocation(FVector(0.f, 0.f, 0.f));

	UStaticMeshComponent* Body = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StatueBody"));"""
if s.count(old.replace("\n", NL)) != 1:
    fails.append("questgiver ctor anchor count=%d" % s.count(old.replace("\n", NL)))
else:
    s = s.replace(old.replace("\n", NL), new.replace("\n", NL), 1)
    print("quest giver: cast figure component added")

old2 = '\tUE_LOG(LogVEIL, Log, TEXT("OMNI QUEST CONTENT: the %s stands ready."), *GiverName.ToString());'
new2 = old2 + """

	// THE STATUE STEPS ASIDE. Cylinder, column head and visor were the placeholder; the giver
	// now wears a body chosen by name, and the stone parts go dark so the person is what the
	// player meets rather than a pillar doing a person's job.
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
	}"""
if s.count(old2) != 1:
    fails.append("questgiver beginplay anchor count=%d" % s.count(old2))
else:
    s = s.replace(old2, new2.replace("\n", NL), 1)
    print("quest giver: statue hidden, cast body applied")

write(P, s)

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("CAST FIX APPLIED")
