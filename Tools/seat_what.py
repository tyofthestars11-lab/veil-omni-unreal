# VEIL - WHAT IS SOLID AT THE SEAT? (component bounds + a trace that fits the API)
#
# Three probes have failed to name the gold mass the runner wakes inside. All of
# them asked "which ACTOR is near this point", and every one answered "nothing",
# while Play shows a gold wall. So this asks the two questions that actually
# describe a solid:
#
#   1. COMPONENT bounds - each primitive component's own bounds box measured
#      against the seat. Actor bounds are a union of pivots; component bounds are
#      the geometry, and an instanced component carries its instances inside one
#      box, so instanced masses finally show up.
#   2. A downward trace through the seat, which names the exact actor and
#      component holding the surface up.
#
# UE 5.8's Python signature for LineTraceSingle takes the HitResult as a return
# value, not an argument, so the call is attempted in both shapes and the one that
# worked is reported rather than assumed.
import math
import unreal

SEAT = unreal.Vector(-512.05, -90.57, 598.0)
CENTRE = unreal.Vector(0.0, 0.0, 300.0)
RADIUS = 2500.0


def box_gap(p, o, e):
    dx = max(abs(p.x - o.x) - e.x, 0.0)
    dy = max(abs(p.y - o.y) - e.y, 0.0)
    dz = max(abs(p.z - o.z) - e.z, 0.0)
    return math.sqrt(dx * dx + dy * dy + dz * dz)


def component_bounds(a):
    """Every primitive component on the actor, as (name, origin, extent)."""
    out = []
    try:
        comps = a.get_components_by_class(unreal.PrimitiveComponent)
    except Exception:
        return out
    for c in comps:
        try:
            b = c.get_editor_property("bounds")
        except Exception:
            continue
        out.append((c.get_name(), b.origin, b.box_extent, c))
    return out


def trace_down(world, at, label):
    start = unreal.Vector(at.x, at.y, 3000.0)
    end = unreal.Vector(at.x, at.y, -500.0)
    ch = getattr(unreal.TraceTypeQuery, "ECC_VISIBILITY", None) or \
        unreal.TraceTypeQuery.TRACE_TYPE_QUERY1
    result = None
    shape = "none"
    try:
        r = unreal.SystemLibrary.line_trace_single(
            world, start, end, ch, True, [], unreal.DrawDebugTrace.NONE)
        if isinstance(r, unreal.HitResult):
            result, shape = r, "return"
        elif isinstance(r, (tuple, list)) and len(r) == 2:
            result, shape = r[1], "tuple"
    except Exception as ex:
        unreal.log("[SEAT WHAT] %s: trace shape A failed: %s" % (label, ex))
    if result is None:
        unreal.log("[SEAT WHAT] %s: no trace result" % label)
        return
    actor = result.get_editor_property("actor")
    comp = result.get_editor_property("component")
    loc = result.get_editor_property("location")
    mesh = "-"
    try:
        if comp and comp.get_class().get_name().find("StaticMesh") >= 0:
            m = comp.get_editor_property("static_mesh")
            if m:
                mesh = m.get_name()
    except Exception:
        pass
    unreal.log("[SEAT WHAT] %s (trace via %s): surface z=%.0f actor=%s (%s) comp=%s mesh=%s"
               % (label, shape, loc.z,
                  actor.get_name() if actor else "None",
                  actor.get_class().get_name() if actor else "-",
                  comp.get_name() if comp else "-", mesh))


def main():
    # THE MEASUREMENT NEEDS THE DISTRICT, NOT AN EMPTY WORLD. Every earlier
    # version of this probe reported "nothing near the seat" for the simplest
    # possible reason: -run=pythonscript opens the EDITOR, not the map, so it was
    # measuring an empty default level. The seat audit script loads the map before
    # it looks; this must too, or it is a confident answer about nowhere.
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    loaded = les.load_level("/Game/Maps/VEILDistrict")
    unreal.log("[SEAT WHAT] level loaded=%s" % loaded)
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    trace_down(world, SEAT, "SEAT")
    trace_down(world, CENTRE, "PLAZA CENTRE")

    eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    hits = []
    for a in eas.get_all_level_actors():
        for cname, o, e, c in component_bounds(a):
            d = box_gap(SEAT, o, e)
            if d <= RADIUS:
                hits.append((d, a.get_name(), a.get_class().get_name(), cname, o, e))
    hits.sort(key=lambda t: t[0])
    for d, aname, acls, cname, o, e in hits[:10]:
        unreal.log("[SEAT WHAT] gap=%.0f cm %s (%s) comp=%s centre=(%.0f,%.0f,%.0f) ext=(%.0f,%.0f,%.0f)"
                   % (d, aname, acls, cname, o.x, o.y, o.z, e.x, e.y, e.z))
    unreal.log("[SEAT WHAT] components within 25 m of the seat: %d" % len(hits))
    inside = [h for h in hits if h[0] <= 0.5]
    for d, aname, acls, cname, o, e in inside:
        unreal.log("[SEAT WHAT] INSIDE %s (%s) comp=%s ext=(%.0f,%.0f,%.0f)"
                   % (aname, acls, cname, e.x, e.y, e.z))
    unreal.log("[SEAT WHAT] components containing the seat: %d" % len(inside))


main()
