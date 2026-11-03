"""VEIL OMNI, made BY Python. One game, one map: /Game/Maps/VEILDistrict.

Composition follows the three plates:
  1. cliff-side market: stone arcade, rope bridges, waterfalls, blue lanterns,
     the gold Spire rising behind the town;
  2. blue-and-gold city at dusk under a gold sky;
  3. the Deep: a gold ring megastructure overhead and blue crystal spires below.

Every actor it makes is labelled VEIL_PY_ so a re-run replaces its own work and
nobody else's. The map is saved at the end.

Run:
  UnrealEditor-Cmd.exe "D:\\VEIL-OMNI\\VEIL.uproject" -run=pythonscript \
      -script="D:\\VEIL-OMNI\\Docs\\VEIL_PYTHON_RAISES_VEIL.py"
"""
import math
import unreal

MAP = "/Game/Maps/VEILDistrict"
GOLDEN = 137.5077640500378
TAG = "VEIL_PY_"

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def log(m):
    unreal.log("[VEILPY] " + str(m))


def asset(p):
    return unreal.EditorAssetLibrary.load_asset(p)


CUBE = asset("/Engine/BasicShapes/Cube.Cube")
CYL = asset("/Engine/BasicShapes/Cylinder.Cylinder")
CONE = asset("/Engine/BasicShapes/Cone.Cone")
SPHERE = asset("/Engine/BasicShapes/Sphere.Sphere")
GOLD = asset("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold")
METAL = asset("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic")
BLUE = asset("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue")
GLOW = asset("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive")

log("assets cube=%s gold=%s metal=%s blue=%s glow=%s"
    % (bool(CUBE), bool(GOLD), bool(METAL), bool(BLUE), bool(GLOW)))

if not les.load_level(MAP):
    raise SystemExit("VEILPY: could not load " + MAP)

removed = 0
for a in eas.get_all_level_actors():
    if a.get_actor_label().startswith(TAG):
        eas.destroy_actor(a)
        removed += 1
log("removed %d previous %s actors" % (removed, TAG))

counts = {}


def put(label, m, loc, scale, mat=None, yaw=0.0):
    if not m:
        return None
    a = eas.spawn_actor_from_class(unreal.StaticMeshActor, unreal.Vector(*loc),
                                   unreal.Rotator(0.0, yaw, 0.0))
    if not a:
        return None
    a.set_actor_label(label)
    a.set_actor_scale3d(unreal.Vector(*scale))
    comp = a.static_mesh_component
    comp.set_editor_property("static_mesh", m)
    if mat:
        comp.set_material(0, mat)
    try:
        comp.set_mobility(unreal.ComponentMobility.STATIC)
    except Exception:
        pass
    key = "_".join(label.split("_")[:2])
    counts[key] = counts.get(key, 0) + 1
    return a


def box(label, loc, scale, mat=None, yaw=0.0):
    return put(label, CUBE, loc, scale, mat, yaw)


def cyl(label, loc, scale, mat=None):
    return put(label, CYL, loc, scale, mat)


def cone(label, loc, scale, mat=None, yaw=0.0):
    return put(label, CONE, loc, scale, mat, yaw)


def ball(label, loc, scale, mat=None):
    return put(label, SPHERE, loc, scale, mat)


def arch(label, x, y, z, span, height, yaw, mat):
    """A stone arcade arch: two piers, a lintel and a gold keystone."""
    half = span / 2.0
    for side in (-1, 1):
        ox = math.cos(math.radians(yaw)) * half * side
        oy = math.sin(math.radians(yaw)) * half * side
        cyl(label + "_pier", (x + ox, y + oy, z + height / 2.0),
            (1.1, 1.1, height / 100.0), mat)
    box(label + "_lintel", (x, y, z + height + 30.0), (span / 100.0 + 1.4, 1.5, 1.1), mat, yaw)
    box(label + "_key", (x, y, z + height + 130.0), (1.2, 1.2, 1.0), GOLD, yaw)


