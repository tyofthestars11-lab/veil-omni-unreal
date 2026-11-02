"""Give the district's tall decorative pieces real collision.

Why: the plaza's carved stone and the cliffs around it were imported as
DECORATION with no collision at all, so every trace - the runner's floor seat,
the camera's spring arm, the clearance test - passes straight through them. The
body then stands inside stone and the lens fills with the back of a wall that
the game cannot see.

This only touches actors that are TALL (extent Z over 300 cm): cliffs, walls,
monuments. Floors, tiles, fragments, pickups and everything flat stay exactly as
they are.
"""
import unreal

actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
fixed = []
skipped = 0

for actor in actors:
    if not isinstance(actor, unreal.StaticMeshActor):
        continue
    component = actor.static_mesh_component
    if component is None:
        continue
    try:
        origin, extent = actor.get_actor_bounds(False)
    except Exception:
        skipped += 1
        continue
    if extent.z <= 300.0:
        skipped += 1
        continue
    current = component.get_editor_property("body_instance").get_editor_property("collision_profile_name")
    if str(current) == "BlockAll":
        continue
    body = component.get_editor_property("body_instance")
    body.set_editor_property("collision_profile_name", "BlockAll")
    component.set_editor_property("body_instance", body)
    fixed.append(f"{actor.get_name()} (z extent {extent.z:.0f})")

unreal.log(f"PHI COLLISION: {len(fixed)} tall piece(s) now solid, {skipped} flat piece(s) left alone.")
for line in fixed[:40]:
    unreal.log(f"PHI COLLISION:   {line}")
unreal.EditorLoadingAndSavingUtils.save_current_level()
unreal.log("PHI COLLISION: level saved.")
