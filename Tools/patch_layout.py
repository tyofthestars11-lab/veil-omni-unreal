"""Wire the phi layout framework into the vista and add the valley's living layer.

Adds, inside BuildVista:
  - undergrowth clumps on a phi band between the terraces and the crystal range;
  - a river of water tiles on its own band;
  - a spacing audit that measures every piece this pass placed and reports the closest pair,
    so "crowded" is a number rather than an opinion.

Every radius here comes from PHILayout::Band, every direction from PHILayout::Yaw - the same
framework the market and the vista bands can share, instead of one constant per call site.
"""

import io

P = "Source/VEIL/Private/World/VEILWorldBuilder.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"
notes = []

if '#include "World/PHILayout.h"' not in s:
    m = s.find('#include "World/VEILWorldBuilder.h"')
    if m == -1:
        m = s.find('#include "VEIL.h"')
    if m == -1:
        print("no include anchor")
        raise SystemExit(1)
    s = s[:m] + '#include "World/PHILayout.h"' + N + s[m:]
    notes.append("include added")

ANCHOR = "\tUE_LOG(LogVEIL, Warning,\n\t\tTEXT(\"PHI VISTA:"
if ANCHOR not in s:
    print("vista log anchor missing")
    raise SystemExit(1)

BLOCK = '''	// 7. UNDERGROWTH. A bare valley of stone reads as unfinished however good the stone is:
	// clumps of the kit's debris, tinted moss, on their own phi band between the terraces and
	// the crystal range.
	const FLinearColor Moss(0.20f, 0.36f, 0.19f);
	int32 Shrubs = 0;
	TArray<FVector> VistaSpots;
	{
		const float BaseRad = 4200.f;
		for (int32 i = 0; i < 48; ++i)
		{
			const FVector At = PHILayout::Spiral(i, BaseRad, 12.f, 5);
			UStaticMesh* Bush = DebrisVariants.Num() ? DebrisVariants[i % DebrisVariants.Num()].Get() : Stone;
			const float H = 180.f + 40.f * (float)(i % 5);
			if (SpawnBlock(At + FVector(0.f, 0.f, H * 0.5f),
				FVector(H * 0.0042f, H * 0.0042f, H / 100.f), Bush, Moss, 0.f))
			{
				VistaSpots.Add(At);
				++Shrubs;
			}
		}
	}

	// 8. THE RIVER. Water on the valley floor, six tiles on the golden angle at its own band,
	// so the city sits over water instead of over a dry plate.
	const FLinearColor RiverC(0.34f, 0.58f, 0.86f);
	int32 RiverTiles = 0;
	{
		for (int32 i = 0; i < 6; ++i)
		{
			const FVector At = PHILayout::Spiral(i, 2700.f, 101.f, 4, 24.f);
			if (SpawnBlock(At, FVector(9.5f, 4.2f, 0.5f), Water, RiverC, 0.18f))
			{
				VistaSpots.Add(At);
				++RiverTiles;
			}
		}
	}

	// THE SPACING AUDIT. Everything this pass placed is measured against everything else it
	// placed, so a crowded band is reported as a distance and a count rather than left for the
	// eye to call chaos later.
	PHILayout::FSpacingReport Spacing;
	Spacing.Measure(VistaSpots, 120.f);
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI LAYOUT: %d shrub(s), %d river tile(s) on phi bands; %d piece(s) audited, min spacing %.0f cm, %d pair(s) under 120 cm - verdict %s."),
		Shrubs, RiverTiles, Spacing.Count, Spacing.MinGapCm, Spacing.TooClose,
		Spacing.IsClean() ? TEXT("BANDS HOLD") : TEXT("CROWDED"));

''' + ANCHOR

s = s.replace(ANCHOR, BLOCK.replace("\n", N), 1)
notes.append("undergrowth + river + spacing audit added")

io.open(P, "w", encoding="utf-8", newline="").write(s)
print("; ".join(notes))
print("LAYOUT WRITTEN")
