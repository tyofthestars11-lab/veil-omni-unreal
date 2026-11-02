import unreal

LOG = r'D:\VEIL-OMNI\proof\beacons.txt'


def log(msg):
    print(msg)
    with open(LOG, 'a', encoding='utf-8') as f:
        f.write(str(msg) + '\n')


unreal.EditorLevelLibrary.load_level('/Game/Maps/VEILDistrict')
counts = {}
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    cls = a.get_class().get_name()
    counts[cls] = counts.get(cls, 0) + 1
    if cls in ('VEILBeacon', 'VEILEnemy', 'OMNIWardenPrime', 'VEILWaveSpawner', 'PlayerStart'):
        log('{} {} @ {}'.format(cls, a.get_actor_label(), a.get_actor_location()))
log('COUNTS ' + str(sorted(counts.items(), key=lambda kv: -kv[1])[:12]))
log('DONE')