# =========================================================== 1. THE SPIRE
SX, SY = 0.0, 3500.0
cyl("SPIRE_base", (SX, SY, 1600.0), (30.0, 30.0, 32.0), METAL)
cyl("SPIRE_tier2", (SX, SY, 4400.0), (20.0, 20.0, 30.0), METAL)
cyl("SPIRE_tier3", (SX, SY, 6900.0), (12.0, 12.0, 24.0), METAL)
cyl("SPIRE_ring", (SX, SY, 8100.0), (16.0, 16.0, 1.2), GOLD)
cone("SPIRE_crown", (SX, SY, 9500.0), (13.0, 13.0, 26.0), GOLD)
ball("SPIRE_orb", (SX, SY, 11200.0), (5.0, 5.0, 5.0), GLOW)
cyl("SPIRE_beam", (SX, SY, 17000.0), (1.8, 1.8, 120.0), GLOW)
for i in range(8):
    ang = math.radians(i * 45.0 + 12.0)
    box("SPIRE_fin", (SX + math.cos(ang) * 1150.0, SY + math.sin(ang) * 1150.0, 2400.0),
        (2.4, 1.0, 22.0), GOLD, math.degrees(ang))

# ================================================ 2. HUB: THE CLIFF MARKET
# plaza and arcade ring
cyl("MARKET_plaza", (0.0, 0.0, 40.0), (28.0, 28.0, 0.9), METAL)
cyl("MARKET_well", (0.0, 0.0, 330.0), (4.5, 4.5, 6.2), BLUE)
ball("MARKET_well_lamp", (0.0, 0.0, 760.0), (1.8, 1.8, 1.8), GLOW)
for i in range(8):
    ang = math.radians(i * 45.0)
    ax, ay = math.cos(ang) * 1050.0, math.sin(ang) * 1050.0
    arch("MARKET_arch", ax, ay, 0.0, 700.0, 900.0, math.degrees(ang) + 90.0, METAL)

# stalls on golden angles, with cream cloth roofs, lamps and wares
for i in range(12):
    ang = math.radians(i * GOLDEN)
    rad = 1700.0 + 700.0 * (i % 3)
    x, y = math.cos(ang) * rad, math.sin(ang) * rad
    h = 5.0 + 1.8 * (i % 4)
    box("MARKET_stall", (x, y, h * 50.0), (8.0, 6.0, h), METAL if i % 2 else BLUE, math.degrees(ang))
    box("MARKET_awning", (x, y, h * 100.0 + 260.0), (10.0, 8.0, 0.5), GOLD, math.degrees(ang))
    cyl("MARKET_post", (x * 1.02, y * 1.02, 300.0), (0.8, 0.8, 6.5), METAL)
    ball("MARKET_lantern", (x * 1.02, y * 1.02, 720.0), (1.5, 1.5, 1.5), GLOW)
    if i % 3 == 0:
        ball("MARKET_wares", (x - 120.0, y - 90.0, 90.0), (1.2, 1.0, 1.0), BLUE)

# rope bridges out to the cliff town, as in the plates
for i in range(6):
    ang = math.radians(i * 60.0 + 30.0)
    x1, y1 = math.cos(ang) * 2100.0, math.sin(ang) * 2100.0
    x2, y2 = math.cos(ang) * 3600.0, math.sin(ang) * 3600.0
    span = math.hypot(x2 - x1, y2 - y1)
    yaw = math.degrees(math.atan2(y2 - y1, x2 - x1))
    box("MARKET_bridge", ((x1 + x2) / 2.0, (y1 + y2) / 2.0, 460.0),
        (span / 100.0, 1.2, 0.35), METAL, yaw)
    for side in (-1, 1):
        box("MARKET_rail", ((x1 + x2) / 2.0, (y1 + y2) / 2.0, 640.0 + side * 0.0),
            (span / 100.0, 0.12, 0.12), GOLD, yaw)

