"""Two fixes, both in paths proven to execute.

1. THE BLUE SWEEP, IN BEGINPLAY AND AT 4 s. Three earlier sweeps lived in code paths
   that never ran (nothing printed), so they changed nothing. This one is a lambda
   called synchronously from BeginPlay - the path that prints PHI BODY CHECK - and
   again four seconds later, after the concept composition and the fab dressing have
   finished spawning. It logs to LogTemp, the category that reaches the file, and it
   names every slab it re-dresses so the next run cannot be ambiguous.
2. THE CLOCK LEAVES THE MINIMAP. It was anchored ClipX-30-LW at y=8, which put it over
   the top of the gold ring. It now sits top-centre on the same phi safe margin, so the
   two instruments no longer share a zone.
"""

import io

fails = []

# ---------------------------------------------------------------- blue sweep
CH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(CH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

ANCHOR = '\t\tGetMesh()->GetAnimInstance() ? *GetMesh()->GetAnimInstance()->GetClass()->GetName() : TEXT("none"));'

SWEEP = """
	// THE BLUE SWEEP, WHERE IT CANNOT BE SKIPPED. A flat blue box is a cube or plane
	// carrying M_PHI_DeepBlue: not a crystal cone, not water. Called here (the path that
	// prints PHI BODY CHECK) and again at 4 s, after the composition and the fab dressing
	// have spawned, so both the map's actors and the freshly built ones are covered.
	{
		auto SweepBlue = [this]()
		{
			UWorld* SweepWorld = GetWorld();
			if (!SweepWorld) { return; }
			UStaticMesh* KitFloor = LoadObject<UStaticMesh>(nullptr,
				TEXT("/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Mesh/Modular/SM_Floor1_Big.SM_Floor1_Big"));
			UMaterialInterface* Stone = LoadObject<UMaterialInterface>(nullptr,
				TEXT("/Game/StarterContent/Materials/M_Brick_Cut_Stone.M_Brick_Cut_Stone"));
			int32 Touched = 0, Rebuilt = 0, KeptCrystal = 0;
			for (TActorIterator<AStaticMeshActor> It(SweepWorld); It; ++It)
			{
				UStaticMeshComponent* Comp = It->GetStaticMeshComponent();
				if (!Comp || !Comp->GetStaticMesh()) { continue; }
				bool bBlue = false;
				for (int32 Slot = 0; Slot < Comp->GetNumMaterials(); ++Slot)
				{
					const UMaterialInterface* Mat = Comp->GetMaterial(Slot);
					if (Mat && Mat->GetName().Contains(TEXT("DeepBlue"))) { bBlue = true; break; }
				}
				if (!bBlue) { continue; }
				const FString MeshName = Comp->GetStaticMesh()->GetName();
				if (MeshName.Contains(TEXT("Cone")) || MeshName.Contains(TEXT("Crystal")))
				{
					++KeptCrystal;
					continue;
				}
				const bool bFlatBox = MeshName.Contains(TEXT("Cube")) || MeshName.Contains(TEXT("Plane"));
				if (bFlatBox && KitFloor) { Comp->SetStaticMesh(KitFloor); ++Rebuilt; }
				if (Stone)
				{
					for (int32 Slot = 0; Slot < Comp->GetNumMaterials(); ++Slot) { Comp->SetMaterial(Slot, Stone); }
				}
				++Touched;
				if (Touched <= 12)
				{
					UE_LOG(LogTemp, Display, TEXT("PHI BLUE FIX: %s mesh=%s -> stone"), *It->GetName(), *MeshName);
				}
			}
			UE_LOG(LogTemp, Display,
				TEXT("PHI BLUE FIX TOTAL: %d slab(s) re-dressed, %d rebuilt as kit art, %d crystal(s) kept blue."),
				Touched, Rebuilt, KeptCrystal);
		};
		SweepBlue();
		if (UWorld* SweepWorld = GetWorld())
		{
			FTimerHandle SweepAgain;
			SweepWorld->GetTimerManager().SetTimer(SweepAgain, FTimerDelegate::CreateWeakLambda(this, [SweepBlue]()
			{
				SweepBlue();
			}), 4.f, false);
		}
	}"""

o = ANCHOR.replace("\n", NL)
if src.count(o) != 1:
    fails.append("character anchor count=%d" % src.count(o))
else:
    text = src.replace(o, (ANCHOR + SWEEP).replace("\n", NL), 1)
    if '#include "Engine/StaticMeshActor.h"' not in text:
        text = text.replace('#include "EngineUtils.h"',
                            '#include "EngineUtils.h"' + NL + '#include "Engine/StaticMeshActor.h"', 1)
    io.open(CH, "w", encoding="utf-8", newline="").write(text)
    print("%s: %d -> %d (blue sweep)" % (CH, len(src), len(text)))

# ---------------------------------------------------------------- clock zone
HD = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
h = io.open(HD, encoding="utf-8", newline="").read()
HNL = "\r\n" if "\r\n" in h else "\n"

COLD = "\t\tconst float LX = Canvas->ClipX - 30.f - LW;"
CNEW = (
    "\t\t// TOP CENTRE, NOT OVER THE RING. Anchored to ClipX-30 the clock sat on the\n"
    "\t\t// minimap's crown - two instruments in one zone. Centred, it keeps the phi\n"
    "\t\t// safe margin from the top edge and clears both the ring and the left plate.\n"
    "\t\tconst float LX = FMath::Max(24.f, (Canvas->ClipX - LW) * 0.5f);"
)

co, cn = COLD.replace("\n", HNL), CNEW.replace("\n", HNL)
if h.count(co) != 1:
    fails.append("clock anchor count=%d" % h.count(co))
else:
    io.open(HD, "w", encoding="utf-8", newline="").write(h.replace(co, cn, 1))
    print("%s: clock anchored top-centre" % HD)

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("BLUE SWEEP + CLOCK ZONE WRITTEN")
