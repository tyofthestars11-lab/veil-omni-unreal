# VEIL palette bake: the owner's own art, as materials.
#
# WHERE THESE COLOURS COME FROM - not invented, taken:
#   * The design system in his HTML files (VEIL-OMNI-HUGE-KRAKEN.html) uses #E8C368 x34,
#     #8F7A44 x16, #F6ECD2 x10, #121A2C x8, #BFE6FF x7, #FFE6A6 x6, #0B1E3A x5,
#     #9FD8FF x4, #8A6A3A x4, #C99A6A x4, #2C5FA8 x3, #F0C96A x4.
#   * Docs/VEIL_Bible.md section 2 and 3: gold #C9A24A, navy #0E1424, electric blue
#     #3FA9FF for signal tech and crystals, cream/sandstone for the living world.
#   * A 14-colour median cut of the six paintings embedded in that HTML: navy 13.3%,
#     warm grey-brown 12.8%, navy 9.5%, green 9.3%, blue-grey 9.3%.
#
# HEX, CONVERTED PROPERLY. The first two bakes wrote sRGB numbers straight into an emissive
# pin, which is linear space, so every colour came out washed toward white. The conversion
# now happens here, once, from the hex the owner actually wrote.
#
# TWO BUGS ALREADY PAID FOR, both guarded below:
#   1. Running with no level open -> get_all_level_actors() empty -> "0 re-dressed".
#   2. connect_material_property(expr, "", ...) does not raise when it fails; it leaves the
#      emissive pin empty and the material compiles to solid black. The pin is read back.
import unreal

MAP_PATH = "/Game/Maps/VEILDistrict"
ASSET_DIR = "/Game/Materials/Palette"
unreal.EditorLoadingAndSavingUtils.load_map(MAP_PATH)

PALETTE = {
    #  name                hex        what it is in the painting
    "VEIL_Navy":         ("0B1E3A", "sky, HUD body, shadow"),
    "VEIL_NavyDeep":     ("121A2C", "Deep shadow, panel fill"),
    "VEIL_Crystal":      ("3FA9FF", "signal crystal glow (Bible)"),
    "VEIL_CrystalDeep":  ("2C5FA8", "crystal body, far range"),
    "VEIL_Gold":         ("E8C368", "gold trim, spire, wings"),
    "VEIL_GoldDeep":     ("C9A24A", "gold in shadow (Bible frame)"),
    "VEIL_Cream":        ("F6ECD2", "sandstone walls, parchment"),
    "VEIL_Brown":        ("8A6A3A", "earth, market deck, cliff"),
    "VEIL_Green":        ("1B3932", "meadow band, foliage"),
    "VEIL_Water":        ("9FD8FF", "waterfall, foam, pale ice"),
    "VEIL_Moon":         ("0A0C12", "black sun / moon disc"),
}


def srgb_to_linear(c):
    c = c / 255.0
    return c / 12.92 if c <= 0.04045 else ((c + 0.055) / 1.055) ** 2.4


def linear_rgb(hex_string):
    h = hex_string.lstrip("#")
    return tuple(srgb_to_linear(int(h[i:i+2], 16)) for i in (0, 2, 4))


