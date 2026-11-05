"""The plaza must be a plaza: remove level scenery that stands inside it.

Only plain StaticMeshActors are touched, never the Landscape, never the camera
or lights, and never anything tagged VEIL_AUTHORED. Every removal is logged with
its label, centre and size so the change is auditable.
"""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_clear_plaza.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
RADIUS = 2700.0
unreal.EditorLevelLibrary.load_level(MAP)

doomed = []
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    try:
        if a.get_class().get_name() != "StaticMeshActor":
            continue
        if "VEIL_AUTHORED" in [str(t) for t in a.tags]:
            continue
        o, e = a.get_actor_bounds(False)
    except Exception:  # noqa: BLE001
        continue
    if (o.x ** 2 + o.y ** 2) ** 0.5 > RADIUS:
        continue
    doomed.append((a, a.get_actor_label(), o, e))

w("scenery inside the %0.f-radius plaza: %d" % (RADIUS, len(doomed)))
for a, label, o, e in doomed:
    w("  remove %-26s at (%8.0f, %8.0f, %10.0f) size (%5.0f x %5.0f x %5.0f)" %
      (str(label)[:26], o.x, o.y, o.z, e.x * 2, e.y * 2, e.z * 2))

for a, _, _, _ in doomed:
    try:
        unreal.EditorLevelLibrary.destroy_actor(a)
    except Exception as err:  # noqa: BLE001
        w("  FAILED %r" % err)

ok = unreal.EditorLevelLibrary.save_current_level()
w("removed %d, save: %s" % (len(doomed), bool(ok)))
w("actors now %d" % len(unreal.EditorLevelLibrary.get_all_level_actors()))
w("V CLEAR DONE")
