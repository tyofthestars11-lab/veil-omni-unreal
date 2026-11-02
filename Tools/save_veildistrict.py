# Save VEILDistrict with static lighting switched off for the level itself.
#
# Why: the editor showed "LIGHTING NEEDS TO BE REBUILT (4,134 unbuilt objects)".
# This project is Lumen + Virtual Shadow Maps, so there is nothing to bake.
# Setting ForceNoPrecomputedLighting on the level's World Settings is the
# authoring-side way to say that - it lives in the map, not just in an ini.
#
# Nothing is deleted. The level is loaded, one world-setting is set, and the
# level is saved in place.

import unreal

unreal.log("VEIL SAVE: SCRIPT REACHED (module import)")

LEVEL_PATH = "/Game/Maps/VEILDistrict"


def log(msg):
    unreal.log("VEIL SAVE: " + msg)


def main():
    # Load the level through the editor subsystem (UE5.1+).
    try:
        les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        les.load_level(LEVEL_PATH)
        log("loaded " + LEVEL_PATH)
    except Exception as exc:  # noqa: BLE001
        log("load_level failed: %s" % exc)
        return 1

    # Mark the level fully dynamic so it never asks for a lightmass build.
    try:
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        ws = world.get_world_settings()
        ws.set_editor_property("force_no_precomputed_lighting", True)
        log("force_no_precomputed_lighting = True")
    except Exception as exc:  # noqa: BLE001
        log("world settings failed: %s" % exc)

    # Save the level in place.
    try:
        saved = unreal.EditorLoadingAndSavingUtils.save_map(
            unreal.EditorAssetLibrary.load_asset(LEVEL_PATH), LEVEL_PATH)
        log("save_map -> %s" % saved)
    except Exception as exc:  # noqa: BLE001
        log("save_map failed: %s" % exc)
        try:
            ok = unreal.EditorLevelLibrary.save_current_level()
            log("fallback save_current_level -> %s" % ok)
        except Exception as exc2:  # noqa: BLE001
            log("fallback failed: %s" % exc2)
            return 1

    return 0


try:
    exit_code = main()
except Exception as top_exc:  # noqa: BLE001
    unreal.log("VEIL SAVE: UNCAUGHT %s" % top_exc)
    exit_code = 1
unreal.log("VEIL SAVE: exit=%d" % exit_code)
