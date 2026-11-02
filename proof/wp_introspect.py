# VEIL - WHAT WORLD PARTITION CAN THIS BUILD ACTUALLY DO?
#
# Converting a level is destructive-ish, so the API surface is checked first: which classes the
# Python binding exposes, whether the current map already has a world partition object, and what
# the world settings say. Reads only; nothing is saved.
import unreal

MAP = "/Game/Maps/VEILDistrict"


def log(msg):
    unreal.log("[VEIL WP] " + str(msg))


def main():
    api = sorted([n for n in dir(unreal) if "Partition" in n])
    log("APIs with 'Partition': " + (", ".join(api) if api else "none"))

    for n in ("WorldPartitionEditorSubsystem", "WorldPartitionSubsystem",
              "WorldPartitionBlueprintLibrary", "DataLayerEditorSubsystem",
              "LevelInstanceEditorSubsystem", "HLODEditorSubsystem"):
        log("has {}: {}".format(n, hasattr(unreal, n)))

    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    log("load_level: {}".format(les.load_level(MAP) if les else "no subsystem"))

    sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    world = sub.get_editor_world() if sub else None
    log("world: {}".format(world.get_name() if world else "None"))
    if not world:
        return

    for attr in ("get_world_partition", "world_partition"):
        try:
            wp = getattr(world, attr)
            val = wp() if callable(wp) else wp
            log("{}() -> {}".format(attr, val))
        except Exception as exc:  # noqa: BLE001
            log("{} not usable: {}".format(attr, exc))

    try:
        ws = world.get_world_settings()
        log("world settings: {}".format(ws.get_class().get_name() if ws else "None"))
        for prop in ("enable_world_partition", "world_partition_class"):
            try:
                log("  {} = {}".format(prop, ws.get_editor_property(prop)))
            except Exception as exc:  # noqa: BLE001
                log("  {} unavailable: {}".format(prop, exc))
    except Exception as exc:  # noqa: BLE001
        log("world settings unavailable: {}".format(exc))

    # How many actors are external (partitioned) versus in the level file itself.
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    ext = 0
    for a in actors[:400]:
        try:
            if a.get_editor_property("external_package"):
                ext += 1
        except Exception:  # noqa: BLE001
            continue
    log("actors: {} (external among first 400: {})".format(len(actors), ext))


main()
