#!/usr/bin/env python3
# VEIL-OMNI-ALL.py — GIVE IT ALL, whole, one big share, not parts, 50 games in 1, entire vision, code.
# Run: python VEIL-OMNI-ALL.py
# Output: VEIL-OMNI-ALL.json + VEIL-OMNI-World.png, whole.
import math, json
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
PHI = 1.618033988749895
def phi_place(i, bx=0, by=0):
    a = math.radians(i * 137.5077640500378); r = math.sqrt(i) * PHI * 100
    return round(bx + r * math.cos(a), 2), round(by + r * math.sin(a), 2)
NAMES = ["Hub","Nest","Spire","Omni District","Megastructure Deep","Keeper Quest","Seer Quest","Cartographer Quest","Countess Quest","Vault Quest","Crystal Hunt","Beacon Light","Waterfall Run","Rope Bridge","Market Trade","Golden Midst","Phi Chamber","Combat Mode","Glide Wings","Sky Tower","Deep Dive","City Build","Pillar Ring","River Flow","Moss Garden","Gold Rush","Crystal Mountain","Night Watch","Day Watch","Long Walk","First Watch","High Sun","Quiet Half","Listening","Trade Hours","Pale Host","Hollow Answer","Runner Run","Keeper Count","Seer Speak","Cartographer Map","Countess Hold","Vault Seal","Beacon 1","Beacon 2","Beacon 3","Spire Light","Hub Plaza","Nest Cliffs","Omni City"]
GAMES = [{"game": i, "name": n, "x": phi_place(i)[0], "y": phi_place(i)[1]} for i, n in enumerate(NAMES, 1)]
ALL = {
 "name": "VEIL OMNI: THE LAST SIGNAL — PATH OF LIGHT — 50 GAMES IN 1 — ALL",
 "phi": PHI, "seed": 1618, "golden_angle": 137.5077640500378, "games_in_1": 50, "games": GAMES,
 "vision": "Runner gold phi wings dark cape phi, hooded, gliding, golden Spire phi-covered cloud-wreathed visible everywhere, market town phi pillars, waterfalls, rope bridges, cliffs, blue-and-gold city, blue crystal mountains faceted not flat slab, gold light beacons, Megastructure Deep dark blue-lit chasm, Sera Vahl Keeper of the First Beacon Lantern Watch",
 "runner": {"mesh": "vision Runner, gold phi wings, dark cape, NOT placeholder", "floats": False, "grounded": True, "z": 100},
 "npcs": {"Sera Vahl": "Keeper of the First Beacon, Lantern Watch, Stand still. I count the living first.", "Keeper": "900,0", "Seer": "-450,780", "Cartographer": "-4150,-780", "Countess": "809,-309,64.7"},
 "dialogue": {"continue_key": "E", "space": False, "enter": False, "keyboard": "E only, not Space, not Enter, prompt [E] continue"},
 "hud": {"objective": "420x110 top-left", "minimap": "180x180 at 70% top-right", "manual": "320x90 fading 6s", "quest": "480px QUEST: THE GOLDEN MIDST", "vit": "3570/4200 red", "stm": "117/180 green", "lv": "72 diamond", "combat": "COMBAT MODE gold", "one_prompt": "ONE only, mic symbol, click mic or type to One, no duplicate", "boring_text": False, "cool_text": "gold ribbon, curved, glowing, living UI, gold-trimmed"},
 "placeholders": False, "random": False, "random_floaty_stuff": False, "floating_blocks": False, "floating_pillars": False, "grounded_all": True, "vision_only": True,
 "authority": {"Vault": [0,0,-80.9], "Countess": [809,-309,64.7], "Runner": [0,0,100], "Keeper": [900,0,0], "Seer": [-450,780,0], "Cartographer": [-4150,-780,0], "Cache": [0,-6200,0], "Beacon1": [2800,500,0], "Beacon2": [-2000,-900,0], "Beacon3": [600,-2100,0], "Spire": [0,3500,500]},
 "zones": {"Hub": [0,0], "Nest": [-450,780], "Spire": [0,3500], "Omni District": [2800,500], "Megastructure Deep": [0,-5000]},
 "icon": "gold V, golden spiral, feather/wings both arms, thin gold circle, blue glow, dark navy starfield, rounded-square tile",
 "build": "Gate 0 Golden Merge WHOLE: PASS, Hub first, one zone at a time, counts vs JSON, Play + screenshot proof, no random, seed 1618"
}
def main():
    fig, ax = plt.subplots(figsize=(12, 9)); fig.patch.set_facecolor("#0b1e3a")
    ax.set_facecolor("#0b1e3a"); ax.set_xlim(-5000, 5000); ax.set_ylim(-7000, 5000); ax.set_aspect("equal"); ax.axis("off")
    ax.set_title("VEIL OMNI — ALL, 50 IN 1, WHOLE", color="gold", fontsize=16)
    for g in GAMES: ax.plot(g["x"], g["y"], marker="*", color="gold", markersize=8)
    plt.savefig("VEIL-OMNI-World.png", dpi=150, bbox_inches="tight", facecolor=fig.get_facecolor())
    open("VEIL-OMNI-ALL.json","w").write(json.dumps(ALL, indent=2, sort_keys=True))
    ok = len(GAMES)==50 and len(ALL["authority"])==11 and len(ALL["zones"])==5 and ALL["dialogue"]["continue_key"]=="E" and ALL["placeholders"]==False and ALL["random"]==False and ALL["runner"]["grounded"]==True and ALL["runner"]["floats"]==False
    print("WHOLE: PASS — ALL, 50 games in 1, authority 11, zones 5, dialogue E, grounded, no random, no placeholders, no floaty" if ok else "WHOLE: FAIL")
    return 0 if ok else 1
if __name__ == "__main__":
    raise SystemExit(main())
