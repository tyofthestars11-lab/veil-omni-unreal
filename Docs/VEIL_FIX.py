"""Run INSIDE the Unreal editor (console: py exec(open(...).read())).

Assigns the PHI palette to everything this build placed, removes the competing
directional lights (the exact cause of "Multiple directional lights are competing
... fallback"), sets a single gold sun plus sky, and saves VEILDistrict.
"""
import unreal

PALETTE = {
    "gold": "/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold",
    "metal": "/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic",
    "blue": "/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue",
    "glow": "/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive",
}

eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

mats = {}
for key, path in PALETTE.items():
    m = unreal.EditorAssetLibrary.load_asset(path)
    mats[key] = m
    unreal.log("[VEILFIX] material %s -> %s" % (key, bool(m)))


def mat_for(label):
    up = label.upper()
    if any(k in up for k in ("FALL", "GLOW", "LAMP", "ORB", "BEACON", "LIGHT")):
        return mats["glow"]
    if any(k in up for k in ("CRYSTAL", "OMNI", "DEEP", "BLUE")):
        return mats["blue"]
    if "SPIRE" in up:
        return mats["metal"]
    return mats["gold"]


painted = 0
actors = eas.get_all_level_actors()
for a in actors:
    label = a.get_actor_label()
    if not label.startswith("VEIL_"):
        continue
    comps = a.get_components_by_class(unreal.StaticMeshComponent)
    mat = mat_for(label)
    for c in comps:
        if not mat:
            continue
        slots = max(1, int(c.get_num_materials()))
        for s in range(slots):
            c.set_material(s, mat)
        painted += 1
unreal.log("[VEILFIX] painted %d components on VEIL actors" % painted)

# ONE SUN. The warning is literal: several directional lights fight over forward shading.
suns = [a for a in actors if isinstance(a, unreal.DirectionalLight)]
keep = None
for a in suns:
    if "VEIL_PY_Sun" in a.get_actor_label():
        keep = a
        break
for a in suns:
    if a is keep:
        continue
    eas.destroy_actor(a)
unreal.log("[VEILFIX] directional lights: had %d, destroyed %d" % (len(suns), len(suns) - (1 if keep else 0)))

if keep is None:
    keep = eas.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 6000),
                                      unreal.Rotator(-18.0, 150.0, 0.0))
    keep.set_actor_label("VEIL_PY_Sun")
try:
    lc = keep.directional_light_component
    lc.set_intensity(12.0)
    lc.set_light_color(unreal.LinearColor(1.0, 0.83, 0.6, 1.0))
    lc.set_mobility(unreal.ComponentMobility.MOVABLE)
    lc.set_editor_property("atmosphere_sun_light", True)
except Exception as e:
    unreal.log("[VEILFIX] sun tuning failed: %s" % e)

# sky light so the underside of the world is not black
sky = None
for a in eas.get_all_level_actors():
    if isinstance(a, unreal.SkyLight):
        sky = a
        break
if sky is None:
    sky = eas.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 4000))
    sky.set_actor_label("VEIL_PY_Sky")
try:
    sky.light_component.set_intensity(3.0)
    sky.light_component.set_light_color(unreal.LinearColor(0.85, 0.88, 1.0, 1.0))
    sky.set_editor_property("real_time_capture", True)
except Exception as e:
    unreal.log("[VEILFIX] sky tuning failed: %s" % e)

# unbound post process: fixed exposure, no black crush
ppv = None
for a in eas.get_all_level_actors():
    if isinstance(a, unreal.PostProcessVolume):
        ppv = a
        break
if ppv is None:
    ppv = eas.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(0, 0, 2000))
    ppv.set_actor_label("VEIL_PY_Post")
try:
    ppv.set_editor_property("unbound", True)
    st = ppv.get_editor_property("settings")
    st.set_editor_property("auto_exposure_min_brightness", 1.0)
    st.set_editor_property("auto_exposure_max_brightness", 1.0)
    st.set_editor_property("auto_exposure_bias", 0.5)
    st.set_editor_property("bloom_intensity", 1.2)
    ppv.set_editor_property("settings", st)
except Exception as e:
    unreal.log("[VEILFIX] post tuning failed: %s" % e)

# LIGHT PASS, component-typed: the actor has no directional_light_component and
# SkyLight has no real_time_capture, so the properties are set on the components.
sun_done = 0
for a in eas.get_all_level_actors():
    if isinstance(a, unreal.DirectionalLight):
        for c in a.get_components_by_class(unreal.DirectionalLightComponent):
            try:
                c.set_intensity(18.0)
                c.set_light_color(unreal.LinearColor(1.0, 0.84, 0.62, 1.0))
                c.set_mobility(unreal.ComponentMobility.MOVABLE)
                try:
                    c.set_editor_property("atmosphere_sun_light", True)
                except Exception as e:
                    unreal.log("[VEILFIX] atmosphere flag skipped: %s" % e)
                sun_done += 1
            except Exception as e:
                unreal.log("[VEILFIX] sun component failed: %s" % e)
    if isinstance(a, unreal.SkyLight):
        for c in a.get_components_by_class(unreal.SkyLightComponent):
            try:
                c.set_intensity(6.0)
                c.set_light_color(unreal.LinearColor(0.86, 0.9, 1.0, 1.0))
                try:
                    c.set_editor_property("real_time_capture", True)
                except Exception as e:
                    unreal.log("[VEILFIX] sky realtime skipped: %s" % e)
            except Exception as e:
                unreal.log("[VEILFIX] sky component failed: %s" % e)
unreal.log("[VEILFIX] sun components tuned: %d" % sun_done)

saved = bool(les.save_current_level())
unreal.log("[VEILFIX] saved=%s painted=%d actors=%d" % (saved, painted, len(eas.get_all_level_actors())))
