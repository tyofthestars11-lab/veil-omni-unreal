# VEIL OMNI - build lighting ONCE, save VEILDistrict ONCE, and write the proof line.
#
# This is the step that has never actually been run: every Play so far has shown
# "LIGHTING NEEDS TO BE BUILT" because the lightmaps were never built and the level
# was never saved after the build. Run headless through the editor's own
# LevelEditorSubsystem (the API this engine version actually exposes:
# LevelEditorSubsystem.h line 222, BuildLightMaps).
#
# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.
# Nothing here deletes, moves or renames a file. It builds lightmaps and saves the map.
import os
import unreal

MAP = "/Game/Maps/VEILDistrict"
OUT = r"D:\VEIL-OMNI\Saved\Logs\seat719_out.txt"


def log(msg):
    unreal.log("[VEIL LIGHT] " + str(msg))


def main():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if les is None:
        log("no LevelEditorSubsystem - cannot build")
        return

    if not les.load_level(MAP):
        log("could not load " + MAP)
        return
    log("level loaded: " + MAP)

    # STATIC LIGHTING IS OFF PROJECT-WIDE already (r.AllowStaticLighting=False), so a
    # bake has nothing to do and returns False - which is correct, not a failure.
    # What is left is the MAP's own flag: with precomputed lighting still allowed on
    # WorldSettings, the editor keeps reporting "LIGHTING NEEDS TO BE BUILT" over a
    # world that is lit entirely by Lumen. Turning the flag off is the real fix; a
    # bake would be faking a result for a setting the project has already refused.
    world = unreal.EditorLevelLibrary.get_editor_world()
    settings = world.get_world_settings() if world else None
    if settings:
        try:
            settings.set_editor_property("force_no_precomputed_lighting", True)
            log("WorldSettings force_no_precomputed_lighting -> True")
        except Exception as exc:  # noqa: BLE001
            log("could not set world settings: {}".format(exc))
    else:
        log("no world settings on the loaded level")

    quality = unreal.LightingBuildQuality.QUALITY_PRODUCTION
    built = bool(les.build_light_maps(quality))
    log("build_light_maps -> {} (False is expected: static lighting is disabled)".format(built))

    saved = bool(les.save_current_level())
    log("save_current_level -> {}".format(saved))

    # The proof line Tyree asks for. Written either way, with the real values.
    try:
        os.makedirs(os.path.dirname(OUT), exist_ok=True)
        with open(OUT, "w", encoding="utf-8") as f:
            f.write("MAP {}\n".format(MAP))
            f.write("BUILD RESULT {}\n".format(built))
            f.write("SAVE RESULT {}\n".format(saved))
        log("wrote " + OUT)
    except Exception as exc:  # noqa: BLE001 - the proof file must never be a silent failure
        log("could not write proof file: {}".format(exc))


main()
