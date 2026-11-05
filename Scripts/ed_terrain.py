"""Where is there actually room for a town? Trace the terrain on a grid."""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_terrain.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
unreal.EditorLevelLibrary.load_level(MAP)
world = unreal.EditorLevelLibrary.get_editor_world()


def hit_z(x, y, top=190000.0, bottom=175000.0):
    try:
        r = unreal.SystemLibrary.line_trace_single(
            world, unreal.Vector(x, y, top), unreal.Vector(x, y, bottom),
            unreal.TraceTypeQuery.TRACE_TYPE_QUERY1, False, [],
            unreal.DrawDebugTrace.NONE, True)
        res = r[1] if isinstance(r, (tuple, list)) and len(r) == 2 else r
        if isinstance(r, (tuple, list)) and len(r) == 2 and not r[0]:
            return None
        z = res.impact_point.z if hasattr(res, "impact_point") else None
        return z
    except Exception as err:  # noqa: BLE001
        w("trace error %r" % err)
        return None


STEP = 900
LIM = 9000
heights = {}
for x in range(-LIM, LIM + 1, STEP):
    row = ""
    for y in range(-LIM, LIM + 1, STEP):
        z = hit_z(float(x), float(y))
        heights[(x, y)] = z
        row += " .. " if z is None else "%5d" % (z - 182021)
    w("x=%6d %s" % (x, row))

# flattest clear disc: lowest height spread inside 2400
best = None
for cx in range(-LIM + 2700, LIM - 2700 + 1, STEP):
    for cy in range(-LIM + 2700, LIM - 2700 + 1, STEP):
        vals = [heights[(x, y)] for (x, y) in heights
                if heights[(x, y)] is not None and (x - cx) ** 2 + (y - cy) ** 2 <= 2400 ** 2]
        if len(vals) < 12:
            continue
        spread = max(vals) - min(vals)
        if best is None or spread < best[0]:
            best = (spread, cx, cy, sum(vals) / len(vals), min(vals), max(vals), len(vals))

if best:
    w("FLATTEST disc r=2400: spread %.0f at (%d, %d) mean %.1f min %.1f max %.1f from %d samples"
      % best)
else:
    w("FLATTEST: none")
w("V TERRAIN DONE")
