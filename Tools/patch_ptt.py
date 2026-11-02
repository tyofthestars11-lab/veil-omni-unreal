import io

ROOT = r"D:\VEIL-OMNI" + "\\"

def rd(p):
    return io.open(ROOT + p, encoding="utf-8", newline="").read()

def wr(p, s):
    io.open(ROOT + p, "w", encoding="utf-8", newline="").write(s)

# ---------------- Build.cs: Voice module ----------------
p = "Source/VEIL/VEIL.Build.cs"
s = rd(p)
if '"Voice"' not in s:
    s = s.replace('"ProceduralMeshComponent",', '"ProceduralMeshComponent",\n\t\t\t"Voice",', 1)
    wr(p, s)
    print("build.cs: Voice module added")
else:
    print("build.cs: Voice already there")

# ---------------- PHIONE.h ----------------
p = "Source/VEIL/Public/Systems/PHIONE.h"
t = rd(p)
t = t.replace(
    "enum class EONEState : uint8\n{\n\tListening,",
    "enum class EONEState : uint8\n{\n"
    "\t/** Awake and available, microphone CLOSED. This is where ONE lives by default. */\n"
    "\tReady,\n"
    "\t/** Only while the talk key is held: the microphone is open. */\n"
    "\tListening,", 1)

t = t.replace(
    "\t/** Speak an intent. ONE answers in one breath. True if a skill fired. */\n"
    "\tbool Say(const FString& Utterance);",
    "\t/** Speak an intent. ONE answers in one breath. True if a skill fired. */\n"
    "\tbool Say(const FString& Utterance);\n\n"
    "\t/** PUSH TO TALK. ONE is not always listening. The microphone opens while the\n"
    "\t *  key is held and closes the instant it is let go; between presses the state\n"
    "\t *  is Ready, not Listening. Always available, never always listening. */\n"
    "\tvoid BeginTalk();\n"
    "\tvoid EndTalk();\n"
    "\tbool IsTalking() const { return bTalking; }\n"
    "\tfloat LastTalkSeconds() const { return LastHeld; }", 1)

t = t.replace("\tEONEState State = EONEState::Listening;", "\tEONEState State = EONEState::Ready;", 1)

t = t.replace(
    "\t/** The flight ONE is currently driving, if any. */",
    "\t/** Push-to-talk state: captured only while the key is down. */\n"
    "\tbool bTalking = false;\n"
    "\tfloat TalkStart = 0.f;\n"
    "\tfloat LastHeld = 0.f;\n"
    "\tbool bMicOpened = false;\n"
    "\tTSharedPtr<class IVoiceCapture> Voice;\n\n"
    "\t/** The flight ONE is currently driving, if any. */", 1)
wr(p, t)
print("PHIONE.h: Ready state + push-to-talk surface")

# ---------------- PHIONE.cpp ----------------
p = "Source/VEIL/Private/Systems/PHIONE.cpp"
t = rd(p)

t = t.replace('#include "Misc/Paths.h"', '#include "Misc/Paths.h"\n#include "VoiceModule.h"', 1)

t = t.replace(
    "\tState = EONEState::Listening;\n\tUE_LOG(LogVEIL, Warning, TEXT(\"PHI ONE: awake.",
    "\tState = EONEState::Ready;\n\tUE_LOG(LogVEIL, Warning, TEXT(\"PHI ONE: awake, ready - hold LeftAlt to speak.", 1)

t = t.replace(
    "void UPHIONE::Tick(float DeltaTime)\n{\n\tSuper::Tick(DeltaTime);\n",
    "void UPHIONE::Tick(float DeltaTime)\n{\n\tSuper::Tick(DeltaTime);\n\n"
    "\t// A key held and never let go (alt-tab, unplugged controller) must not hold the\n"
    "\t// microphone open forever. Twenty seconds is a ceiling on one breath, not a\n"
    "\t// conversation limit.\n"
    "\tif (bTalking && GetWorld() && GetWorld()->GetTimeSeconds() - TalkStart > 20.f)\n"
    "\t{\n\t\tEndTalk();\n\t}\n", 1)

t = t.replace(
    "\tconst TCHAR* StateWord = State == EONEState::Listening ? TEXT(\"listening\")\n"
    "\t\t: State == EONEState::Thinking ? TEXT(\"working\") : TEXT(\"speaking\");",
    "\tconst TCHAR* StateWord = State == EONEState::Ready ? TEXT(\"ready - hold LeftAlt to speak\")\n"
    "\t\t: State == EONEState::Listening ? TEXT(\"listening\")\n"
    "\t\t: State == EONEState::Thinking ? TEXT(\"working\") : TEXT(\"speaking\");", 1)

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

t = t.replace("static FAutoConsoleCommandWithWorldAndArgs GOneState(", CMD, 1)
wr(p, t)
print("PHIONE.cpp: PTT gate, hold measured, ledger, one.talk")

# ---------------- Character ----------------
p = "Source/VEIL/Public/Character/VEILCharacter.h"
t = rd(p)
if "OneTalkPressed" not in t:
    t = t.replace(
        "\tvirtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;",
        "\tvirtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;\n\n"
        "\t/** PUSH TO TALK: LeftAlt opens ONE's microphone while held, closes it on release. */\n"
        "\tvoid OneTalkPressed();\n"
        "\tvoid OneTalkReleased();", 1)
    wr(p, t)
    print("VEILCharacter.h: talk handlers declared")

p = "Source/VEIL/Private/Character/VEILCharacter.cpp"
t = rd(p)
if "OneTalkPressed" not in t:
    if '#include "Systems/PHIONE.h"' not in t:
        t = t.replace('#include "Character/VEILCharacter.h"',
                      '#include "Character/VEILCharacter.h"\n#include "Systems/PHIONE.h"', 1)
    t = t.replace(
        "\t// 1st/3rd person toggle: V on keyboard, right-stick click (R3) on the pad.",
        "\t// PUSH TO TALK: LeftAlt, held. ONE's microphone is open only between these\n"
        "\t// two events - there is no always-listening mode to switch off.\n"
        "\tPlayerInputComponent->BindKey(EKeys::LeftAlt, IE_Pressed, this, &AVEILCharacter::OneTalkPressed);\n"
        "\tPlayerInputComponent->BindKey(EKeys::LeftAlt, IE_Released, this, &AVEILCharacter::OneTalkReleased);\n\n"
        "\t// 1st/3rd person toggle: V on keyboard, right-stick click (R3) on the pad.", 1)
    t = t + r'''

void AVEILCharacter::OneTalkPressed()
{
	if (UWorld* W = GetWorld())
	{
		if (UPHIONE* One = W->GetSubsystem<UPHIONE>()) { One->BeginTalk(); }
	}
}

void AVEILCharacter::OneTalkReleased()
{
	if (UWorld* W = GetWorld())
	{
		if (UPHIONE* One = W->GetSubsystem<UPHIONE>()) { One->EndTalk(); }
	}
}
'''
    wr(p, t)
    print("VEILCharacter.cpp: LeftAlt bound, handlers added")

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
    print("HUD: ORB PATTERN MISSED")
