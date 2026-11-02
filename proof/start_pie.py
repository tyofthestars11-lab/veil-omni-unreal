"""Start PIE on a bare map so the world builder seats machines, then leave it running.

The game's own log category (LogVEIL) is Log-level; in a -game run the console
verbosity is Display, which hides it. The editor log keeps Log, so PIE is where the
AI's rung lines are visible without touching any project config.
"""
import unreal

LOG = r"D:\VEIL-OMNI\proof\phi_ai_pie.txt"


def log(msg):
    print(msg)
    with open(LOG, "a", encoding="utf-8") as f:
        f.write(str(msg) + "\n")


les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
log("load Entry: {}".format(les.load_level("/Engine/Maps/Entry")))
les.editor_request_begin_play()
log("PIE requested - leaving the editor running")
