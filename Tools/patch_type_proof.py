"""Prove the click-to-type path without a mouse, and let the HUD hook call it.

OnOneTextCommitted is private to the controller, so the proof hook cannot reach it. The send
logic moves into a public SubmitTypedLine(); the commit handler calls it and then closes the
field. The -phitype hook opens the field through OpenTypeToONE and sends through
SubmitTypedLine - the same two doors the click and the Enter key use.
"""

import io

fails = []

# 1. header: public send door
P1 = "Source/VEIL/Public/Game/VEILGameMode.h"
h = io.open(P1, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in h else "\n"
old = "\tbool IsTypingToONE() const { return bTypingToONE; }"
new = ("\tbool IsTypingToONE() const { return bTypingToONE; }\n"
       "\n"
       "\t/** Send a typed line to ONE. This is the door both Enter and the proof hook use,\n"
       "\t *  and it ends in UPHIONE::Say - the same call a spoken sentence makes. */\n"
       "\tvoid SubmitTypedLine(const FString& Line);")
if h.count(old) != 1:
    fails.append("header submit anchor count=%d" % h.count(old))
else:
    h = h.replace(old, new.replace("\n", NL), 1)
    io.open(P1, "w", encoding="utf-8", newline="").write(h)
    print("header: SubmitTypedLine declared")

# 2. controller: move the send logic into SubmitTypedLine
P2 = "Source/VEIL/Private/Game/VEILGameMode.cpp"
c = io.open(P2, encoding="utf-8", newline="").read()
NC = "\r\n" if "\r\n" in c else "\n"
old2 = '''void AVEILPlayerController::OnOneTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	const FString Line = Text.ToString().TrimStartAndEnd();
	bool bSent = false;
	if (CommitType == ETextCommit::OnEnter && !Line.IsEmpty())
	{
		if (UWorld* W = GetWorld())
		{
			if (UPHIONE* One = W->GetSubsystem<UPHIONE>())
			{
				One->Say(Line);
				bSent = true;
				UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE: sent to ONE - %s"), *Line);
			}
		}
	}
	CloseTypeToONE();
	UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE: field closed, sent=%d, chars=%d."),
		bSent ? 1 : 0, Line.Len());
}'''
new2 = '''void AVEILPlayerController::SubmitTypedLine(const FString& Line)
{
	const FString Clean = Line.TrimStartAndEnd();
	if (Clean.IsEmpty())
	{
		UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE: empty line, nothing sent."));
		return;
	}
	if (UWorld* W = GetWorld())
	{
		if (UPHIONE* One = W->GetSubsystem<UPHIONE>())
		{
			One->Say(Clean);
			UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE: sent to ONE - %s (%d chars)"), *Clean, Clean.Len());
			return;
		}
	}
	UE_LOG(LogVEIL, Error, TEXT("PHI TYPE: ONE subsystem is missing, the line went nowhere."));
}

void AVEILPlayerController::OnOneTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
	if (CommitType == ETextCommit::OnEnter)
	{
		SubmitTypedLine(Text.ToString());
	}
	CloseTypeToONE();
	UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE: field closed after %s."),
		CommitType == ETextCommit::OnEnter ? TEXT("Enter") : TEXT("focus loss"));
}'''
if c.count(old2.replace("\n", NC)) != 1:
    fails.append("commit body anchor count=%d" % c.count(old2.replace("\n", NC)))
else:
    c = c.replace(old2.replace("\n", NC), new2.replace("\n", NC), 1)
    print("controller: SubmitTypedLine extracted")

# 3. the proof hook, before the -phione beat
anchor = "\t// -phione: speaks to ONE through the same Say() a spoken sentence reaches,"
hook = '''	// -phitype: THE CLICK, WITHOUT A MOUSE. The proof run opens the field through
	// OpenTypeToONE - the click handler's own door - and sends through SubmitTypedLine, the
	// door Enter uses. Nothing here bypasses the field; it exercises it.
	if (FParse::Param(FCommandLine::Get(), TEXT("phitype")))
	{
		FTimerHandle TypeOpen;
		GetWorldTimerManager().SetTimer(TypeOpen, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (UWorld* W = GetWorld())
			{
				if (AVEILPlayerController* TypePC = Cast<AVEILPlayerController>(W->GetFirstPlayerController()))
				{
					TypePC->OpenTypeToONE();
					UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE PROOF: field opened through OpenTypeToONE."));
				}
			}
		}), 4.f, false);

		FTimerHandle TypeSend;
		GetWorldTimerManager().SetTimer(TypeSend, FTimerDelegate::CreateWeakLambda(this, [this]()
		{
			if (UWorld* W = GetWorld())
			{
				if (AVEILPlayerController* TypePC = Cast<AVEILPlayerController>(W->GetFirstPlayerController()))
				{
					TypePC->SubmitTypedLine(TEXT("fly me to the keeper"));
					TypePC->CloseTypeToONE();
					UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE PROOF: line submitted and field closed."));
				}
			}
		}), 7.f, false);
	}

'''
if c.count(anchor) != 1:
    fails.append("phione anchor count=%d" % c.count(anchor))
else:
    c = c.replace(anchor, (hook + anchor).replace("\n", NC), 1)
    print("controller/gamemode: -phitype hook added")

io.open(P2, "w", encoding="utf-8", newline="").write(c)

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("PROOF HOOK APPLIED")
