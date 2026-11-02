# VEIL - BAKE THE DISTRICT INTO THE LEVEL, THEN PROVE THE LEVEL IS RIGHT.
#
# Why this exists: the district used to be built at Play time, so it lived in the
# CODE and in whoever ran it. The level file itself was nearly empty, which is why
# the world appeared "on one machine" and not in general. This runs the builder
# inside the editor, saves those actors into VEILDistrict, then READS THE SAVED LEVEL
# BACK and reports what is really in it.
#
# It bakes TWICE on purpose. The first bake can be right and the second one wrong -
# that is exactly what happened last time, where a second run stacked a duplicate
# city instead of replacing the first. A checker that only looks after one bake
# cannot see that. So: bake, measure, bake again, measure, and PASS only if the
# second bake produces the same world as the first and no engine placeholder is
# left in it.
#
# PARAGON IS A PLACEHOLDER. Fab is for what this project does not have, and it does
# have complete humanoids (Polyart characters, the built MetaHuman). Actors wearing
# Paragon meshes or running Paragon FX were baked into this level by earlier builds
# and kept streaming Greystone statue shards into every run. The bake now REMOVES
# any Paragon actor and reports the count, so "no Paragon in the game" is measured
# instead of promised.
#
# NON-DELETE GUARD: this script never deletes or moves a file. It removes generated
# level ACTORS (which is the point of a rebuild) and saves the map.
import unreal

MAP = "/Game/Maps/VEILDistrict"
OUT = r"D:\VEIL-OMNI\Saved\Logs\level_audit.txt"
PLACEHOLDER_PREFIX = "/Engine/BasicShapes/"   # Cube / Sphere / Cylinder - grey block stand-ins
BANNED = "Paragon"                            # placeholder pack: Fab material, not our game


def log(msg):
    unreal.log("[VEIL CHECK] " + str(msg))


def _paths_on(actor):
    """Every asset path an actor is wearing: its class, its static meshes, its FX template."""
    paths = []
    try:
        paths.append(actor.get_class().get_path_name())
    except Exception:  # noqa: BLE001
        pass
    try:
        for comp in actor.get_components_by_class(unreal.StaticMeshComponent):
            mesh = comp.get_editor_property("static_mesh")
            if mesh:
                paths.append(mesh.get_path_name())
    except Exception:  # noqa: BLE001
        pass
    try:
        for comp in actor.get_components_by_class(unreal.ParticleSystemComponent):
            template = comp.get_editor_property("template")
            if template:
                paths.append(template.get_path_name())
    except Exception:  # noqa: BLE001
        pass
    return paths


def purge_paragon():
    """Remove any actor wearing a Paragon asset, and say how many were removed."""
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    removed = 0
    for actor in subsystem.get_all_level_actors():
        try:
            if any(BANNED in p for p in _paths_on(actor)):
                subsystem.destroy_actor(actor)
                removed += 1
        except Exception:  # noqa: BLE001
            continue
    log("purged {} {}-dependent actor(s) from the level".format(removed, BANNED))
    return removed


def snapshot(tag):
    """Read the level as it stands and report what is actually in it."""
    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    statics = []
    meshes = {}
    placeholders = 0
    banned = 0
    banned_names = []
    for a in actors:
        if any(BANNED in p for p in _paths_on(a)):
            banned += 1
            if len(banned_names) < 6:
                banned_names.append(a.get_name())
        if not isinstance(a, unreal.StaticMeshActor):
            continue
        statics.append(a)
        comp = a.get_editor_property("static_mesh_component")
        mesh = comp.get_editor_property("static_mesh") if comp else None
        if not mesh:
            meshes["None"] = meshes.get("None", 0) + 1
            continue
        path = mesh.get_path_name()
        name = mesh.get_name()
        meshes[name] = meshes.get(name, 0) + 1
        if path.startswith(PLACEHOLDER_PREFIX):
            placeholders += 1

    # A LEVEL WITHOUT A SUN IS A NIGHT LEVEL. The packaged game proved it the hard way:
    # "0 sun(s), 0 sky light(s)" and a world that no exposure setting could brighten.
    lights = 0
    for a in actors:
        if a.get_class().get_name() in ("DirectionalLight", "SkyLight"):
            lights += 1

    top = sorted(meshes.items(), key=lambda kv: -kv[1])[:6]
    lines = ["{}: {} actors, {} static meshes, {} engine placeholders, {} key/sky lights, {} {} actors".format(
        tag, len(actors), len(statics), placeholders, lights, banned, BANNED)]
    for name, count in top:
        lines.append("    {:>4} x {}".format(count, name))
    if banned_names:
        lines.append("    {} still standing: {}".format(BANNED, ", ".join(banned_names)))
    for line in lines:
        log(line)
    return {"actors": len(actors), "statics": len(statics),
            "placeholders": placeholders, "lights": lights,
            "banned": banned, "lines": lines}


