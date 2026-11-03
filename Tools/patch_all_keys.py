# -*- coding: utf-8 -*-
"""E ONLY. The vision's dialogue spec is explicit: continue with [E], not Space,
not Enter, and the prompt must say [E]. Space was still bound to TravelOrAdvance,
which advanced the dialogue; now Space is only the runner's jump. Every printed
prompt follows.

Also puts a microphone on the ONE chip (the spec asks for a mic symbol, not a
sentence) and changes the label to talk about the mic, not a second instruction.
"""
import io
import sys

ROOT = "D:/VEIL-OMNI/"
fails = []


def read(p):
    with io.open(ROOT + p, "r", encoding="utf-8", newline="") as f:
        raw = f.read()
    return raw.replace("\r\n", "\n"), ("\r\n" in raw)


def write(p, text, crlf):
    with io.open(ROOT + p, "w", encoding="utf-8", newline="") as f:
        f.write(text.replace("\n", "\r\n") if crlf else text)


def sub(t, old, new, label, need=1):
    n = t.count(old)
    if n != need:
        fails.append("%s count=%d (wanted %d)" % (label, n, need))
        return t
    return t.replace(old, new)


# 1. HUD dialogue prompts.
HUD = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
hud, hud_crlf = read(HUD)
hud = sub(hud, 'TEXT("[SPACE] continue")', 'TEXT("[E] continue")', "continue prompt")
hud = sub(hud, 'TEXT("[SPACE] reply")', 'TEXT("[E] reply")', "reply prompt")
hud = sub(hud, 'TEXT("[SPACE] close")', 'TEXT("[E] close")', "close prompt")
hud = sub(hud, 'TEXT("[E] SPEAK   [SPACE] NEXT   [TAB] CHART   [H] MANUAL   [P] PAUSE")',
          'TEXT("[E] SPEAK / NEXT   [TAB] CHART   [H] MANUAL   [P] PAUSE")', "hint row")

# 2. Mic glyph on the ONE chip, and copy that names it.
if "THE MIC." not in hud:
    mic_anchor = "\t\tDrawText(Label, (bTypingNow || bListening)"
    if hud.count(mic_anchor) != 1:
        fails.append("one label anchor count=%d" % hud.count(mic_anchor))
    else:
        MIC = ("\t\t// THE MIC. The reference asks for a microphone, not a sentence: a capsule,\n"
               "\t\t// a cradle and a stand, drawn from the primitives so it needs no icon asset and\n"
               "\t\t// cannot go missing.\n"
               "\t\t{\n"
               "\t\t\tconst FLinearColor MicCol = (bTypingNow || bListening)\n"
               "\t\t\t\t? FLinearColor(1.f, 0.88f, 0.50f) : FLinearColor(0.86f, 0.80f, 0.62f);\n"
               "\t\t\tFVeilUI::RoundRect(this, FLinearColor(0.f, 0.f, 0.f, 0.55f), Zone.X + 13.f, Zone.Y + 8.f, 7.f, 12.f, 3.5f);\n"
               "\t\t\tFVeilUI::RoundRect(this, MicCol, Zone.X + 14.f, Zone.Y + 9.f, 5.f, 10.f, 2.5f);\n"
               "\t\t\tDrawLine(Zone.X + 12.f, Zone.Y + 15.f, Zone.X + 12.f, Zone.Y + 18.f, MicCol, 1.1f);\n"
               "\t\t\tDrawLine(Zone.X + 21.f, Zone.Y + 15.f, Zone.X + 21.f, Zone.Y + 18.f, MicCol, 1.1f);\n"
               "\t\t\tDrawLine(Zone.X + 12.f, Zone.Y + 18.f, Zone.X + 21.f, Zone.Y + 18.f, MicCol, 1.1f);\n"
               "\t\t\tDrawLine(Zone.X + 16.5f, Zone.Y + 18.f, Zone.X + 16.5f, Zone.Y + 21.f, MicCol, 1.1f);\n"
               "\t\t\tDrawLine(Zone.X + 13.f, Zone.Y + 21.f, Zone.X + 20.f, Zone.Y + 21.f, MicCol, 1.1f);\n"
               "\t\t}\n")
        hud = hud.replace(mic_anchor, MIC + mic_anchor, 1)

hud = sub(hud, 'TEXT("ONE  -  click to speak or type")',
          'TEXT("ONE  -  click the mic to speak, or type")', "one label")
hud = sub(hud, "Zone.X + 26.f, Zone.Y + 5.f", "Zone.X + 30.f, Zone.Y + 5.f", "one label x")
hud = sub(hud, "Zone.X + 26.f, Zone.Y + 22.f", "Zone.X + 30.f, Zone.Y + 22.f", "one state x")

# 3. Space no longer advances anything.
GM = "Source/VEIL/Private/Game/VEILGameMode.cpp"
gm, gm_crlf = read(GM)
gm = sub(gm, "\t\tInputComponent->BindKey(EKeys::SpaceBar, IE_Pressed, this, &AVEILPlayerController::TravelOrAdvance);\n",
         "\t\t// SPACE IS THE JUMP, NOT THE CONVERSATION. The vision's dialogue spec is E only:\n"
         "\t\t// Space was bound here as well, so a line could be advanced by the same key that\n"
         "\t\t// leaves the ground. Travel stays on the chart's Enter and the pad's A.\n", "space binding")

# 4. The OS key legend says the same thing.
OS = "Source/VEIL/Private/Systems/VEILOS.cpp"
os_, os_crlf = read(OS)
os_ = sub(os_, "[SPACE] continue", "[E] continue", "os legend")

if fails:
    print("FAILS:", fails)
    sys.exit(1)

write(HUD, hud, hud_crlf)
write(GM, gm, gm_crlf)
write(OS, os_, os_crlf)
print("mic glyph =", "THE MIC." in hud)
print("E prompts =", hud.count("[E]"))
print("space unbound =", "EKeys::SpaceBar, IE_Pressed, this, &AVEILPlayerController::TravelOrAdvance" not in gm)
print("PATCH: PASS")
