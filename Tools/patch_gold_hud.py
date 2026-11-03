# -*- coding: utf-8 -*-
"""THE GOLD RIBBON PASS.

The reference art (and Tyree's own spec: "gold ribbon, curved, glowing, living UI,
gold-trimmed, full colour") is an ornate lit frame around a navy body, bars that
look like lit liquid with a sparkle at the fill line, a compass ringed with ticks,
and a gold-edged quest banner and combat plate. The canvas HUD was a rounded
grey-blue card with a straight trim.

Everything here is drawn from the primitives already in FVeilUI - no new assets,
no textures - so it holds at every resolution and cannot fault on a missing
resource.
"""
import io
import sys

P = "D:/VEIL-OMNI/Source/VEIL/Private/Game/VEILHudCanvas.cpp"
with io.open(P, "r", encoding="utf-8", newline="") as f:
    raw = f.read()
crlf = "\r\n" in raw
t = raw.replace("\r\n", "\n")
fails = []


def swap(old, new, label, allow_multi=False):
    global t
    n = t.count(old)
    if n == 0:
        fails.append("%s missing" % label)
        return
    if n > 1 and not allow_multi:
        fails.append("%s count=%d" % (label, n))
        return
    t = t.replace(old, new)


# ---------------------------------------------------------------------------
# 1. New primitives: the curl and the gold frame.
# ---------------------------------------------------------------------------
if "static void Curl(" not in t:
    swap("}\n\nstatic const TCHAR* PhiStageText(int32 Stage)",
         r'''	/** A GOLD CURL. The reference frame is drawn with flourishes that leave the panel
	 *  and taper into the air, not with a straight border. This walks a short spiral in
	 *  segments so it thins as it turns; DirX/DirY mirror it cleanly. */
	static void Curl(AHUD* H, float X, float Y, float R, float StartDeg, float SweepDeg,
		float DirX, float DirY, FLinearColor C, float Th)
	{
		if (!H) { return; }
		const int32 Steps = 18;
		float LastX = X, LastY = Y;
		for (int32 i = 0; i <= Steps; ++i)
		{
			const float T = i / (float)Steps;
			const float A = FMath::DegreesToRadians(StartDeg + SweepDeg * T);
			const float Rad = R * (1.f - 0.58f * T);
			const float PXx = X + DirX * FMath::Cos(A) * Rad;
			const float PYy = Y + DirY * FMath::Sin(A) * Rad;
			if (i > 0)
			{
				H->DrawLine(LastX, LastY, PXx, PYy,
					FLinearColor(C.R, C.G, C.B, C.A * (1.f - 0.55f * T)), FMath::Max(0.6f, Th * (1.f - 0.55f * T)));
			}
			LastX = PXx; LastY = PYy;
		}
	}

	/** AN ORNATE FRAME: halo, bright gold edge, dark bezel, hairline, navy body.
	 *  Layered like a metal fitting rather than a coloured rectangle. */
	static void GoldFrame(AHUD* H, FLinearColor Body, float X, float Y, float W, float Ht, float R)
	{
		if (!H || W <= 0.f || Ht <= 0.f) { return; }
		const FLinearColor G(1.f, 0.82f, 0.36f, 0.95f);
		const FLinearColor G2(0.80f, 0.60f, 0.22f, 0.50f);
		RoundRect(H, FLinearColor(G.R, G.G, G.B, 0.08f), X - 7.f, Y - 7.f, W + 14.f, Ht + 14.f, R + 7.f);
		RoundRect(H, FLinearColor(G.R, G.G, G.B, 0.22f), X - 4.f, Y - 4.f, W + 8.f, Ht + 8.f, R + 4.f);
		RoundRect(H, G, X - 1.5f, Y - 1.5f, W + 3.f, Ht + 3.f, R + 1.5f);
		RoundRect(H, FLinearColor(0.03f, 0.04f, 0.08f, 0.97f), X, Y, W, Ht, R);
		RoundRect(H, G2, X + 2.5f, Y + 2.5f, W - 5.f, Ht - 5.f, FMath::Max(1.f, R - 2.f));
		RoundRect(H, Body, X + 3.5f, Y + 3.5f, W - 7.f, Ht - 7.f, FMath::Max(1.f, R - 3.f));
	}

static const TCHAR* PhiStageText(int32 Stage)''', "FVeilUI helpers")

