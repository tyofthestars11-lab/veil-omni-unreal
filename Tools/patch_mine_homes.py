import io

ROOT = r"D:\VEIL-OMNI" + "\\"
P = "Source/VEIL/Private/World/VEILWilds.cpp"

def rd(p):
    return io.open(ROOT + p, encoding="utf-8", newline=None).read()

def wr(p, s):
    io.open(ROOT + p, "w", encoding="utf-8", newline="\r\n").write(s)

t = rd(P)

# ---- 1. homes table, after the mine book count ----
anchor = "\tconstexpr int32 GMineBookCount = UE_ARRAY_COUNT(GMineBook);"
HOMES = '''	constexpr int32 GMineBookCount = UE_ARRAY_COUNT(GMineBook);

	/**
	 * EVERY CLAIM SITS ON THE GROUND OF THE ONE WHO CLAIMS IT.
	 *
	 * The centres are the region anchors the rest of the game already uses (the
	 * Keeper at 900,0; the Seer at -450,780; the Spire's fragment at 2800,500;
	 * Omni at 600,-2100; the Vault at 0,0). The radius is how far inside the
	 * owner's reach a seam can sit. The Warden's ground is the Deep's inner disc;
	 * the Hollow works the rim around it, which is where they were first heard.
	 */
	struct FWileHome
	{
		const TCHAR* Owner;
		FVector2D Center;
		float RadiusCm;
	};

	const FWileHome GMineHomes[] =
	{
		{ TEXT("Keeper's Compact"),     FVector2D(  900.f,     0.f), 2600.f },
		{ TEXT("Seer's Nest"),          FVector2D( -450.f,   780.f), 2600.f },
		{ TEXT("Cartographer's Guild"), FVector2D( 2800.f,   500.f), 2600.f },
		{ TEXT("Countess"),             FVector2D(  600.f, -2100.f), 2600.f },
		{ TEXT("the Warden"),           FVector2D(    0.f,     0.f),  620.f },
		{ TEXT("the Hollow"),           FVector2D(    0.f,     0.f), 1250.f },
	};
	constexpr int32 GMineHomeCount = UE_ARRAY_COUNT(GMineHomes);'''
if "GMineHomes" not in t:
    t = t.replace(anchor, HOMES, 1)
    print("cpp: mine homes table (6 owners, region anchors)")
else:
    print("cpp: homes already present")

# ---- 2. placement loop ----
old = '''	for (int32 m = 0; m < Mines_; ++m)
	{
		const float Angle = FMath::DegreesToRadians(m * WildsGoldenDeg + 200.f);
		const float Radius = 150000.f + 14000.f * m;
		const float X = FMath::Cos(Angle) * Radius;
		const float Y = FMath::Sin(Angle) * Radius;
		float Z = 0.f;
		if (!GroundAt(X, Y, Z)) { continue; }'''

new = '''	for (int32 m = 0; m < Mines_; ++m)
	{
		const FWileSpec& Spec = GMineBook[m % GMineBookCount];

		// Whose ground is it? The claim is placed inside THAT owner's reach, so a
		// mine is never found on someone else's land by accident of a spiral step.
		const FWileHome* Home = nullptr;
		for (int32 h = 0; h < GMineHomeCount; ++h)
		{
			if (FCString::Strcmp(GMineHomes[h].Owner, Spec.Owner) == 0) { Home = &GMineHomes[h]; break; }
		}
		if (!Home) { continue; }

		const float Angle = FMath::DegreesToRadians(m * WildsGoldenDeg);
		const float Ring = Home->RadiusCm * (0.55f + 0.45f * FMath::Frac(m * 0.6180339887f));
		const float X = Home->Center.X + FMath::Cos(Angle) * Ring;
		const float Y = Home->Center.Y + FMath::Sin(Angle) * Ring;
		float Z = 0.f;
		if (!GroundAt(X, Y, Z)) { continue; }'''

if old in t:
    t = t.replace(old, new, 1)
    print("cpp: mines placed on their owner's ground")
else:
    print("cpp: PLACEMENT PATTERN MISSED")

# ---- 3. name the ground in the spawn line ----
old_log = '''		Node->Init(m);
		++SpawnedMines;'''
new_log = '''		Node->Init(m);
		UE_LOG(LogVEIL, Log, TEXT("PHI MINE %02d %s stands %.0f cm inside %s ground at %s."),
			m, *Node->MineName(), Ring, *Node->MineOwner(), *Node->GetActorLocation().ToCompactString());
		++SpawnedMines;'''
if old_log in t:
    t = t.replace(old_log, new_log, 1)
    print("cpp: ground named in the spawn log")
else:
    print("cpp: SPAWN LOG PATTERN MISSED")

t = t.replace(
    "TEXT(\"PHI WILDS: %d species bodies in %d camps, %d mines, all on phi clocks (1.5-5 km out).\"),",
    "TEXT(\"PHI WILDS: %d species bodies in %d camps, %d named mines on their owners' ground, all on phi clocks.\"),", 1)

wr(P, t)
print("done")