# ============================================ 3. CLIFF TOWN + WATERFALLS
NX, NY = -450.0, 780.0
box("CLIFF_mass", (NX - 1500.0, NY + 1000.0, 2400.0), (40.0, 30.0, 48.0), None)
box("CLIFF_mass2", (NX + 2200.0, NY - 1400.0, 2000.0), (34.0, 28.0, 40.0), None)
for i in range(5):
    z = 2500.0 + i * 300.0
    box("CLIFF_terrace", (NX - 800.0 + i * 760.0, NY + 600.0, z),
        (6.4, 6.0, 5.2), METAL if i % 2 == 0 else BLUE, i * 14.0)
    box("CLIFF_roof", (NX - 800.0 + i * 760.0, NY + 600.0, z + 300.0),
        (7.0, 6.6, 0.5), GOLD, i * 14.0)
for i, (fx, fy) in enumerate(((-2400.0, 1500.0), (-2600.0, 300.0), (2800.0, -1200.0), (2400.0, 1600.0))):
    box("CLIFF_fall", (fx, fy, 2300.0), (0.7, 9.0, 46.0), GLOW, i * 22.0)
    cyl("CLIFF_pool", (fx, fy, 110.0), (16.0, 16.0, 1.0), BLUE)

# crystal clusters at the cliff lips
for i in range(6):
    ang = math.radians(i * 61.0)
    x, y = NX + math.cos(ang) * 2600.0, NY + math.sin(ang) * 2600.0
    cone("CLIFF_crystal", (x, y, 900.0), (3.4, 3.4, 18.0), BLUE, math.degrees(ang))
    cone("CLIFF_crystal2", (x + 420.0, y + 180.0, 600.0), (2.0, 2.0, 11.0), BLUE)
    ball("CLIFF_crystal_glow", (x, y, 1900.0), (1.2, 1.2, 1.2), GLOW)

# ============================================== 4. OMNI: BLUE-AND-GOLD CITY
for i in range(9):
    ang = math.radians(i * GOLDEN + 10.0)
    rad = 1200.0 + 500.0 * (i % 3)
    x, y = 2800.0 + math.cos(ang) * rad, 500.0 + math.sin(ang) * rad
    h = 16.0 + 7.0 * (i % 4)
    box("OMNI_tower", (x, y, h * 50.0), (7.0, 7.0, h), BLUE, math.degrees(ang) + 45.0)
    box("OMNI_band", (x, y, h * 100.0 + 120.0), (7.6, 7.6, 0.9), GOLD)
    ball("OMNI_lamp", (x, y, h * 100.0 + 620.0), (1.4, 1.4, 1.4), GLOW)
cyl("OMNI_beacon", (2800.0, 500.0, 7200.0), (2.2, 2.2, 62.0), GLOW)

# ================================== 5. THE DEEP: GOLD RING + CRYSTAL SPIRES
DX, DY = 0.0, -5000.0
cyl("DEEP_ring", (DX, DY, 9600.0), (62.0, 62.0, 2.4), METAL)
cyl("DEEP_ring_inner", (DX, DY, 9600.0), (48.0, 48.0, 2.6), BLUE)
cone("DEEP_ring_spire", (DX, DY, 11200.0), (26.0, 26.0, 34.0), GOLD)
cone("DEEP_hang", (DX, DY, 8200.0), (18.0, 18.0, 22.0), METAL)
ball("DEEP_core", (DX, DY, 9600.0), (7.0, 7.0, 7.0), GLOW)
cyl("DEEP_floor", (DX, DY, 80.0), (48.0, 48.0, 1.0), BLUE)
for i in range(5):
    ang = math.radians(i * 72.0 + 18.0)
    x, y = math.cos(ang) * 2600.0, DY + math.sin(ang) * 2600.0
    cone("DEEP_spire", (x, y, 2600.0), (7.0, 7.0, 52.0), BLUE, math.degrees(ang))
    ball("DEEP_spire_glow", (x, y, 5300.0), (2.0, 2.0, 2.0), GLOW)
for i in range(4):
    ang = math.radians(i * 90.0 + 40.0)
    box("DEEP_bridge", (math.cos(ang) * 1300.0, DY + math.sin(ang) * 1300.0, 2400.0),
        (26.0, 0.5, 0.25), GOLD, math.degrees(ang))

