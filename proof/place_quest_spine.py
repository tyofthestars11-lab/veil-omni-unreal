# Seat the quest spine in the SAVED level (not only at runtime), so the three
# talking statues, the three fragments and the cache are visible in the editor,
# editable, and survive a restart. Anchors are Tyree's, never moved.
import unreal

LOG = r'D:\VEIL-OMNI\proof\place_quest_spine.txt'


def log(msg):
    print(msg)
    with open(LOG, 'a', encoding='utf-8') as f:
        f.write(str(msg) + '\n')


log('--- run ---')
MAP = '/Game/Maps/VEILDistrict'
unreal.EditorLevelLibrary.load_level(MAP)

statues = [
    ('Keeper',       unreal.Vector(900.0,    0.0,   0.0)),
    ('Seer',         unreal.Vector(-450.0,   780.0, 0.0)),
    ('Cartographer', unreal.Vector(-4150.0, -780.0, 0.0)),
]
fragments = [
    unreal.Vector(2800.0, 500.0, 150.0),
    unreal.Vector(-2000.0, -900.0, 150.0),
    unreal.Vector(600.0, -2100.0, 150.0),
]
cache = unreal.Vector(0.0, -6200.0, 0.0)

existing_givers = {}
existing_frags = 0
has_cache = False
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    cls = a.get_class().get_name()
    if cls == 'OMNIQuestGiver':
        existing_givers[str(a.get_editor_property('giver_name'))] = a
    elif cls == 'OMNIFragment':
        existing_frags += 1
    elif cls == 'OMNICache':
        has_cache = True

log('BEFORE: givers={} fragments={} cache={}'.format(sorted(existing_givers), existing_frags, has_cache))

placed = 0
for name, loc in statues:
    if name in existing_givers:
        log('  {} already saved at {}'.format(name, existing_givers[name].get_actor_location()))
        continue
    a = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.OMNIQuestGiver, loc, unreal.Rotator(0, 0, 0))
    if not a:
        log('  FAILED {}'.format(name))
        continue
    a.set_editor_property('giver_name', name)
    a.set_actor_label('STATUE_{}'.format(name))
    placed += 1
    log('  placed {} at {}'.format(name, a.get_actor_location()))

for i, loc in enumerate(fragments):
    a = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.OMNIFragment, loc, unreal.Rotator(0, 0, 0))
    if a:
        a.set_actor_label('SIGNAL_FRAGMENT_{}'.format(i + 1))
        placed += 1
        log('  placed fragment {} at {}'.format(i + 1, a.get_actor_location()))

if not has_cache:
    a = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.OMNICache, cache, unreal.Rotator(0, 0, 0))
    if a:
        a.set_actor_label('SURVEY_CACHE')
        placed += 1
        log('  placed cache at {}'.format(a.get_actor_location()))

saved = unreal.EditorLevelLibrary.save_current_level()
log('PLACED {} | SAVE RESULT {}'.format(placed, saved))

givers = []
frags = 0
cache_now = False
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    cls = a.get_class().get_name()
    if cls == 'OMNIQuestGiver':
        givers.append('{}@{}'.format(a.get_editor_property('giver_name'), a.get_actor_location()))
    elif cls == 'OMNIFragment':
        frags += 1
    elif cls == 'OMNICache':
        cache_now = True
log('AFTER: {} | fragments={} cache={}'.format(givers, frags, cache_now))
log('DONE')
