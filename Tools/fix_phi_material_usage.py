"""The palette materials were authored without usage flags, so the game swaps in the
Default Material - gold reads as flat cream. Turn the usages on and save each asset."""
import unreal

PATHS = [
    "/Game/PHI/Palette/M_PHI_Gold",
    "/Game/PHI/Palette/M_PHI_GoldMetallic",
    "/Game/PHI/Palette/M_PHI_DeepBlue",
    "/Game/PHI/Palette/M_PHI_WhiteEmissive",
]
PROPS = ["used_with_instanced_static_meshes", "used_with_static_meshes",
         "used_with_skeletal_meshes", "used_with_particles"]

for p in PATHS:
    mat = unreal.EditorAssetLibrary.load_asset(p)
    if not mat:
        unreal.log("[PHIFIX] missing " + p)
        continue
    for prop in PROPS:
        try:
            mat.set_editor_property(prop, True)
            unreal.log("[PHIFIX] %s %s = True" % (p.split("/")[-1], prop))
        except Exception as e:
            unreal.log("[PHIFIX] %s %s FAILED %s" % (p.split("/")[-1], prop, e))
    try:
        mat.post_edit_change()
    except Exception:
        pass
    ok = unreal.EditorAssetLibrary.save_asset(p)
    unreal.log("[PHIFIX] saved %s -> %s" % (p, ok))
unreal.log("[PHIFIX] DONE")
