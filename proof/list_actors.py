# VEIL - WHAT CLASSES ARE ACTUALLY SAVED IN THE LEVEL?
#
# The packaged game crashed with the callstack pointing at AOMNICache's constructor
# running on the async loading thread. Any class saved in the level is constructed
# that way on load, so every saved class whose constructor touches the renderer is a
# crash waiting for its turn. This lists them, so they can all be fixed at once
# instead of one crash per eight-minute cook.
import unreal

MAP = "/Game/Maps/VEILDistrict"
OUT = r"D:\VEIL-OMNI\Saved\Logs\level_classes.txt"


def log(msg):
    unreal.log("[VEIL CLASSES] " + str(msg))


def main():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not les.load_level(MAP):
        log("could not load " + MAP)
        return

    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    counts = {}
    for a in actors:
        name = a.get_class().get_name()
        counts[name] = counts.get(name, 0) + 1

    rows = sorted(counts.items(), key=lambda kv: -kv[1])
    lines = ["ACTOR CLASSES IN {} ({} actors)".format(MAP, len(actors))]
    for name, count in rows:
        lines.append("  {:>5} x {}".format(count, name))

    for line in lines[:22]:
        log(line)
    try:
        with open(OUT, "w", encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")
        log("wrote " + OUT)
    except Exception as exc:  # noqa: BLE001
        log("could not write: {}".format(exc))


main()
