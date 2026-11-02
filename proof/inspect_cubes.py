# VEIL - NAME THE PLACEHOLDERS IN THE SAVED LEVEL.
#
# The bake audit said 374 actors in VEILDistrict still use /Engine/BasicShapes/Cube,
# and the count did not change when the builder's own cube substitution landed - so
# they are NOT coming from the build path that was just fixed. This reads the saved
# level and writes down what each one is: label, actor name, location, size, tags.
# That is how they get a name and a provenance instead of a guess.
import unreal

MAP = "/Game/Maps/VEILDistrict"
OUT = r"D:\VEIL-OMNI\Saved\Logs\cube_dump.txt"


def log(msg):
    unreal.log("[VEIL CUBES] " + str(msg))


def main():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not les.load_level(MAP):
        log("could not load " + MAP)
        return

    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    rows = []
    for a in actors:
        if not isinstance(a, unreal.StaticMeshActor):
            continue
        comp = a.get_editor_property("static_mesh_component")
        mesh = comp.get_editor_property("static_mesh") if comp else None
        name = mesh.get_name() if mesh else "None"
        path = mesh.get_path_name() if mesh else ""
        # By PATH, not by name: an engine placeholder is a placeholder because of
        # where it lives, and a kit asset could legitimately be called Cube*.
        if not path.startswith("/Engine/BasicShapes/"):
            continue
        origin, extent = a.get_actor_bounds(False)
        tags = [str(t) for t in a.tags]
        rows.append(
            "label={!r:34} actor={!r:26} at=({:>8.0f},{:>8.0f},{:>8.0f}) size=({:>7.0f},{:>7.0f},{:>7.0f}) tags={}".format(
                str(a.get_actor_label())[:33],
                str(a.get_name())[:25],
                origin.x, origin.y, origin.z,
                extent.x * 2, extent.y * 2, extent.z * 2,
                tags,
            )
        )

    labels = {}
    for r in rows:
        key = r.split("label=")[1].split(" actor=")[0]
        labels[key] = labels.get(key, 0) + 1
    top = sorted(labels.items(), key=lambda kv: -kv[1])[:10]

    lines = ["CUBE ACTORS IN {}: {}".format(MAP, len(rows)),
             "top labels:"]
    for label, count in top:
        lines.append("  {:>4} x {}".format(count, label))
    lines.append("all rows:")
    lines.extend(rows)

    for line in lines[:14]:
        log(line)
    try:
        with open(OUT, "w", encoding="utf-8") as f:
            f.write("\n".join(lines) + "\n")
        log("wrote " + OUT)
    except Exception as exc:  # noqa: BLE001
        log("could not write: {}".format(exc))


main()
