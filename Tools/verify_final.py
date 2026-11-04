"""One verification pass: is the composition in the level, and what does it wear."""
import unreal

MAP = "/Game/Maps/VEILDistrict"
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
les.load_level(MAP)


def log(m):
    unreal.log("[VEIL VERIFY] " + str(m))


hub = mkt = None
mats, heights = {}, []
statics = 0
for a in eas.get_all_level_actors():
    cls = a.get_class().get_name()
    if cls == "VEILGoldenHub":
        hub = a
    elif cls == "VEILHubMarket":
        mkt = a
    if cls != "StaticMeshActor":
        continue
    statics += 1
    try:
        o, e = a.get_actor_bounds(False)
        heights.append(2.0 * e.z)
        c = a.get_editor_property("static_mesh_component")
        for m in c.get_editor_property("override_materials"):
            if m:
                mats[m.get_name()] = mats.get(m.get_name(), 0) + 1
    except Exception:
        pass

log("HUB present: %s  parts: %s" % (hub is not None,
     len(hub.get_components_by_class(unreal.StaticMeshComponent)) if hub else 0))
log("MARKET present: %s  parts: %s" % (mkt is not None,
     len(mkt.get_components_by_class(unreal.StaticMeshComponent)) if mkt else 0))
heights.sort()
n = len(heights)
if n:
    tall = sum(1 for h in heights if h >= 300.0)
    flat = sum(1 for h in heights if h < 30.0)
    log("static meshes %d  flat %.1f%%  building-scale %.1f%%  median height %.0f cm"
        % (n, 100.0 * flat / n, 100.0 * tall / n, heights[n // 2]))
for k in sorted(mats, key=lambda x: -mats[x])[:6]:
    log("material %-28s %d" % (k, mats[k]))
log("saved -> %s" % les.save_current_level())
