"""One boot pass that re-dresses every engine primitive with its kit mesh.

45 call sites still load /Engine/BasicShapes, and several of them (crowd instances, agent
machine bodies, statue parts, ocean skin) are components, not actors. So the swap happens at
component level for every actor in the world, and the kit mesh is fitted to the exact volume the
placeholder occupied. An audit runs straight after and names anything still wearing an engine
shape, with a verdict rather than a claim.
"""

import io

P = "Source/VEIL/Private/Game/VEILGameMode.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"

if '#include "World/VEILKitMesh.h"' not in s:
    m = s.find('#include "VEIL.h"')
    if m == -1:
        print("no VEIL.h include anchor")
        raise SystemExit(1)
    s = s[:m] + '#include "World/VEILKitMesh.h"' + N + s[m:]
    print("include added")

ANCHOR = "\t// THE CUT, THE LIFT AND THE MEASUREMENT"
BLOCK = '''	// NO PLACEHOLDER SURVIVES THE BOOT. Every component still wearing an engine shape is
	// re-dressed with its kit mesh and fitted to the exact volume the placeholder occupied. It
	// runs at component level because the placeholders are not all actors: crowd walkers are
	// instanced components, agent bodies and statue parts are default subobjects, the ocean skin
	// and gear relics are plain components. One pass means no call site can leak a cube into the
	// frame, and the audit straight after names anything the pass refused.
	{
		FTimerHandle PlaceholderTimer;
		GetWorldTimerManager().SetTimer(PlaceholderTimer, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			UWorld* W = GetWorld();
			if (!W) { return; }
			VEILKit::Report();

			int32 Swapped = 0, Refused = 0;
			TArray<AActor*> Everyone;
			for (TActorIterator<AActor> It(W); It; ++It) { Everyone.Add(*It); }
			for (AActor* One : Everyone)
			{
				if (!One) { continue; }
				TInlineComponentArray<UStaticMeshComponent*> Comps;
				One->GetComponents(Comps);
				for (UStaticMeshComponent* Comp : Comps)
				{
					if (!Comp || !Comp->GetStaticMesh()) { continue; }
					if (!VEILKit::IsEngineShape(Comp->GetStaticMesh())) { continue; }
					int32 Kind = 0;
					if (VEILKit::Redress(Comp, Kind)) { ++Swapped; }
					else { ++Refused; }
				}
			}

			int32 Still = 0, Named = 0;
			for (TActorIterator<AActor> It(W); It; ++It)
			{
				TInlineComponentArray<UStaticMeshComponent*> Comps;
				It->GetComponents(Comps);
				for (UStaticMeshComponent* Comp : Comps)
				{
					if (!Comp || !VEILKit::IsEngineShape(Comp->GetStaticMesh())) { continue; }
					++Still;
					if (Named++ < 6)
					{
						UE_LOG(LogVEIL, Warning, TEXT("PHI PLACEHOLDER LEFT: %s / %s wears %s"),
							*It->GetName(), *Comp->GetName(), *Comp->GetStaticMesh()->GetName());
					}
				}
			}
			UE_LOG(LogVEIL, Warning,
				TEXT("PHI PLACEHOLDER SWAP: %d component(s) re-dressed with kit meshes, %d refused, %d engine primitive(s) still in the world - verdict %s."),
				Swapped, Refused, Still,
				Still == 0 ? TEXT("NO PLACEHOLDER IN THE FRAME") : TEXT("STILL PLACEHOLDERS"));
		}), 2.2f, false);
	}

''' + ANCHOR

if s.count(ANCHOR) != 1:
    print("anchor count=%d" % s.count(ANCHOR))
    raise SystemExit(1)
s = s.replace(ANCHOR, BLOCK.replace("\n", N), 1)
io.open(P, "w", encoding="utf-8", newline="").write(s)
print("PLACEHOLDER SWAP WRITTEN")
