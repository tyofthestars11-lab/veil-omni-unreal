import io

ROOT = r"D:\VEIL-OMNI" + "\\"

def rd(p):
    return io.open(ROOT + p, encoding="utf-8", newline=None).read()

def wr(p, s):
    io.open(ROOT + p, "w", encoding="utf-8", newline="\r\n").write(s)

# ---------- camp fields for hysteresis ----------
p = "Source/VEIL/Public/World/VEILWilds.h"
t = rd(p)
if "PendingCount" not in t:
    t = t.replace(
        "\t/** Last order issued, as an int so the enum stays out of the header. */\n\tint32 Order = -1;",
        "\t/** Last order issued, as an int so the enum stays out of the header. */\n"
        "\tint32 Order = -1;\n"
        "\t/** An order that differs from the current one, and how many decisions in a\n"
        "\t *  row it has held. Two in a row before the camp changes what it is doing. */\n"
        "\tint32 Pending = -1;\n"
        "\tint32 PendingCount = 0;", 1)
    wr(p, t)
    print("wilds header: hysteresis slots added")
else:
    print("wilds header: already has hysteresis")

# ---------- order selection with hysteresis ----------
p = "Source/VEIL/Private/World/VEILWilds.cpp"
t = rd(p)
old = ("\t\t\tEWildsOrder Order = EWildsOrder::Hunt;\n"
       "\t\t\tif (Camp.Intent < WildsRestDigit) { Order = EWildsOrder::Hold; }\n"
       "\t\t\telse if (OrderDigit < WildsRestDigit) { Order = EWildsOrder::Encircle; }")
new = ("\t\t\tEWildsOrder Candidate = EWildsOrder::Hunt;\n"
       "\t\t\tif (Camp.Intent < WildsRestDigit) { Candidate = EWildsOrder::Hold; }\n"
       "\t\t\telse if (OrderDigit < WildsRestDigit) { Candidate = EWildsOrder::Encircle; }\n\n"
       "\t\t\t// HYSTERESIS. A digit sitting on the threshold flips the whole camp every\n"
       "\t\t\t// 0.618 s, which read in the log as 2,600 order changes in two minutes and\n"
       "\t\t\t// looked, correctly, like noise. An order now has to hold for two decisions\n"
       "\t\t\t// in a row before it is obeyed - the camp still decides on phi, it just does\n"
       "\t\t\t// not stutter about it.\n"
       "\t\t\tEWildsOrder Order = (Camp.Order < 0) ? Candidate : (EWildsOrder)Camp.Order;\n"
       "\t\t\tif (Camp.Order < 0)\n"
       "\t\t\t{\n"
       "\t\t\t\tOrder = Candidate;\n"
       "\t\t\t}\n"
       "\t\t\telse if ((int32)Candidate != Camp.Order)\n"
       "\t\t\t{\n"
       "\t\t\t\tif (Camp.Pending == (int32)Candidate) { ++Camp.PendingCount; }\n"
       "\t\t\t\telse { Camp.Pending = (int32)Candidate; Camp.PendingCount = 1; }\n"
       "\t\t\t\tif (Camp.PendingCount >= 2) { Order = Candidate; }\n"
       "\t\t\t}\n"
       "\t\t\telse\n"
       "\t\t\t{\n"
       "\t\t\t\tCamp.Pending = -1;\n"
       "\t\t\t\tCamp.PendingCount = 0;\n"
       "\t\t\t}")
if old in t:
    t = t.replace(old, new, 1)
    wr(p, t)
    print("wilds cpp: order hysteresis")
else:
    print("wilds cpp: PATTERN MISSED")

# ---------- StandDown: one line per engagement ----------
p = "Source/VEIL/Private/Systems/VEILEnemy.cpp"
t = rd(p)
old = "\tStandDownUntil = FMath::Max(StandDownUntil, (float)GetWorld()->GetTimeSeconds() + Seconds);"
new = ("\tconst float Now = (float)GetWorld()->GetTimeSeconds();\n"
       "\t// A record, not a heartbeat: the wilds ask every 0.618 s and this used to\n"
       "\t// write 6,198 identical lines in two minutes, which is not evidence of\n"
       "\t// anything except that the log was being used as a debugger.\n"
       "\tconst bool bWasStanding = StandDownUntil > Now;\n"
       "\tStandDownUntil = FMath::Max(StandDownUntil, Now + Seconds);")
if old in t:
    t = t.replace(old, new, 1)
    print("stand down: transition detected")
else:
    print("stand down: TIMER PATTERN MISSED")

old_log = ("\tUE_LOG(LogVEIL, Log, TEXT(\"PHI STAND DOWN: %s lets the returned runner go for %.2fs.\"),\n"
           "\t\t*GetActorNameOrLabel(), Seconds);")
new_log = ("\tif (!bWasStanding)\n"
           "\t{\n"
           "\t\tUE_LOG(LogVEIL, Log, TEXT(\"PHI STAND DOWN: %s lets the returned runner go for %.2fs.\"),\n"
           "\t\t\t*GetActorNameOrLabel(), Seconds);\n"
           "\t}")
if old_log in t:
    t = t.replace(old_log, new_log, 1)
    print("stand down: log gated on the transition")
else:
    print("stand down: LOG PATTERN MISSED")

wr(p, t)
