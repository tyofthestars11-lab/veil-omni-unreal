"""The definitions were skipped because the SetTimer line matched the guard."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
SIG = "void AVeilGameMode::AiTestMove()"

s = open(P, encoding="utf-8", newline="").read().replace("\r\n", "\n")
if SIG in s:
    print("definitions already present")
else:
    s += '''

void AVeilGameMode::AiTestMove()
{
	APawn* P = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
	AVeilRunner* R = Cast<AVeilRunner>(P);
	if (!R)
	{
		UE_LOG(LogVEILWorld, Display, TEXT("VEIL AI PROOF: no Runner to test against"));
		return;
	}
	for (TActorIterator<AVeilNpc> It(GetWorld()); It; ++It)
	{
		if (It->IsHunter())
		{
			It->SetActorLocation(R->GetActorLocation() + FVector(PHI_Constants::Reach(1.8), 0.0, 0.0));
			AiTestSubject = FString(It->GetName()) + TEXT(" (") + It->GetRaceName() + TEXT(")");
			UE_LOG(LogVEILWorld, Display,
				TEXT("VEIL AI PROOF: %s posted inside its own reach, Runner VIT %.0f"),
				*AiTestSubject, R->VIT);
			return;
		}
	}
	UE_LOG(LogVEILWorld, Display, TEXT("VEIL AI PROOF: no hunter found in the world"));
}

void AVeilGameMode::AiTestReport()
{
	APawn* P = GetWorld()->GetFirstPlayerController() ? GetWorld()->GetFirstPlayerController()->GetPawn() : nullptr;
	AVeilRunner* R = Cast<AVeilRunner>(P);
	if (!R)
	{
		return;
	}
	const bool bStruck = R->VIT < AiTestVitBefore - 1.0;
	UE_LOG(LogVEILWorld, Display,
		TEXT("VEIL AI PROOF: %s | VIT %.0f -> %.0f | %s"),
		AiTestSubject.IsEmpty() ? TEXT("unknown body") : *AiTestSubject,
		AiTestVitBefore, R->VIT,
		bStruck ? TEXT("STRIKE FIRED FROM ITS POST") : TEXT("NO STRIKE"));
}
'''
    open(P, "w", encoding="utf-8", newline="").write(s)
    print("definitions appended")
