import unreal

actors = unreal.EditorLevelLibrary.get_all_level_actors()
for a in actors:
    if a.get_class().get_name() == 'VEILVault':
        print('VAULT {} loc={} scale={}'.format(a.get_name(), a.get_actor_location(), a.get_actor_scale3d()))
        print('  origin={} extent={}'.format(*[str(v) for v in (a.get_actor_bounds(False)[0], a.get_actor_bounds(False)[1])]))
        for c in a.get_components_by_class(unreal.StaticMeshComponent):
            sm = c.get_editor_property('static_mesh')
            print('  mesh comp {} static_mesh={} world_scale={} visible={}'.format(
                c.get_name(), sm.get_path_name() if sm else None, c.get_component_scale3d(), c.is_visible()))
            break
    if a.get_actor_label().startswith('RUNNER') or a.get_class().get_name() == 'PlayerStart':
        print('PLAYERSTART {} loc={}'.format(a.get_actor_label(), a.get_actor_location()))
print('DONE')
