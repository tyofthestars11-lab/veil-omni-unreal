"""Author real materials from the imported CC0 textures, by exact asset name.

Writes Assets/CC0/mat.log so success can be checked from disk.
"""
import os

import unreal

DEST = "/Game/VEIL/CC0"
TEX = DEST + "/Textures/"
LOG = "C:/Users/Admin/HarkGame/Assets/CC0/mat.log"
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()
EAL = unreal.EditorAssetLibrary
MEL = unreal.MaterialEditingLibrary

PALETTE = {
    "M_CC0_Canvas": "caban_diff_1k",
    "M_CC0_Cloth": "floral_jacquard_diff_1k",
    "M_CC0_Timber": "wood_planks_diff_1k",
    "M_CC0_RoofPlanks": "roof_planks_diff_1k",
    "M_CC0_Sandstone": "old_sandstone_02_diff_1k",
    "M_CC0_Pavement": "pavement_05_diff_1k",
    "M_CC0_Cliff": "rock_06_diff_1k",
    "M_CC0_Rock": "lichen_rock_diff_1k",
    "M_CC0_Gravel": "river_small_rocks_diff_1k",
    "M_CC0_DryRiverbed": "dry_river_pebbles_diff_1k",
}

lines = []


def say(m):
    lines.append(str(m))
    unreal.log("VEIL MAT: " + str(m))


made = 0
for mat_name, tex_name in PALETTE.items():
    tex_path = "{}{}.{}".format(TEX, tex_name, tex_name)
    tex = EAL.load_asset(tex_path)
    if not tex:
        say("missing texture {}".format(tex_path))
        continue
    full = DEST + "/Materials/" + mat_name
    try:
        if EAL.does_asset_exist(full):
            mat = EAL.load_asset(full)
        else:
            mat = TOOLS.create_asset(mat_name, DEST + "/Materials", unreal.Material, unreal.MaterialFactoryNew())
        if not mat:
            say("create_asset returned None for " + mat_name)
            continue
        sample = MEL.create_material_expression(mat, unreal.MaterialExpressionTextureSample, -420, -120)
        sample.set_editor_property("texture", tex)
        MEL.connect_material_property(sample, "RGB", unreal.MaterialProperty.MP_BASE_COLOR)
        rough = MEL.create_material_expression(mat, unreal.MaterialExpressionConstant, -420, 120)
        rough.set_editor_property("r", 0.72)
        MEL.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
        MEL.recompile_material(mat)
        EAL.save_asset(full)
        say("built {} from {}".format(mat_name, tex_name))
        made += 1
    except Exception as exc:
        say("FAILED {} :: {}".format(mat_name, exc))

say("SUMMARY materials={}/{}".format(made, len(PALETTE)))
os.makedirs(os.path.dirname(LOG), exist_ok=True)
with open(LOG, "w", encoding="utf-8") as f:
    f.write("\n".join(lines))
