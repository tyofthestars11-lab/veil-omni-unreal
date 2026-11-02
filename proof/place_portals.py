# One door, three regions reachable: the hub carries a door to each region map,
# and each region map carries a door home. Idempotent per map.
import unreal

LOG = r'D:\VEIL-OMNI\proof\portals.txt'


def log(msg):
    print(msg)
    with open(LOG, 'a', encoding='utf-8') as f:
        f.write(str(msg) + '\n')


PLAN = {
    '/Game/Maps/VEILDistrict': [
        ('OMNIWorld', unreal.Vector(1400.0, 0.0, 100.0)),
        ('VEILWorld', unreal.Vector(-1400.0, 0.0, 100.0)),
    ],
    '/Game/Maps/OMNIWorld': [
        ('VEILDistrict', unreal.Vector(600.0, 0.0, 160.0)),
    ],
    '/Game/Maps/VEILWorld': [
        ('VEILDistrict', unreal.Vector(600.0, 0.0, 160.0)),
    ],
}

for map_path, doors in PLAN.items():
    if not unreal.EditorLevelLibrary.load_level(map_path):
        log('FAILED to load {}'.format(map_path))
        continue
    existing = {}
    for a in unreal.EditorLevelLibrary.get_all_level_actors():
        if a.get_class().get_name() == 'PhiPortal':
            existing[str(a.get_editor_property('region_tag'))] = a
    log('{}: {} portal(s) already'.format(map_path, len(existing)))

    placed = 0
    for target, loc in doors:
        if target in existing:
            log('  -> {} already stands at {}'.format(target, existing[target].get_actor_location()))
            continue
        p = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.PhiPortal, loc, unreal.Rotator(0, 0, 0))
        if not p:
            log('  FAILED to place portal to {}'.format(target))
            continue
        p.set_editor_property('target_map', target)
        p.set_editor_property('region_tag', target)
        p.set_actor_label('PORTAL_{}'.format(target))
        placed += 1
        log('  placed portal -> {} at {}'.format(target, p.get_actor_location()))
    if placed:
        log('  SAVE {} -> {}'.format(map_path, unreal.EditorLevelLibrary.save_current_level()))
log('DONE')
