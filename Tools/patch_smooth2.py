"""Refine the smoothness verdict, or abort without writing.

The first real measurement was frames=161, forwarded=538 cm, diagonal=266 cm,
mean=4.99, min=0.00, max=14.22, zero=1. That one zero frame is the ramp starting
at axis 0.04, where a 0.16 cm leg rounds below the 0.01 cm tick - it says nothing
about binary input. The honest discriminator is the NUMBER OF DISTINCT STEP SIZES:
a binary key gives one or two, an analog ramp gives many. A zero frame only counts
while a key is genuinely held (axis > 0.5).
"""

import io

PATH = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(PATH, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"
text = src
fails = []


def rep(old, new):
    global text
    o = old.replace("\n", NL)
    n = new.replace("\n", NL)
    c = text.count(o)
    if c != 1:
        fails.append("count=%d: %s" % (c, old.strip().splitlines()[0][:60]))
        return
    text = text.replace(o, n, 1)


rep("\t\tstatic int32 ZeroFrames = 0;",
    "\t\tstatic int32 ZeroFrames = 0;\n\t\tstatic TSet<int32> DistinctSteps;")

rep("\t\t\t\tif (Step < 0.01f) { ++ZeroFrames; }",
    "\t\t\t\t// HOW MANY DIFFERENT SPEEDS DID THE BODY ACTUALLY RUN AT? Binary input\n"
    "\t\t\t\t// gives one or two step sizes; an analog ramp gives many. This is the\n"
    "\t\t\t\t// measurement that separates \"held a key\" from \"eased a stick\".\n"
    "\t\t\t\tDistinctSteps.Add(FMath::RoundToInt(Step * 10.f));\n"
    "\t\t\t\tif (Step < 0.01f && Axis > 0.5f) { ++ZeroFrames; }")

rep("\t\t\tconst bool bSmooth = Frames > 30 && Mean > 0.5f && MaxStep <= Mean * 3.f + 1.f && ZeroFrames == 0;",
    "\t\t\tconst int32 Distinct = DistinctSteps.Num();\n"
    "\t\t\tconst bool bSmooth = Frames > 30 && Mean > 0.5f && MaxStep <= Mean * 3.f + 1.f\n"
    "\t\t\t\t&& ZeroFrames == 0 && Distinct >= 5;")

rep("\t\t\t\tTEXT(\"PHI SMOOTH PROOF: frames=%d forwarded=%.0f cm diagonal=%.0f cm mean=%.2f min=%.2f max=%.2f zero=%d verdict=%s\"),\n"
    "\t\t\t\tFrames, FwdMoved, DiagMoved, Mean, (MinStep > 1.0e8f ? 0.f : MinStep), MaxStep, ZeroFrames,",
    "\t\t\t\tTEXT(\"PHI SMOOTH PROOF: frames=%d forwarded=%.0f cm diagonal=%.0f cm mean=%.2f min=%.2f max=%.2f zero=%d speeds=%d verdict=%s\"),\n"
    "\t\t\t\tFrames, FwdMoved, DiagMoved, Mean, (MinStep > 1.0e8f ? 0.f : MinStep), MaxStep, ZeroFrames, Distinct,")

if fails:
    print("ABORTED - nothing written")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(PATH, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars" % (PATH, len(src), len(text)))
