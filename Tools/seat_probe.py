# VEIL - WHAT IS THE RUNNER STANDING IN? (bounds, not pivots)
#
# The first version of this asked for actors whose PIVOT was near the seat and
# found nothing, while the Play frame clearly showed a gold mass at the runner's
# shoulder. That is this project's oldest trap: kit meshes are authored with their
# pivot wherever the artist left it, so "distance to the actor" says nothing about
# where the geometry is. This measures the point against each actor's actual
# BOUNDS box instead, which is the only number that describes what is in the way.
import math
import unreal

SEAT = unreal.Vector(-512.05, -90.57, 598.0)
RADIUS = 1500.0


def dist_to_box(p, o, e):
    dx = max(abs(p.x - o.x) - e.x, 0.0)
    dy = max(abs(p.y - o.y) - e.y, 0.0)
    dz = max(abs(p.z - o.z) - e.z, 0.0)
    return math.sqrt(dx * dx + dy * dy + dz * dz)


def main():
    eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    hits = []
    for a in eas.get_all_level_actors():
        if not isinstance(a, unreal.StaticMeshActor):
            continue
        comp = a.get_editor_property("static_mesh_component")
        mesh = comp.get_editor_property("static_mesh") if comp else None
        if not mesh:
            continue
        try:
            o, e = a.get_actor_bounds(False)
        except Exception:
            continue
        d = dist_to_box(SEAT, o, e)
        if d <= RADIUS:
            hits.append((d, a.get_name(), mesh.get_name(), o, e))
    hits.sort(key=lambda t: t[0])
    for d, name, mesh, o, e in hits[:10]:
        unreal.log("[SEAT BOX] gap=%.0f cm  %s  mesh=%s  centre=(%.0f,%.0f,%.0f) ext=(%.0f,%.0f,%.0f)"
                   % (d, name, mesh, o.x, o.y, o.z, e.x, e.y, e.z))
    unreal.log("[SEAT BOX] actors whose bounds are within %.0f cm of the seat: %d" % (RADIUS, len(hits)))
    inside = [h for h in hits if h[0] <= 1.0]
    unreal.log("[SEAT BOX] boxes CONTAINING the seat: %d" % len(inside))
    for d, name, mesh, o, e in inside:
        unreal.log("[SEAT BOX] INSIDE %s mesh=%s ext=(%.0f,%.0f,%.0f)" % (name, mesh, e.x, e.y, e.z))


main()
