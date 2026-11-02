# One fragment per site. The level already held one at each anchor, so the
# placement pass added a second set - that would double-count the chain (6/3).
import unreal

LOG = r'D:\VEIL-OMNI\proof\dedupe_fragments.txt'


def log(msg):
    print(msg)
    with open(LOG, 'a', encoding='utf-8') as f:
        f.write(str(msg) + '\n')


unreal.EditorLevelLibrary.load_level('/Game/Maps/VEILDistrict')

seen = {}
doomed = []
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if a.get_class().get_name() != 'OMNIFragment':
        continue
    loc = a.get_actor_location()
    key = (round(loc.x / 50.0), round(loc.y / 50.0))
    if key in seen:
        doomed.append(a)
    else:
        seen[key] = a

log('fragments before={} sites={} duplicates={}'.format(len(seen) + len(doomed), len(seen), len(doomed)))
for a in doomed:
    log('  removing duplicate at {}'.format(a.get_actor_location()))
    unreal.EditorLevelLibrary.destroy_actor(a)

saved = unreal.EditorLevelLibrary.save_current_level() if doomed else True
log('SAVE RESULT {}'.format(saved))

left = [(a.get_actor_label(), a.get_actor_location()) for a in unreal.EditorLevelLibrary.get_all_level_actors()
        if a.get_class().get_name() == 'OMNIFragment']
log('fragments after={}'.format(len(left)))
for label, loc in left:
    log('  {} @ {}'.format(label, loc))
log('DONE')
