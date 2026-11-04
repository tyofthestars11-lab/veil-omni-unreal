"""Compose VEILDistrict from the vision's own actors, headless, and audit what landed.

No editor window: this runs under UnrealEditor-Cmd -run=pythonscript. It rebuilds the Golden
Hub and the 25-site Hub Market (12 pillars, 8 stalls, 5 peaks) into the level file, then reads
the level back and reports actor and material counts, so "composed" is a number in a log.
"""
import unreal

MAP = "/Game/Maps/VEILDistrict"
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def log(m):
    unreal.log("[VEIL COMPOSE] " + str(m))


les.load_level(MAP)

removed = 0
for a in eas.get_all_level_actors():
    if a.get_class().get_name() in ("VEILHubMarket", "VEILGoldenHub"):
        eas.destroy_actor(a)
        removed += 1
log("removed old hub/market actors: %d" % removed)

hub_cls = getattr(unreal, "VEILGoldenHub", None)
mkt_cls = getattr(unreal, "VEILHubMarket", None)
log("exposed to python: hub=%s market=%s" % (hub_cls is not None, mkt_cls is not None))

if hub_cls:
    hub = eas.spawn_actor_from_class(hub_cls, unreal.Vector(0, 0, 0))
    hub.build()
    log("HUB parts: %d  viewpoint=%s target=%s" % (
        len(hub.get_components_by_class(unreal.StaticMeshComponent)),
        hub.get_editor_property("view_point"), hub.get_editor_property("view_target")))
else:
    log("HUB MISSING: class not exposed")

if mkt_cls:
    mkt = eas.spawn_actor_from_class(mkt_cls, unreal.Vector(0, 0, 0))
    mkt.build()
    # Pillars/Stalls/Peaks are plain int32 counters, not UPROPERTYs, so Python cannot read
    # them - the C++ Build() already logs them as the PHI HUB MARKET line. Count the parts here.
    log("MARKET parts: %d" % len(mkt.get_components_by_class(unreal.StaticMeshComponent)))
else:
    log("MARKET MISSING: class not exposed")

counts, mats = {}, {}
for a in eas.get_all_level_actors():
    cls = a.get_class().get_name()
    counts[cls] = counts.get(cls, 0) + 1
    if cls != "StaticMeshActor":
        continue
    comp = a.get_editor_property("static_mesh_component")
    if not comp:
        continue
    try:
        for m in comp.get_editor_property("override_materials"):
            if m:
                mats[m.get_name()] = mats.get(m.get_name(), 0) + 1
    except Exception:
        pass

for k in sorted(counts, key=lambda x: -counts[x])[:6]:
    log("actor %-28s %d" % (k, counts[k]))
for k in sorted(mats, key=lambda x: -mats[x])[:8]:
    log("material %-32s %d" % (k, mats[k]))
log("saved -> %s" % les.save_current_level())
