"""Add rung(x) = ln(x)/ln(phi) and tidy the new controller source."""
P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


# --- rung / unrung on the single phi source of truth ---
p = P + "PHI_Constants.h"
s = rd(p)
if "static double Rung" not in s:
    anchor = "\tstatic FVector2D GoldenLoc(int32 Index, double InSpacing);"
    add = ("\t// rung(x) = ln(x) / ln(phi). The round trip phi^rung(x) = x holds every time.\n"
           "\tstatic double Rung(double X) { return FMath::Loge(X) / FMath::Loge(Phi); }\n"
           "\tstatic double UnRung(double R) { return FMath::Pow(Phi, R); }\n\n")
    assert s.count(anchor) == 1, "GoldenLoc anchor"
    s = s.replace(anchor, add + anchor)
    wr(p, s)
    print("PHI_Constants: Rung/UnRung added")
else:
    print("PHI_Constants: rung already present")

# --- controller: drop the stray label, add the keys header ---
p = P + "VeilPlayerController.cpp"
s = rd(p)
before = s
s = s.replace("\t\tSpeech:\n", "")
if '#include "InputCoreTypes.h"' not in s:
    s = s.replace('#include "Components/InputComponent.h"',
                  '#include "Components/InputComponent.h"\n#include "InputCoreTypes.h"', 1)
if s != before:
    wr(p, s)
print("controller cleaned:", s != before, "| InputCoreTypes:", '#include "InputCoreTypes.h"' in s)
