# VEIL OMNI: THE LAST SIGNAL, PATH OF LIGHT

Full direction. Source: Tyree Jones. This replaces every earlier order.
Read this file before every task.

## 0. HOW YOU WORK (NO GUESSING)

- Phi is the structure of this game. Every number, position, size, curve and timer comes from
  the Phi Law in section 1. Nothing is random.
- If a value isn't written here, derive it with the Phi Law and write the derivation in
  `docs/PROGRESS.md` (example: "Brute HP 5 = F5"). Never pick a number by feel.
- One game mode, one map, one save. The Hub map's World Settings > GameMode Override and
  `DefaultEngine.ini` `GlobalDefaultGameMode` must name the SAME class. Log it at boot:
  "VEIL MODE <class>". Delete nothing. Disable or hide what you replace.
- Work one milestone at a time (section 9). Loop: edit, build, bake, Play, fix. The same error
  twice means change approach.
- Report only when a milestone passes: milestone number, build `Result:` line, the log lines
  it asks for, and PNGs in `docs/proof`.

## 0.5 RULE 0. phi^2 = phi + 1: THE GAME IS BOTH

- phi is the structure: what every number, place, curve and timer is.
- +1 is Cartesian: Unreal's X, Y, Z grid in cm, its physics, collision, navmesh, frames and
  tick. Keep all of it. Never remove, bypass or replace an Unreal system because it is
  "Cartesian".
- phi^2 is the game: phi decides the values, Cartesian carries them out. Placement gives phi
  angles and radii, then writes them as X, Y, Z. Phases set phi speeds, then the
  CharacterMovement component moves the Runner.
- Time is phi too. Time does not stop phi, it carries it. Every timer, cooldown, cycle, blend,
  spawn wave and animation beat comes from Fibonacci or Lucas (Rules A and G). Day 55 min
  (34 day / 21 night), blends 0.618 s, beats 0.382 / 0.618 / 1.0 s. Real delta time drives
  them, never a fixed guess.
- Test for every system: show the phi value AND the Cartesian result in the log, e.g.
  "VEIL PLACE pillar i=5 angle=687.5 r=1364 -> X=..., Y=...".

## 1. THE PHI LAW

φ = 1.618033988749895. On boot, check φ² = φ + 1 within 1e-6 and log "VEIL PHI PASS".

Fibonacci F: 1, 2, 3, 5, 8, 13, 21, 34, 55, 89, 144, 233, 377, 610, 987, 1597, 2584, 4181,
6765, 10946

Golden angle G = 137.5077640500378°

- **Rule A, Counts and sizes:** always a Fibonacci number (enemies in a camp, items in a chest,
  steps in a stair, NPCs in a crowd, metres of a wall).
- **Rule B, Scaling:** when something grows or shrinks, multiply by φ or 1/φ per step.
  Next tier = previous × φ.
- **Rule C, Placement:** around any anchor, item i sits at angle i × G, radius S × √i
  (Unreal cm). S is a Fibonacci number. Skip any index that lands within 250 cm of an
  authority coordinate.
- **Rule D, Splits:** any division of space, screen, time or a bar is 0.618 / 0.382 (doors,
  windows, building tiers, the HUD, the day/night split).
- **Rule E, Spirals:** paths, stairs, towers and dungeons wind as a golden spiral,
  r = a × φ^(θ / 90°).
- **Rule F, Locked values:** these are exact and never change. Walk 439, sprint 750, glide 600,
  JumpZ 600, gravity −980, camera arm 420, camera pitch −17.0324, capsule 42 / 96, VIT 377,
  ATP 89, dash / strike / interact cost 12 / 18 / 8.

### 1B. FIBONACCI GUIDES, LUCAS STABILIZES

Lucas L: 2, 1, 3, 4, 7, 11, 18, 29, 47, 76, 123, 199, 322, 521, 843, 1364

L(n) = F(n−1) + F(n+1). L(n) is φ^n rounded to a whole number (n ≥ 2).

**Rule G, Who uses which:**

