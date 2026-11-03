"""The locked 2026-11-01 HUD, applied as written.

objective 420x110 top-left (already), minimap 180x180 at 70% opacity top-right, manual
320x90 that fades out after six seconds, one-line quest band 480 px, panels at 55%,
centre 60% kept clear, and no blue blips on the map - the fragment marker was cyan and
the Remnant marker was a blue-white.

Each change is a stated number from the brief, not a taste call.
"""

import io

P = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
fails = []


def rep(old, new, label, count=1):
    global text
    o, n = old.replace("\n", NL), new.replace("\n", NL)
    c = text.count(o)
    if c != count:
        fails.append("%s: count=%d want %d" % (label, c, count))
        return
    text = text.replace(o, n, count)


# 1. Minimap: 180 x 180 at 70% opacity.
rep("\t\t// MINIMAP DIAMETER = SCREEN HEIGHT / PHI. 1080 / 1.618033988749895 = 667 px,\n"
    "\t\t// 720 / phi = 445 px - the ring is sized by the law, not by a guess.\n"
    "\t\tconst float MS = Canvas->ClipY / UPHIMITO_PHI_SAFE;",
    "\t\t// SPEC, AS LOCKED: 180 x 180 AT 70%. The height/phi ring was my reading of the\n"
    "\t\t// law; the brief states 180 and 70%, and the brief is the design.\n"
    "\t\tconst float MS = 180.f;",
    "minimap size")

rep("\t\t\t\t\tFLinearColor::White, BLEND_Opaque);",
    "\t\t\t\t\tFLinearColor(1.f, 1.f, 1.f, 0.70f), BLEND_Translucent);",
    "minimap opacity")

# 2. Real marker colours: no cyan fragment, no blue-white Remnant.
rep("\t\tfor (TActorIterator<AOMNIFragment> It(W); It; ++It) { Blip(It->GetActorLocation(), FLinearColor(0.35f, 0.95f, 1.f), 6.f); }",
    "\t\t// FRAGMENTS ARE GOLD PHI SHARDS, NOT CYAN BLOBS. The blue marker read as a\n"
    "\t\t// placeholder on a gold instrument.\n"
    "\t\tfor (TActorIterator<AOMNIFragment> It(W); It; ++It) { Blip(It->GetActorLocation(), FLinearColor(1.f, 0.78f, 0.30f), 6.f); }",
    "fragment colour")

rep("\t\t\t\t: It->Faction == EVEILFaction::Remnant ? FLinearColor(0.75f, 0.75f, 1.f)",
    "\t\t\t\t: It->Faction == EVEILFaction::Remnant ? FLinearColor(1.f, 0.70f, 0.35f)",
    "remnant colour")

# 3. Panels at 55%.
rep("FVeilUI::Glass(this, FLinearColor(0.055f, 0.078f, 0.141f, 0.85f), PX, PY, PW, PH, 11.f);",
    "FVeilUI::Glass(this, FLinearColor(0.055f, 0.078f, 0.141f, 0.55f), PX, PY, PW, PH, 11.f);",
    "plate 55%")

# 4. Manual card: 320 x 90, fades after six seconds, sits under the 180 px ring.
rep("\t\t\tconst float CW = 320.f;",
    "\t\t\t// SIX SECONDS, THEN IT STANDS DOWN. The card teaches one gesture and then\n"
    "\t\t\t// stops being a window on the world; it used to sit there all run.\n"
    "\t\t\tstatic int32 CardStep = -1;\n"
    "\t\t\tstatic float CardClock = 0.f;\n"
    "\t\t\tif (Tut->GetStepIndex() != CardStep) { CardStep = Tut->GetStepIndex(); CardClock = 0.f; }\n"
    "\t\t\tCardClock += W->GetDeltaSeconds();\n"
    "\t\t\tconst float CardA = CardClock < 6.f ? 1.f : FMath::Max(0.f, 1.f - (CardClock - 6.f) / 1.5f);\n"
    "\t\t\tconst float CW = 320.f;",
    "card fade clock")

rep("\t\tif (!Tut->IsHidden() && !bPageOpen && !bTalking)",
    "\t\tif (!Tut->IsHidden() && !bPageOpen && !bTalking && CardA > 0.02f)",
    "card visibility")

rep("const float CY = 30.f + Canvas->ClipY / UPHIMITO_PHI_SAFE + 18.f;   // under the ring",
    "const float CY = 30.f + 180.f + 18.f;   // under the 180 px ring",
    "card position")

rep("FVeilUI::Glass(this, FLinearColor(0.022f, 0.030f, 0.056f, 0.85f), CX, CY, CW, CH, 12.f);",
    "FVeilUI::Glass(this, FLinearColor(0.022f, 0.030f, 0.056f, 0.55f * CardA), CX, CY, CW, CH, 12.f);",
    "card 55% + fade")

# 5. One-line quest band, 480 px.
rep("FVeilUI::Ribbon(this, FLinearColor(0.16f, 0.11f, 0.03f, 0.88f), 30.f, QY, QW * 0.86f + 46.f, 15.f, true);",
    "// ONE LINE, 480 PX. The band is the specification, not the text length.\n"
    "\t\t\t\tFVeilUI::Ribbon(this, FLinearColor(0.16f, 0.11f, 0.03f, 0.88f), 30.f, QY, 480.f, 21.f, true);",
    "quest band 480")

# 6. Centre 60% stays clear of world labels.
rep("\t\t\t\tif (S.X < Canvas->ClipX * 0.46f && S.Y < Canvas->ClipY * 0.46f) { return; }",
    "\t\t\t\tif (S.X < Canvas->ClipX * 0.46f && S.Y < Canvas->ClipY * 0.46f) { return; }\n"
    "\t\t\t\t// THE CENTRE 60% IS THE WORLD'S. A name plate in the middle of the shot is\n"
    "\t\t\t\t// the one thing the brief forbids outright.\n"
    "\t\t\t\tif (S.X > Canvas->ClipX * 0.20f && S.X < Canvas->ClipX * 0.80f\n"
    "\t\t\t\t\t&& S.Y > Canvas->ClipY * 0.20f && S.Y < Canvas->ClipY * 0.80f) { return; }",
    "centre clear")

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(P, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars (locked HUD)" % (P, len(src), len(text)))
