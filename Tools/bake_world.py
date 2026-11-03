"""Bake the generated world INTO VEILDistrict, using the builder's own BakeToLevel.

Run from UnrealEditor-Cmd. No cook, no package: this only builds and saves the map so the
district is level data instead of something a session rolls at runtime.
"""
import unreal

MAP = "/Game/Maps/VEILDistrict"

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
ok = les.load_level(MAP)
print("BAKE: loaded %s -> %s" % (MAP, ok))

actors = unreal.EditorLevelLibrary.get_all_level_actors()
print("BAKE: actors before %d" % len(actors))

builders = [a for a in actors if a.get_class().get_name() == "VEILWorldBuilder"]
print("BAKE: builders in level %d" % len(builders))
if not builders:
    b = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.VEILWorldBuilder, unreal.Vector(0, 0, 0))
    builders = [b] if b else []
    print("BAKE: spawned builder %s" % (builders[0].get_name() if builders else "FAILED"))

if not builders:
    raise SystemExit("BAKE: no builder, nothing to bake")

builder = builders[0]
try:
    builder.bake_to_level()
    print("BAKE: bake_to_level returned")
except Exception as e:
    print("BAKE: bake_to_level raised %s" % e)

after = unreal.EditorLevelLibrary.get_all_level_actors()
print("BAKE: actors after %d" % len(after))

saved = les.save_current_level()
print("BAKE: saved %s" % saved)

# Count what actually landed in the level by class, so the number is evidence.
counts = {}
for a in after:
    n = a.get_class().get_name()
    counts[n] = counts.get(n, 0) + 1
for n in sorted(counts, key=lambda k: -counts[k])[:12]:
    print("BAKE: %-32s %d" % (n, counts[n]))
print("BAKE: DONE")
