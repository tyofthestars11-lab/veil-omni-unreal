import unreal

LEVEL = "/Game/Maps/VEILDistrict"

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
les.load_level(LEVEL)

sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)

# Idempotent: drop any previous phi nav bounds first.
for a in sub.get_all_level_actors():
    if isinstance(a, unreal.NavMeshBoundsVolume):
        print("NAV REMOVE existing: %s" % a.get_actor_label())
        sub.destroy_actor(a)

nav = sub.spawn_actor_from_class(
    unreal.NavMeshBoundsVolume, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0))
nav.set_actor_label("PhiNavBounds")
# Default volume brush is a 200 uu cube: scale 90 = 18000 uu (180 m) across,
# Z 12 = 2400 uu tall, enough to capture the plaza floor and its ramps.
nav.set_actor_scale3d(unreal.Vector(90.0, 90.0, 12.0))

origin, extent = nav.get_actor_bounds(False)
print("NAV SPAWNED %s origin=%s extent=%s" % (nav.get_actor_label(), origin, extent))

les.save_current_level()
print("NAV SAVED %s" % LEVEL)
