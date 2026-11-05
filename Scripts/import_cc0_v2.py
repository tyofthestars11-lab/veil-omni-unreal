"""Second import pass: every curated mesh, and palette materials that really exist.

Writes its own log to Assets/CC0/import.log so the result can be checked from disk
instead of trusted from stdout.
"""
import os
import traceback

import unreal

SRC = "C:/Users/Admin/HarkGame/Assets/CC0"
DEST = "/Game/VEIL/CC0"
LOG_PATH = os.path.join(SRC, "import.log")
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
EAL = unreal.EditorAssetLibrary

lines = []


def say(msg):
    lines.append(str(msg))
    unreal.log("VEIL IMPORT2: " + str(msg))


def collect(folder, exts):
    found = []
    if not os.path.isdir(folder):
        return found
    for root, _d, files in os.walk(folder):
        for f in files:
            if f.lower().endswith(exts):
                found.append(os.path.join(root, f))
    return sorted(found)


def import_one(path, dest, tag):
    try:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", path)
        task.set_editor_property("destination_path", dest)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)
        TOOLS.import_asset_tasks([task])
        say("ok {} {}".format(tag, os.path.basename(path)))
        return True
    except Exception as exc:
        say("FAIL {} {} :: {}".format(tag, os.path.basename(path), exc))
        return False


textures = collect(os.path.join(SRC, "Textures2"), (".jpg", ".png"))
meshes = collect(os.path.join(SRC, "Models2"), (".gltf",))
say("found {} textures, {} meshes".format(len(textures), len(meshes)))

ok_tex = sum(1 for p in textures if import_one(p, DEST + "/Textures", "tex"))
ok_mesh = sum(1 for p in meshes if import_one(p, DEST + "/Meshes", "mesh"))
say("imported textures {}/{}, meshes {}/{}".format(ok_tex, len(textures), ok_mesh, len(meshes)))

# --- palette materials, texture located by name search so a stem mismatch cannot kill it ---
PALETTE = {
    "M_CC0_Canvas": "canvas|fabric|jacquard|caban",
    "M_CC0_Timber": "plank",
    "M_CC0_Sandstone": "sandstone|pavement",
    "M_CC0_Cliff": "rock",
    "M_CC0_River": "river",
}

texture_assets = [p for p in EAL.list_assets(DEST + "/Textures", recursive=True) if "Texture2D" in p or True]
say("texture assets visible: {}".format(len(texture_assets)))

made = 0
for mat_name, pattern in PALETTE.items():
    words = pattern.split("|")
    match = None
    for path in texture_assets:
        leaf = path.split("/")[-1].split(".")[0].lower()
        if any(w in leaf for w in words):
            match = path
            break
    if not match:
        say("no texture for {} (pattern {})".format(mat_name, pattern))
        continue
    try:
        tex = EAL.load_asset(match)
        full = DEST + "/Materials/" + mat_name
        if EAL.does_asset_exist(full):
            mat = EAL.load_asset(full)
        else:
            mat = TOOLS.create_asset(mat_name, DEST + "/Materials", unreal.Material, unreal.MaterialFactoryNew())
        sample = unreal.MaterialEditingLibrary.create_material_expression(
            mat, unreal.MaterialExpressionTextureSample, -420, -120)
        sample.set_editor_property("texture", tex)
        unreal.MaterialEditingLibrary.connect_material_property(
            sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
        unreal.MaterialEditingLibrary.recompile_material(mat)
        EAL.save_asset(full)
        say("material {} from {}".format(mat_name, match))
        made += 1
    except Exception as exc:
        say("material {} FAILED :: {}".format(mat_name, exc))
        say(traceback.format_exc().splitlines()[-1])

say("SUMMARY textures={}/{} meshes={}/{} materials={}/{}".format(
    ok_tex, len(textures), ok_mesh, len(meshes), made, len(PALETTE)))

os.makedirs(os.path.dirname(LOG_PATH), exist_ok=True)
with open(LOG_PATH, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))
