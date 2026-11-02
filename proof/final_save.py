# THE WIN LINE. Save VEILDistrict, then prove the save by reading the map back.
#
# A save call that returns True and a map that actually changed are two different
# claims. This script makes the first, reopens the file, counts it again, and only
# writes "SAVE RESULT True" when the read-back agrees: actors present, zero engine
# placeholders, and a sun and sky light still in the level.
#
# NON-DELETE: nothing here deletes or moves a file. One world setting is set, the
# level is saved in place, and the audit is written next to it.
import unreal

MAP = "/Game/Maps/VEILDistrict"
OUT = r"D:\VEIL-OMNI\seat719_out.txt"
PLACEHOLDER_PREFIX = "/Engine/BasicShapes/"


def log(msg):
    unreal.log("[VEIL SAVE] " + str(msg))


def census():
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    statics = [a for a in actors if isinstance(a, unreal.StaticMeshActor)]
    placeholders = 0
    for a in statics:
        comp = a.get_editor_property("static_mesh_component")
        mesh = comp.get_editor_property("static_mesh") if comp else None
        if mesh and mesh.get_path_name().startswith(PLACEHOLDER_PREFIX):
            placeholders += 1
    lights = 0
    for a in actors:
        if a.get_class().get_name() in ("DirectionalLight", "SkyLight"):
            lights += 1
    return len(actors), len(statics), placeholders, lights


def main():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    les.load_level(MAP)
    before = census()
    log("BEFORE actors=%d meshes=%d placeholders=%d lights=%d" % before)

    # Static lighting off for the level itself: Lumen + VSM, nothing to bake.
    try:
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        world.get_world_settings().set_editor_property("force_no_precomputed_lighting", True)
        log("force_no_precomputed_lighting = True")
    except Exception as exc:  # noqa: BLE001
        log("world settings failed: %s" % exc)

    saved = False
    try:
        saved = unreal.EditorLoadingAndSavingUtils.save_map(
            unreal.EditorAssetLibrary.load_asset(MAP), MAP)
        log("save_map -> %s" % saved)
    except Exception as exc:  # noqa: BLE001
        log("save_map failed: %s" % exc)
        try:
            saved = unreal.EditorLevelLibrary.save_current_level()
            log("fallback save_current_level -> %s" % saved)
        except Exception as exc2:  # noqa: BLE001
            log("fallback failed: %s" % exc2)

    # READ IT BACK.
    after = (0, 0, 0, 0)
    try:
        les.load_level(MAP)
        after = census()
        log("AFTER  actors=%d meshes=%d placeholders=%d lights=%d" % after)
    except Exception as exc:  # noqa: BLE001
        log("read-back failed: %s" % exc)

    ok = bool(saved) and after[0] > 0 and after[2] == 0 and after[3] >= 2
    lines = [
        "SAVE ok=%s used=EditorLoadingAndSavingUtils.save_map(world, MAP)" % saved,
        "BEFORE actors=%d meshes=%d placeholders=%d lights=%d" % before,
        "AFTER  actors=%d meshes=%d placeholders=%d lights=%d" % after,
        "SAVE RESULT %s" % ("True" if ok else "False"),
        "VERIFY_PASS=%s MAP=%s" % (ok, MAP),
    ]
    with open(OUT, "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines) + "\n")
    log(" | ".join(lines))
    return 0


exit_code = 0
try:
    exit_code = main()
except Exception as top_exc:  # noqa: BLE001
    unreal.log("[VEIL SAVE] UNCAUGHT %s" % top_exc)
    exit_code = 1
unreal.log("[VEIL SAVE] exit=%d" % exit_code)
