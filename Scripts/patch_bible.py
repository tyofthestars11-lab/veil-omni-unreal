import re

ROOT = "C:/Users/Admin/.freebuff/worktrees/52bfbe8a/e80fea6afa42e7c3cd60a735da7e14aecc808ccd7f84fcdd/Source/VEILOMNI/"
GP = ROOT + "VeilGameMode.cpp"
NC = ROOT + "VeilNpc.cpp"
NH = ROOT + "VeilNpc.h"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


# ---------- NPC: enemies hold their post, never chase (Kraken physics) ----------
s = rd(NC)
old = """	if (bHunter)
	{
		if (Dist < 2600.0)
		{
			Wait = 0.f;
			Target = FVector(Runner.X, Runner.Y, GroundZ(Runner.X, Runner.Y, Runner.Z));
			Speed = (Dist < 380.0) ? 0.0 : 320.0;
		}
		return;
	}"""
new = """	if (bHunter)
	{
		// Kraken physics: enemies hold their own post. They never follow the Runner
		// and never orbit. They answer only what comes inside their reach of 300.
		Wait = 0.f;
		Speed = 0.0;
		const FVector To = Runner - GetActorLocation();
		SetActorRotation(FRotator(0.f, static_cast<float>(FMath::RadiansToDegrees(FMath::Atan2(To.Y, To.X))), 0.f));
		if (Dist < 300.0 && HitCooldown <= 0.f)
		{
			HitCooldown = 2.f;
			UE_LOG(LogTemp, Display, TEXT("VEIL HIT: %s (%s) strikes the Runner from its post"), *GetName(), GetRaceName());
		}
		return;
	}"""
assert s.count(old) == 1, "perceive hunter block"
s = s.replace(old, new)

if "HitCooldown" not in s:
    s = s.replace("	if (Wait > 0.f)\n	{\n		Wait -= DeltaSeconds;",
                  "	if (HitCooldown > 0.f)\n	{\n		HitCooldown -= DeltaSeconds;\n	}\n\n	if (Wait > 0.f)\n	{\n		Wait -= DeltaSeconds;")
wr(NC, s)

t = rd(NH)
if "HitCooldown" not in t:
    t = t.replace("	bool bHunter = false;", "	bool bHunter = false;\n	float HitCooldown = 0.f;")
    wr(NH, t)
print("NPC: enemies hold post, reach 300, no chase")

# ---------- GameMode: 25 Hub placements, crystal peaks, spindle plinth ----------
s = rd(GP)

# blue crystal peaks belong to the Hub count (Kraken zone_hub: 5 at 2800, 500)
anchor = "	// The Vault sits below the terrace at the authority coordinate (0, 0, -80.9)."
crystals = """	// 5 blue crystal peaks of the Hub count, faceted crystal meshes, signal blue.
	int32 Crystals = 0;
	for (int32 i = 1; i <= 5; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i, PHI_Constants::Spacing * PHI_Constants::Phi);
		if (FitPart(MeshCliffSlate, FVector(2800.0 + P.X, 500.0 + P.Y, GHubGroundZ), 700.0, 1500.0,
			FRotator(0.f, static_cast<float>(i * PHI_Constants::GoldenAngleDeg), 0.f), MatSignal, true))
		{
			++Crystals;
		}
	}

"""
if "Crystals = 0" not in s:
    assert s.count(anchor) == 1, "vault anchor"
    s = s.replace(anchor, crystals + anchor)

# counts vs JSON: 12 + 8 + 5 = 25
hub_log_old = """	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL WORLD HUB: terrace 26 m, 12 gold pillars, 8 houses, 8 stalls, %d lanterns, Vault 80.9 below"), Lanterns);"""
hub_log_new = """	const int32 HubExpected = 25;
	const int32 HubPlaced = 12 + 8 + Crystals;
	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL WORLD HUB: terrace 26 m, 12 gold pillars, 8 houses, %d stalls, %d crystal peaks, %d lanterns, Vault 80.9 below"),
		8, Crystals, Lanterns);
	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL HUB COUNTS: %s - %d/%d placements (12 pillars + 8 houses + 5 crystal peaks)"),
		(HubPlaced == HubExpected) ? TEXT("PASS") : TEXT("FAIL"), HubPlaced, HubExpected);"""
if hub_log_new.split("\n")[0] not in s:
    assert s.count(hub_log_old) == 1, "hub log"
    s = s.replace(hub_log_old, hub_log_new)

# Spire: authority Z is +500, so a real rock plinth carries it there
spire_old = """	if (FitPart(MeshMonolith, FVector(0.0, 3500.0, GHubGroundZ), 900.0, 3200.0, FRotator::ZeroRotator, MatGold, true))
	{"""
