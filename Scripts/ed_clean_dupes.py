"""Remove the untagged first-pass Hub so only the authored, tagged world remains."""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_clean_log.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"
w("open: %s" % bool(unreal.EditorLevelLibrary.load_level(MAP)))


def near_hub(loc):
    d1 = (loc.x ** 2 + loc.y ** 2) ** 0.5
    d2 = ((loc.x - 2800.0) ** 2 + (loc.y - 500.0) ** 2) ** 0.5
    return d1 < 2600.0 or d2 < 2600.0


removed = 0
kept = 0
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    try:
        if a.actor_has_tag("VEIL_AUTHORED"):
            kept += 1
            continue
        is_mesh = isinstance(a, unreal.StaticMeshActor)
        is_light = isinstance(a, unreal.PointLight)
        if not (is_mesh or is_light):
            continue
        loc = a.get_actor_location()
        # the first pass stood on the old deck (182148); the authored deck is 182281
        if near_hub(loc) and 182000.0 < loc.z < 182210.0:
            unreal.EditorLevelLibrary.destroy_actor(a)
            removed += 1
    except Exception:  # noqa: BLE001
        pass

w("removed untagged duplicates: %d" % removed)
w("kept authored: %d" % kept)
w("actors now: %d" % len(unreal.EditorLevelLibrary.get_all_level_actors()))

meshes = [a for a in unreal.EditorLevelLibrary.get_all_level_actors()
          if isinstance(a, unreal.StaticMeshActor)]
w("static mesh actors now: %d" % len(meshes))
zs = sorted({round(a.get_actor_location().z) for a in meshes if near_hub(a.get_actor_location())})
w("distinct piece heights near the hub: %s" % zs)

w("save: %s" % bool(unreal.EditorLevelLibrary.save_current_level()))
w("CLEAN DONE")
