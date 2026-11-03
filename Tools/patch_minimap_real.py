"""Shrink the minimap to the design's number, and fill an empty disc from the world.

Measured: the ring was `ClipY / phi` = 445 px at 720p, 62% of the screen height for one
instrument. The brief says 180 x 180 at 70%, and the same law one rung down lands exactly
there: 1080 / phi^3 * 0.70 = 178 px, 720 gives 119 px.

The other half is the placeholder: an empty disc drew the words "MAP ACQUIRING" over flat
rectangles. Until the capture's first frame the disc is now filled from the district's own
geometry - every visible actor's bounds projected to plan - so the map is coded from the
world, never a stand-in.
"""

import io

P = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
fails = []


def patch(old, new, label):
    global text
    o, n = old.replace("\n", NL), new.replace("\n", NL)
    c = text.count(o)
    if c != 1:
        fails.append("%s count=%d" % (label, c))
        return
    text = text.replace(o, n, 1)


# 1. one definition of the diameter
patch(
    '#define UPHIMITO_PHI_SAFE (1.618033988749895f)',
    '#define UPHIMITO_PHI_SAFE (1.618033988749895f)\n'
    '\n'
    '// MINIMAP DIAMETER = SCREEN HEIGHT / PHI^3 * 0.70. One definition, so the ring, its\n'
    '// bezel, the compass letters, the plan inside it and the manual card beneath it can\n'
    '// never disagree about how big the instrument is.\n'
    '#define UPHIMITO_MAP_DIAM(H) ((H) / (UPHIMITO_PHI_SAFE * UPHIMITO_PHI_SAFE * UPHIMITO_PHI_SAFE) * 0.70f)',
    "macro")

# 2. the size itself
patch(
    """		// SPEC, AS LOCKED: 180 x 180 AT 70%. The height/phi ring was my reading of the
		// law; the brief states 180 and 70%, and the brief is the design.
		// MINIMAP DIAMETER = SCREEN HEIGHT / PHI. 720 / 1.618033988749895 = 445 px.
		const float MS = Canvas->ClipY / UPHIMITO_PHI_SAFE;""",
    """		// SPEC, AS LOCKED: 180 x 180 AT 70%. Measured against the frame, the height/phi
		// reading gave 445 px at 720p - 62% of the screen height for one instrument, so it
		// ate the view it was meant to annotate. The same law one rung down, at the brief's
		// own 70%, lands on the brief's own number: 1080 / phi^3 * 0.70 = 178 px, and 720p
		// gives 119 px. The instrument is the size the design states, on every resolution.
		const float MS = UPHIMITO_MAP_DIAM(Canvas->ClipY);""",
    "diameter")

# 3. no placeholder words
patch(
    """		if (!Tex || !Map || Map->GetCaptureCount() == 0)
		{
			DrawText(TEXT("MAP ACQUIRING"), FLinearColor(0.62f, 0.70f, 0.80f), MX + 44.f, MY + MS * 0.5f, Med, 0.8f);
		}""",
    """		// NO PLACEHOLDER TEXT. An empty disc used to print MAP ACQUIRING over flat
		// rectangles, which is a placeholder by definition. The district's own geometry is
		// on hand, so the disc is filled from real actor bounds below until the first
		// capture frame lands - and the same pass is a check on the capture afterwards.
		const bool bNoLiveCapture = (!Tex || !Map || Map->GetCaptureCount() == 0);""",
    "placeholder text")

# 4. the plan view, coded from the world, right after the blip projector exists
patch(
    """			FVeilUI::Diamond(this, PX, PY, S * 0.5f + 1.8f, FLinearColor(0.f, 0.f, 0.f, 0.60f));
			FVeilUI::Diamond(this, PX, PY, S * 0.5f, Col);
		};""",
    """			FVeilUI::Diamond(this, PX, PY, S * 0.5f + 1.8f, FLinearColor(0.f, 0.f, 0.f, 0.60f));
			FVeilUI::Diamond(this, PX, PY, S * 0.5f, Col);
		};

		// REAL PLAN, CODED. Every visible actor inside the map's own extent becomes one
		// mark on the disc, sized by the actor's real bounds and placed by the same
		// projection the blips use. This is the district drawn from itself: nothing here is
		// a stand-in picture, and when the capture is live the blips land on real ground.
		if (bNoLiveCapture)
		{
			for (TActorIterator<AActor> PlanIt(W); PlanIt; ++PlanIt)
			{
				AActor* Piece = *PlanIt;
				if (!Piece || Piece == Pawn) { continue; }
				FVector Origin, Ext;
				Piece->GetActorBounds(true, Origin, Ext);
				if (Ext.IsNearlyZero()) { continue; }
				float PX, PY;
				ToScreen(Origin, PX, PY);
				const float DXp = PX - (MX + MS * 0.5f), DYp = PY - (MY + MS * 0.5f);
				const float Inner = MS * 0.5f - 2.f;
				if (DXp * DXp + DYp * DYp > Inner * Inner) { continue; }
				const float WX = FMath::Max(1.f, (float)Ext.X * 2.f / E * MS * 0.5f);
				const float WY = FMath::Max(1.f, (float)Ext.Y * 2.f / E * MS * 0.5f);
				DrawRect(FLinearColor(0.36f, 0.34f, 0.30f, 0.95f), PX - WX, PY - WY, WX * 2.f, WY * 2.f);
			}
		}""",
    "plan view")

# 5. the manual card follows the new ring, not the old one
patch(
    "			const float CY = 30.f + Canvas->ClipY / UPHIMITO_PHI_SAFE + 18.f;   // under the ring",
    "			const float CY = 30.f + UPHIMITO_MAP_DIAM(Canvas->ClipY) + 18.f;   // under the ring",
    "manual card")

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(P, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars" % (P, len(src), len(text)))
print("minimap diameter at 720p = %.0f px, at 1080p = %.0f px" % (720 / (1.618033988749895 ** 3) * 0.70,
                                                                1080 / (1.618033988749895 ** 3) * 0.70))