def make_material(name, hex_string):
    full = ASSET_DIR + "/" + name
    rgb = linear_rgb(hex_string)
    # Rebuilt every run: an existing asset would keep whatever graph it had.
    if unreal.EditorAssetLibrary.does_asset_exist(full):
        unreal.EditorAssetLibrary.delete_asset(full)
    mat = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, ASSET_DIR, unreal.Material, unreal.MaterialFactoryNew())
    if not mat:
        unreal.log_error("PALETTE: could not create %s" % name)
        return None
    # THE SHADING MODEL WAS THE ILLUSION. All eleven of these were MSM_UNLIT with the hex
    # wired into Emissive. Unlit outputs emissive and nothing else: base colour, roughness,
    # normal and shadow maps are dead inputs, so no wall in the district could take the sun,
    # take the sky light, or show a corner. That is the flat-slab look the owner kept
    # reporting, and it is why a whole session of sun and skylight tuning moved the eye
    # frame by only a few points. Default Lit costs one lighting pass and buys form, contact
    # shadow and GI. Glow stays only where the thing IS light - crystal and gold.
    glow = ("Crystal" in name and "Deep" not in name) or name.endswith("_Gold")
    mat.set_editor_property("shading_model",
                            unreal.MaterialShadingModel.MSM_UNLIT if glow
                            else unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("two_sided", True)   # flat art reads from both faces

    expr = unreal.MaterialEditingLibrary.create_material_expression(
        mat, unreal.MaterialExpressionVectorParameter, -400, 0)
    expr.set_editor_property("parameter_name", "BaseColour")
    expr.set_editor_property("default_value",
                             unreal.LinearColor(rgb[0], rgb[1], rgb[2], 1.0))

    # THE PIN IS READ BACK, NOT ASSUMED - on whichever pin this model actually reads.
    pin = (unreal.MaterialProperty.MP_EMISSIVE_COLOR if glow
           else unreal.MaterialProperty.MP_BASE_COLOR)
    got = None
    for out in ("RGB", "RGBA", "", "R"):
        try:
            unreal.MaterialEditingLibrary.connect_material_property(expr, out, pin)
        except Exception:
            continue
        got = unreal.MaterialEditingLibrary.get_material_property_input_node(mat, pin)
        if got:
            break
    if not got:
        unreal.log_error("PALETTE: %s pin stayed empty on %s" % (pin, name))
        return None

    if not glow:
        # Stone is not a mirror. 0.72 roughness keeps the key light readable on a wall
        # instead of turning every surface into a specular sheet.
        rough = unreal.MaterialEditingLibrary.create_material_expression(
            mat, unreal.MaterialExpressionScalarParameter, -400, 220)
        rough.set_editor_property("parameter_name", "Roughness")
        rough.set_editor_property("default_value", 0.72)
        try:
            unreal.MaterialEditingLibrary.connect_material_property(
                rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
        except Exception:
            unreal.log_error("PALETTE: roughness not wired on %s" % name)

    unreal.MaterialEditingLibrary.recompile_material(mat)
    unreal.EditorAssetLibrary.save_asset(full, False)
    return mat


mats, bad = {}, []
for n, (hx, _why) in PALETTE.items():
    m = make_material(n, hx)
    if m:
        mats[n] = m
    else:
        bad.append(n)
unreal.log("PALETTE BAKE: %d/%d material(s) from the owner's own hex%s"
           % (len(mats), len(PALETTE), ("" if not bad else "; FAILED: " + ",".join(bad))))

# WHICH PART WEARS WHICH COLOUR - chosen by the mesh it already uses, so the re-dress
# follows the art that is actually in the level rather than a guess by actor name.
RULES = [
    ("crystal", "VEIL_Crystal"), ("monolith", "VEIL_CrystalDeep"),
    ("water",   "VEIL_Water"),   ("column",   "VEIL_Gold"),
    ("spire",   "VEIL_Gold"),    ("decor",    "VEIL_GoldDeep"),
    # THE GROUND IS GREEN, NOT MUD. Measured, not guessed: the last bake reported
    # Brown=5460 against Cream=413, so five and a half thousand of the district's
    # meshes are floor/cliff/rock and one brown covered all of it - which is why
    # the whole world read as a mustard wash. The owner's art has a GREEN ground
    # band with brown only in the foreground, so the ground takes the green and
    # brown stays on rock and cliff where it belongs.
    ("floor",   "VEIL_Green"),    ("wall",     "VEIL_Cream"),
    ("arch",    "VEIL_Cream"),
    # ROOFS ARE BLUE. In the owner's art every house reads as a pale body under a
    # deep blue triangular roof; this rule had them cream, which is why the whole
    # district came back as one pale wash with no second colour anywhere. Blue is
    # the colour the vision puts on the roofs and on the crystal range, so the two
    # agree instead of the roofs competing with the walls.
    ("roof",    "VEIL_CrystalDeep"),
    ("cliff",   "VEIL_Brown"),   ("rock",     "VEIL_Brown"),
    ("grass",   "VEIL_Green"),   ("mountain", "VEIL_CrystalDeep"),
]


def colour_for(mesh_name):
    low = (mesh_name or "").lower()
    for key, mat in RULES:
        if key in low:
            return mat
    return None


# WHOSE COLOUR IS IT. A mesh name says what a mesh IS, not what it is being USED as.
# The Golden Hub raises SM_Floor1_Big as its blue ROOF planes and SM_Wall5_Big as its
# cream walls, and it sets those materials itself, deliberately. This pass then matched
# "floor" in the mesh name and repainted every roof brown, and matched "wall" and
# repainted the gold pillars cream - the hub's own art overwritten by a substring.
# So an actor that composes its own palette is left alone: the bake only dresses things
# that have no authored intent of their own.
COMPOSERS = ("GoldenHub", "HubMarket", "ConceptWorld", "NestSanctum")


def is_composer(actor):
    cls = actor.get_class().get_name() if actor else ""
    label = actor.get_actor_label() if actor else ""
    return any(name in cls or name in label for name in COMPOSERS)


world = unreal.EditorLevelLibrary.get_editor_world()
dressed, skipped, used, kept = 0, 0, {}, 0
for actor in unreal.EditorLevelLibrary.get_all_level_actors():
    if not actor:
        continue
    if is_composer(actor):
        kept += 1
        continue
    for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
        mesh = comp.static_mesh
        if not mesh:
            continue
        target = colour_for(mesh.get_name())
        if not target or target not in mats:
            skipped += 1
            continue
        for i in range(max(1, comp.get_num_materials())):
            comp.set_material(i, mats[target])
        dressed += 1
        used[target] = used.get(target, 0) + 1

try:
    unreal.EditorLoadingAndSavingUtils.save_map(world, MAP_PATH)
    unreal.log("PALETTE SAVED: %s" % MAP_PATH)
except Exception as e:
    unreal.log_warning("save_map: %s" % e)

unreal.log("PALETTE APPLIED: %d component(s) re-dressed, %d left alone, %d composing actor(s) kept their own art. %s"
           % (dressed, skipped, kept, ", ".join("%s=%d" % kv for kv in sorted(used.items()))))