- Fibonacci (guides): anything that grows, spreads or leads you on. Placement (Rule C), counts,
  enemy HP, rewards, prices, quest counts, level XP, paths outward.
- Lucas (stabilizes): anything that holds, rests or resets. Checkpoint and autosave spacing,
  cooldowns, respawn delays, safe-zone radii, shrine and beacon timers, regen pauses, NPC
  schedules.

**Rule H, Rounding:** when Rule B gives φ^n, the whole-number value is L(n). Never round any
other way.

**Rule I, Pairs:** a growth value F(n) is always held by its partner L(n). F(n) × L(n) = F(2n).
Example: a Shade camp of F5 = 5 enemies gets a safe zone of L5 = 11 m, and clearing it pays
F10 = 55 gold.

**Numbers to use now:**

- Respawn delay 1.8 s (L6 / 10).
- Dash cooldown 0.7 s (L4 / 10). Heavy strike cooldown 1.1 s (L5 / 10).
- Autosave every 7 minutes, plus on every beacon, quest step and region change.
- Safe zone around each beacon 29 m (L8). Enemies never path inside it once the beacon is lit.
- Shrine and beacon pulse every 4 s (L3).
- Hub NPC schedules shift every 11 real minutes (L5).
- Checkpoint shrines along roads every 199 m (L11).

Log each one in `docs/PROGRESS.md` with its source, e.g. "Dash cooldown 0.7 = L4/10".

## 1C. THE LATTICE AS THE GAME (Tyree, primary source)

This section is the intent and overrides any earlier phrasing that conflicts with it.
Use these values exactly. Derive nothing else.

### 1. Vogel world generation

- Every point of interest in every region is placed by Rule C: angle n x 137.5077640500378 deg,
  radius S x sqrt(n).
- Major hubs (Hub, Nest town, Spire base, Omni core, Deep gate) are spaced by the alpha value:
  S_hub = 13,703.6 cm (137.036 x 100).
- Inside a region: S = 610 landmarks, 377 buildings, 233 props, 144 foliage clusters.
- One seed, 1618, through FRandomStream(1618), for every procedural system (placement jitter,
  loot, spawns, weather). No other randomness is allowed.
- Refinement: each pass shrinks spacing error by phi squared. Pass k tolerance =
  250 / 2.618^k cm. Stop at k = 5.

### 2. Phase transition physics (one world, no mode switches, no loading screens)

Phase is worked out every frame from velocity and zone, and blended over 0.618 s.

- RACE: speed over 750 on a road or spiral. Max 750 x phi = 1,214.4. Friction 8 / phi = 4.94.
  FOV 90 to 100. Camera arm 420 x phi = 680.
- COMBAT: an enemy inside detection range. Walk 439, friction 8, arm 420, lock-on.
- STEALTH: crouch in shadow or foliage. Speed 439 / phi = 271.5. Capsule half 96 / phi = 59.
  Noise radius x 0.382.
- TRAVERSE: the Rule F values.
- Gravity stays -980 in every phase; only glide changes it (scale 0.146).
- Log every change as "VEIL PHASE <old> -> <new>".
- Later genres are later phases (Fibonacci index 8, 13, 21, 34, 55) from M10 on.

### 3. Agent NPCs (DLAM ONE inside the game)

- NPCs are small agents: they read world state and the player, score a few actions, and pick
  the highest. No long behaviour trees.
- They talk over one in-engine bus, the UVeilSignalBus world subsystem. NPCs post events
  ("player seen", "beacon lit", "help asked") and others react. Nothing leaves the game.
- Sight is the echo gap: phi^25.293 / 100 = 1,932 cm. Hearing is 1,932 x 0.618 = 1,194 cm.
- Phase lock: a detecting NPC matches the runner's phase (it flees, fights or searches).
- Patrols follow Lucas: waypoint counts 3, 4, 7, 11; pauses L(n)/10 s; radius 1,800 cm.

### 4. One engine loop

