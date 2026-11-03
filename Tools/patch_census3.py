"""Census inside the lambda that prints, on LogTemp.

Every earlier census used LogVEIL from a block that never reached the log, so the blue
slabs stayed unnamed. This one sits inside the same timer body as PHI LOCO STATE - a
line proven to print in every run - and uses LogTemp Display, the category the rest of
the diagnostics use. It names each blue-material actor with mesh and size.
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

ANCHOR = (
    "\t\t\t\tM ? *M->Velocity.ToCompactString() : TEXT(\"-\"),\n"
    "\t\t\t\t*GetActorLocation().ToCompactString());"
)

CENSUS = """
			// BLUE CENSUS, INSIDE THE LINE THAT PRINTS. LogTemp, because every LogVEIL
			// census so far vanished; this rides the timer body that already reports
			// PHI LOCO STATE, so it cannot be skipped.
			{
				int32 Total = 0;
				int32 Shown = 0;
				for (TActorIterator<AStaticMeshActor> BlueIt(GetWorld()); BlueIt; ++BlueIt)
				{
					UStaticMeshComponent* BlueComp = BlueIt->GetStaticMeshComponent();
					if (!BlueComp || !BlueComp->GetStaticMesh()) { continue; }
					bool bBlueSurf = false;
					for (int32 Slot = 0; Slot < BlueComp->GetNumMaterials(); ++Slot)
					{
						const UMaterialInterface* M2 = BlueComp->GetMaterial(Slot);
						if (M2 && M2->GetName().Contains(TEXT("DeepBlue"))) { bBlueSurf = true; break; }
					}
					if (!bBlueSurf) { continue; }
					++Total;
					if (Shown++ < 14)
					{
						const FBoxSphereBounds B = BlueComp->GetStaticMesh()->GetBounds();
						const FVector Sz = BlueComp->GetComponentScale() * B.BoxExtent * 2.f;
						UE_LOG(LogTemp, Display,
							TEXT("PHI BLUE ACTOR: %s mesh=%s size=%.0fx%.0fx%.0f at %s"),
							*BlueIt->GetName(), *BlueComp->GetStaticMesh()->GetName(),
							Sz.X, Sz.Y, Sz.Z, *BlueIt->GetActorLocation().ToCompactString());
					}
				}
				UE_LOG(LogTemp, Display, TEXT("PHI BLUE TOTAL: %d actor(s) wear M_PHI_DeepBlue."), Total);
			}"""

o = ANCHOR.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - anchor count=%d" % src.count(o))
    raise SystemExit(1)
text = src.replace(o, (ANCHOR + CENSUS).replace("\n", NL), 1)
if '#include "Engine/StaticMeshActor.h"' not in text:
    text = text.replace('#include "EngineUtils.h"',
                        '#include "EngineUtils.h"' + NL + '#include "Engine/StaticMeshActor.h"', 1)
io.open(PATH, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars (LogTemp census in the printing lambda)" % (PATH, len(src), len(text)))
