"""Import the real CC0 assets into /Game/VEIL/CC0 and author palette materials.

Runs inside UnrealEditor-Cmd -run=pythonscript, so nothing is opened by hand.
"""
import os
import unreal

SRC = "C:/Users/Admin/HarkGame/Assets/CC0"
DEST = "/Game/VEIL/CC0"
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
EAL = unreal.EditorAssetLibrary


def import_files(paths, dest, label):
    tasks = []
    for p in paths:
        t = unreal.AssetImportTask()
        t.set_editor_property("filename", p)
        t.set_editor_property("destination_path", dest)
        t.set_editor_property("automated", True)
        t.set_editor_property("replace_existing", True)
        t.set_editor_property("save", True)
        tasks.append(t)
    if not tasks:
        return 0
    try:
        TOOLS.import_asset_tasks(tasks)
    except Exception as exc:
        unreal.log_warning("VEIL IMPORT: {} batch failed: {}".format(label, exc))
        return 0
    unreal.log("VEIL IMPORT: {} -> {} files".format(label, len(tasks)))
    return len(tasks)


def collect(folder, exts):
    found = []
    if not os.path.isdir(folder):
        return found
    for root, _dirs, files in os.walk(folder):
        for f in files:
            if f.lower().endswith(exts):
                found.append(os.path.join(root, f))
    return sorted(found)


textures = collect(os.path.join(SRC, "Textures2"), (".jpg", ".png"))
meshes = collect(os.path.join(SRC, "Models2"), (".gltf",))

n_tex = import_files(textures, DEST + "/Textures", "textures")
n_mesh = import_files(meshes, DEST + "/Meshes", "meshes")

# --- palette materials from the real textures ---
MEL = unreal.MaterialEditingLibrary
PALETTE = {
    "M_CC0_Canvas": "floral_jacquard",
    "M_CC0_Timber": "wood_planks",
    "M_CC0_Sandstone": "old_sandstone_02",
    "M_CC0_Cliff": "rock_06",
    "M_CC0_Pavement": "pavement_05",
}

made = 0
for mat_name, tex_stem in PALETTE.items():
    tex_path = "{}/Textures/{}.{}".format(DEST, tex_stem, tex_stem)
    tex = EAL.load_asset(tex_path)
    if not tex:
        unreal.log_warning("VEIL MATERIAL: texture missing for {} at {}".format(mat_name, tex_path))
        continue
    full = DEST + "/Materials/" + mat_name
    if EAL.does_asset_exist(full):
        mat = EAL.load_asset(full)
    else:
        mat = TOOLS.create_asset(mat_name, DEST + "/Materials", unreal.Material, unreal.MaterialFactoryNew())
    if not mat:
        unreal.log_warning("VEIL MATERIAL: could not create {}".format(mat_name))
        continue
    try:
        sample = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -420, -120)
        sample.set_editor_property("texture", tex)
        MEL.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
        rough = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant, -420, 120)
        rough.set_editor_property("r", 0.72)
        MEL.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
        MEL.recompile_material(mat)
        EAL.save_asset(full)
        made += 1
        unreal.log("VEIL MATERIAL: {} from {}".format(mat_name, tex_stem))
    except Exception as exc:
        unreal.log_warning("VEIL MATERIAL: {} failed: {}".format(mat_name, exc))

unreal.log("VEIL IMPORT SUMMARY: textures={} meshes={} materials={}".format(n_tex, n_mesh, made))
