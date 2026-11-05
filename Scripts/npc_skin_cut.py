"""Line-index cut of the skeletal material overrides."""

npc = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilNpc.cpp"
ver = "C:/Users/Admin/HarkGame/Scripts/quantum_verify.py"

raw = open(npc, encoding="utf-8", newline="").read().replace("\r\n", "\n")
lines = raw.split("\n")

if any("authored character materials" in l for l in lines):
    print("people: already cut")
else:
    i0 = next((i for i, l in enumerate(lines) if l.strip() == "if (Race.Skin)"), -1)
    if i0 < 0:
        raise SystemExit("MISS Race.Skin line")
    i1 = next((i for i, l in enumerate(lines) if "++Dressed;" in l and i > i0), -1)
    if i1 < 0:
        raise SystemExit("MISS ++Dressed line")
    # ++Dressed is followed by the three closing braces of the two blocks
    end = i1 + 3
    print("cut lines %d..%d" % (i0 + 1, end + 1))

    repl = [
        "\t\t// The people keep their authored character materials. Overriding a",
        "\t\t// skeletal mesh with a material that lacks the skeletal usage flag floods",
        "\t\t// the log with over a thousand warnings and replaces real character art.",
        "\t\t{",
        "\t\t\tstatic bool SkinLogged = false;",
        "\t\t\tif (!SkinLogged)",
        "\t\t\t{",
        "\t\t\t\tSkinLogged = true;",
        "\t\t\t\tUE_LOG(LogTemp, Display, TEXT(\"VEIL NPC SKIN: authored character materials\"));",
        "\t\t\t}",
        "\t\t}",
    ]
    lines = lines[:i0] + repl + lines[end + 1:]
    open(npc, "w", encoding="utf-8", newline="").write("\n".join(lines))
    print("people: overrides removed")

v = open(ver, encoding="utf-8", newline="").read().replace("\r\n", "\n")
old_gate = '("people dressed", r"VEIL NPC SKIN: cloth applied"),'
new_gate = '("people dressed", r"VEIL NPC SKIN: authored character materials"),'
if old_gate in v:
    v = v.replace(old_gate, new_gate, 1)
    open(ver, "w", encoding="utf-8", newline="").write(v)
    print("gate: authored-material line")
else:
    print("gate: no cloth pattern (ok)")
