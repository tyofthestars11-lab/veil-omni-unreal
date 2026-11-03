"""Fix the two errors from the first type-bar build.

1. In the HUD, PC is an APlayerController*, so IsTypingToONE needs a cast to the VEIL controller.
2. SEditableTextBox's OnTextCommitted is a SLATE_EVENT; a UObject method binds through the
   OnTextCommitted_UObject overload, not the plain fluent one (that expects a shared ref).
"""

import io

fails = []

P1 = "Source/VEIL/Private/Game/VEILHudCanvas.cpp"
s = io.open(P1, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in s else "\n"
old = "\t\tconst bool bTypingNow = PC && PC->IsTypingToONE();"
new = ("\t\t// PC is the base controller here; the type-bar state lives on the VEIL controller.\n"
       "\t\tconst AVEILPlayerController* VPC = Cast<AVEILPlayerController>(PC);\n"
       "\t\tconst bool bTypingNow = VPC && VPC->IsTypingToONE();")
if s.count(old) != 1:
    fails.append("hud typing anchor count=%d" % s.count(old))
else:
    s = s.replace(old, new.replace("\n", NL), 1)
    io.open(P1, "w", encoding="utf-8", newline="").write(s)
    print("hud: cast to AVEILPlayerController")

P2 = "Source/VEIL/Private/Game/VEILGameMode.cpp"
c = io.open(P2, encoding="utf-8", newline="").read()
old2 = ".OnTextCommitted(this, &AVEILPlayerController::OnOneTextCommitted);"
new2 = ".OnTextCommitted_UObject(this, &AVEILPlayerController::OnOneTextCommitted);"
if c.count(old2) != 1:
    fails.append("delegate anchor count=%d" % c.count(old2))
else:
    c = c.replace(old2, new2, 1)
    io.open(P2, "w", encoding="utf-8", newline="").write(c)
    print("controller: OnTextCommitted_UObject")

if fails:
    print("FAILURES:")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)
print("FIXES APPLIED")
