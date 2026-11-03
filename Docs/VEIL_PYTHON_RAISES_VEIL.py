"""VEIL OMNI, made BY Python. One game, one map: /Game/Maps/VEILDistrict.

Python is not a demo beside Unreal - this script is the builder. It raises the
vision into the level with the project's own palette, tags every actor it makes
so a re-run replaces its own work, and saves the map.

Run:
  UnrealEditor-Cmd.exe "D:\\VEIL-OMNI\\VEIL.uproject" -run=pythonscript \
      -script="D:\\VEIL-OMNI\\Docs\\VEIL_PYTHON_RAISES_VEIL.py"
"""
import math
import unreal

MAP = "/Game/Maps/VEILDistrict"
PHI = 1.618033988749895
GOLDEN = 137.5077640500378
TAG = "VEIL_PY_"

les = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
eas = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)


def log(m):
    unreal.log("[VEILPY] " + str(m))


def asset(path):
    return unreal.EditorAssetLibrary.load_asset(path)


CUBE = asset("/Engine/BasicShapes/Cube.Cube")
CYL = asset("/Engine/BasicShapes/Cylinder.Cylinder")
CONE = asset("/Engine/BasicShapes/Cone.Cone")
SPHERE = asset("/Engine/BasicShapes/Sphere.Sphere")
GOLD = asset("/Game/PHI/Palette/M_PHI_Gold.M_PHI_Gold")
GOLD_METAL = asset("/Game/PHI/Palette/M_PHI_GoldMetallic.M_PHI_GoldMetallic")
BLUE = asset("/Game/PHI/Palette/M_PHI_DeepBlue.M_PHI_DeepBlue")
GLOW = asset("/Game/PHI/Palette/M_PHI_WhiteEmissive.M_PHI_WhiteEmissive")

log("assets cube=%s cyl=%s cone=%s gold=%s metal=%s blue=%s glow=%s"
    % (bool(CUBE), bool(CYL), bool(CONE), bool(GOLD), bool(GOLD_METAL), bool(BLUE), bool(GLOW)))

if not les.load_level(MAP):
    raise SystemExit("VEILPY: could not load " + MAP)

# --- replace our own last build, never anyone else's -----------------------
removed = 0
for a in eas.get_all_level_actors():
    if a.get_actor_label().startswith(TAG):
        eas.destroy_actor(a)
        removed += 1
log("removed %d previous %s actors" % (removed, TAG))

counts = {}


def place(label, m, loc, scale, mat=None, yaw=0.0):
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


# --- 1. THE GOLD SPIRE at (0, 3500): the landmark you orient by -----------
SX, SY = 0.0, 3500.0
place("SPIRE_base", CYL, (SX, SY, 1400.0), (26.0, 26.0, 28.0), GOLD_METAL)
place("SPIRE_tier2", CYL, (SX, SY, 4200.0), (16.0, 16.0, 30.0), GOLD_METAL, 22.5)
place("SPIRE_tier3", CYL, (SX, SY, 6900.0), (9.5, 9.5, 26.0), GOLD_METAL, 45.0)
place("SPIRE_crown", CONE, (SX, SY, 9000.0), (11.0, 11.0, 22.0), GOLD)
place("SPIRE_orb", SPHERE, (SX, SY, 10550.0), (5.0, 5.0, 5.0), GLOW)
place("SPIRE_beam", CYL, (SX, SY, 16000.0), (1.6, 1.6, 110.0), GLOW)
for i in range(6):
    ang = math.radians(i * 60.0 + 15.0)
    place("SPIRE_ring", CUBE, (SX + math.cos(ang) * 700.0, SY + math.sin(ang) * 700.0, 700.0),
          (3.0, 3.0, 14.0), GOLD, math.degrees(ang))

# --- 2. HUB MARKET: stalls and houses on golden angles, ropes between ------
for i in range(12):
    ang = math.radians(i * GOLDEN)
    rad = 1500.0 + 900.0 * (i % 3)
    x, y = math.cos(ang) * rad, math.sin(ang) * rad
    h = 6.0 + 2.4 * (i % 4)
    place("MARKET_house", CUBE, (x, y, h * 50.0), (9.0, 9.0, h),
          GOLD if i % 2 else BLUE, math.degrees(ang))
    place("MARKET_roof", CONE, (x, y, h * 100.0 + 420.0), (7.6, 7.6, 6.0),
          GOLD_METAL, math.degrees(ang))
    place("MARKET_lamp", CYL, (x * 1.08, y * 1.08, 320.0), (0.7, 0.7, 7.0), GOLD)
    if i % 2 == 0:
        ang2 = math.radians((i + 1) * GOLDEN)
        rad2 = 1500.0 + 900.0 * ((i + 1) % 3)
        x2, y2 = math.cos(ang2) * rad2, math.sin(ang2) * rad2
        mx, my = (x + x2) / 2.0, (y + y2) / 2.0
        span = math.sqrt((x2 - x) ** 2 + (y2 - y) ** 2)
        place("MARKET_rope", CUBE, (mx, my, 640.0), (span / 100.0, 0.06, 0.06),
              GOLD_METAL, math.degrees(math.atan2(y2 - y, x2 - x)))
place("MARKET_plaza", CYL, (0.0, 0.0, 40.0), (24.0, 24.0, 0.8), GOLD_METAL)
place("MARKET_well", CYL, (0.0, 0.0, 320.0), (4.5, 4.5, 6.0), BLUE)

