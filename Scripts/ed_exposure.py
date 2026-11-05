"""A clean sky sent auto-exposure wide open. Clamp it with a post-process volume."""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_exposure.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

MAP = "/Game/VEIL/Maps/Lv_Veil_Town"
unreal.EditorLevelLibrary.load_level(MAP)

vol = None
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_class().get_name() == "PostProcessVolume":
        vol = a
        break
if vol is None:
    vol = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PostProcessVolume, unreal.Vector(0, 0, 1500), unreal.Rotator(0, 0, 0))
    w("post-process volume spawned")
else:
    w("post-process volume reused")

vol.set_editor_property("unbound", True)

s = vol.get_editor_property("settings")
pairs = [
    ("override_auto_exposure_bias", True),
    ("auto_exposure_bias", -1.4),
    ("override_auto_exposure_min_brightness", True),
    ("auto_exposure_min_brightness", 0.55),
    ("override_auto_exposure_max_brightness", True),
    ("auto_exposure_max_brightness", 0.85),
    ("override_bloom_intensity", True),
    ("bloom_intensity", 0.35),
]
for key, val in pairs:
    try:
        s.set_editor_property(key, val)
        w("  %s = %s" % (key, val))
    except Exception as err:  # noqa: BLE001
        w("  %s FAILED %r" % (key, err))

vol.set_editor_property("settings", s)
saved = unreal.EditorLevelLibrary.save_current_level()
w("save: %s" % bool(saved))
w("EXPOSURE DONE")