# ---------------------------------------------------------------------------
# 2. The plate becomes a gold fitting, with flourishes at both ends.
# ---------------------------------------------------------------------------
swap("\tFVeilUI::Glass(this, FLinearColor(0.055f, 0.078f, 0.141f, 0.55f), PX, PY, PW, PH, 11.f);",
     "\t// NINE LAYERS OF GOLD, NOT A GREY CARD. The reference plate is a fitting: a halo,\n"
     "\t// a bright edge, a dark bezel, a hairline and a navy body, with gold curls leaving\n"
     "\t// the corners. All of it is drawn from rows and lines, so it holds at any size.\n"
     "\tFVeilUI::GoldFrame(this, FLinearColor(0.050f, 0.072f, 0.135f, 0.66f), PX, PY, PW, PH, 11.f);\n"
     "\t{\n"
     "\t\tconst FLinearColor CurlGold(1.f, 0.83f, 0.38f, 0.80f);\n"
     "\t\tFVeilUI::Curl(this, PX + PW - 4.f, PY + 15.f, 34.f, -95.f, 120.f, 1.f, -1.f, CurlGold, 2.2f);\n"
     "\t\tFVeilUI::Curl(this, PX + PW - 4.f, PY + 15.f, 20.f, -75.f, 100.f, 1.f, -1.f,\n"
     "\t\t\tFLinearColor(CurlGold.R, CurlGold.G, CurlGold.B, 0.45f), 1.3f);\n"
     "\t\tFVeilUI::Curl(this, PX + 4.f, PY + 15.f, 34.f, -85.f, 120.f, -1.f, -1.f,\n"
     "\t\t\tFLinearColor(CurlGold.R, CurlGold.G, CurlGold.B, 0.60f), 1.9f);\n"
     "\t\tFVeilUI::Curl(this, PX + PW - 4.f, PY + PH - 12.f, 22.f, 95.f, -110.f, 1.f, 1.f,\n"
     "\t\t\tFLinearColor(CurlGold.R, CurlGold.G, CurlGold.B, 0.35f), 1.2f);\n"
     "\t}", "plate frame")

# ---------------------------------------------------------------------------
# 3. Bars: lit liquid, sparkle at the fill line.
# ---------------------------------------------------------------------------
swap("\t\tconst float R = Ht * 0.5f;\n\t\tFVeilUI::RoundRect(this, FLinearColor(0.f, 0.f, 0.f, 0.45f), X - 1.f, Yy - 1.f, Wd + 2.f, Ht + 2.f, R + 1.f);",
     "\t\tconst float R = Ht * 0.5f;\n"
     "\t\t// A HALO UNDER THE INSTRUMENT: the reference bars sit in their own light.\n"
     "\t\tFVeilUI::RoundRect(this, FLinearColor(Fill.R, Fill.G, Fill.B, 0.12f), X - 4.f, Yy - 4.f, Wd + 8.f, Ht + 8.f, R + 4.f);\n"
     "\t\tFVeilUI::RoundRect(this, FLinearColor(0.f, 0.f, 0.f, 0.50f), X - 1.f, Yy - 1.f, Wd + 2.f, Ht + 2.f, R + 1.f);",
     "bar halo")

swap('''		const float FW = FMath::Max(0.f, (Wd - 2.f) * FMath::Clamp(Pct, 0.f, 1.f));
		if (FW > 0.f)
		{
			FVeilUI::RoundRect(this, FLinearColor(Fill.R * 0.50f, Fill.G * 0.50f, Fill.B * 0.50f, 0.95f), X + 1.f, Yy + 1.f, FW, Ht - 2.f, R);
			FVeilUI::RoundRect(this, FLinearColor(Fill.R, Fill.G, Fill.B, 0.85f), X + 1.f, Yy + 1.f, FW, (Ht - 2.f) * 0.5f, R);
			if (FW > 4.f)
			{
				FVeilUI::RoundRect(this, FLinearColor(1.f, 1.f, 1.f, 0.80f), X + FW - 1.5f, Yy + 1.f, 3.f, Ht - 2.f, 1.5f);
			}
		}''',
     '''		const float FW = FMath::Max(0.f, (Wd - 2.f) * FMath::Clamp(Pct, 0.f, 1.f));
		if (FW > 0.f)
		{
			// THREE BANDS: deep base, hot body, pale crest - a lit liquid inside a trough,
			// which is what the reference's VIT and STM read as at a glance.
			FVeilUI::RoundRect(this, FLinearColor(Fill.R * 0.42f, Fill.G * 0.42f, Fill.B * 0.42f, 0.96f), X + 1.f, Yy + 1.f, FW, Ht - 2.f, R);
			FVeilUI::RoundRect(this, FLinearColor(Fill.R, Fill.G, Fill.B, 0.92f), X + 1.f, Yy + 1.f, FW, (Ht - 2.f) * 0.60f, R);
			FVeilUI::RoundRect(this, FLinearColor(FMath::Min(1.f, Fill.R + 0.30f), FMath::Min(1.f, Fill.G + 0.30f), FMath::Min(1.f, Fill.B + 0.30f), 0.55f),
				X + 1.f, Yy + 1.f, FW, (Ht - 2.f) * 0.30f, R);
			if (FW > 6.f)
			{
				// THE SPARKLE AT THE TIP, with its own halo: the one detail that makes a
				// filling bar look alive instead of printed.
				FVeilUI::Diamond(this, X + FW, Yy + Ht * 0.5f, Ht * 1.05f, FLinearColor(1.f, 1.f, 1.f, 0.10f));
				FVeilUI::Diamond(this, X + FW, Yy + Ht * 0.5f, Ht * 0.50f, FLinearColor(1.f, 0.97f, 0.90f, 0.92f));
			}
		}''', "bar fill")

