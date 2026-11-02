import io

ROOT = r"D:\VEIL-OMNI" + "\\"

def rd(p):
    # universal newlines: patterns written with \n match CRLF files
    return io.open(ROOT + p, encoding="utf-8", newline=None).read()

def wr(p, s):
    io.open(ROOT + p, "w", encoding="utf-8", newline="\r\n").write(s)

# ---------------- PHIONE.cpp: complete anything the first pass missed ----------------
p = "Source/VEIL/Private/Systems/PHIONE.cpp"
t = rd(p)
changed = False

if "void UPHIONE::BeginTalk" not in t:
    TALK = r'''void UPHIONE::BeginTalk()
{
	UWorld* W = GetWorld();
	if (!W || bTalking) { return; }
	// Never talk over itself: if ONE is mid-sentence the key is ignored rather
	// than cutting the reply off half-spoken.
	if (State == EONEState::Speaking) { return; }

	// The microphone is opened HERE and only here. If the machine has no capture
	// device the state still changes and the hold is still measured - the honest
	// answer is "held, but I could not open a microphone", not silence.
	if (!Voice.IsValid() && FVoiceModule::IsAvailable())
	{
		Voice = FVoiceModule::Get().CreateVoiceCapture(FString(), 16000, 1);
	}
	bMicOpened = false;
	if (Voice.IsValid())
	{
		Voice->Start();
		bMicOpened = Voice->IsCapturing();
	}

	bTalking = true;
	TalkStart = W->GetTimeSeconds();
	State = EONEState::Listening;
	Reply = TEXT("Listening - let go to send.");
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI ONE: TALK DOWN. microphone %s. Nothing is captured before this key goes down."),
		bMicOpened ? TEXT("open") : TEXT("could not be opened"));
}

void UPHIONE::EndTalk()
{
	if (!bTalking) { return; }
	UWorld* W = GetWorld();
	LastHeld = W ? FMath::Max(0.f, W->GetTimeSeconds() - TalkStart) : 0.f;
	bTalking = false;

	if (Voice.IsValid() && bMicOpened) { Voice->Stop(); }
	const bool bHadMic = bMicOpened;
	bMicOpened = false;

	State = EONEState::Ready;
	// HONEST: this project has no speech-to-text wired in, so the hold is measured
	// and reported instead of pretending a sentence was understood and acted on.
	Reply = bHadMic
		? FString::Printf(TEXT("Heard %.1f s. No words yet - nothing here turns sound into text."), LastHeld)
		: TEXT("No microphone on this machine.");
	WriteLedger(TEXT("[push-to-talk] key held"), Reply, TEXT("voice"), RungOf(FMath::Max(LastHeld, 0.01f)));
	UE_LOG(LogVEIL, Warning,
		TEXT("PHI ONE: TALK UP. held %.2f s, microphone opened=%d. No transcriber is wired, so no skill fired."),
		LastHeld, bHadMic ? 1 : 0);
}

void UPHIONE::Speak(const FString& InReply, const FString& InHand)'''
    t = t.replace("void UPHIONE::Speak(const FString& InReply, const FString& InHand)", TALK, 1)
    changed = True

if "TalkStart > 20.f" not in t:
    t = t.replace(
        "void UPHIONE::Tick(float DeltaTime)\n{\n\tSuper::Tick(DeltaTime);\n",
        "void UPHIONE::Tick(float DeltaTime)\n{\n\tSuper::Tick(DeltaTime);\n\n"
        "\t// A key held and never let go (alt-tab, unplugged controller) must not hold the\n"
        "\t// microphone open forever. Twenty seconds is a ceiling on one breath, not a\n"
        "\t// conversation limit.\n"
        "\tif (bTalking && GetWorld() && GetWorld()->GetTimeSeconds() - TalkStart > 20.f)\n"
        "\t{\n\t\tEndTalk();\n\t}\n", 1)
    changed = True

if "void UPHIONE::StartTalk" in t:
    t = t.replace("void UPHIONE::StartTalk", "void UPHIONE::BeginTalk", 1)

if "one.talk" not in t:
    CMD = r'''static FAutoConsoleCommandWithWorldAndArgs GOneTalk(
	TEXT("one.talk"),
	TEXT("Push to talk: one.talk start | one.talk stop"),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		UPHIONE* One = World ? World->GetSubsystem<UPHIONE>() : nullptr;
		if (!One) { return; }
		const FString What = Args.Num() > 0 ? Args[0].ToLower() : TEXT("start");
		if (What == TEXT("stop")) { One->EndTalk(); } else { One->BeginTalk(); }
		UE_LOG(LogVEIL, Warning, TEXT("PHI ONE STATE: %s  talking=%d"),
			*One->StateLine(), One->IsTalking() ? 1 : 0);
	}));

static FAutoConsoleCommandWithWorldAndArgs GOneState('''
    if "static FAutoConsoleCommandWithWorldAndArgs GOneState(" in t:
        t = t.replace("static FAutoConsoleCommandWithWorldAndArgs GOneState(", CMD, 1)
        changed = True

