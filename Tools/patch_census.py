"""Census every actor holding M_PHI_DeepBlue, from the character's own 7 s timer.

The sweep in the game mode never printed, so the measurement moves to a timer that is
proven to fire (PHI LOCO STATE runs from the same lambda family). It names the first
twelve actors with mesh, world size and location - enough to see whether the blue
slabs are water sheets, kit meshes wearing the palette, or engine boxes.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

ANCHOR = (
    "\t\t\t\tM ? *M->Velocity.ToCompactString() : TEXT(\"-\"),\n"
    "\t\t\t\t*GetActorLocation().ToCompactString());\n"
    "\t\t}, 5.f, false);"
)

CENSUS = """			// BLUE CENSUS: every actor wearing the placeholder palette, named with its
			// mesh and world size. The photo shows flat blue slabs standing in the air
			// over real stone; this says exactly which actors they are, so the repair
			// is aimed instead of guessed.
			FTimerHandle BlueCensus;
			GetWorldTimerManager().SetTimer(BlueCensus, [this]()
			{
				int32 Total = 0;
				int32 Shown = 0;
				for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
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
					++Total;
					if (Shown++ < 12)
					{
						const FBoxSphereBounds B = Comp->GetStaticMesh()->GetBounds();
						const FVector S = Comp->GetComponentScale() * B.BoxExtent * 2.f;
						UE_LOG(LogVEIL, Warning,
							TEXT("PHI BLUE CENSUS: %s mesh=%s size=%.0fx%.0fx%.0f at %s"),
							*It->GetName(), *Comp->GetStaticMesh()->GetName(),
							S.X, S.Y, S.Z, *It->GetActorLocation().ToCompactString());
					}
				}
				UE_LOG(LogVEIL, Warning,
					TEXT("PHI BLUE CENSUS: %d actor(s) hold M_PHI_DeepBlue (first %d named above)."),
					Total, FMath::Min(Shown, 12));
			}, 7.f, false);"""

o = ANCHOR.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - anchor count=%d" % src.count(o))
    raise SystemExit(1)
text = src.replace(o, (ANCHOR + "\n" + CENSUS).replace("\n", NL), 1)

inc = '#include "EngineUtils.h"'
if '#include "Engine/StaticMeshActor.h"' not in text:
    text = text.replace(inc, inc + NL + '#include "Engine/StaticMeshActor.h"', 1)

io.open(PATH, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars (blue census at 7 s)" % (PATH, len(src), len(text)))
