# 16 x 26000 blew the Spire out white. Bring each lamp down until the SICKA
# stone reads, and save.
import unreal

LOG = r'D:\VEIL-OMNI\proof\tune_spire.txt'


def log(msg):
    print(msg)
    with open(LOG, 'a', encoding='utf-8') as f:
        f.write(str(msg) + '\n')


unreal.EditorLevelLibrary.load_level('/Game/Maps/VEILDistrict')
touched = 0
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if not a.get_actor_label().startswith('SPIRE_LIGHT_'):
        continue
    comp = a.get_editor_property('point_light_component')
    comp.set_editor_property('intensity', 5200.0)
    comp.set_editor_property('attenuation_radius', 3200.0)
    comp.set_editor_property('light_color', unreal.Color(r=255, g=178, b=110, a=255))
    touched += 1
log('lights tuned: {}'.format(touched))
log('SAVE {}'.format(unreal.EditorLevelLibrary.save_current_level()))
log('DONE')
