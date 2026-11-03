"""L: the system log, hidden by default.

The brief lists the log as a hidden panel behind L. There was no log flag and no L
binding at all, so this adds the smallest honest version: a flag on the controller, the
key, and a 480 px panel that reads real lines - the campaign's activity line, the quest
stage, and the golden angle it is all built on - drawn only while the key is on.
"""

import io

fails = []

# ---- header: the flag and the key handler
H = "Source/VEIL/Public/Game/VEILGameMode.h"
h = io.open(H, encoding="utf-8", newline="").read()
HNL = "\r\n" if "\r\n" in h else "\n"
OLD = "\tbool bShowDebug = false;"
NEW = ("\tbool bShowDebug = false;\n"
       "\t/** L: the system log. Hidden by default - the brief puts it behind a key. */\n"
       "\tbool bShowLog = false;\n"
       "\tvoid ToggleLog();")
if h.count(OLD) != 1:
    fails.append("header flag count=%d" % h.count(OLD))
else:
    io.open(H, "w", encoding="utf-8", newline="").write(h.replace(OLD, NEW.replace("\n", HNL), 1))
    print("%s: log flag + handler declared" % H)

# ---- cpp: bind L, define the handler
C = "Source/VEIL/Private/Game/VEILGameMode.cpp"
c = io.open(C, encoding="utf-8", newline="").read()
CNL = "\r\n" if "\r\n" in c else "\n"

BIND = "\t\tInputComponent->BindKey(EKeys::F10, IE_Pressed, this, &AVEILPlayerController::ToggleDebugStrips);"
BIND_NEW = (BIND + "\n"
            "\t\t// L: the log. Hidden by default, per the locked HUD spec.\n"
            "\t\tInputComponent->BindKey(EKeys::L, IE_Pressed, this, &AVEILPlayerController::ToggleLog);")
if c.count(BIND) != 1:
    fails.append("bind anchor count=%d" % c.count(BIND))
else:
    c = c.replace(BIND, BIND_NEW.replace("\n", CNL), 1)

DEF = "void AVEILPlayerController::ToggleDebugStrips()"
DEF_NEW = ("void AVEILPlayerController::ToggleLog()\n"
           "{\n"
           "\tbShowLog = !bShowLog;\n"
           "\tUE_LOG(LogVEIL, Log, TEXT(\"PHI LOG: %s\"), bShowLog ? TEXT(\"shown\") : TEXT(\"hidden\"));\n"
           "}\n\n" + DEF)
if c.count(DEF) != 1:
    fails.append("definition anchor count=%d" % c.count(DEF))
else:
    c = c.replace(DEF, DEF_NEW.replace("\n", CNL), 1)

if not fails:
    io.open(C, "w", encoding="utf-8", newline="").write(c)
    print("%s: L bound, ToggleLog defined" % C)

# ---- canvas: the panel itself, drawn only while the key is on
P = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
SNL = "\r\n" if "\r\n" in s else "\n"

ANCHOR = "\tif (const AVEILPlayerController* OsPC = Cast<AVEILPlayerController>(PC))"
PANEL = """	// L: THE SYSTEM LOG. Hidden by default so the play view is the world; real lines
	// only - the campaign's activity row, the quest stage, the golden angle.
	if (const AVEILPlayerController* LogPC = Cast<AVEILPlayerController>(PC))
	{
		if (LogPC->bShowLog)
		{
			const float LogW = 480.f, LogH = 132.f, LogX = 30.f, LogY = 136.f;
			FVeilUI::Glass(this, FLinearColor(0.055f, 0.078f, 0.141f, 0.55f), LogX, LogY, LogW, LogH, 11.f);
			DrawText(TEXT("LOG"), FLinearColor(0.910f, 0.768f, 0.416f), LogX + 14.f, LogY + 8.f, Big, 0.70f);
			float Row = LogY + 34.f;
			if (const UVEILCampaignSubsystem* LogCamp = W->GetSubsystem<UVEILCampaignSubsystem>())
			{
				DrawText(LogCamp->ActivityLine(), FLinearColor(1.f, 0.85f, 0.45f), LogX + 14.f, Row, Med, 0.66f);
				Row += 19.f;
			}
			if (Q)
			{
				DrawText(FString::Printf(TEXT("STAGE %d   FRAGMENTS %d"), (int32)Q->Stage, Q->Fragments),
					FLinearColor(0.86f, 0.89f, 0.95f), LogX + 14.f, Row, Med, 0.66f);
				Row += 19.f;
			}
			DrawText(TEXT("PHI 1.618033988749895   GOLDEN 137.5077640500378"),
				FLinearColor(0.788f, 0.635f, 0.290f), LogX + 14.f, Row, Med, 0.62f);
		}
	}
	""" + ANCHOR

if s.count(ANCHOR) != 1:
    fails.append("canvas anchor count=%d" % s.count(ANCHOR))
else:
    io.open(P, "w", encoding="utf-8", newline="").write(s.replace(ANCHOR, PANEL.replace("\n", SNL), 1))
    print("%s: log panel drawn only while L is on" % P)

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("LOG TOGGLE WRITTEN")
