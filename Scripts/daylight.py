"""Manual exposure makes brightness proportional. 24000 reads as night: use daylight."""

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


swap("L->SetIntensity(24000.f);", "L->SetIntensity(130000.f);", "sun")
swap("SkyLight->SetIntensity(static_cast<float>(PHI_Constants::PhiPow(0.8)));",
     "SkyLight->SetIntensity(static_cast<float>(PHI_Constants::PhiPow(2.0)));",
     "skylight")

open(GM, "w", encoding="utf-8", newline="").write(s)
print("\n".join(notes))