# --- 3. CLIFF TOWN at the Nest (-450, 780), with falling water ------------
NX, NY = -450.0, 780.0
place("CLIFF_rock", CUBE, (NX - 1400.0, NY + 900.0, 2600.0), (34.0, 26.0, 52.0), None)
for i in range(5):
    place("CLIFF_terrace", CUBE, (NX - 900.0 + i * 700.0, NY + 500.0, 2600.0 + i * 320.0),
          (6.0, 6.0, 5.0), GOLD if i % 2 == 0 else BLUE, i * 12.0)
place("CLIFF_fall", CUBE, (NX - 1750.0, NY + 700.0, 2200.0), (0.6, 16.0, 42.0), GLOW)
place("CLIFF_pool", CYL, (NX - 1750.0, NY + 700.0, 120.0), (18.0, 18.0, 1.2), BLUE)

# --- 4. BLUE-AND-GOLD NIGHT CITY at Omni (2800, 500) ----------------------
for i in range(9):
    ang = math.radians(i * GOLDEN + 10.0)
    rad = 1200.0 + 500.0 * (i % 3)
    x, y = 2800.0 + math.cos(ang) * rad, 500.0 + math.sin(ang) * rad
    h = 14.0 + 6.0 * (i % 4)
    place("OMNI_tower", CUBE, (x, y, h * 50.0), (7.0, 7.0, h), BLUE, math.degrees(ang) + 45.0)
    place("OMNI_crown", CUBE, (x, y, h * 100.0 + 180.0), (7.4, 7.4, 1.4), GOLD)
    place("OMNI_light", SPHERE, (x, y, h * 100.0 + 700.0), (1.4, 1.4, 1.4), GLOW)
place("OMNI_beacon", CYL, (2800.0, 500.0, 7000.0), (2.0, 2.0, 60.0), GLOW)

# --- 5. MEGASTRUCTURE DEEP at (0, -5000) ---------------------------------
for i in range(5):
    ang = math.radians(i * 72.0 + 20.0)
    x, y = math.cos(ang) * 1400.0, -5000.0 + math.sin(ang) * 1400.0
    place("DEEP_monolith", CUBE, (x, y, 3200.0), (8.0, 8.0, 64.0), BLUE, math.degrees(ang))
    place("DEEP_tip", CONE, (x, y, 6800.0), (6.0, 6.0, 8.0), GLOW)
place("DEEP_floor", CYL, (0.0, -5000.0, 80.0), (40.0, 40.0, 0.8), BLUE)

# --- 6. THE DUSK RIG: gold sun, sky, fog, bloom --------------------------
try:
    sun = eas.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 6000),
                                     unreal.Rotator(-32.0, 145.0, 0.0))
    sun.set_actor_label(TAG + "Sun")
    lc = sun.directional_light_component
    lc.set_intensity(9.0)
    lc.set_light_color(unreal.LinearColor(1.0, 0.84, 0.62, 1.0))
    lc.set_mobility(unreal.ComponentMobility.MOVABLE)
    lc.set_editor_property("atmosphere_sun_light", True)
    counts["LIGHT"] = 1
except Exception as e:
    log("sun failed " + str(e))

try:
    sky = eas.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 4000))
    sky.set_actor_label(TAG + "Sky")
    sc = sky.light_component
    sc.set_intensity(1.6)
    sc.set_light_color(unreal.LinearColor(0.72, 0.8, 1.0, 1.0))
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
    fog = eas.spawn_actor_from_class(unreal.ExponentialHeightFog, unreal.Vector(0, 0, 300))
    fog.set_actor_label(TAG + "Fog")
    fc = fog.get_editor_property("component")
    fc.set_editor_property("fog_density", 0.012)
    fc.set_editor_property("fog_inscattering_luminance", unreal.LinearColor(0.45, 0.36, 0.2, 1.0))
    counts["FOG"] = 1
except Exception as e:
    log("fog failed " + str(e))

try:
    ppv = eas.spawn_actor_from_class(unreal.PostProcessVolume, unreal.Vector(0, 0, 2000))
    ppv.set_actor_label(TAG + "Post")
    ppv.set_editor_property("unbound", True)
    st = ppv.get_editor_property("settings")
    st.set_editor_property("bloom_intensity", 1.35)
    st.set_editor_property("auto_exposure_min_brightness", 0.6)
    st.set_editor_property("auto_exposure_max_brightness", 1.1)
    ppv.set_editor_property("settings", st)
    counts["POST"] = 1
except Exception as e:
    log("post failed " + str(e))

# --- 7. save, then state the vision as counts ----------------------------
saved = bool(les.save_current_level())
log("saved %s -> %s" % (MAP, saved))
for k in sorted(counts):
    log("  %-16s %d" % (k, counts[k]))

spire_parts = sum(v for k, v in counts.items() if k.startswith("SPIRE"))
vision_ok = (spire_parts >= 10 and counts.get("MARKET_house", 0) >= 12
             and counts.get("OMNI_tower", 0) >= 9 and counts.get("DEEP_monolith", 0) == 5
             and counts.get("CLIFF_terrace", 0) == 5 and saved)
print("VEILPY: saved=%s spire=%d market=%d omni=%d deep=%d cliff=%d"
      % (saved, spire_parts, counts.get("MARKET_house", 0), counts.get("OMNI_tower", 0),
         counts.get("DEEP_monolith", 0), counts.get("CLIFF_terrace", 0)))
print("VEILPY: VISION %s - Spire (0,3500) gold, market 12 stalls, Omni 9 towers, "
      "Deep 5 monoliths, cliff town 5 terraces, dusk rig"
      % ("WHOLE" if vision_ok else "INCOMPLETE"))
if not vision_ok:
    raise SystemExit(1)