# Thicker gauges, as the reference draws them.
t = t.replace("Bar(BARX, Y, BARW, 7.f, Pct, Vit);", "Bar(BARX, Y, BARW, 9.f, Pct, Vit);")
t = t.replace("Bar(BARX, Y, BARW, 7.f, Stam, FLinearColor(0.184f, 0.702f, 0.416f));",
              "Bar(BARX, Y, BARW, 9.f, Stam, FLinearColor(0.184f, 0.702f, 0.416f));")
t = t.replace("Bar(BARX, Y, BARW, 7.f, Pct, FLinearColor(0.55f, 0.80f, 1.f));",
              "Bar(BARX, Y, BARW, 9.f, Pct, FLinearColor(0.55f, 0.80f, 1.f));")

# ---------------------------------------------------------------------------
# 4. LV belongs under the phi diamond, as the reference has it.
# ---------------------------------------------------------------------------
swap("const float BadgeY = PY + 34.f;", "const float BadgeY = PY + 30.f;", "badge Y")
swap("FVeilUI::Diamond(this, BadgeC, BadgeY, 26.f, FLinearColor(1.f, 0.86f, 0.45f, 0.95f));",
     "FVeilUI::Diamond(this, BadgeC, BadgeY, 22.f, FLinearColor(1.f, 0.86f, 0.45f, 0.95f));", "badge outer")
swap("FVeilUI::Diamond(this, BadgeC, BadgeY, 20.f, FLinearColor(0.10f, 0.07f, 0.02f, 0.96f));",
     "FVeilUI::Diamond(this, BadgeC, BadgeY, 17.f, FLinearColor(0.10f, 0.07f, 0.02f, 0.96f));", "badge inner")

swap("\t\tY += 11.f;\n\t}",
     "\t\t// LV UNDER THE BADGE. The diamond is the level's house; the row belongs to XP.\n"
     "\t\tShadow(FString::Printf(TEXT(\"LV %d\"), Q->Level), FLinearColor(1.f, 0.92f, 0.62f),\n"
     "\t\t\tBadgeC - 13.f, PY + 54.f, Med, 0.52f);\n"
     "\t\tY += 11.f;\n\t}", "badge level")

# ---------------------------------------------------------------------------
# 5. Compass ticks: a dial, not a hole cut in the frame.
# ---------------------------------------------------------------------------
swap("\t\tFVeilUI::Ring(this, CCX, CCY, CR, FLinearColor(Gold.R, Gold.G, Gold.B, 0.95f), 1.6f, 72);",
     "\t\tFVeilUI::Ring(this, CCX, CCY, CR, FLinearColor(Gold.R, Gold.G, Gold.B, 0.95f), 1.6f, 72);\n"
     "\t\t// THE RUNE RING. The reference compass is ringed with ticks - long at the quarters,\n"
     "\t\t// short between - so the instrument reads as a dial instead of a plain circle.\n"
     "\t\tfor (int32 Tick = 0; Tick < 24; ++Tick)\n"
     "\t\t{\n"
     "\t\t\tconst float TA = (Tick / 24.f) * 2.f * PI - PI * 0.5f;\n"
     "\t\t\tconst bool bMajor = (Tick % 6) == 0;\n"
     "\t\t\tconst bool bMid = (Tick % 3) == 0;\n"
     "\t\t\tconst float R0 = CR + 3.f;\n"
     "\t\t\tconst float R1 = CR + (bMajor ? 12.f : bMid ? 8.f : 6.f);\n"
     "\t\t\tDrawLine(CCX + FMath::Cos(TA) * R0, CCY + FMath::Sin(TA) * R0,\n"
     "\t\t\t\tCCX + FMath::Cos(TA) * R1, CCY + FMath::Sin(TA) * R1,\n"
     "\t\t\t\tFLinearColor(Gold.R, Gold.G, Gold.B, bMajor ? 0.95f : 0.42f), bMajor ? 2.f : 1.f);\n"
     "\t\t}", "compass ticks")

