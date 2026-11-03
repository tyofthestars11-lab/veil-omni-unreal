import math
import unreal

MAP = "/Game/Maps/VEILDistrict"
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
les.load_level(MAP)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
actors = eas.get_all_level_actors()
starts = [a for a in actors if isinstance(a, unreal.PlayerStart)]
unreal.log("[AUDIT] actors %d starts %d" % (len(actors), len(starts)))
for s in starts:
    loc = s.get_actor_location()
    unreal.log("[AUDIT] START %s at (%.0f, %.0f, %.0f) rot %s"
               % (s.get_actor_label(), loc.x, loc.y, loc.z, s.get_actor_rotation()))

at = starts[0].get_actor_location() if starts else unreal.Vector(0, 0, 0)
rows = []
for a in actors:
    if not isinstance(a, unreal.StaticMeshActor):
        continue
    loc = a.get_actor_location()
    d = math.sqrt((loc.x - at.x) ** 2 + (loc.y - at.y) ** 2 + (loc.z - at.z) ** 2)
    if d > 2500:
        continue
    comp = a.static_mesh_component
    mesh = comp.get_editor_property("static_mesh") if comp else None
    mats = comp.get_editor_property("override_materials") if comp else []
    mat = mats[0].get_name() if mats and mats[0] else "-"
    origin, extent = a.get_actor_bounds(False)
    vol = extent.x * extent.y * extent.z
    rows.append((vol, d, mesh.get_name() if mesh else "?", mat, loc, extent, a.get_actor_label()))
rows.sort(reverse=True)
for r in rows[:14]:
    unreal.log("[AUDIT] vol %.3e d=%.0f mesh=%s mat=%s at(%.0f,%.0f,%.0f) ext(%.0f,%.0f,%.0f) %s"
               % (r[0], r[1], r[2], r[3], r[4].x, r[4].y, r[4].z, r[5].x, r[5].y, r[5].z, r[6]))
unreal.log("[AUDIT] DONE near=%d" % len(rows))
