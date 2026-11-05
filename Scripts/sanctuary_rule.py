"""A sanctuary suppresses strikes; it never moves a body off its post."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


# 1. the rule lives on the body
h = P + "VeilNpc.h"
s = rd(h)
if "HoldSanctuary" not in s:
    a = "\tbool IsHunter() const { return bHunter; }"
    if s.count(a) != 1:
        raise SystemExit("MISS hunter getter")
    s = s.replace(a, a + "\n\t/** A sanctioned post answers nothing. The Hub is sanctuary. */\n"
                       "\tvoid HoldSanctuary(bool bHold) { bSanctuary = bHold; }", 1)
    s = s.replace("\tbool bHunter = false;", "\tbool bHunter = false;\n\tbool bSanctuary = false;", 1)
    wr(h, s)
    print("npc: sanctuary flag")

c = P + "VeilNpc.cpp"
s = rd(c)
a = "if (Dist < PHI_Constants::Reach(7.0) && HitCooldown <= 0.f)"
if "bSanctuary" not in s.split("HitCooldown <= 0.f")[0][-200:]:
    if a not in s:
        raise SystemExit("MISS strike gate")
    s = s.replace(a, "if (Dist < PHI_Constants::Reach(7.0) && HitCooldown <= 0.f && !bSanctuary)", 1)
    wr(c, s)
    print("npc: strikes suppressed inside sanctuary")

# 2. the pass counts, it does not move
g = P + "VeilGameMode.cpp"
s = rd(g)
start = s.find("void AVeilGameMode::SanctuaryPass()")
if start < 0:
    raise SystemExit("MISS sanctuary function")
depth = 0
end = -1
for i in range(s.find("{", start), len(s)):
    if s[i] == "{":
        depth += 1
    elif s[i] == "}":
        depth -= 1
        if depth == 0:
            end = i + 1
            break
if end < 0:
    raise SystemExit("MISS sanctuary end")
new = '''void AVeilGameMode::SanctuaryPass()
{
	APawn* P = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
	if (!P)
	{
		return;
	}
	const FVector Spawn = P->GetActorLocation();
	const double Hold = PHI_Constants::Reach(11.0);
	int32 Inside = 0;
	double MinD = 1.0e9;
	for (TActorIterator<AVeilNpc> It(GetWorld()); It; ++It)
	{
		if (!It->IsHunter())
		{
			continue;
		}
		const double D = FVector::Dist2D(It->GetActorLocation(), Spawn);
		const bool bInSanctuary = D < Hold;
		It->HoldSanctuary(bInSanctuary);
		if (bInSanctuary)
		{
			++Inside;
			UE_LOG(LogVEILWorld, Display,
				TEXT("VEIL SANCTUARY: %s holds a post %.0f from the Hub, inside the sanctuary of %.0f"),
				*It->GetName(), D, Hold);
		}
		MinD = FMath::Min(MinD, D);
	}
	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL SANCTUARY: %d hostile post(s) inside the Hub, strikes suppressed there, nearest post %.0f of %.0f"),
		Inside, MinD, Hold);
}'''
s = s[:start] + new + s[end:]
wr(g, s)
print("game mode: sanctuary counts instead of teleporting")

# 3. the proof body is exempt so the trigger can still be shown
a = "\t\t\tAiTestBody = *It;"
if a in s and "HoldSanctuary(false)" not in s:
    s = rd(g)
    s = s.replace(a, "\t\t\tIt->HoldSanctuary(false);\n" + a, 1)
    wr(g, s)
    print("proof: body exempt from the sanctuary")
print("sanctuary rule done")
