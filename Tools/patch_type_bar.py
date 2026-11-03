"""Click to type to ONE: shared bar geometry, controller field, HUD bar.

The request: talking to ONE must be a field you click and type into, not only a key you
hold. So the HUD draws a gold bar, the player controller hit-tests the same rectangle from
the same header, and the typed line goes through UPHIONE::Say - the same door a spoken
sentence uses, so the skill, the hand and the ledger are identical.
"""

import io
import re

fails = []


def rw(p):
    return io.open(p, encoding="utf-8", newline="").read()


def wr(p, s):
    io.open(p, "w", encoding="utf-8", newline="").write(s)


def nl(s):
    return "\r\n" if "\r\n" in s else "\n"


# ------------------------------------------------------------------ 1. shared geometry
HDR = '''#pragma once

#include "CoreMinimal.h"

/**
 * THE TYPE BAR - one geometry, two readers.
 *
 * ONE is talked to by clicking this bar and typing, not only by holding B. The HUD draws the
 * bar and the player controller hit-tests the click, so both read the same rectangle from
 * here: a click can never land on a bar that is not drawn, or miss one that is.
 *
 * Width is the brief's one-line figure (480 px), height is one text row, and it sits above
 * the ONE state line and clear of the bottom action band so it owns its own strip.
 */
namespace VEILTypeBar
{
	struct FZone
	{
		float X = 0.f;
		float Y = 0.f;
		float W = 480.f;
		float H = 30.f;

		bool Contains(float PX, float PY) const
		{
			return PX >= X && PX <= X + W && PY >= Y && PY <= Y + H;
		}
	};

	inline FZone ForScreen(float ClipX, float ClipY)
	{
		FZone Z;
		Z.W = FMath::Min(480.f, FMath::Max(240.f, ClipX - 80.f));
		Z.H = 30.f;
		Z.X = (ClipX - Z.W) * 0.5f;
		Z.Y = ClipY - 212.f;   // above the ONE line at ClipY-170, clear of the action band
		return Z;
	}
}
'''
wr("Source/VEIL/Public/UI/VEILTypeBar.h", HDR)
print("WROTE Source/VEIL/Public/UI/VEILTypeBar.h")

# ------------------------------------------------------------------ 2. controller header
PH = "Source/VEIL/Public/Game/VEILGameMode.h"
h = rw(PH)
N = nl(h)
anchor = "\t/** Fires only if the world's timer manager advances: the timer cross-check. */\n\tvoid PhiOnTimerCheck();"
add = anchor + '''

	/** CLICK TO TYPE TO ONE. The bar the HUD draws opens a real text field; Enter sends the
	 *  line through UPHIONE::Say, the same door a spoken sentence uses, so the skill, the
	 *  hand and the ledger come out identical either way. */
	void OpenTypeToONE();
	void CloseTypeToONE();
	bool IsTypingToONE() const { return bTypingToONE; }

private:
	/** Slate objects are not UObjects, so these are plain pointers, not UPROPERTY. */
	bool bTypingToONE = false;
	TSharedPtr<class SWidget> TypeContainer;
	TSharedPtr<class SEditableTextBox> TypeBox;
	void OnLeftClickType();
	void OnOneTextCommitted(const FText& Text, ETextCommit::Type CommitType);

public:'''
o, a = anchor.replace("\n", N), add.replace("\n", N)
if h.count(o) != 1:
    fails.append("controller header anchor count=%d" % h.count(o))
else:
    wr(PH, h.replace(o, a, 1))
    print("header: type-bar API added")

# ------------------------------------------------------------------ 3. controller impl
PC = "Source/VEIL/Private/Game/VEILGameMode.cpp"
c = rw(PC)
NC = nl(c)

inc_anchor = '#include "World/VEILMapCapture.h"'
inc_add = inc_anchor + '''
#include "UI/VEILTypeBar.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBox.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/CoreStyle.h"
#include "Engine/GameViewportClient.h"
'''
if c.count(inc_anchor) != 1:
    fails.append("include anchor count=%d" % c.count(inc_anchor))
else:
    c = c.replace(inc_anchor, inc_add, 1)

