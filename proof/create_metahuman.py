"""VEIL OMNI - create a real MetaHuman with the in-editor MetaHuman Creator.

Uses the plugin's own supported path (the same one its shipped Python tests use):
create the asset with UMetaHumanCharacterFactoryNew, add it for editing, import the
face/body templates that ship with the plugin, then build the MetaHuman.

Cloud steps (auto-rigging, texture synthesis) are only requested if the build reports
it still needs them, so a template-only character can build offline.
"""
import time

import unreal

LOG = r"D:\VEIL-OMNI\proof\metahuman_creation.txt"
PKG_PATH = "/Game/MetaHumans"
ASSET_NAME = "MH_Runner"
BUILD_PATH = "/Game/MetaHumans/MH_Runner_Built"
COMMON_PATH = "/Game/MetaHumans/MH_Runner_Built/Common"


def log(msg):
    line = "[{}] {}".format(round(time.time() - T0, 1), msg)
    print(line)
    with open(LOG, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")


def flush():
    try:
        unreal.SystemLibrary.exec_console_command("FlushLog", None, None)
    except Exception:
        pass


T0 = time.time()
unreal.log("VEIL: MetaHuman Creator run - start")

# ---------------------------------------------------------------- create asset
asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
factory = unreal.new_object(type=unreal.MetaHumanCharacterFactoryNew)
mh = asset_tools.create_asset(asset_name=ASSET_NAME, package_path=PKG_PATH,
                              asset_class=unreal.MetaHumanCharacter, factory=factory)
if mh is None:
    log("FAILED: could not create MetaHuman Character asset")
    raise SystemExit(1)
log("created asset: {} (class {})".format(mh.get_path_name(), mh.get_class().get_name()))

sub = unreal.get_editor_subsystem(unreal.MetaHumanCharacterEditorSubsystem)

# --------------------------------------------------------------- add for edit
added = sub.try_add_object_to_edit(character=mh)
log("try_add_object_to_edit: {}".format(added))
if not added:
    log("FAILED: subsystem refused to open the asset for editing")
    raise SystemExit(1)

# --------------------------------------------------- import shipped templates
face = unreal.EditorAssetLibrary.load_asset("/MetaHumanCharacter/Face/SKM_Face")
body = unreal.EditorAssetLibrary.load_asset("/MetaHumanCharacter/Body/IdentityTemplate/SKM_Body")
log("template face={} body={}".format(bool(face), bool(body)))

params = unreal.ImportFromTemplateParams()
params.use_eye_meshes = True
params.use_teeth_mesh = True
params.alignment_options = unreal.MetaHumanAlignmentOptions.SCALING_ROTATION_TRANSLATION

face_result = sub.import_from_template(mh, face, None, None, None, params)
log("import_from_template: {}".format(face_result))

# Save immediately: whatever happens at build time, a real MetaHuman asset exists on disk.
unreal.EditorAssetLibrary.save_loaded_asset(mh, only_if_is_dirty=False)
log("saved character asset early: {}".format(mh.get_path_name()))

if body is not None:
    body_result = sub.import_body_from_template(mh, body, face, params) \
        if hasattr(sub, "import_body_from_template") else None
    log("import_body_from_template: {}".format(body_result))

# ------------------------------------------------------------- what we need
can_build = sub.can_build_meta_human(character=mh)
log("can_build_meta_human (before rigging): {}".format(can_build))

if not can_build:
    log("requesting auto-rigging (Epic cloud) - blocking")
    rig = unreal.MetaHumanCharacterAutoRiggingRequestParams()
    rig.blocking = True
    rig.report_progress = False
    rig.rig_type = unreal.MetaHumanRigType.JOINTS_ONLY
    sub.request_auto_rigging(character=mh, params=rig)
    log("auto-rigging returned")

    log("requesting texture sources (Epic cloud) - blocking")
    tex = unreal.MetaHumanCharacterTextureRequestParams()
    tex.blocking = True
    tex.report_progress = False
    sub.request_texture_sources(character=mh, params=tex)
    log("texture sources returned; has_high_res_textures={}".format(
        getattr(mh, "has_high_resolution_textures", "?")))

can_build = sub.can_build_meta_human(character=mh)
log("can_build_meta_human (before build): {}".format(can_build))
if not can_build:
    log("STOP: the creator says this character is not buildable yet - see lines above")
    sub.remove_object_to_edit(character=mh)
    raise SystemExit(2)

# ------------------------------------------------------------------- build it
build = unreal.MetaHumanCharacterEditorBuildParameters()
build.pipeline_type = unreal.MetaHumanDefaultPipelineType.OPTIMIZED
build.pipeline_quality = unreal.MetaHumanQualityLevel.MEDIUM
build.absolute_build_path = BUILD_PATH
build.common_folder_path = COMMON_PATH
build.enable_wardrobe_item_validation = False
log("build_meta_human -> {} ({})".format(BUILD_PATH, "OPTIMIZED/MEDIUM"))
try:
    sub.build_meta_human(character=mh, params=build)
    log("build_meta_human returned cleanly")
except Exception as exc:
    log("build_meta_human raised: {}".format(exc))

# -------------------------------------------------------------------- persist
sub.remove_object_to_edit(character=mh)
unreal.EditorAssetLibrary.save_asset(mh.get_path_name(), only_if_is_dirty=False)
try:
    sub.save_meta_human(mh) if hasattr(sub, "save_meta_human") else None
except Exception as exc:
    log("save_meta_human not available: {}".format(exc))

# ---------------------------------------------------------------- what it made
built = unreal.EditorAssetLibrary.list_assets(BUILD_PATH, recursive=True, include_folder=False)
log("built assets under {}: {}".format(BUILD_PATH, len(built)))
for path in built[:40]:
    log("   {}".format(path))

meshes = [a for a in built if "SKM_" in a or "SK_" in a]
log("skeletal meshes produced: {}".format(len(meshes)))
log("DONE in {}s".format(round(time.time() - T0, 1)))
unreal.SystemLibrary.quit_editor()
