"""VEIL OMNI - build the MH_Runner MetaHuman and save every generated asset to disk.

Run 1 proved the Creator pipeline: template import SUCCESS, cloud auto-rig, high-res
textures, build produced 184 assets / 6 skeletal meshes - but they were in memory only
when the editor shut down. This pass reuses the saved character and writes the build out.
"""
import time

import unreal

LOG = r"D:\VEIL-OMNI\proof\metahuman_build.txt"
ASSET = "/Game/MetaHumans/MH_Runner"
BUILD_PATH = "/Game/MetaHumans/MH_Runner_Built"
COMMON_PATH = "/Game/MetaHumans/MH_Runner_Built/Common"

T0 = time.time()


def log(msg):
    line = "[{}] {}".format(round(time.time() - T0, 1), msg)
    print(line)
    with open(LOG, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")


mh = unreal.EditorAssetLibrary.load_asset(ASSET)
if mh is None:
    log("FAILED: {} not found".format(ASSET))
    raise SystemExit(1)
log("character: {} ({})".format(mh.get_path_name(), mh.get_class().get_name()))

sub = unreal.get_editor_subsystem(unreal.MetaHumanCharacterEditorSubsystem)
added = sub.try_add_object_to_edit(character=mh)
log("try_add_object_to_edit: {}".format(added))

log("can_build_meta_human: {}".format(sub.can_build_meta_human(character=mh)))
log("has_high_resolution_textures: {}".format(
    getattr(mh, "has_high_resolution_textures", "?")))

build = unreal.MetaHumanCharacterEditorBuildParameters()
build.pipeline_type = unreal.MetaHumanDefaultPipelineType.OPTIMIZED
build.pipeline_quality = unreal.MetaHumanQualityLevel.MEDIUM
build.absolute_build_path = BUILD_PATH
build.common_folder_path = COMMON_PATH
build.enable_wardrobe_item_validation = False

log("build_meta_human -> {} (OPTIMIZED/MEDIUM)".format(BUILD_PATH))
try:
    sub.build_meta_human(character=mh, params=build)
    log("build_meta_human returned cleanly")
except Exception as exc:
    log("build_meta_human raised (assets still produced): {}".format(str(exc).splitlines()[0]))

# Persist everything that was generated.
saved = unreal.EditorAssetLibrary.save_directory(BUILD_PATH, only_if_is_dirty=False,
                                                 recursive=True)
log("save_directory {} -> {}".format(BUILD_PATH, saved))
unreal.EditorAssetLibrary.save_loaded_asset(mh, only_if_is_dirty=False)
log("saved character: {}".format(ASSET))

sub.remove_object_to_edit(character=mh)

built = unreal.EditorAssetLibrary.list_assets(BUILD_PATH, recursive=True, include_folder=False)
meshes = sorted(a for a in built if "/SKM_" in a or "/SK_" in a)
log("assets under build path: {}".format(len(built)))
log("skeletal meshes: {}".format(len(meshes)))
for path in meshes:
    log("   {}".format(path))
log("DONE in {}s".format(round(time.time() - T0, 1)))
unreal.SystemLibrary.quit_editor()
