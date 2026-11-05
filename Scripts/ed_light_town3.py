"""A clean level has a full-strength sky; the Iceland map did not. Cut hard."""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_light_town3.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

MAP = "/Game/VEIL/Maps/Lv_Veil_Town"
unreal.EditorLevelLibrary.load_level(MAP)

for a in unreal.EditorLevelLibrary.get_all_level_actors():
    cls = a.get_class().get_name()
    label = a.get_actor_label()
    if cls == "DirectionalLight":
        c = a.get_component_by_class(unreal.DirectionalLightComponent)
        if label == "VEIL_Fill":
            c.set_intensity(1200.0)
            w("fill 1200")
        else:
            c.set_intensity(16000.0)
            w("sun 16000")
        c.set_mobility(unreal.ComponentMobility.MOVABLE)
    elif cls == "SkyLight":
        c = a.get_component_by_class(unreal.SkyLightComponent)
        c.set_intensity(0.45)
        try:
            c.set_editor_property("real_time_capture", False)
            w("skylight 0.45, capture off")
        except Exception:  # noqa: BLE001
            w("skylight 0.45")
        c.set_mobility(unreal.ComponentMobility.MOVABLE)
    elif cls == "ExponentialHeightFog":
        c = a.get_component_by_class(unreal.ExponentialHeightFogComponent)
        try:
            c.set_editor_property("fog_density", 0.012)
            w("fog 0.012")
        except Exception:  # noqa: BLE001
            pass

saved = unreal.EditorLevelLibrary.save_current_level()
w("save: %s" % bool(saved))
w("LIGHT3 DONE")
