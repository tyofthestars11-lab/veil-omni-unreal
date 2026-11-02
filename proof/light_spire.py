# The Spire now has SICKA architecture and Iceland mountains, but no light of
# its own - the district's lamps never reached 2.8 km east. Lights go in on the
# same golden angle so the region reads at night as well as day.
import math
import unreal

LOG = r'D:\VEIL-OMNI\proof\light_spire.txt'
GOLDEN_DEG = 137.507764
CX, CY = 2800.0, 500.0


def log(msg):
    print(msg)
    with open(LOG, 'a', encoding='utf-8') as f:
        f.write(str(msg) + '\n')


unreal.EditorLevelLibrary.load_level('/Game/Maps/VEILDistrict')

existing = 0
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_actor_label().startswith('SPIRE_LIGHT_'):
        existing += 1
log('existing spire lights={}'.format(existing))

placed = 0
if existing == 0:
    for i in range(16):
        ang = math.radians(i * GOLDEN_DEG)
        radius = 1100.0 + (i % 4) * 520.0
        loc = unreal.Vector(CX + math.cos(ang) * radius, CY + math.sin(ang) * radius, 620.0)
        a = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PointLight, loc, unreal.Rotator(0, 0, 0))
        if not a:
            continue
        comp = a.get_editor_property('point_light_component')
        comp.set_editor_property('intensity', 26000.0)
        comp.set_editor_property('attenuation_radius', 4200.0)
        comp.set_editor_property('light_color', unreal.Color(r=255, g=190, b=110, a=255))
        comp.set_editor_property('mobility', unreal.ComponentMobility.STATIC)
        a.set_actor_label('SPIRE_LIGHT_{:02d}'.format(i + 1))
        placed += 1

saved = unreal.EditorLevelLibrary.save_current_level()
log('LIGHTS PLACED {} | SAVE {}'.format(placed, saved))

count = 0
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_actor_label().startswith('SPIRE_'):
        count += 1
log('SPIRE ACTORS IN LEVEL: {}'.format(count))
log('DONE')