if "ready - hold LeftAlt" in t:
    t = t.replace("ready - hold LeftAlt", "ready - hold B")
    changed = True
if "awake, ready - hold LeftAlt" in t:
    t = t.replace("hold LeftAlt to speak", "hold B to speak")
    changed = True
if "State == EONEState::Ready ? TEXT(\"ready" not in t:
    t = t.replace(
        "\tconst TCHAR* StateWord = State == EONEState::Listening ? TEXT(\"listening\")\n"
        "\t\t: State == EONEState::Thinking ? TEXT(\"working\") : TEXT(\"speaking\");",
        "\tconst TCHAR* StateWord = State == EONEState::Ready ? TEXT(\"ready - hold B to speak\")\n"
        "\t\t: State == EONEState::Listening ? TEXT(\"listening\")\n"
        "\t\t: State == EONEState::Thinking ? TEXT(\"working\") : TEXT(\"speaking\");", 1)
    changed = True

wr(p, t)
print("PHIONE.cpp: BeginTalk present =", "void UPHIONE::BeginTalk" in t,
      "| tick guard =", "TalkStart > 20.f" in t,
      "| one.talk =", "one.talk" in t,
      "| ready line =", "ready - hold B" in t)

# ---------------- Character: key B, not LeftAlt ----------------
p = "Source/VEIL/Private/Character/VEILCharacter.cpp"
t = rd(p)
t = t.replace(
    "\t// PUSH TO TALK: LeftAlt, held. ONE's microphone is open only between these\n"
    "\t// two events - there is no always-listening mode to switch off.\n"
    "\tPlayerInputComponent->BindKey(EKeys::LeftAlt, IE_Pressed, this, &AVEILCharacter::OneTalkPressed);\n"
    "\tPlayerInputComponent->BindKey(EKeys::LeftAlt, IE_Released, this, &AVEILCharacter::OneTalkReleased);",
    "\t// PUSH TO TALK: B, held. ONE's microphone is open only between these two\n"
    "\t// events - there is no always-listening mode to switch off. LeftAlt is the\n"
    "\t// dodge, so talking could never share it.\n"
    "\tPlayerInputComponent->BindKey(EKeys::B, IE_Pressed, this, &AVEILCharacter::OneTalkPressed);\n"
    "\tPlayerInputComponent->BindKey(EKeys::B, IE_Released, this, &AVEILCharacter::OneTalkReleased);", 1)
wr(p, t)
print("VEILCharacter.cpp: talk is B, LeftAlt left to the dodge")

ch = "Source/VEIL/Public/Character/VEILCharacter.h"
t = rd(ch)
t = t.replace("/** PUSH TO TALK: LeftAlt opens ONE's microphone while held, closes it on release. */",
              "/** PUSH TO TALK: B opens ONE's microphone while held, closes it on release. */")
wr(ch, t)

# ---------------- HUD orb ----------------
p = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
t = rd(p)
old = ("\t\t\tconst FLinearColor Orb = bSpeaking ? FLinearColor(1.f, 0.85f, 0.35f)\n"
       "\t\t\t\t: bWorking ? FLinearColor(0.45f, 0.95f, 0.85f)\n"
       "\t\t\t\t: FLinearColor(0.52f, 0.60f, 0.76f);")
new = ("\t\t\t// Ready is deliberately dim: the dot only lights while the key is\n"
       "\t\t\t// actually down, so \"listening\" can never be shown when it is not.\n"
       "\t\t\tconst bool bHearing = One->GetState() == EONEState::Listening;\n"
       "\t\t\tconst FLinearColor Orb = bSpeaking ? FLinearColor(1.f, 0.85f, 0.35f)\n"
       "\t\t\t\t: bWorking ? FLinearColor(0.45f, 0.95f, 0.85f)\n"
       "\t\t\t\t: bHearing ? FLinearColor(0.85f, 0.92f, 1.f)\n"
       "\t\t\t\t: FLinearColor(0.30f, 0.34f, 0.44f);")
if old in t:
    t = t.replace(old, new, 1)
    wr(p, t)
    print("HUD: orb lights only while the key is held")
else:
    print("HUD: ORB PATTERN MISSED AGAIN")
