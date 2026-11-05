"""Who is standing in the plaza? Name, class and bounds for every actor there."""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_probe_plaza.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
unreal.EditorLevelLibrary.load_level(MAP)

rows = []
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    try:
        o, e = a.get_actor_bounds(False)
        tags = ",".join(str(t) for t in a.tags)
    except Exception as err:  # noqa: BLE001
        w("skip %r" % err)
        continue
    d = ((o.x) ** 2 + (o.y) ** 2) ** 0.5
    if d < 3600.0:
        rows.append((o.z, a.get_class().get_name(), a.get_actor_label(),
                     o.x, o.y, o.z, e.x * 2, e.y * 2, e.z * 2, tags))

rows.sort(key=lambda r: -r[5])
w("actors with bounds centre within 3600 of origin: %d" % len(rows))
w("%-26s %-28s %8s %8s %10s %7s %7s %7s  %s" %
  ("class", "label", "x", "y", "z", "dx", "dy", "dz", "tags"))
for r in rows:
    w("%-26s %-28s %8.0f %8.0f %10.0f %7.0f %7.0f %7.0f  %s" %
      (r[1][:26], str(r[2])[:28], r[3], r[4], r[5], r[6], r[7], r[8], r[9][:40]))
w("V PROBE DONE")