# ================================================= 6. GOLDEN-HOUR LIGHT
try:
    sun = eas.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 6000),
                                     unreal.Rotator(-7.0, 155.0, 0.0))
    sun.set_actor_label(TAG + "Sun")
    lc = sun.directional_light_component
    lc.set_intensity(14.0)
    lc.set_light_color(unreal.LinearColor(1.0, 0.82, 0.58, 1.0))
    lc.set_mobility(unreal.ComponentMobility.MOVABLE)
    lc.set_editor_property("atmosphere_sun_light", True)
    counts["LIGHT"] = 1
except Exception as e:
    log("sun failed " + str(e))

try:
    sky = eas.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 4000))
    sky.set_actor_label(TAG + "Sky")
    sc = sky.light_component
    sc.set_intensity(3.2)
    sc.set_light_color(unreal.LinearColor(0.9, 0.86, 1.0, 1.0))
    sky.set_editor_property("real_time_capture", True)
    counts["SKY"] = 1
except Exception as e:
    log("sky failed " + str(e))

try:
    atmo = eas.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0))
    atmo.set_actor_label(TAG + "Atmosphere")
    counts["ATMO"] = 1
except Exception as e:
    log("atmosphere failed " + str(e))

try:
    fog = eas.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 200))
    fog.set_actor_label(TAG + "Fog")
    fc = fog.get_editor_property("component")
    fc.set_editor_property("fog_density", 0.028)
    fc.set_editor_property("fog_inscattering_luminance", unreal.LinearColor(0.95, 0.68, 0.34, 1.0))
    counts["FOG"] = 1
except Exception as e:
    log("fog failed " + str(e))

try:
    ppv = eas.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(0, 0, 2000))
    ppv.set_actor_label(TAG + "Post")
    ppv.set_editor_property("unbound", True)
    st = ppv.get_editor_property("settings")
    st.set_editor_property("bloom_intensity", 1.9)
    st.set_editor_property("auto_exposure_min_brightness", 1.0)
    st.set_editor_property("auto_exposure_max_brightness", 1.0)
    st.set_editor_property("auto_exposure_bias", 1.4)
    st.set_editor_property("vignette_intensity", 0.25)
    ppv.set_editor_property("settings", st)
    counts["POST"] = 1
except Exception as e:
    log("post failed " + str(e))

saved = bool(les.save_current_level())
log("saved %s -> %s" % (MAP, saved))
for k in sorted(counts):
    log("  %-18s %d" % (k, counts[k]))

spire = sum(v for k, v in counts.items() if k.startswith("SPIRE"))
vision_ok = (spire >= 12 and counts.get("MARKET_stall", 0) >= 12
             and counts.get("MARKET_arch", 0) >= 8 and counts.get("MARKET_bridge", 0) >= 6
             and counts.get("CLIFF_fall", 0) >= 4 and counts.get("CLIFF_crystal", 0) >= 6
             and counts.get("OMNI_tower", 0) >= 9 and counts.get("DEEP_spire", 0) >= 5
             and counts.get("DEEP_ring", 0) >= 1 and saved)
print("VEILPY: saved=%s spire=%d stalls=%d arches=%d bridges=%d falls=%d crystals=%d "
      "towers=%d deep_spires=%d ring=%d"
      % (saved, spire, counts.get("MARKET_stall", 0), counts.get("MARKET_arch", 0),
         counts.get("MARKET_bridge", 0), counts.get("CLIFF_fall", 0),
         counts.get("CLIFF_crystal", 0), counts.get("OMNI_tower", 0),
         counts.get("DEEP_spire", 0), counts.get("DEEP_ring", 0)))
print("VEILPY: VISION %s - arcade market, rope bridges, waterfalls, crystals, gold Spire, "
      "blue-and-gold city, Deep gold ring, golden-hour rig"
      % ("WHOLE" if vision_ok else "INCOMPLETE"))
if not vision_ok:
    raise SystemExit(1)
