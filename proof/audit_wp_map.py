# VEIL - AUDIT THE WORLD PARTITION MAP THROUGH THE PARTITION ITSELF
#
# A WP level only loads the cells a streaming source asks for, so get_all_level_actors() in a
# headless commandlet returns the persistent level's handful of actors. The partition inventory is
# the truth. This binding exposes get_actor_descs() with no arguments.
import unreal

MAP = "/Game/Maps/VEILDistrictWP"
OUT = r"D:\VEIL-OMNI\Saved\Logs\wp_audit.txt"
lines = []


def say(msg):
    line = "[VEIL WPA] " + str(msg)
    lines.append(line)
    print(line)


def main():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    say("load {} -> {}".format(MAP, les.load_level(MAP)))
    say("actors loaded without a streaming source: {}".format(
        len(unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors())))

    try:
        descs = unreal.WorldPartitionBlueprintLibrary.get_actor_descs()
        say("actor descs in the partition: {}".format(len(descs)))
        classes = {}
        for d in descs:
            try:
                cls = str(d.get_editor_property("actor_class").get_name())
            except Exception:  # noqa: BLE001
                cls = "?"
            classes[cls] = classes.get(cls, 0) + 1
        top = sorted(classes.items(), key=lambda kv: -kv[1])[:10]
        say("top classes: " + ", ".join("{} x{}".format(k, v) for k, v in top))
    except Exception as exc:  # noqa: BLE001
        say("partition inventory failed: {}".format(exc))

    with open(OUT, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")


main()
