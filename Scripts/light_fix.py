"""Nothing in a town should render as a black hole."""

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


# A sky light at phi could not fill the mesa's shadowed side.
swap("SkyLight->SetIntensity(static_cast<float>(PHI_Constants::Phi));",
     "SkyLight->SetIntensity(static_cast<float>(PHI_Constants::PhiPow(2.4)));",
     "skylight")

# The sun is the only strong key; 61800 left whole walls in black.
swap("L->SetIntensity(61800.f);", "L->SetIntensity(88000.f);", "sun")

# The last record shot looked at a distant mountain, not at the town.
swap("\t\tFVector(0.0, -5000.0, 0.0)\n", "\t\tFVector(0.0, -2600.0, 0.0)\n", "anchor5")
swap("\t\tFVector2D(2800.0, 500.0), FVector2D(0.0, -5000.0)\n",
     "\t\tFVector2D(2800.0, 500.0), FVector2D(0.0, 600.0)\n", "aim5")

open(GM, "w", encoding="utf-8", newline="").write(s)
print("\n".join(notes))
