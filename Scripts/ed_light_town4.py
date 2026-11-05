"""The runtime sun stands down on an authored town, so the level lights are final.

Manual exposure now reads those lights honestly: 16000 / 0.45 is night. Daylight.
"""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_light_town4.txt")
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
            c.set_intensity(26000.0)
            w("fill 26000")
        else:
            c.set_intensity(120000.0)
            w("sun 120000")
        c.set_mobility(unreal.ComponentMobility.MOVABLE)
    elif cls == "SkyLight":
        c = a.get_component_by_class(unreal.SkyLightComponent)
        c.set_intensity(2.6)
        w("skylight 2.6")
        c.set_mobility(unreal.ComponentMobility.MOVABLE)
    elif cls == "ExponentialHeightFog":
        c = a.get_component_by_class(unreal.ExponentialHeightFogComponent)
        try:
            c.set_editor_property("fog_density", 0.008)
            w("fog 0.008")
        except Exception:  # noqa: BLE001
            pass

w("save: %s" % bool(unreal.EditorLevelLibrary.save_current_level()))
w("LIGHT4 DONE")