# ---------------------------------------------------------------------------
# 6. The quest banner and the combat plate become gold fittings.
# ---------------------------------------------------------------------------
swap('''				const float QY = Canvas->ClipY - 70.f;      // band 3: the quest ribbon
				float QW = 0.f, QH = 0.f;
				Canvas->StrLen(Med, FString(TEXT("THE GOLDEN MIDST")), QW, QH);
				// ONE LINE, 480 PX. The band is the specification, not the text length.
				FVeilUI::Ribbon(this, FLinearColor(0.16f, 0.11f, 0.03f, 0.88f), 30.f, QY, Canvas->ClipX / (UPHIMITO_PHI_SAFE * UPHIMITO_PHI_SAFE), 21.f, true);
				DrawText(TEXT("QUEST"), FLinearColor(0.62f, 0.47f, 0.20f), 38.f, QY + 2.f, Med, 0.5f);
				DrawText(TEXT("THE GOLDEN MIDST"), FLinearColor(1.f, 0.88f, 0.50f), 76.f, QY + 2.f, Med, 0.58f);
				// The step rides on the same line, dimmer: two lines of banner was a block.
				DrawText(TEXT("Find the hidden phi chamber"), FLinearColor(0.86f, 0.82f, 0.68f),
					76.f + QW * 0.50f + 14.f, QY + 3.f, Med, 0.48f);''',
     '''				const float QY = Canvas->ClipY - 76.f;      // band 3: the quest banner
				const float QWid = Canvas->ClipX / (UPHIMITO_PHI_SAFE * UPHIMITO_PHI_SAFE);
				// A BANNER, NOT A BROWN BAR. Gold fitting around navy cloth, a phi diamond
				// bullet, the quest in gold and the step in cream beneath it - the way the
				// reference frames the work in progress.
				FVeilUI::GoldFrame(this, FLinearColor(0.045f, 0.065f, 0.125f, 0.90f), 30.f, QY, QWid, 44.f, 9.f);
				FVeilUI::Diamond(this, 48.f, QY + 22.f, 8.f, FLinearColor(1.f, 0.86f, 0.42f));
				FVeilUI::Diamond(this, 48.f, QY + 22.f, 4.f, FLinearColor(0.06f, 0.08f, 0.14f, 0.95f));
				DrawText(TEXT("QUEST"), FLinearColor(0.72f, 0.57f, 0.26f), 64.f, QY + 5.f, Med, 0.5f);
				DrawText(TEXT("THE GOLDEN MIDST"), FLinearColor(1.f, 0.90f, 0.56f), 64.f, QY + 16.f, Med, 0.62f);
				DrawText(TEXT("Find the hidden phi chamber"), FLinearColor(0.86f, 0.82f, 0.68f),
					64.f + 236.f, QY + 21.f, Med, 0.50f);''', "quest banner")

swap('''				FVeilUI::Ribbon(this, FLinearColor(0.30f, 0.18f, 0.02f, 0.92f), CCX - 88.f, CY, 176.f, 22.f, false);
				DrawText(TEXT("COMBAT MODE"), FLinearColor(1.f, 0.92f, 0.62f), CCX - 56.f, CY + 4.f, Med, 0.78f);''',
     '''				// THE COMBAT PLATE. Reference: gold fitting, dark body, a live red dot, the
				// words in warm gold. It is a state, so it should look switched on.
				FVeilUI::GoldFrame(this, FLinearColor(0.075f, 0.080f, 0.135f, 0.94f), CCX - 92.f, CY - 2.f, 184.f, 28.f, 10.f);
				FVeilUI::Diamond(this, CCX - 74.f, CY + 12.f, 4.5f, FLinearColor(0.95f, 0.20f, 0.15f, 0.95f));
				DrawText(TEXT("COMBAT MODE"), FLinearColor(1.f, 0.92f, 0.62f), CCX - 60.f, CY + 4.f, Med, 0.74f);''', "combat plate")

if fails:
    print("FAILS:", fails)
    sys.exit(1)

with io.open(P, "w", encoding="utf-8", newline="") as f:
    f.write(t.replace("\n", "\r\n") if crlf else t)

print("helpers =", "static void GoldFrame" in t)
print("plate =", "FVeilUI::GoldFrame(this, FLinearColor(0.050f" in t)
print("bars 9 =", t.count("Bar(BARX, Y, BARW, 9.f"))
print("ticks =", "THE RUNE RING" in t)
print("banner =", "band 3: the quest banner" in t)
print("combat =", "THE COMBAT PLATE" in t)
print("PATCH: PASS")
