"""Probe the editor Python surface. Reads only; never touches the map."""
import os
import sys

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_probe.txt")


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


open(OUT, "w", encoding="utf-8").close()
w("python %s" % sys.version.split()[0])

try:
    import unreal
    w("unreal module: OK")
except Exception as err:  # noqa: BLE001
    w("unreal module FAILED %r" % err)
    raise SystemExit(0)

for name in ("EditorActorSubsystem", "EditorLevelLibrary", "LevelEditorSubsystem",
             "EditorLoadingAndSavingUtils", "StaticMeshActor", "PointLight",
             "PointLightComponent", "ComponentMobility", "CollisionEnabled", "EditorAssetLibrary"):
    w("%s: %s" % (name, "yes" if hasattr(unreal, name) else "no"))

try:
    world = unreal.EditorLevelLibrary.get_editor_world()
    w("editor world: %s" % (world.get_name() if world else "none"))
except Exception as err:  # noqa: BLE001
    w("get_editor_world FAILED %r" % err)

try:
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    w("actor subsystem: %s" % ("ok" if actors else "none"))
except Exception as err:  # noqa: BLE001
    w("actor subsystem FAILED %r" % err)

try:
    all_actors = unreal.EditorLevelLibrary.get_all_level_actors()
    w("level actors: %d" % len(all_actors))
except Exception as err:  # noqa: BLE001
    w("get_all_level_actors FAILED %r" % err)

w("PROBE DONE")
