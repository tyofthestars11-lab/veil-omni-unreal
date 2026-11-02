# EVERY VEIL MAP, BAKED FROM THE SAME BUILDER.
#
# The district was generated in one map and the other three were left as whatever they
# happened to contain. This runs the same build into every project map, saves it, and
# reads each level back so "generated" is a count in a file rather than an intention.
#
# The builder is deterministic by construction - placement follows the golden angle and
# phi powers, and the file contains no random call - so two runs produce the same world.
# This script never deletes or moves a file: it removes generated level ACTORS, which is
# how a rebuild replaces the last one, and saves the map.
import unreal

MAPS = [
    "/Game/Maps/VEILDistrict",
    "/Game/Maps/VEILWorld",
    "/Game/Maps/OMNIWorld",
    "/Game/Maps/CountessRegion",
]
OUT = r"D:\VEIL-OMNI\Saved\Logs\all_maps_audit.txt"
PLACEHOLDER_PREFIX = "/Engine/BasicShapes/"


def log(msg):
    unreal.log("[VEIL ALLMAPS] " + str(msg))


def snapshot():
    """What the level really holds: meshes, materials, placeholders, light rig."""
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    meshes, mats = {}, {}
    statics = placeholders = rigs = 0
    for a in actors:
        cls = a.get_class().get_name()
        if cls in ("DirectionalLight", "SkyLight", "SkyAtmosphere",
                   "ExponentialHeightFog", "PostProcessVolume"):
            rigs += 1
        if not isinstance(a, unreal.StaticMeshActor):
            continue
        statics += 1
        comp = a.get_editor_property("static_mesh_component")
        mesh = comp.get_editor_property("static_mesh") if comp else None
        if not mesh:
            continue
        path = mesh.get_path_name()
        if path.startswith(PLACEHOLDER_PREFIX):
            placeholders += 1
            continue
        meshes[mesh.get_name()] = meshes.get(mesh.get_name(), 0) + 1
        try:
            mi = comp.get_editor_property("override_materials")
            mat = mi[0] if mi else None
            if mat:
                mats[mat.get_name()] = mats.get(mat.get_name(), 0) + 1
        except Exception:
            pass
    return {"actors": len(actors), "statics": statics, "meshes": meshes,
            "materials": mats, "placeholders": placeholders, "rig": rigs}


def bake_map(les, map_path):
    if not les.load_level(map_path):
        log("could not load " + map_path)
        return None
    builder = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).spawn_actor_from_class(
        unreal.VEILWorldBuilder, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
    if not builder:
        log("could not spawn the builder in " + map_path)
        return None
    try:
        builder.bake_to_level()
    except Exception as e:
        log("bake_to_level failed in {}: {}".format(map_path, e))
    # The builder is the rig that made the level, not part of it.
    unreal.get_editor_subsystem(unreal.EditorActorSubsystem).destroy_actor(builder)
    saved = bool(les.save_current_level())
    log("save {} -> {}".format(map_path, saved))
    return saved


def main():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    report = []
    for map_path in MAPS:
        saved = bake_map(les, map_path)
        snap = snapshot()
        top = sorted(snap["meshes"].items(), key=lambda kv: -kv[1])[:5]
        line = ("{:<28} saved={:<5} actors={:<5} static={:<5} distinctMeshes={:<4} "
                "materials={:<3} placeholders={:<3} rig={}").format(
            map_path.split("/")[-1], str(saved), snap["actors"], snap["statics"],
            len(snap["meshes"]), len(snap["materials"]), snap["placeholders"], snap["rig"])
        log(line)
        log("    top meshes: " + ", ".join("{} {}".format(n, c) for n, c in top))
        report.append(line)
        report.append("    top meshes: " + ", ".join("{} {}".format(n, c) for n, c in top))
        report.append("    materials: " + ", ".join(sorted(snap["materials"])))
        report.append("")

    with open(OUT, "w", encoding="utf-8") as f:
        f.write("VEIL ALL-MAP BAKE\n")
        f.write("\n".join(report) + "\n")
    log("wrote " + OUT)


main()
