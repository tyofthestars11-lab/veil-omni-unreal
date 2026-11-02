# VEIL - WORLD PARTITION PROBE (INCREMENTAL FILE OUTPUT)
#
# unreal.log() does not reach stdout in this commandlet, and a script that only writes at the end
# loses everything if one line throws. So every finding is written immediately, and every probe
# is guarded. Reads only: nothing is created, converted or saved.
import unreal

OUT = r"D:\VEIL-OMNI\Saved\Logs\wp_probe.txt"
MAP = "/Game/Maps/VEILDistrict"


def say(msg):
    line = "[VEIL WPP] " + str(msg)
    print(line)
    try:
        with open(OUT, "a", encoding="utf-8") as f:
            f.write(line + "\n")
    except Exception:  # noqa: BLE001
        pass


def main():
    open(OUT, "w", encoding="utf-8").close()

    try:
        apis = sorted(str(n) for n in dir(unreal) if "Partition" in n)
        say("partition APIs: " + (", ".join(apis) if apis else "NONE"))
    except Exception as exc:  # noqa: BLE001
        say("API listing failed: {}".format(exc))

    for n in ("WorldPartitionEditorSubsystem", "WorldPartitionSubsystem",
              "WorldPartitionBlueprintLibrary", "LevelInstanceEditorSubsystem"):
        say("has {}: {}".format(n, hasattr(unreal, n)))

    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    try:
        say("level editor methods: " + ", ".join(sorted(str(m) for m in dir(les) if not str(m).startswith("_"))))
    except Exception as exc:  # noqa: BLE001
        say("method listing failed: {}".format(exc))

    # Templates: an OpenWorld template is how a level is born partitioned.
    try:
        ar = unreal.AssetRegistryHelpers.get_asset_registry()
        try:
            ar.search_all_assets(True)
        except Exception:  # noqa: BLE001
            pass
        found = []
        for path in ("/Engine/Maps/Templates", "/Engine/Maps"):
            try:
                for a in ar.get_assets_by_path(path, recursive=True):
                    found.append(str(a.package_name))
            except Exception as exc:  # noqa: BLE001
                say("path {} raised {}".format(path, exc))
        say("templates: " + (", ".join(sorted(set(found))[:15]) if found else "NONE"))
    except Exception as exc:  # noqa: BLE001
        say("asset registry unavailable: {}".format(exc))

    try:
        say("load_level {} -> {}".format(MAP, les.load_level(MAP)))
    except Exception as exc:  # noqa: BLE001
        say("load_level failed: {}".format(exc))

    try:
        sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        world = sub.get_editor_world() if sub else None
        say("world: {}".format(world.get_name() if world else "None"))
        if world:
            for attr in ("get_world_partition", "world_partition"):
                try:
                    obj = getattr(world, attr)
                    say("{} -> {}".format(attr, obj() if callable(obj) else obj))
                except Exception as exc:  # noqa: BLE001
                    say("{} unavailable: {}".format(attr, exc))
        actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
        say("actor count: {}".format(len(actors)))
    except Exception as exc:  # noqa: BLE001
        say("world probe failed: {}".format(exc))

    say("done")


main()
