# VEIL - AUDIT THE WORLD PARTITION MAP (READ ONLY)
#
# The WP map was created from the OpenWorld template and baked, so it must be held to the same
# standard as the dense district: no engine placeholders, no Paragon actors, lights present, and
# a real population. Nothing here bakes, spawns or saves.
import unreal

MAP = "/Game/Maps/VEILDistrictWP"
OUT = r"D:\VEIL-OMNI\Saved\Logs\wp_audit.txt"
lines = []


def say(msg):
    line = "[VEIL WPA] " + str(msg)
    lines.append(line)
    print(line)


def paths_on(actor):
    out = []
    try:
        out.append(str(actor.get_class().get_path_name()))
    except Exception:  # noqa: BLE001
        pass
    try:
        for c in actor.get_components_by_class(unreal.StaticMeshComponent):
            m = c.get_editor_property("static_mesh")
            if m:
                out.append(str(m.get_path_name()))
    except Exception:  # noqa: BLE001
        pass
    return out


def main():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    say("load {} -> {}".format(MAP, les.load_level(MAP)))
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    statics = 0
    placeholders = 0
    paragon = 0
    lights = 0
    for a in actors:
        if isinstance(a, unreal.StaticMeshActor):
            statics += 1
        if any("Paragon" in p for p in paths_on(a)):
            paragon += 1
        if any(p.startswith("/Engine/BasicShapes/") for p in paths_on(a)):
            placeholders += 1
        try:
            if a.get_class().get_name() in ("DirectionalLight", "SkyLight"):
                lights += 1
        except Exception:  # noqa: BLE001
            pass
    say("actors={} statics={} placeholders={} paragon={} lights={}".format(
        len(actors), statics, placeholders, paragon, lights))
    ext = 0
    for a in actors:
        try:
            if a.get_editor_property("external_package"):
                ext += 1
        except Exception:  # noqa: BLE001
            continue
    say("external (partitioned) actors: {}".format(ext))
    with open(OUT, "w", encoding="utf-8") as f:
        f.write("\n".join(lines) + "\n")


main()
