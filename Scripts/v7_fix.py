"""Slim the Spire and let the plaza clear catch blueprint scenery too."""

HUB = "C:/Users/Admin/HarkGame/Scripts/ed_hub_v6.py"
CLR = "C:/Users/Admin/HarkGame/Scripts/ed_clear2.py"
notes = []


def read(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def write(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


# --- Spire: 6x width on a 70-wide column made 420-wide squat tiers, which read
#     as a stack of gold slabs from 800 units away. Tall and slender instead.
hub = read(HUB)
old = """        width = 6.0 - tier
        if put(sp, PROPS + "SM_AncientColumn_Big.SM_AncientColumn_Big",
               V + "M_VEIL_Gold.M_VEIL_Gold", 0.0, 3350.0, z,
               yaw=tier * 36.0, sx=width, sy=width, sz=1.0):
            bump("spire")
        z += 530.0 * 0.94"""
new = """        width = 2.2 - tier * 0.18
        if put(sp, PROPS + "SM_AncientColumn_Big.SM_AncientColumn_Big",
               V + "M_VEIL_Gold.M_VEIL_Gold", 0.0, 3350.0, z,
               yaw=tier * 36.0, sx=width, sy=width, sz=2.0):
            bump("spire")
        z += 530.0 * 2.0 * 0.94"""
if new in hub:
    notes.append("spire already slender")
elif old in hub:
    hub = hub.replace(old, new, 1)
    write(HUB, hub)
    notes.append("spire slimmed to 154 wide x 1060 per tier")
else:
    notes.append("MISS spire block")

# --- the plaza clear skipped scenery that is not literally class StaticMeshActor
clr = read(CLR)
old_cls = """        if a.get_class().get_name() != "StaticMeshActor":
            continue"""
new_cls = """        if not a.get_component_by_class(unreal.StaticMeshComponent):
            continue"""
if new_cls in clr:
    notes.append("clear already mesh-based")
elif old_cls in clr:
    clr = clr.replace(old_cls, new_cls, 1)
    write(CLR, clr)
    notes.append("clear now catches any actor carrying a static mesh")
else:
    notes.append("MISS clear class filter")

print("\n".join(notes))