IMPL = '''// ---------------------------------------------------------------------------
// CLICK TO TYPE TO ONE. A key you have to hold is not a conversation; a field you
// click and type into is. The HUD draws the bar from VEILTypeBar::ForScreen and
// this hit-tests the very same rectangle, so the click and the drawing agree.
// ---------------------------------------------------------------------------
void AVEILPlayerController::OnLeftClickType()
{
	float MX = 0.f, MY = 0.f;
	if (!GetMousePosition(MX, MY)) { return; }

	float ClipX = 1920.f, ClipY = 1080.f;
	if (GEngine && GEngine->GameViewport && GEngine->GameViewport->Viewport)
	{
		const FIntPoint Size = GEngine->GameViewport->Viewport->GetSizeXY();
		ClipX = (float)Size.X;
		ClipY = (float)Size.Y;
	}
	const VEILTypeBar::FZone Zone = VEILTypeBar::ForScreen(ClipX, ClipY);
	if (!Zone.Contains(MX, MY)) { return; }

	OpenTypeToONE();
}

void AVEILPlayerController::OpenTypeToONE()
{
	if (!GEngine || !GEngine->GameViewport || !GEngine->GameViewport->Viewport) { return; }
	if (bTypingToONE && TypeBox.IsValid())
	{
		FSlateApplication::Get().SetKeyboardFocus(TypeBox, EFocusCause::SetDirectly);
		return;
	}

	SAssignNew(TypeBox, SEditableTextBox)
		.HintText(FText::FromString(TEXT("Type to ONE, then press Enter")))
		.Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
		.ForegroundColor(FSlateColor(FLinearColor(1.f, 0.88f, 0.45f)))
		.BackgroundColor(FSlateColor(FLinearColor(0.035f, 0.045f, 0.075f, 0.95f)))
		.Padding(FMargin(10.f, 5.f))
		.SelectAllTextWhenFocused(true)
		.OnTextCommitted(this, &AVEILPlayerController::OnOneTextCommitted);

	const FIntPoint Size = GEngine->GameViewport->Viewport->GetSizeXY();
	const VEILTypeBar::FZone Zone = VEILTypeBar::ForScreen((float)Size.X, (float)Size.Y);
	SAssignNew(TypeContainer, SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Top)
		.Padding(FMargin(0.f, Zone.Y, 0.f, 0.f))
		.WidthOverride(Zone.W)
		[ TypeBox.ToSharedRef() ];

	GEngine->GameViewport->AddViewportWidgetContent(TypeContainer.ToSharedRef(), 100);

	FInputModeGameAndUI Mode;
	Mode.SetWidgetToFocus(TypeBox);
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	Mode.SetHideCursorDuringCapture(false);
	SetInputMode(Mode);
	bShowMouseCursor = true;
	bTypingToONE = true;
	UE_LOG(LogVEIL, Warning, TEXT("PHI TYPE: field open - click landed in the type bar."));
}

void AVEILPlayerController::CloseTypeToONE()
{
	if (TypeContainer.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(TypeContainer.ToSharedRef());
	}
	TypeContainer.Reset();
	TypeBox.Reset();
	bTypingToONE = false;
	SetInputMode(FInputModeGameOnly());
	bShowMouseCursor = false;
}

void AVEILPlayerController::OnOneTextCommitted(const FText& Text, ETextCommit::Type CommitType)
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
}

'''
m = re.search(r"void AVEILPlayerController::SetupInputComponent\(\)", c)
if not m:
    fails.append("SetupInputComponent not found")
else:
    c = c[:m.start()] + IMPL.replace("\n", NC) + c[m.start():]

bind_anchor = "InputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AVEILPlayerController::TravelOrAdvance);"
bind_add = ("// CLICK TO TYPE. Left mouse over the type bar opens the field; everywhere else the\n"
            "\t\t// click is left to the world, which is why this tests only the bar's own rectangle.\n"
            "\t\tInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AVEILPlayerController::OnLeftClickType);\n"
            "\t\t") + bind_anchor
if c.count(bind_anchor) < 1:
    fails.append("SpaceBar bind anchor count=%d" % c.count(bind_anchor))
else:
    c = c.replace(bind_anchor, bind_add, 1)

m2 = re.search(r"bool AVEILPlayerController::UIWantsCursor\(\) const\r?\n\{", c)
if not m2:
    fails.append("UIWantsCursor not found")
else:
    guard = ("\n\t// The type bar owns the pointer while it is open, or the cursor is hidden the\n"
             "\t// same frame it is needed and the player cannot click into the field at all.\n"
             "\tif (bTypingToONE) { return true; }")
    c = c[:m2.end()] + guard.replace("\n", NC) + c[m2.end():]

wr(PC, c)
print("controller: click-to-type implemented")

# ------------------------------------------------------------------ 4. HUD bar
PHUD = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
u = rw(PHUD)
NH = nl(u)
inc2 = '#include "World/VEILMapCapture.h"'
inc2a = inc2 + '\n#include "UI/VEILTypeBar.h"\n#include "Game/VEILGameMode.h"'
if u.count(inc2) >= 1:
    u = u.replace(inc2, inc2a, 1)
    print("hud includes added")
else:
    fails.append("hud include anchor count=%d" % u.count(inc2))

BAR = '''	// CLICK HERE TO TYPE TO ONE. A push-to-talk key is not a conversation; this is the door
	// you can see. The controller hit-tests this exact rectangle, so the bar that is drawn is
	// the bar that opens the field, and it lights while the field owns the pointer so a click
	// never happens without an answer.
	{
		const VEILTypeBar::FZone Zone = VEILTypeBar::ForScreen(Canvas->ClipX, Canvas->ClipY);
		const bool bTypingNow = PC && PC->IsTypingToONE();
		FVeilUI::Chip(this, FLinearColor(0.035f, 0.045f, 0.075f, bTypingNow ? 0.92f : 0.70f),
			Zone.X, Zone.Y, Zone.W, Zone.H);
		const FLinearColor Edge(1.f, 0.78f, 0.30f, bTypingNow ? 0.95f : 0.55f);
		DrawRect(Edge, Zone.X, Zone.Y, Zone.W, 1.4f);
		DrawRect(Edge, Zone.X, Zone.Y + Zone.H - 1.4f, Zone.W, 1.4f);
		DrawText(bTypingNow ? TEXT("TYPING TO ONE  -  ENTER SENDS") : TEXT("CLICK HERE TO TYPE TO ONE"),
			bTypingNow ? FLinearColor(1.f, 0.90f, 0.55f) : FLinearColor(0.88f, 0.82f, 0.64f),
			Zone.X + 12.f, Zone.Y + 6.f, Med, 0.62f);
	}

'''
one_anchor = "\tif (const UPHIONE* One = W->GetSubsystem<UPHIONE>())"
if u.count(one_anchor) != 1:
    fails.append("ONE block anchor count=%d" % u.count(one_anchor))
else:
    u = u.replace(one_anchor, (BAR + one_anchor), 1)
    wr(PHUD, u)
    print("hud: type bar drawn")

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("ALL EDITS APPLIED")
