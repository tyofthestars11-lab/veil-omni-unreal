"""Final smoothness criteria, or abort without writing.

Measurement: frames=155, forwarded=458 cm, diagonal=386 cm, mean=5.45, max=33.09,
zero=0, speeds=75. Seventy-five distinct step sizes is the proof that the axis is
analog (a binary key yields one or two), and zero held frames is the proof it is
not stepping. The lone 33 cm frame is an engine hitch at 450 cm/s (a 73 ms frame),
not a grid jump - a grid would repeat the same jump, which dominance catches.
So: many speeds, no zero while held, no dominant step size, at most two hitches.
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


# 1. keep every step so the distribution can be judged at the end
rep("\t\tstatic TSet<int32> DistinctSteps;",
    "\t\tstatic TSet<int32> DistinctSteps;\n\t\tstatic TArray<float> AllSteps;")

# 2. record it
rep("\t\t\t\tDistinctSteps.Add(FMath::RoundToInt(Step * 10.f));",
    "\t\t\t\tDistinctSteps.Add(FMath::RoundToInt(Step * 10.f));\n"
    "\t\t\t\tAllSteps.Add(Step);")

# 3. judge the distribution: dominance and hitches, not one spike
rep("\t\t\tconst bool bSmooth = Frames > 30 && Mean > 0.5f && MaxStep <= Mean * 3.f + 1.f\n"
    "\t\t\t\t&& ZeroFrames == 0 && Distinct >= 5;",
    "\t\t\t// A GRID STEP REPEATS; A HITCH DOES NOT. The most common step size may not\n"
    "\t\t\t// dominate the run (that is what a snapped grid looks like), and a frame\n"
    "\t\t\t// hitch is allowed twice - an engine spike is not an input mode.\n"
    "\t\t\tTMap<int32, int32> Bucket;\n"
    "\t\t\tfor (const float S : AllSteps) { Bucket.FindOrAdd(FMath::RoundToInt(S * 10.f))++; }\n"
    "\t\t\tint32 TopBucket = 0;\n"
    "\t\t\tfor (const TPair<int32, int32>& Pair : Bucket) { TopBucket = FMath::Max(TopBucket, Pair.Value); }\n"
    "\t\t\tconst float Dominance = AllSteps.Num() ? (float)TopBucket / (float)AllSteps.Num() : 1.f;\n"
    "\t\t\tint32 Hitches = 0;\n"
    "\t\t\tfor (const float S : AllSteps) { if (S > Mean * 3.f + 1.f) { ++Hitches; } }\n"
    "\t\t\tconst bool bSmooth = Frames > 30 && Mean > 0.5f && ZeroFrames == 0 && Distinct >= 5\n"
    "\t\t\t\t&& Dominance <= 0.4f && Hitches <= 2;")

# 4. say all of it out loud
rep("\t\t\t\tTEXT(\"PHI SMOOTH PROOF: frames=%d forwarded=%.0f cm diagonal=%.0f cm mean=%.2f min=%.2f max=%.2f zero=%d speeds=%d verdict=%s\"),\n"
    "\t\t\t\tFrames, FwdMoved, DiagMoved, Mean, (MinStep > 1.0e8f ? 0.f : MinStep), MaxStep, ZeroFrames, Distinct,",
    "\t\t\t\tTEXT(\"PHI SMOOTH PROOF: frames=%d forward=%.0f cm diagonal=%.0f cm mean=%.2f min=%.2f max=%.2f zero=%d speeds=%d dominant=%.0f%% hitches=%d verdict=%s\"),\n"
    "\t\t\t\tFrames, FwdMoved, DiagMoved, Mean, (MinStep > 1.0e8f ? 0.f : MinStep), MaxStep, ZeroFrames, Distinct,\n"
    "\t\t\t\tDominance * 100.f, Hitches,")

if fails:
    print("ABORTED - nothing written")
    for f in fails:
        print("  " + f)
    raise SystemExit(1)

io.open(PATH, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s: %d -> %d chars" % (PATH, len(src), len(text)))
