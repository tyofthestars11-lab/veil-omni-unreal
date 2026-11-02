# WHAT IS ACTUALLY STANDING AT THE PLAZA.
#
# Every guess about the opening frame so far has been about the runner. This asks the
# saved level the other question: name each mesh within 30 m of the plaza, its base
# height and its size, so the clutter in the shot can be traced to a line of code
# instead of described.
import unreal

MAP = "/Game/Maps/VEILDistrict"
RADIUS = 3000.0
Z_MIN, Z_MAX = -400.0, 1500.0

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(MAP)

world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()

rows = []
for a in actors:
    if not isinstance(a, unreal.StaticMeshActor):
        continue
    try:
        comp = a.static_mesh_component
        mesh = comp.static_mesh
        if mesh is None:
            continue
        loc = a.get_actor_location()
        if abs(loc.x) > RADIUS or abs(loc.y) > RADIUS:
            continue
        if loc.z < Z_MIN or loc.z > Z_MAX:
            continue
        bounds = comp.get_local_bounds()
        rows.append((loc.z, a.get_actor_label(), mesh.get_name(),
                     loc.x, loc.y, bounds.box_extent.z * 2.0,
                     a.get_actor_scale3d().x))
    except Exception:
        continue

rows.sort()
unreal.log("VEIL SCENE: %d static meshes within %.0f cm of the plaza" % (len(rows), RADIUS))
for z, label, mesh, x, y, height, scale in rows:
    unreal.log("VEIL SCENE  z=%7.1f  %-28s  %-34s  xy=(%7.1f,%7.1f)  h=%7.1f  sx=%.3f"
               % (z, label, mesh, x, y, height, scale))

# What the eye is looking at: the biggest pieces by height, which are the ones that
# fill the frame when the camera is low.
tall = sorted(rows, key=lambda r: -r[5])[:12]
unreal.log("VEIL SCENE: the twelve tallest pieces near the plaza")
for z, label, mesh, x, y, height, scale in tall:
    unreal.log("VEIL TALL  h=%7.1f  z=%7.1f  %-28s  %-34s  sx=%.3f"
               % (height, z, label, mesh, scale))
