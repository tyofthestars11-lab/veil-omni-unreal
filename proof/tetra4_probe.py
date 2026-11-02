import unreal

ed = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ed.load_level("/Game/Maps/VEILDistrict")

PX, PY = 124.14, -293.82
sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
found = 0
for a in sub.get_all_level_actors():
    try:
        o, e = a.get_actor_bounds(False)
    except Exception:
        continue
    if abs(o.x - PX) < 200.0 and abs(o.y - PY) < 200.0:
        top = o.z + e.z
        if top > 150.0:
            found += 1
            print("PROBE TOP=%.1f base=%.1f  %s" % (top, o.z - e.z, a.get_actor_label()))

print("PROBE SUMMARY: %d tall actor(s) within 200cm of (%.1f, %.1f)" % (found, PX, PY))