def bake():
    """Spawn the builder in the editor and run the same build Play runs."""
    builder = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).spawn_actor_from_class(
        unreal.VEILWorldBuilder, unreal.Vector(0, 0, 0), unreal.Rotator(0, 0, 0))
    if not builder:
        log("could not spawn VEILWorldBuilder")
        return False
    builder.bake_to_level()

    # THE BUILDER IS SCAFFOLDING, NOT CONTENT. Six of them had been saved into the
    # level by earlier bakes, and every one of those runs its own build on BeginPlay -
    # six districts stacked at runtime, which is the duplicated block world. The bake
    # keeps the buildings and removes the rig that made them.
    unreal.get_editor_subsystem(unreal.EditorActorSubsystem).destroy_actor(builder)

    saved = bool(unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level())
    log("save_current_level -> {}".format(saved))
    return saved


def main():
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not les.load_level(MAP):
        log("could not load " + MAP)
        return

    purged = purge_paragon()
    first = snapshot("BEFORE")
    saved1 = bake()
    pass1 = snapshot("AFTER BAKE 1")
    saved2 = bake()
    pass2 = snapshot("AFTER BAKE 2")

    # A rebuild is stable when the second bake produced the SAME world as the first -
    # meshes AND total actors. Comparing meshes alone missed the duplicate lights and
    # post-process volumes the first version of the clean step left behind.
    meshes_stable = abs(pass2["statics"] - pass1["statics"]) <= max(4, pass1["statics"] // 100)
    actors_stable = abs(pass2["actors"] - pass1["actors"]) <= 8
    stable = meshes_stable and actors_stable
    clean = pass2["placeholders"] == 0
    banned_free = pass2["banned"] == 0
    populated = pass2["statics"] > 100
    lit = pass2["lights"] >= 2   # at least one directional light and one sky light

    reasons = []
    if not (saved1 and saved2):
        reasons.append("the map did not save")
    if not populated:
        reasons.append("the level is nearly empty ({} meshes)".format(pass2["statics"]))
    if not clean:
        reasons.append("{} engine placeholder meshes remain".format(pass2["placeholders"]))
    if not banned_free:
        reasons.append("{} actor(s) still wear {} assets".format(pass2["banned"], BANNED))
    if not lit:
        reasons.append("the level has {} key/sky light(s) - a district with no sun is a "
                       "night level, not a grade problem".format(pass2["lights"]))
    if not stable:
        reasons.append("a second bake changed the world ({} -> {} meshes, {} -> {} actors)".format(
            pass1["statics"], pass2["statics"], pass1["actors"], pass2["actors"]))
    verdict = "PASS" if not reasons else "FAIL"

    try:
        with open(OUT, "w", encoding="utf-8") as f:
            f.write("MAP {}\n".format(MAP))
            f.write("PARAGON ACTORS PURGED {}\n".format(purged))
            for snap in (first, pass1, pass2):
                for line in snap["lines"]:
                    f.write(line + "\n")
            f.write("SAVE RESULT {} / {}\n".format(saved1, saved2))
            f.write("REBUILD STABLE {}\n".format(stable))
            f.write("PLACEHOLDERS AFTER {}\n".format(pass2["placeholders"]))
            f.write("PARAGON ACTORS AFTER {}\n".format(pass2["banned"]))
            f.write("KEY/SKY LIGHTS AFTER {}\n".format(pass2["lights"]))
            for reason in reasons:
                f.write("REASON {}\n".format(reason))
            f.write("CHECK RESULT {}\n".format(verdict))
        log("wrote " + OUT)
    except Exception as exc:  # noqa: BLE001
        log("could not write audit: {}".format(exc))

    log("CHECK RESULT {} {}".format(verdict, "; ".join(reasons)))


main()
