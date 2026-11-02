# omni_seat_probe.py - run INSIDE Unreal Editor 5.8.
# Names every actor that holds the plaza seat, so "the plaza centre is occupied"
# becomes an actor with a class and a height instead of a guess.
import math
import unreal

MAP = "/Game/Maps/VEILDistrict"
POINTS = [(0.0, 0.0, 598.0), (0.0, 0.0, 502.0), (0.0, 0.0, 220.0), (0.0, 0.0, 154.0), (0.0, 0.0, 100.0)]


def vec(loc):
    return (float(loc.x), float(loc.y), float(loc.z))


def main():
    print("OMNI SEAT PROBE start")
    try:
        unreal.EditorLoadingAndSavingUtils.load_map(MAP)
        print("OMNI SEAT PROBE loaded " + MAP)
    except Exception as e:
        print("OMNI SEAT PROBE load failed %r" % (e,))

    subsys = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actors = subsys.get_all_level_actors()
    print("OMNI SEAT PROBE actors=%d" % len(actors))

    rows = []
    for a in actors:
        try:
            label = a.get_actor_label()
            cls = a.get_class().get_name()
            loc = vec(a.get_actor_location())
        except Exception:
            continue
        origin = loc
        extent = (0.0, 0.0, 0.0)
        try:
            b = a.get_actor_bounds(False)
            if isinstance(b, (tuple, list)) and len(b) >= 2:
                origin = vec(b[0])
                extent = vec(b[1])
        except Exception:
            pass
        rows.append((label, cls, loc, origin, extent, a))

    for p in POINTS:
        print("OMNI SEAT PROBE point %.1f %.1f %.1f" % p)
        hits = 0
        for label, cls, loc, origin, extent, a in rows:
            if (abs(p[0] - origin[0]) <= extent[0] + 1e-3 and
                    abs(p[1] - origin[1]) <= extent[1] + 1e-3 and
                    abs(p[2] - origin[2]) <= extent[2] + 1e-3):
                hits += 1
                print("  HOLD %s | %s | loc=%s origin=%s extent=%s" % (
                    label, cls,
                    ("%.0f,%.0f,%.0f" % loc), ("%.0f,%.0f,%.0f" % origin), ("%.0f,%.0f,%.0f" % extent)))
                if hits >= 40:
                    print("  ... capped at 40")
                    break
        print("  OMNI SEAT PROBE holds=%d" % hits)

    # What stands near the seat, low enough to be the floor under it.
    near = []
    for label, cls, loc, origin, extent, a in rows:
        d = math.hypot(loc[0], loc[1])
        if d < 900.0:
            near.append((d, loc[2], label, cls, extent))
    near.sort()
    print("OMNI SEAT PROBE near-seat actors=%d" % len(near))
    for d, z, label, cls, extent in near[:40]:
        print("  NEAR d=%.0f z=%.0f %s | %s | extent=%.0f,%.0f,%.0f" % (
            d, z, label, cls, extent[0], extent[1], extent[2]))

    unreal.SystemLibrary.quit_editor()


main()
