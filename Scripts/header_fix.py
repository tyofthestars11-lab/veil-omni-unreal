"""Declarations must live after GENERATED_BODY(), not after the class brace."""

ROOT = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"
notes = []


def read(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def write(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


# --- Runner: walk clip member
p = ROOT + "VeilRunner.h"
h = read(p)
if "WalkAnim" not in h:
    if "class UAnimSequence" not in h:
        h = h.replace("#include \"CoreMinimal.h\"",
                      "#include \"CoreMinimal.h\"\n\nclass UAnimSequence;", 1)
        if "class UAnimSequence" not in h:
            h = "class UAnimSequence;\n" + h
    i = h.find("GENERATED_BODY()")
    if i < 0:
        notes.append("runner.h: MISS GENERATED_BODY")
    else:
        end = h.find("\n", i)
        h = h[:end + 1] + "\t// Locomotion clip from the real character set.\n\tUPROPERTY()\n\tUAnimSequence* WalkAnim = nullptr;\n" + h[end + 1:]
        write(p, h)
        notes.append("runner.h: WalkAnim declared")
else:
    notes.append("runner.h: already declared")

# --- Controller: pause member and function
p = ROOT + "VeilPlayerController.h"
c = read(p)
if "OnPauseToggle" not in c:
    i = c.find("GENERATED_BODY()")
    if i < 0:
        notes.append("controller.h: MISS GENERATED_BODY")
    else:
        end = c.find("\n", i)
        add = ("\n\t// Pause state, read by the HUD.\n\tbool bPauseMenu = false;\n"
               "\tvoid OnPauseToggle();\n")
        c = c[:end + 1] + add + c[end + 1:]
        write(p, c)
        notes.append("controller.h: pause declared")
else:
    notes.append("controller.h: already declared")

print("\n".join(notes))
