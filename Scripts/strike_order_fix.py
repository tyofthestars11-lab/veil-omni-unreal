"""A strike costs ATP when it lands, the save survives placement, the proof faces its target."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


def block_end(s, start):
    i = s.find("{", start)
    depth = 0
    for j in range(i, len(s)):
        if s[j] == "{":
            depth += 1
        elif s[j] == "}":
            depth -= 1
            if depth == 0:
                return j + 1
    return -1


# ---------- strike: find first, spend second ----------
p = P + "VeilRunner.cpp"
s = rd(p)
start = s.find("bool AVeilRunner::Strike()")
if start < 0:
    raise SystemExit("MISS Strike")
end = block_end(s, start)
new = '''bool AVeilRunner::Strike()
{
	// The strike answers only a hostile post, only what stands in front of the
	// Runner, and only through open air. Its reach is the enemies' own reach.
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
	// Charge follows the blow, not the attempt: a whiff costs nothing.
	if (!Mito.Spend(static_cast<float>(Mito.MaxATP * PHI_Constants::PhiPow(-2))))
	{
		UE_LOG(LogTemp, Display, TEXT("VEIL COMBAT: target in reach but no ATP, gradient %.2f"), Mito.Gradient);
		return false;
	}
	const double Blow = Target->HealthMax * PHI_Constants::PhiPow(-1.0);
	Target->TakeStrike(Blow);
	UE_LOG(LogTemp, Display, TEXT("VEIL COMBAT: strike lands at %.0f of %.0f reach for %.1f"),
		FVector::Dist(Eye, Target->GetActorLocation()), ReachLaw, Blow);
	return true;
}'''
s = s[:start] + new + s[end:]
wr(p, s)
print("strike: charged on landing, not on attempt")

# ---------- the saved position survives placement ----------
p = P + "VeilGameMode.cpp"
s = rd(p)
if "SetActorLocation(PlacedRunner->RespawnAt)" not in s:
    a = "\tPlaceRunner();\n"
    if s.count(a) != 1:
        raise SystemExit("MISS PlaceRunner call")
    s = s.replace(a, a + '''\tif (AVeilRunner* PlacedRunner = GetWorld()->GetFirstPlayerController()
\t\t? Cast<AVeilRunner>(GetWorld()->GetFirstPlayerController()->GetPawn()) : nullptr)
\t{
\t\t// Placement is the world's, unless the slot already holds a position.
\t\tif (PlacedRunner->bHasSaved)
\t\t{
\t\t\tPlacedRunner->SetActorLocation(PlacedRunner->RespawnAt);
\t\t}
\t}
''', 1)
    print("placement: saved position honoured")

# ---------- the proof posts, faces, then strikes ----------
s = rd(p)
drop = "\t\t\tIt->HoldSanctuary(false);\n\t\t\tR->Strike();\n"
if drop in s:
    s = s.replace(drop, "\t\t\tIt->HoldSanctuary(false);\n", 1)
    print("proof: strike moved after posting")
a = "\t\t\tAiTestSubject = FString(It->GetName()) + TEXT(\" (\") + It->GetRaceName() + TEXT(\")\");"
if a in s and "Strike after the body is posted" not in s:
    s = s.replace(a, a, 1)
anchor = "\t\t\t\t*AiTestSubject, R->VIT);\n\t\t\treturn;"
if anchor in s:
    s = s.replace(anchor, "\t\t\t\t*AiTestSubject, R->VIT);\n"
                             "\t\t\t// Strike after the body is posted, facing it, so the rule is tested.\n"
                             "\t\t\tif (AController* C = R->GetController())\n"
                             "\t\t\t{\n"
                             "\t\t\t\tC->SetControlRotation((It->GetActorLocation() - R->GetActorLocation()).Rotation());\n"
                             "\t\t\t}\n"
                             "\t\t\tR->Strike();\n\t\t\treturn;", 1)
    print("proof: faces the posted body before striking")
else:
    print("proof: MISS strike-after-post anchor")
wr(p, s)
print("strike order fix done")
