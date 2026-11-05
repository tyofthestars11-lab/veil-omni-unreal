"""Clear the plaza by overlap, not by centre.

The last pass missed cases like StaticMeshActor_273: bounds centre just outside
the plaza, geometry 1500 units tall standing on top of it. Anything that is not
ours and whose box intersects the plaza column goes, and every removal is logged.
"""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_clear2_log.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
DECK = 190900.0
RADIUS = 3100.0
BAND_LO = DECK - 3500.0
BAND_HI = DECK + 8000.0
unreal.EditorLevelLibrary.load_level(MAP)

doomed = []
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    try:
        if a.get_class().get_name() != "StaticMeshActor":
            continue
        if "VEIL_AUTHORED" in [str(t) for t in a.tags] or "VEIL_DRESS" in [str(t) for t in a.tags]:
            continue
        o, e = a.get_actor_bounds(False)
    except Exception:  # noqa: BLE001
        continue
    cx = max(abs(o.x) - e.x, 0.0)
    cy = max(abs(o.y) - e.y, 0.0)
    if (cx ** 2 + cy ** 2) ** 0.5 >= RADIUS:
        continue
    lo, hi = o.z - e.z, o.z + e.z
    if hi < BAND_LO or lo > BAND_HI:
        continue
    doomed.append((a, a.get_actor_label(), o, e))

w("level scenery overlapping the plaza column: %d" % len(doomed))
for a, label, o, e in doomed:
    w("  remove %-22s centre (%8.0f, %8.0f, %9.0f) size (%5.0f x %5.0f x %5.0f) top %.0f"
      % (str(label)[:22], o.x, o.y, o.z, e.x * 2, e.y * 2, e.z * 2, o.z + e.z))

for a, _, _, _ in doomed:
    try:
        unreal.EditorLevelLibrary.destroy_actor(a)
    except Exception as err:  # noqa: BLE001
        w("  FAILED %r" % err)

ok = unreal.EditorLevelLibrary.save_current_level()
w("removed %d, save: %s" % (len(doomed), bool(ok)))
w("actors now %d" % len(unreal.EditorLevelLibrary.get_all_level_actors()))
w("CLEAR2 DONE")
