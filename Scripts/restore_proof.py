"""The boot rewrite removed the proof that lived inside the old boot-save block."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

if "VEIL COMBAT SELFTEST" in s:
    print("self-test already present")
else:
    anchor = s.find("\tif (AVeilRunner* BootRunner")
    if anchor < 0:
        raise SystemExit("MISS boot runner block")
    i = s.find("{", anchor)
    depth = 0
    end = -1
    for j in range(i, len(s)):
        if s[j] == "{":
            depth += 1
        elif s[j] == "}":
            depth -= 1
            if depth == 0:
                end = j + 1
                break
    if end < 0:
        raise SystemExit("MISS boot block end")

    block = '''

	// Both directions of combat are exercised on boot, so the path is proven,
	// then a hostile is posted inside its reach to prove the post answers.
	if (AVeilRunner* SelfTestRunner = GetWorld()->GetFirstPlayerController()
		? Cast<AVeilRunner>(GetWorld()->GetFirstPlayerController()->GetPawn()) : nullptr)
	{
		SelfTestRunner->RespawnAt = SelfTestRunner->GetActorLocation();
		SelfTestRunner->TakeHit(SelfTestRunner->VITMax * PHI_Constants::PhiPow(-3.0), this);
		SelfTestRunner->TakeHit(SelfTestRunner->VITMax, this);
		SelfTestRunner->Strike();
		UE_LOG(LogVEILWorld, Display,
			TEXT("VEIL COMBAT SELFTEST: runner VIT %.0f/%.0f, ATP %.0f"),
			SelfTestRunner->VIT, SelfTestRunner->VITMax, SelfTestRunner->Mito.ATP);
		SanctuaryPass();
		AiTestVitBefore = SelfTestRunner->VIT;
		GetWorldTimerManager().SetTimer(AiTestMoveTimer, this, &AVeilGameMode::AiTestMove, 3.0f, false);
		GetWorldTimerManager().SetTimer(AiTestReportTimer, this, &AVeilGameMode::AiTestReport,
			static_cast<float>(PHI_Constants::PhiPow(4.045)), false);
	}'''
    s = s[:end] + block + s[end:]
    open(p, "w", encoding="utf-8", newline="").write(s)
    print("self-test, sanctuary and AI proof scheduling restored")
