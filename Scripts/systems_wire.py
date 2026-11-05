"""Wire the imported cues, a pause system and a walk cycle into the game."""
import re

ROOT = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"
runner_cpp = ROOT + "VeilRunner.cpp"
runner_h = ROOT + "VeilRunner.h"
pc_cpp = ROOT + "VeilPlayerController.cpp"
pc_h = ROOT + "VeilPlayerController.h"
hud_cpp = ROOT + "VeilHud.cpp"

notes = []


def read(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def write(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


# ---------------- Runner: audio + walk cycle ----------------
s = read(runner_cpp)
if "Kismet/GameplayStatics.h" not in s:
    s = s.replace('#include "VeilRunner.h"', '#include "VeilRunner.h"\n#include "Kismet/GameplayStatics.h"', 1)
    notes.append("runner: gameplay statics include")

if "veil_strike" not in s:
    idx = s.find('TEXT("VEIL COMBAT: strike lands at')
    if idx > 0:
        end = s.find(");", idx)
        if end > 0:
            end += 2
            s = s[:end] + '''
	// A landed strike is heard: real imported cue, not silence.
	if (USoundBase* Cue = LoadObject<USoundBase>(nullptr, TEXT("/Game/VEIL/Audio/veil_strike.veil_strike")))
	{
		UGameplayStatics::PlaySoundAtLocation(this, Cue, GetActorLocation());
		UE_LOG(LogTemp, Display, TEXT("VEIL AUDIO: strike cue played"));
	}''' + s[end:]
            notes.append("runner: strike cue")
        else:
            notes.append("runner: MISS strike end")
    else:
        notes.append("runner: MISS strike log")

if "MM_Walk" not in s:
    anchor = "\tMove->MaxWalkSpeed = bSprint ? 750.f : 439.f;"
    if anchor in s:
        block = anchor + '''
	// Real locomotion clip when the character has one; idle remains the fallback.
	if (Avatar && !WalkAnim)
	{
		WalkAnim = LoadObject<UAnimSequence>(nullptr,
			TEXT("/Game/Polyart/SharedResources/Characters/AnimationStuff/Animations/Locomotion/Male/MM_Walk.MM_Walk"));
		if (WalkAnim)
		{
			UE_LOG(LogTemp, Display, TEXT("VEIL RUNNER: walk cycle loaded"));
		}
	}
	if (Avatar && Move)
	{
		const double Speed = Move->Velocity.Size2D();
		if (WalkAnim && Speed > 10.0 && Avatar->GetAnimInstance())
		{
			Avatar->PlayAnimation(WalkAnim, true);
		}
		else if (IdleAnim && Speed <= 10.0)
		{
			Avatar->PlayAnimation(IdleAnim, true);
		}
	}'''
        s = s.replace(anchor, block, 1)
        notes.append("runner: walk cycle")
    else:
        notes.append("runner: MISS walk anchor")
write(runner_cpp, s)

h = read(runner_h)
if "WalkAnim" not in h:
    m = re.search(r"(class\s+AVeilRunner\s*:[^{]*\{)", h)
    if m:
        h = h[:m.end(1)] + "\n\t// Locomotion clip, loaded at runtime from the real character set.\n\tUPROPERTY()\n\tUAnimSequence* WalkAnim = nullptr;\n" + h[m.end(1):]
        write(runner_h, h)
        notes.append("runner.h: WalkAnim")
    else:
        notes.append("runner.h: MISS class")

# ---------------- Controller: pause ----------------
c = read(pc_cpp)
if "OnPauseToggle" not in c:
    anchor = "\tInputComponent->BindKey(EKeys::C, IE_Pressed, this, &AVeilPlayerController::OnCombatMode);"
    if anchor in c:
        c = c.replace(anchor, anchor + '''
	InputComponent->BindKey(EKeys::P, IE_Pressed, this, &AVeilPlayerController::OnPauseToggle);
	InputComponent->BindKey(EKeys::Escape, IE_Pressed, this, &AVeilPlayerController::OnPauseToggle);''', 1)
        notes.append("controller: pause keys")
    else:
        notes.append("controller: MISS bind anchor")

    fn = '''
void AVeilPlayerController::OnPauseToggle()
{
	// A real pause: the world stops and the menu says so.
	bPauseMenu = !bPauseMenu;
	SetPause(bPauseMenu);
	UE_LOG(LogTemp, Display, TEXT("VEIL PAUSE: %s"), bPauseMenu ? TEXT("paused") : TEXT("resumed"));
}
'''
    c = c.rstrip("\n") + "\n" + fn
    write(pc_cpp, c)
    notes.append("controller: pause function")

ch = read(pc_h)
if "OnPauseToggle" not in ch:
    m = re.search(r"(class\s+AVeilPlayerController\s*:[^{]*\{)", ch)
    if m:
        add = ("\n\t// Pause state read by the HUD.\n\tbool bPauseMenu = false;\n"
               "\tvoid OnPauseToggle();\n")
        ch = ch[:m.end(1)] + add + ch[m.end(1):]
        write(pc_h, ch)
        notes.append("controller.h: pause")
    else:
        notes.append("controller.h: MISS class")

# ---------------- HUD: pause panel ----------------
u = read(hud_cpp)
if "PAUSED" not in u:
    head = u.find("void AVeilHud::DrawHUD()")
    if head < 0:
        notes.append("hud: MISS DrawHUD")
    else:
        brace = u.find("{", head)
        depth = 0
        end = -1
        for k in range(brace, len(u)):
            if u[k] == "{":
                depth += 1
            elif u[k] == "}":
                depth -= 1
                if depth == 0:
                    end = k
                    break
        if end < 0:
            notes.append("hud: MISS DrawHUD end")
        else:
            panel = '''
	// Pause panel: the state is real, so the screen says so.
	{
		AVeilPlayerController* MenuPC = Cast<AVeilPlayerController>(GetOwningPlayerController());
		if (MenuPC && MenuPC->bPauseMenu)
		{
			DrawRect(FLinearColor(0.02f, 0.03f, 0.08f, 0.88f), W * 0.32f, H * 0.34f, W * 0.36f, H * 0.22f);
			DrawText(TEXT("PAUSED"), Gold, W * 0.44f, H * 0.39f, nullptr, 1.7f, false);
			DrawText(TEXT("[P] resume"), Ink, W * 0.44f, H * 0.47f, nullptr, 0.95f, false);
		}
	}
'''
            u = u[:end] + panel + u[end:]
            write(hud_cpp, u)
            notes.append("hud: pause panel")

print("\n".join(notes) if notes else "nothing changed")
