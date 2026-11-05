"""Record camera and exposure, plus the hub centre clearing."""

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
HUB = "C:/Users/Admin/HarkGame/Scripts/ed_hub_v3.py"
notes = []


def fix(path, pairs):
    s = open(path, encoding="utf-8", newline="").read().replace("\r\n", "\n")
    for old, new in pairs:
        if new in s:
            notes.append("done: " + old.split("\n")[0][:40])
        elif old in s:
            s = s.replace(old, new, 1)
            notes.append("patched: " + old.split("\n")[0][:40])
        else:
            notes.append("MISS: " + old.split("\n")[0][:40])
    open(path, "w", encoding="utf-8", newline="").write(s)


fix(GM, [
    # The Hub record used to stand 162 units from the plaza centre, 1.8 km up,
    # so it saw one paving tile. Back it off along the bearing for a real aerial.
    ("\tstatic const double BackPow[5] = { 0.0, 2.0, 2.0, 2.0, 2.0 };\n"
     "\tstatic const double UpPow[5] = { 5.0, 1.6, 1.6, 1.6, 1.6 };",
     "\tstatic const double BackPow[5] = { 4.6, 2.6, 2.6, 2.6, 2.6 };\n"
     "\tstatic const double UpPow[5] = { 5.6, 1.6, 1.6, 1.6, 1.6 };"),
    ("\t\tFVector2D(900.0, 0.0), FVector2D(0.0, 3500.0), FVector2D(-450.0, 780.0),",
     "\t\tFVector2D(0.0, 900.0), FVector2D(0.0, 3500.0), FVector2D(-450.0, 780.0),"),
    # One stop brighter: -phi was set to cure blown highlights and overcorrected
    # into near-black frames.
    ("\tS.AutoExposureBias = -1.618033988749895f;",
     "\tS.AutoExposureBias = -0.618033988749895f;"),
])

fix(HUB, [
    # The Runner spawns at the plaza centre; the monument cannot stand there.
    ("          V + \"M_VEIL_Signal.M_VEIL_Signal\", 0.0, 0.0, deck, reach(-0.55), reach(1.05))",
     "          V + \"M_VEIL_Signal.M_VEIL_Signal\", 0.0, 1350.0, deck, reach(-0.55), reach(1.05))"),
    ("    light(sp, 0.0, 0.0, deck + reach(1.1), 5200.0, 5200.0)",
     "    light(sp, 0.0, 1350.0, deck + reach(1.1), 5200.0, 5200.0)\n"
     "    light(sp, 0.0, 0.0, deck + reach(1.35), 3400.0, 4200.0)"),
    # Overlap the paving by 14 units: the old 6-unit gap opened black seams.
    ("                     x, y, deck - 24.0, tile - 6.0, 24.0, tags=(\"VEIL_PAVING\",)):",
     "                     x, y, deck - 24.0, tile + 14.0, 24.0, tags=(\"VEIL_PAVING\",)):"),
])

print("\n".join(notes))
