"""The game lights itself in BeginPlay, so the level's lights never applied."""

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(GM, encoding="utf-8", newline="").read().replace("\r\n", "\n")
notes = []


def swap(old, new, tag):
    global s
    if new in s:
        notes.append(tag + ": already")
    elif old in s:
        s = s.replace(old, new)
        notes.append(tag + ": patched")
    else:
        notes.append(tag + ": MISS")


# A clean sky at 88000 with a phi^2.4 sky light renders the town sheet white.
swap("L->SetIntensity(88000.f);", "L->SetIntensity(24000.f);", "sun")

swap("SkyLight->SetIntensity(static_cast<float>(PHI_Constants::PhiPow(2.4)));",
     "SkyLight->SetIntensity(static_cast<float>(PHI_Constants::PhiPow(0.8)));",
     "skylight")

# Two phi stops down instead of less than one: -(phi + 1/phi) = -2.236.
swap("\tS.AutoExposureBias = -0.618033988749895f;",
     "\tS.AutoExposureBias = static_cast<float>(-(PHI_Constants::Phi + PHI_Constants::PhiPow(-1.0)));",
     "exposure")

open(GM, "w", encoding="utf-8", newline="").write(s)
print("\n".join(notes))
