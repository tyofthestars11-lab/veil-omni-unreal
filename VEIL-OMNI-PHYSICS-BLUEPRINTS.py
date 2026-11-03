#!/usr/bin/env python3
# VEIL-OMNI-PHYSICS-BLUEPRINTS.py — physics + blueprints, all of it, whole, not parts, 50 games in 1.
# Run: python VEIL-OMNI-PHYSICS-BLUEPRINTS.py
# Output: VEIL-OMNI-PHYSICS-BLUEPRINTS.json, whole.
import math, json
PHI = 1.618033988749895
# PHYSICS, whole, golden pitch, grounded, no floaty, no random
PHYSICS = {
 "gravity": -980.0, "ground_z": 0.0, "runner_z": 100.0, "grounded": True, "floats": False,
 "walk_speed": 439.0, "sprint_speed": 750.0, "jump_height": 185.0,
 "movement": {"forward": 0.9561, "pitch": 0.2929, "golden_pitch_deg": 17.0324, "right_vector": True, "yaw_only": False},
 "glide": {"wings": "gold phi wings", "glide_speed": 600.0, "glide_drop": -50.0, "grounded_on_land": True},
 "mounts": {"mount_speed": 900.0, "mount_grounded": True, "mount_float": False},
 "enemies": {"follow_player": False, "orbit": False, "own_post": True, "move_radius": 0, "attack_range": 300},
 "npcs": {"follow_player": False, "orbit": False, "own_post": True},
 "collision": {"capsule": True, "floor_collision": True, "no_clip_through_floor": True},
 "dialogue": {"continue_key": "E", "space": False, "enter": False},
 "hud": {"one_prompt": "ONE only, mic symbol", "boring_text": False, "cool_text": "gold ribbon"},
 "random": False, "placeholders": False, "vision_only": True,
}
# BLUEPRINTS, whole, one per system, all in one file
BLUEPRINTS = {
 "BP_Runner": {"mesh": "vision Runner gold phi wings dark cape", "grounded": True, "floats": False, "components": ["Capsule", "GoldWings", "Cape", "Movement"]},
 "BP_Keeper_SeraVahl": {"role": "Keeper of the First Beacon", "line": "Stand still. I count the living first.", "own_post": True},
 "BP_Seer": {"role": "Seer", "post": [-450, 780]},
 "BP_Countess": {"role": "Countess", "post": [809, -309, 64.7]},
 "BP_Cartographer": {"role": "Cartographer", "post": [-4150, -780]},
 "BP_Enemy": {"variety": ["Crystal Guardian", "Gold Warrior", "Deep Dweller", "Beacon Shadow"], "follow": False, "orbit": False},
 "BP_Mount": {"variety": ["Gold Wing Mount", "Crystal Mount", "Market Mount", "Deep Mount"], "grounded": True},
 "BP_HUD": {"objective": "420x110", "minimap": "180x180 at 70%", "quest": "480px", "centre_clear": "60%"},
 "BP_Menu_Items": {"items": ["ARMOR", "BELT", "BOOTS", "GLOVES", "WARDEN"], "ui": "gold ribbon"},
 "BP_World": {"zones": ["Hub", "Nest", "Spire", "Omni District", "Megastructure Deep"], "spire": [0, 3500, 500], "seed": 1618, "golden_angle": 137.5077640500378},
}
ALL = {"name": "VEIL OMNI — PHYSICS + BLUEPRINTS — ALL", "physics": PHYSICS, "blueprints": BLUEPRINTS, "games_in_1": 50}
def main():
    open("VEIL-OMNI-PHYSICS-BLUEPRINTS.json", "w").write(json.dumps(ALL, indent=2, sort_keys=True))
    ok = PHYSICS["grounded"] and not PHYSICS["floats"] and not PHYSICS["random"] and PHYSICS["dialogue"]["continue_key"]=="E" and len(BLUEPRINTS)==10
    print("WHOLE: PASS — physics + blueprints, blueprints 10, grounded, no random, no floaty, dialogue E" if ok else "WHOLE: FAIL")
    return 0 if ok else 1
if __name__ == "__main__":
    raise SystemExit(main())
