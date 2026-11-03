# -*- coding: utf-8 -*-
"""A conversation owns the frame.

The ONE chip kept drawing over the dialogue box, so Sera Vahl's line had "ONE -
click to speak or type" printed across the middle of it. The chip now stands
down entirely while somebody is talking and returns when the box closes.

Also: ONE's acknowledgements were the flattest text in the game ("Down.",
"Beacon lit."). Six short lines, in ONE's own register - present tense, no
flattery, always aware of the district.
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


HUD = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
hud, hud_crlf = read(HUD)
old = ("\t{\n"
       "\t\tconst VEILTypeBar::FZone Zone = VEILTypeBar::ForScreen(Canvas->ClipX, Canvas->ClipY);\n"
       "\t\tconst AVEILPlayerController* VPC = Cast<AVEILPlayerController>(PC);\n")
new = ("\t// A CONVERSATION OWNS THE FRAME. The chip was drawn over the dialogue box, so a line\n"
       "\t// of Sera Vahl's arrived with \"ONE - click to speak or type\" printed across the middle\n"
       "\t// of it. While the box is open the chip stands down; it comes back when the box closes.\n"
       "\tconst UVEILDialogue* OneDlg = W->GetSubsystem<UVEILDialogue>();\n"
       "\tif (!OneDlg || !OneDlg->IsActive())\n"
       "\t{\n"
       "\t\tconst VEILTypeBar::FZone Zone = VEILTypeBar::ForScreen(Canvas->ClipX, Canvas->ClipY);\n"
       "\t\tconst AVEILPlayerController* VPC = Cast<AVEILPlayerController>(PC);\n")
if old in hud:
    hud = hud.replace(old, new, 1)
elif "A CONVERSATION OWNS THE FRAME" not in hud:
    fails.append("HUD chip guard anchor missing")

ONE = "Source/VEIL/Private/Systems/PHIONE.cpp"
one, one_crlf = read(ONE)
LINES = {
    'TEXT("Listening - let go to send.")': 'TEXT("Listening. Say it, or type it.")',
    'TEXT("On my way. Keeper beacon.")': 'TEXT("On my way. The Keeper is expecting a number.")',
    'TEXT("Down.")': 'TEXT("Down. The ground was where you left it.")',
    'TEXT("Spiral raised. Golden angle.")': 'TEXT("Spiral raised. Golden angle held.")',
    'TEXT("Beacon lit.")': 'TEXT("Beacon lit. That is one the dark does not get.")',
    'TEXT("Airborne.")': 'TEXT("Airborne. Mind the rings.")',
    'TEXT("No skill owns that yet.")': 'TEXT("No hand of mine owns that. Yet.")',
}
for a, b in LINES.items():
    if a in one:
        one = one.replace(a, b, 1)

if fails:
    print("FAILS:", fails)
    sys.exit(1)

write(HUD, hud, hud_crlf)
write(ONE, one, one_crlf)
print("chip guarded =", "if (!OneDlg || !OneDlg->IsActive())" in hud)
print("one lines =", sum(1 for b in LINES.values() if b in one), "of", len(LINES))
print("PATCH: PASS")
