"""Which level meshes actually occupy the plaza? Overlap, not centre distance."""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_probe_big.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
unreal.EditorLevelLibrary.load_level(MAP)

DECK = 182281.19
PLAZA_R = 2600.0
mesh_rows, big_rows = [], []

for a in unreal.EditorLevelLibrary.get_all_level_actors():
    try:
        if a.get_class().get_name() != "StaticMeshActor":
            continue
        if "VEIL_AUTHORED" in [str(t) for t in a.tags]:
            continue
        o, e = a.get_actor_bounds(False)
    except Exception:  # noqa: BLE001
        continue
    # closest approach of the box to the vertical plaza axis
    cx = max(abs(o.x) - e.x, 0.0)
    cy = max(abs(o.y) - e.y, 0.0)
    near = (cx ** 2 + cy ** 2) ** 0.5
    top = o.z + e.z
    if near < PLAZA_R and top > DECK - 400.0:
        mesh_rows.append((near, o, e, a.get_actor_label()))
    if e.x * e.y * e.z > 0:
        big_rows.append((max(e.x, e.y, e.z), o, e, a.get_actor_label()))

w("level meshes (non-VEIL) overlapping the plaza footprint and reaching above the deck: %d"
  % len(mesh_rows))
for near, o, e, label in sorted(mesh_rows, key=lambda r: r[0])[:25]:
    w("  %-24s gap %6.0f | centre (%8.0f, %8.0f, %10.0f) | size %6.0f x %6.0f x %6.0f | top %10.0f"
      % (str(label)[:24], near, o.x, o.y, o.z, e.x * 2, e.y * 2, e.z * 2, o.z + e.z))

w("")
w("largest level meshes by half-extent: %d total" % len(big_rows))
for size, o, e, label in sorted(big_rows, key=lambda r: -r[0])[:15]:
    w("  %-24s half %6.0f | centre (%8.0f, %8.0f, %10.0f) | size %6.0f x %6.0f x %6.0f"
      % (str(label)[:24], size, o.x, o.y, o.z, e.x * 2, e.y * 2, e.z * 2))
w("V BIG DONE")
