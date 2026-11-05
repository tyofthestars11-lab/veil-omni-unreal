"""HUD reads the pause flag, so these members must be public."""

ROOT = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"
notes = []


def read(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def write(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


def public_insert(path, block, tag):
    s = read(path)
    if block.strip() in s:
        # drop the old placement first
        s = s.replace(block, "")
    if "public:" not in s:
        notes.append(tag + ": MISS public")
        return
    i = s.find("public:")
    end = s.find("\n", i)
    s = s[:end + 1] + block + s[end + 1:]
    write(path, s)
    notes.append(tag + ": public")


public_insert(ROOT + "VeilPlayerController.h",
              "\t// Pause state, read by the HUD.\n\tbool bPauseMenu = false;\n"
              "\tvoid OnPauseToggle();\n", "controller.h")

public_insert(ROOT + "VeilRunner.h",
              "\t// Locomotion clip from the real character set.\n\tUPROPERTY()\n"
              "\tUAnimSequence* WalkAnim = nullptr;\n", "runner.h")

print("\n".join(notes))