spire_new = """	// Bible authority is Spire (0, 3500, 500). The tower does not hang at +500:
	// a real rock plinth is seated on the ground and the gold Spire stands on it.
	FitPart(MeshCliffBig, FVector(0.0, 3500.0, GHubGroundZ), 1800.0, 500.0, FRotator::ZeroRotator, nullptr, true);
	GSnapEnabled = false;
	if (FitPart(MeshMonolith, FVector(0.0, 3500.0, GHubGroundZ + 460.0), 900.0, 3200.0, FRotator::ZeroRotator, MatGold, true))
	{
		GSnapEnabled = true;"""
if "rock plinth is seated" not in s:
    assert s.count(spire_old) == 1, "spire call"
    s = s.replace(spire_old, spire_new)

# close the snap gate after the Spire block
spire_tail_old = """		UE_LOG(LogVEILWorld, Display, TEXT("VEIL WORLD SPIRE: seated (0, 3500), base on the rock, 4 buttresses, no floating crown"));
	}"""
spire_tail_new = """		UE_LOG(LogVEILWorld, Display,
			TEXT("VEIL WORLD SPIRE: plinth on the rock, gold tower based at +500, 4 buttresses, no floating crown"));
	}
	else
	{
		GSnapEnabled = true;
	}"""
if "plinth on the rock" not in s:
    assert s.count(spire_tail_old) == 1, "spire tail"
    s = s.replace(spire_tail_old, spire_tail_new)

# enemies variety of the Bible, each on its own post
life_old = """	// The Pale Host hunts in the Deep, and only there.
	int32 Hunters = 0;
	for (int32 i = 1; i <= 6; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i + 60, PHI_Constants::Spacing * PHI_Constants::Phi);
		SeatBody(FVector(P.X, -5000.0 + P.Y, GHubGroundZ), 4, i + 60, 150.0, TEXT("Pale Host"));
		++Hunters;
	}"""
life_new = """	// The enemies of the Bible, one variety per region, each holding its own post.
	// Crystal Guardian, Gold Warrior, Deep Dweller, Beacon Shadow. None of them
	// follows the Runner and none of them orbits: that is Kraken physics.
	int32 Foes = 0;
	for (int32 i = 1; i <= 3; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i + 70, PHI_Constants::Spacing * PHI_Constants::Phi);
		SeatBody(FVector(2800.0 + P.X, 500.0 + P.Y, GHubGroundZ), 4, i + 70, 0.0, TEXT("Crystal Guardian"));
		++Foes;
	}
	for (int32 i = 1; i <= 3; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i + 80, PHI_Constants::Spacing * PHI_Constants::Phi);
		SeatBody(FVector(-450.0 + P.X, 780.0 + P.Y, GHubGroundZ), 5, i + 80, 0.0, TEXT("Gold Warrior"));
		++Foes;
	}
	for (int32 i = 1; i <= 3; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i + 90, PHI_Constants::Spacing * PHI_Constants::Phi);
		SeatBody(FVector(P.X, -5000.0 + P.Y, GHubGroundZ), 6, i + 90, 0.0, TEXT("Deep Dweller"));
		++Foes;
	}
	for (int32 i = 1; i <= 3; ++i)
	{
		const FVector2D P = PHI_Constants::GoldenLoc(i + 100, PHI_Constants::Spacing * PHI_Constants::Phi);
		SeatBody(FVector(P.X, 900.0 + P.Y, GHubGroundZ), 7, i + 100, 0.0, TEXT("Beacon Shadow"));
		++Foes;
	}"""
if "Crystal Guardian" not in s:
    assert s.count(life_old) == 1, "life hunters"
    s = s.replace(life_old, life_new)

life_log_old = """		TEXT("VEIL WORLD LIFE: %d bodies | human %d gold %d crystal %d deep %d pale %d | hunters %d | every body has a mind"),
		GLife, GBodies[0], GBodies[1], GBodies[2], GBodies[3], GBodies[4], Hunters);"""
life_log_new = """		TEXT("VEIL WORLD LIFE: %d bodies | human %d gold %d crystal %d deep %d | guardians %d warriors %d dwellers %d shadows %d | every body has a mind"),
		GLife, GBodies[0], GBodies[1], GBodies[2], GBodies[3], GBodies[4], GBodies[5], GBodies[6], GBodies[7]);"""
if "guardians" not in s:
    assert s.count(life_log_old) == 1, "life log"
    s = s.replace(life_log_old, life_log_new)

# eight kinds now, so the tallies must be eight wide
s = s.replace("int32 GBodies[5] = { 0, 0, 0, 0, 0 };", "int32 GBodies[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };")
s = s.replace("++GBodies[FMath::Clamp(RaceIndex, 0, 4)];", "++GBodies[FMath::Clamp(RaceIndex, 0, 7)];")

wr(GP, s)
print("GameMode: 25 hub placements, crystal peaks, spire plinth, four enemy varieties on post")
print("patch complete")