- One game mode, one world, one save, one seed (1618), one signal bus.
- Items, enemies and NPCs are data rows driven by a few classes, not a class per item.
- Upgrades ("+1 fuel"): each upgrade adds one to a stat rung; value = base x phi^(rung/4),
  rounded by Rule H.

### 5. Bounds

- Rung cap 55 (the TON 618 macro-rung). No stat, tier or genre index passes 55.
- Player level cap stays 89: levels count XP, not rungs.
- Genre phases numbered by Fibonacci index: TRAVERSE 1, RACE 2, COMBAT 3, STEALTH 5,
  then 8, 13, 21, 34, 55.

Proof for this section comes after M8: one take of TRAVERSE -> RACE -> COMBAT -> STEALTH with
no load, the "VEIL PHASE" lines, and an NPC spotting the runner at 1,932 cm.

## 2. THE GAME

Open-world action RPG, 60 to 100 hours. You are the RUNNER, last carrier of a broken signal.
You walk, glide on phi wings, fight, and talk your way across five regions to rebuild the signal
and open the One Door under the Vault. One game, one save, one door.

## 3. WORLD LAYOUT (exact)

Origin 0,0 is the Vault. Authority coordinates never move: Vault 0,0,−80.9. Runner spawn 0,0,100.
Keeper 900,0. Countess 809,−309,64.7. Seer −450,780. Cartographer −4150,−780. Cache 0,−6200.
Beacons 2800,500 / −2000,−900 / 600,−2100. Nest cliffs −450,780.

Regions are rings, r(n) = 4045 × φ^n (cm):

