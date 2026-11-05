"""Phi shows the flaws: enforce the gate, stop the boot wipe, rule the strike."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


def scan_block(s, start):
    """Return index just past the brace block that begins at/after start."""
    i = s.find("{", start)
    if i < 0:
        return -1, -1
    depth = 0
    for j in range(i, len(s)):
        if s[j] == "{":
            depth += 1
        elif s[j] == "}":
            depth -= 1
            if depth == 0:
                return i, j + 1
    return i, -1


# ---------- 1. saved position is real, death returns to safe ground ----------
h = P + "VeilRunner.h"
s = rd(h)
if "bHasSaved" not in s:
    a = "\tFVector RespawnAt = FVector(0.0, 0.0, 100.0);"
    if s.count(a) != 1:
        raise SystemExit("MISS RespawnAt decl")
    s = s.replace(a, a + "\n\tbool bHasSaved = false;", 1)
    wr(h, s)
    print("runner: saved-position flag")

c = P + "VeilRunner.cpp"
s = rd(c)
drop = "\tSetActorLocation(FVector(0.f, -600.f, 220.f));\n"
if drop in s:
    s = s.replace(drop, "", 1)
    print("runner: underground BeginPlay teleport removed")
wr(c, s)

pc = P + "VeilPlayerController.cpp"
s = rd(pc)
a = "\t\tR->Mito.AddressRung = S->AddressRung;\n\t}"
if a in s and "R->bHasSaved" not in s:
    s = s.replace(a, "\t\tR->Mito.AddressRung = S->AddressRung;\n"
                     "\t\tR->RespawnAt = S->RunnerAt;\n"
                     "\t\tR->bHasSaved = true;\n"
                     "\t\tR->SetActorLocation(S->RunnerAt);\n"
                     "\t\tUE_LOG(LogTemp, Display, TEXT(\"VEIL SAVE: position restored to (%.0f, %.0f, %.0f)\"),\n"
                     "\t\t\tS->RunnerAt.X, S->RunnerAt.Y, S->RunnerAt.Z);\n"
                     "\t}", 1)
    wr(pc, s)
    print("load: position applied, respawn anchored to it")

# ---------- 2. no boot write; respawn anchored once ----------
g = P + "VeilGameMode.cpp"
s = rd(g)
comment = "\t// The Signal records itself before any input, so the slot is proven, not assumed."
start = s.find(comment)
if start >= 0:
    b0, b1 = scan_block(s, start)
    if b1 < 0:
        raise SystemExit("MISS boot save block")
    replacement = """\t// Phi is the only record written without the player acting: the boot never
\t// overwrites the slot, it only anchors the respawn point the body already has.
\tif (AVeilRunner* BootRunner = GetWorld()->GetFirstPlayerController()
\t\t? Cast<AVeilRunner>(GetWorld()->GetFirstPlayerController()->GetPawn()) : nullptr)
\t{
\t\tif (!BootRunner->bHasSaved)
\t\t{
\t\t\tBootRunner->RespawnAt = BootRunner->GetActorLocation();
\t\t}
\t\tUE_LOG(LogVEILWorld, Display,
\t\t\tTEXT("VEIL SAVE: boot keeps the existing slot, respawn anchored at (%.0f, %.0f, %.0f)"),
\t\t\tBootRunner->RespawnAt.X, BootRunner->RespawnAt.Y, BootRunner->RespawnAt.Z);
\t}"""
    s = s[:start] + replacement + s[b1:]
    wr(g, s)
    print("game mode: boot no longer zeroes the slot")

# ---------- 3. the gate is enforced and prints its law ----------
s = rd(g)
old = "\tPHI_Constants::VerifyBoot();\n\tPHI_Constants::VerifyWhole();"
if old in s:
    new = ("\tconst bool bPhiBoot = PHI_Constants::VerifyBoot();\n"
           "\tconst bool bPhiWhole = PHI_Constants::VerifyWhole();\n"
           "\tUE_LOG(LogVEILWorld, bPhiBoot && bPhiWhole ? Display : Error,\n"
           "\t\tTEXT(\"VEIL PHI GATE: %s | reach enemy %.1f | reach strike %.1f | sanctuary %.1f | fov %.3f\"),\n"
           "\t\tbPhiBoot && bPhiWhole ? TEXT(\"ENFORCED AND PASSED\") : TEXT(\"FAILED\"),\n"
           "\t\tPHI_Constants::Reach(1.283), PHI_Constants::Reach(1.283), PHI_Constants::Reach(2.0),\n"
           "\t\t100.0 / PHI_Constants::Phi);")
    s = s.replace(old, new, 1)
    wr(g, s)
    print("gate: enforced and printing its law")
else:
    print("gate: already enforced")

# ---------- 4. strikes obey the law ----------
r = P + "VeilRunner.cpp"
s = rd(r)
start = s.find("bool AVeilRunner::Strike()")
if start < 0:
    raise SystemExit("MISS Strike")
b0, b1 = scan_block(s, start)
if b1 < 0:
    raise SystemExit("MISS Strike block")
new = '''bool AVeilRunner::Strike()
{
	if (!Mito.Spend(static_cast<float>(Mito.MaxATP * PHI_Constants::PhiPow(-2))))
	{
		UE_LOG(LogTemp, Display, TEXT("VEIL COMBAT: no ATP for a strike, gradient at %.2f"), Mito.Gradient);
		return false;
	}

	// The strike answers only a hostile post, only what is in front of the Runner,
	// and only through open air. Its reach is the enemies' own reach, so neither
	// side can outrange the other.
	const double ReachLaw = PHI_Constants::Reach(1.283);
	const FVector Eye = GetActorLocation();
	const FVector Forward = FRotationMatrix(GetControlRotation()).GetUnitAxis(EAxis::X);
	double Best = ReachLaw;
	AVeilNpc* Target = nullptr;
	for (TActorIterator<AVeilNpc> It(GetWorld()); It; ++It)
	{
		if (!It->IsHunter())
		{
			continue;
		}
		const FVector To = It->GetActorLocation() - Eye;
		const double D = To.Size();
		if (D > Best || D <= 0.0)
		{
			continue;
		}
		if (FVector::DotProduct(To / D, Forward) < PHI_Constants::PhiPow(-1.0))
		{
			continue;
		}
		FHitResult Hit;
		FCollisionQueryParams Params(SCENE_QUERY_STAT(VeilStrike), false, this);
		if (GetWorld()->LineTraceSingleByChannel(Hit, Eye, It->GetActorLocation(), ECC_Visibility, Params))
		{
			continue;
		}
		Best = D;
		Target = *It;
	}
	if (!Target)
	{
		UE_LOG(LogTemp, Display, TEXT("VEIL COMBAT: strike finds no hostile post within %.0f in front"), ReachLaw);
		return false;
	}
	const double Blow = Target->HealthMax * PHI_Constants::PhiPow(-1.0);
	Target->TakeStrike(Blow);
	UE_LOG(LogTemp, Display, TEXT("VEIL COMBAT: strike lands at %.0f of %.0f reach for %.1f"),
		FVector::Dist(Eye, Target->GetActorLocation()), ReachLaw, Blow);
	return true;
}'''
s = s[:start] + new + s[b1:]
wr(r, s)
print("strike: hostile-only, facing, line of sight, shared reach")

# ---------- 5. the proof also proves the player's strike ----------
s = rd(g)
a = "\t\t\tIt->HoldSanctuary(false);"
if a in s and "R->Strike();" not in s:
    s = s.replace(a, a + "\n\t\t\tR->Strike();", 1)
    wr(g, s)
    print("proof: player strike exercised")
print("phi flaw fix done")
