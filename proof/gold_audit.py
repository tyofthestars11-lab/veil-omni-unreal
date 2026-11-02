# VEIL - GOLD, MEASURED.
#
# Two jobs, because both were being guessed at:
#
# 1. CREATE A REAL METALLIC GOLD. The vision says "phi gold metallic with emissive edge,
#    gold is a real metallic material, not flat yellow". Every gold surface in this game
#    wears /Game/PHI/Palette/M_PHI_Gold, and three rounds of tint changes proved we did not
#    know what that material actually is. This builds M_PHI_GoldMetallic properly: metallic
#    1.0, low roughness, gold base colour, a little emissive so edges read on a bright sky.
#
# 2. DUMP WHAT IS ACTUALLY STANDING IN THE PLAZA. The big gold masses have survived a
#    material swap in Place() and a material swap in MakeSet(), which means they are neither.
#    Until something reads their mesh and material names, any further change is guessing.
#    This reads them and writes the list to Saved/Logs/gold_dump.txt.
#
# Read-only on the level: it creates one material asset and reports. It does not save the map.
import unreal

MAT_DIR = "/Game/PHI/Palette"
MAT_NAME = "M_PHI_GoldMetallic"
OUT = r"D:\VEIL-OMNI\Saved\Logs\gold_dump.txt"
MAP = "/Game/Maps/VEILDistrict"
PLAZA_RADIUS = 3000.0


def log(msg):
    unreal.log("[VEIL GOLD] " + str(msg))


def make_metallic_gold():
    """Build the gold as a real metal: metallic 1, roughness ~0.28, warm base, soft emissive."""
    path = "{}/{}".format(MAT_DIR, MAT_NAME)
    if unreal.EditorAssetLibrary.does_asset_exist(path):
        log("material already exists: {}".format(path))
        return unreal.EditorAssetLibrary.load_asset(path)

    tools = unreal.AssetToolsHelpers.get_asset_tools()
    mat = tools.create_asset(MAT_NAME, MAT_DIR, unreal.Material, unreal.MaterialFactoryNew())
    if not mat:
        log("could not create the material asset")
        return None

    mel = unreal.MaterialEditingLibrary
    gold = unreal.LinearColor(1.0, 0.766, 0.336, 1.0)

    base = mel.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -520, -120)
    base.set_editor_property("constant", gold)
    mel.connect_material_property(base, "", unreal.MaterialProperty.MP_BASE_COLOR)

    metallic = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -520, 40)
    metallic.set_editor_property("r", 1.0)
    mel.connect_material_property(metallic, "", unreal.MaterialProperty.MP_METALLIC)

    rough = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -520, 160)
    rough.set_editor_property("r", 0.28)
    mel.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)

    # The emissive edge: gold at low intensity so the phi symbols and trim read at distance
    # without turning the masonry into a lamp.
    glow_colour = unreal.LinearColor(1.0, 0.766, 0.336, 1.0)
    glow = mel.create_material_expression(mat, unreal.MaterialExpressionConstant3Vector, -520, 280)
    glow.set_editor_property("constant", glow_colour)
    mul = mel.create_material_expression(mat, unreal.MaterialExpressionMultiply, -280, 280)
    glow_amount = mel.create_material_expression(mat, unreal.MaterialExpressionConstant, -520, 400)
    glow_amount.set_editor_property("r", 0.35)
    mel.connect_material_expressions(glow, "", mul, "A")
    mel.connect_material_expressions(glow_amount, "", mul, "B")
    mel.connect_material_property(mul, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)

    mel.layout_material_expressions(mat)
    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat)
    log("created {} - metallic 1.0, roughness 0.28, emissive 0.35".format(path))
    return mat


def dump_plaza():
    """Every static mesh component and instanced set standing in the plaza, with materials."""
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    rows = []
    material_counts = {}
    for a in actors:
        try:
            loc = a.get_actor_location()
        except Exception:  # noqa: BLE001
            continue
        if abs(loc.x) > PLAZA_RADIUS or abs(loc.y) > PLAZA_RADIUS:
            continue
        try:
            comps = a.get_components_by_class(unreal.StaticMeshComponent)
        except Exception:  # noqa: BLE001
            comps = []
        for comp in comps:
            try:
                mesh = comp.get_editor_property("static_mesh")
                mat = comp.get_material(0)
            except Exception:  # noqa: BLE001
                continue
            mesh_name = mesh.get_name() if mesh else "None"
            mat_name = mat.get_name() if mat else "None"
            material_counts[mat_name] = material_counts.get(mat_name, 0) + 1
            if "gold" in mat_name.lower() or "phi" in mat_name.lower():
                rows.append("{:<22} mesh={:<34} mat={:<26} at {} {}".format(
                    a.get_name()[:22], mesh_name[:34], mat_name[:26],
                    int(loc.x), int(loc.y)))
    lines = ["PLAZA DUMP: {} gold/phi-material components inside {} cm of origin".format(
        len(rows), int(PLAZA_RADIUS))]
    lines += rows[:60]
    lines.append("")
    lines.append("ALL MATERIALS IN THE PLAZA, by component count:")
    for name, count in sorted(material_counts.items(), key=lambda kv: -kv[1])[:20]:
        lines.append("    {:>4} x {}".format(count, name))

    try:
        with open(OUT, "w", encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")
        log("wrote " + OUT)
    except Exception as exc:  # noqa: BLE001
        log("could not write dump: {}".format(exc))
    for line in lines[:30]:
        log(line)
    return lines


def main():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not les.load_level(MAP):
        log("could not load " + MAP)
        return
    make_metallic_gold()
    dump_plaza()


main()
