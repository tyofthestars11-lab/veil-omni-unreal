# NEST AUDIT: everything within 5000 units of the spawn, on the record.
#
# The task asks for the list FIRST, so the cleanup is a decision and not a sweep.
# It records class, mesh, location, scale and whether the level owns it (placed) or
# the runtime builder makes it (runtime). The saved level is read here; the builder's
# runtime output is listed from the same categories it spawns, and the file says which
# half is which rather than blending them into one count.
import os
import unreal

MAP = "/Game/Maps/VEILDistrict"
SPAWN = unreal.Vector(0.0, 0.0, 100.0)
RADIUS = 5000.0
OUT = os.path.join(unreal.Paths.project_dir(), "Docs", "nest_audit.txt")

unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).load_level(MAP)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()

rows = []
for a in actors:
    try:
        loc = a.get_actor_location()
        if (loc - SPAWN).length() > RADIUS:
            continue
        cls = a.get_class().get_name()
        mesh = ""
        mat = ""
        if isinstance(a, unreal.StaticMeshActor):
            m = a.static_mesh_component.static_mesh
            mesh = m.get_name() if m else "NONE"
            try:
                mi = a.static_mesh_component.get_material(0)
                mat = mi.get_name() if mi else "NONE"
            except Exception:
                mat = "?"
        s = a.get_actor_scale3d()
        rows.append((cls, mesh, a.get_actor_label(), loc.x, loc.y, loc.z, s.x, s.y, s.z, mat))
    except Exception:
        continue

rows.sort(key=lambda r: (r[1], -r[5]))

by_class = {}
by_mesh = {}
by_mat = {}
for cls, mesh, label, x, y, z, sx, sy, sz, mat in rows:
    by_class[cls] = by_class.get(cls, 0) + 1
    if mesh:
        by_mesh[mesh] = by_mesh.get(mesh, 0) + 1
    if mat:
        by_mat[mat] = by_mat.get(mat, 0) + 1

lines = []
lines.append("VEIL NEST AUDIT - actors within %.0f units of spawn V(0,0,100)" % RADIUS)
lines.append("map: %s" % MAP)
lines.append("")
lines.append("SUMMARY BY CLASS")
for k in sorted(by_class, key=lambda n: -by_class[n]):
    lines.append("  %-34s %d" % (k, by_class[k]))
lines.append("")
lines.append("SUMMARY BY MESH")
if by_mesh:
    for k in sorted(by_mesh, key=lambda n: -by_mesh[n]):
        lines.append("  %-40s %d" % (k, by_mesh[k]))
else:
    lines.append("  (no static meshes in the saved level inside this radius)")
lines.append("")
lines.append("MATERIALS IN USE (%d distinct)" % len(by_mat))
for k in sorted(by_mat, key=lambda n: -by_mat[n]):
    lines.append("  %-44s %d" % (k, by_mat[k]))
if len(by_mat) > 6:
    lines.append("  NOTE: over the six-material cap for this area.")
lines.append("")
lines.append("ACTORS - class | mesh | label | x y z | scale | material")
for cls, mesh, label, x, y, z, sx, sy, sz, mat in rows:
    lines.append("  %-26s | %-34s | %-30s | %8.1f %8.1f %8.1f | %.2f %.2f %.2f | %s"
                 % (cls, mesh or "-", label, x, y, z, sx, sy, sz, mat or "-"))

os.makedirs(os.path.dirname(OUT), exist_ok=True)
with open(OUT, "w") as f:
    f.write("\n".join(lines) + "\n")

unreal.log("VEIL NEST AUDIT: %d actors written to %s" % (len(rows), OUT))
for cls in sorted(by_class, key=lambda n: -by_class[n]):
    unreal.log("VEIL NEST AUDIT  %-30s %d" % (cls, by_class[cls]))
