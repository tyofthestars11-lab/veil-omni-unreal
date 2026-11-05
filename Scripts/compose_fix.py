"""Compose the record shots, size the capitals, retire the rock probe."""
import re

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
HUB = "C:/Users/Admin/HarkGame/Scripts/ed_hub_v6.py"
notes = []


def read(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def write(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


# --- cameras: 565 units back put every shot inside a wall
gm = read(GM)
old = "\tstatic const double BackPow[5] = { 4.6, 2.6, 2.6, 2.6, 2.6 };"
new = "\tstatic const double BackPow[5] = { 4.6, 3.6, 3.6, 3.6, 3.6 };"
if new in gm:
    notes.append("cameras already back")
elif old in gm:
    gm = gm.replace(old, new, 1)
    notes.append("cameras pulled back to 929")
else:
    notes.append("MISS BackPow")

# --- the rock probe has done its job; keep the clearance lift, drop the spam
block = gm.find("\t{\n\t\t// Ignore everything we authored and ask only where the map's own rock is.")
if block >= 0:
    end = gm.find("\t\t\tHits, Lo, Hi, AtX, AtY, Ignore.Num());\n\t}\n", block)
    if end > 0:
        end += len("\t\t\tHits, Lo, Hi, AtX, AtY, Ignore.Num());\n\t}\n")
        gm = gm[:block] + gm[end:]
        notes.append("rock probe removed")
    else:
        notes.append("MISS probe end")
else:
    notes.append("probe already gone")
write(GM, gm)

# --- capitals: a 529-wide arch is not a capital
hub = read(HUB)
old_cap = """        if put(sp, PROPS + "SM_AncientDecorArch.SM_AncientDecorArch",
               V + "M_VEIL_Gold.M_VEIL_Gold", x, y, DECK + 520.0, yaw=i * 30.0):"""
new_cap = """        if put(sp, PROPS + "SM_AncientDecorArch.SM_AncientDecorArch",
               V + "M_VEIL_Gold.M_VEIL_Gold", x, y, DECK + 520.0, yaw=i * 30.0,
               sx=0.4, sy=1.3, sz=0.55):"""
if new_cap in hub:
    notes.append("capitals already sized")
elif old_cap in hub:
    hub = hub.replace(old_cap, new_cap, 1)
    notes.append("capitals scaled to a capital")
else:
    notes.append("MISS capitals")
write(HUB, hub)

print("\n".join(notes))
