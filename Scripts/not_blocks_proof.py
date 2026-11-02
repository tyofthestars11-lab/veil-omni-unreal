import unreal, os

def census(project_maps):
    out = []
    pal = ["/Game/PHI/Palette/M_PHI_Gold", "/Game/PHI/Palette/M_PHI_DeepBlue", "/Game/PHI/Palette/M_PHI_WhiteEmissive"]
    for p in pal:
        out.append("PALETTE %s exists=%s" % (p, unreal.EditorAssetLibrary.does_asset_exist(p)))
    for m in project_maps:
        unreal.EditorLevelLibrary.load_level(m)
        actors = unreal.EditorLevelLibrary.get_all_level_actors()
        basic = 0
        total = 0
        labelled = []
        for a in actors:
            total += 1
            try:
                lab = a.get_actor_label().lower()
                if ("cube" in lab or "block" in lab or "box" in lab) and "countess" not in lab:
                    labelled.append(a.get_actor_label())
            except Exception:
                pass
            if isinstance(a, unreal.StaticMeshActor):
                mesh = a.static_mesh_component.static_mesh
                if mesh and "BasicShapes" in mesh.get_path_name():
                    basic += 1
        out.append("MAP %s actors=%d basicshapes=%d" % (m, total, basic))
        out.append("MAP %s label_hits=%d %s" % (m, len(labelled), labelled[:12]))
    return out

proj = unreal.Paths.project_dir()
if "OMNI" in proj:
    lines = census(["/Game/Maps/OMNIWorld"])
else:
    lines = census(["/Game/Maps/VEILWorld", "/Game/Maps/VEILDistrict"])
path = os.path.join(proj, "Saved", "Logs", "not_blocks_proof.txt")
with open(path, "w") as f:
    f.write("\n".join(lines) + "\n")