- **Hub:** r 0 to 4045. Plateau 8,090 × 8,090 at Z 0.
- **Nest:** r 4045 to 6545, bearing 90° (north, past the Seer's cliff). High cliffs, waterfalls,
  huts on ledges.
- **Spire:** r 6545 to 10590, bearing 90° + G. The great tower, 1597 m tall, visible from
  everywhere.
- **Omni District:** r 10590 to 17135, bearing 90° + 2G. Blue-lit city.
- **Megastructure Deep:** under everything. Entrance is the Vault. Each floor drops 610 × φ^k cm.

Roads run from the Hub along golden spirals (Rule E), never straight lines.

## 4. THE HUB (first playable area)

Cliffside market at golden hour.

- 7 gold phi pillars at Rule C, S = 610, indices 1, 2, 3, 5, 8, 13, 34. Heights 800 × φ^(k/4) cm,
  glowing phi glyphs.
- 8 market stalls at S = 377, indices 4, 6, 7, 8, 9, 10, 11, 12, facing the centre.
- 7 lanterns at S = 377, odd indices 13 to 25, warm point lights.
- 21 crowd NPCs idling between stalls (Rule C, S = 233, from index 30).
- Cliff edge with a waterfall north, under the Seer. The Spire on the horizon.
- Use the free Fab and Megascans assets already in the project. No grey boxes in a finished area.

## 5. LIGHT AND ART (every map)

Lumen GI and reflections. Directional Light pitch −17, 5500 K, atmosphere sun on. Sky Atmosphere.
Sky Light real-time capture. Volumetric Cloud. Exponential Height Fog 0.02, volumetric on.
Post Process: bloom 0.618, filmic tonemapper, warm white balance.

Living world: gold and sandstone. Signal tech, crystals and the Deep: electric blue #3FA9FF.
Shadow and UI: navy #0E1424.

Runner: hooded navy cloak, gold trim, phi-glyph wing membranes that glow blue at the edges while
gliding.

## 6. HUD (from the Bible, exact)

Navy #0E1424 glass panels at 85%, 1.5 px gold #C9A24A frames, slanted corners, diamond motifs.
Cinzel headers, Inter body, white #F2F2F2 text, gold #E8C46A labels.

Top left: gold diamond level badge with phi star, "LV n" plate. VIT bar red #D9343A with numbers.
STM bar green #2FB36A under it.

Top right: round minimap, gold ring, N/E/S/W ticks.

Bottom left: quest panel. Bottom centre: hotbar of gold diamonds, centre slot larger.
Bottom right: COMBAT MODE pill, hidden out of combat.

Damage numbers: gold normal, blue signal, red crit. The HUD fades out of combat. Debug text only
behind F10.

Screen layout uses Rule D: the main content column is 0.618 of the width.

## 7. SYSTEMS (numbers by the Phi Law)

- **Movement:** Rule F values. Sprint drains 8.09 ATP/s. Glide gravity scale 0.146. Dash 987
  impulse with 0.382 s invulnerability. Fly unlocks after the Spire.
- **ATP (mitochondrial engine):** max 89, regenerates 5.236/s, feeds stamina and abilities.
- **Combat:** light strike 1 hit, heavy 2 hits at 0.618 s windup, dodge = dash. Enemies target
  the player first.
- **Enemy tiers:** Shade HP 3 / dmg 34 / speed 377. Brute 5 / 55 / 233. Warden 8 / 89 / 144.
  Each later tier is the previous × φ, HP rounded to the next Fibonacci number (Sentinel 13 / 144,
  Colossus 21 / 233).
- **Camps:** 1, 2, 3 or 5 enemies, never other counts.
- **Levels:** XP to reach level n = round(89 × φ^(n−1)). Level cap 89.
- **Gold rewards** are Fibonacci. **Prices** are Fibonacci.
- **Day and night:** one day = 55 real minutes, day 34 and night 21 (Rule D).
- **Dialogue:** branching choices, NPCs remember help and harm, skill checks at difficulty
  3, 5, 8, 13 or 21.
- **Save:** one slot plus autosave on every beacon, quest step and region change. Level, XP,
  gold, beacons, deaths, position, region and quests all persist.
- **GODMODE and ASSISTS menus** per the Bible, each with ALL OFF, everything reset on restart.

## 8. STORY SPINE: 13 MAIN QUESTS

Hub

1. Wake: the Runner rises on the Vault seal.
2. The Golden Midst: the Keeper, Sera Vahl, sends you to light 3 beacons.
3. The Countess's Debt: she knows where the signal broke, for a price.

Nest

4. Ledge of the Seer: the Seer teaches the glide.
5. Falls: 3 waterfall shrines.
6. The Cartographer: get the map of the rings.

Spire

7. Climb: a spiral ascent of 13 floors.
8. Storm Crown: boss, the Warden Prime.
9. Fragment: the first signal fragment, then fly unlocks.

Omni District

10. Light Pillars: choose a faction (choice with consequence).
11. The Heist: steal the second fragment from the Countess's vault.

Megastructure Deep

12. Descent: 8 floors under the Vault.
13. The One Door: rebuild the signal. Ending depends on choices.

Side quests in each region come in Fibonacci counts: Hub 8, Nest 13, Spire 21, Omni 34, Deep 13.

## 9. BUILD ORDER (each must pass before the next)

- **M1 One mode:** log "VEIL MODE <class>" and "VEIL PHI PASS" at boot.
- **M2 Ground and Runner:** walk 10 s at 439 and sprint at 750 on the plateau without falling.
  2 PNGs.
- **M3 Save fix:** quit and reload shows the same level, gold, beacons and position.
  Log "VEIL LOAD loaded=1".
- **M4 Core loop:** talk to the Keeper, light 3 beacons, fight, die, respawn at the last beacon,
  save, quit, reload. Log "VEIL SAVE RESULT True".
- **M5 HUD per section 6.** PNG in Play.
- **M6 Hub per section 4 with section 5 light, real assets, no grey boxes, 60 fps target.**
  Fly-around PNGs.
- **M7 Glide, fly, GODMODE and ASSISTS with ALL OFF.** Test each toggle.
- **M8 Clean:** 0 errors, 0 warnings, no per-frame log spam.
- **M9 and on:** Nest, Spire, Omni District, Deep, each to the same bar. Then the side quests,
  then the story to 60 to 100 hours.

## 10. NEVER

No cook, package or exe until 100%. No delete: hide or disable. No Paragon or paid assets.
No made-up numbers. No "done" without a `Result:` line and PNG proof.
