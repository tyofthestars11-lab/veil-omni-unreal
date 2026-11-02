# DO THE GOLD COLUMNS HAVE COLLISION, OR ARE THEY PICTURES?
#
# The frame is blocked by SM_AncientColumn_Big in MID_M_PHI_Gold_0, and every collision
# probe reports clear. That is only possible if the columns are not in the query world.
# This asks the saved level directly, per actor: collision mode, collision profile, and
# how many box shapes the actor actually carries. Read first, patch second.
import unreal

MAP = "/Game/Maps/VEILDistrict"
COLUMN = "SM_AncientColumn_Big"

def main():
    # LOADING A LEVEL AT MODULE TOP LEVEL SILENTLY ENDS THE SCRIPT. It ran, the log said
    # "executed successfully", and not one line of output appeared - because the load
    # invalidates the top-level execution context. Everything lives inside main() now,
    # which is the shape that actually survives a level load.
    les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not les.load_level(MAP):
        unreal.log_error("[VEIL COL] could not load " + MAP)
        return

    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()

    total = 0
    gold = 0
    no_collision = 0
    with_collision = 0
    box_counts = {}
    samples = []

    for a in actors:
        if not isinstance(a, unreal.StaticMeshActor):
            continue
        comp = a.static_mesh_component
        mesh = comp.static_mesh if comp else None
        if not mesh or mesh.get_name() != COLUMN:
            continue
        total += 1
        try:
            mat = comp.get_material(0)
            mat_name = mat.get_name() if mat else "NONE"
        except Exception:
            mat_name = "?"
        if "Gold" in mat_name:
            gold += 1

        try:
            ce = str(comp.get_collision_enabled())
        except Exception:
            ce = "unknown"
        if "NoCollision" in ce:
            no_collision += 1
        elif "Query" in ce or "Physics" in ce:
            with_collision += 1

        try:
            n = len(a.get_components_by_class(unreal.BoxComponent))
        except Exception:
            n = -1
        box_counts[n] = box_counts.get(n, 0) + 1

        if len(samples) < 6:
            loc = a.get_actor_location()
            samples.append("%-28s %-18s boxes=%d at (%.0f, %.0f, %.0f) mat=%s"
                           % (a.get_actor_label(), ce, n, loc.x, loc.y, loc.z, mat_name))

    unreal.log("[VEIL COL] columns=%d gold=%d collision=%d none=%d box_shape_counts=%s"
               % (total, gold, with_collision, no_collision, box_counts))
    for s in samples:
        unreal.log("[VEIL COL] " + s)


main()
