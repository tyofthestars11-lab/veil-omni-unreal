# PROGRESS

Everything here is a measurement with a source. Numbers come from `docs/VEIL_DIRECTION.md`,
the Phi Law, or a log line; nothing is chosen by feel.

## M1 — One mode — PASS (2026-11-08)

### M1 evidence, re-run 2026-11-08 17:41 (the four required proofs)

- `VEIL MODE /Script/VEIL.VeilOmniGameMode` — plain boot, no map and no `?game=` argument; the
  class comes from `Config/DefaultEngine.ini`, and the map's World Settings agree with it.
- `VEIL PHI PASS residual=0.000000000` — φ² − φ − 1 at boot.
- `Target is up to date` / `Result: Succeeded` / `Total execution time: 2.57 seconds`
  (VEILOMNIEditor Win64 Development). Stated plainly: this was a no-op build, so the line proves
  the target is current and links, not that a translation unit was recompiled; the binary is the
  09:33 build of this source.
- `docs/proof/M1_play_2026-11-08.png` — the run's own opening camera frame, copied from
  `Saved/Screenshots/WindowsEditor/VEIL_hub_beacon.png` (10:41, 1,311,189 bytes).

Same run, other lines: `VEIL HUB: beacons=3`; `VEIL PERF: frames=120 mean_ms=46.06 p50=24.84
p95=152.84 p99=400.00 max_ms=400.00`. That is the third measurement of one build today — 22.87
ms on the proof run, 34.52 ms on the 17:26 boot, 46.06 ms on this one — and the spread is the
point: the mean doubles between runs. Ticket W0-01 covers it; no frame-time claim is made here.

Still not proven: a keypress. `VEIL INPUT:` remains 0 (ticket W0-04). M1 is about one mode and
the boot path, and it stops there.

### Orders on record, 2026-11-08

- **No Paragon in VEIL** (Tyree's rule). Inventory and replacement plan: ticket **L-06** in
  `docs/ASSET_LICENSES.md`. Nothing is deleted, no pack is uninstalled, and the L-01 CC0 re-import
  is not started until Tyree approves.
- **Icons: Kenney UI Pack (CC0)**, not game-icons.net — the plan now owes no attribution at all.
  Fonts stay Cinzel and Inter, with the OFL texts shipped in `Licenses/`.
- **Week-0 sweep**: 58 gates executed, PASS 55 / FAIL 3 / UNMEASURED 0; tickets W0-01..W0-04 in
  `docs/WEEK0_TICKETS.md`.

**What was wrong.** Three names for one thing, and the map quietly won:

| Where | Before | After |
|---|---|---|
| `Config/DefaultEngine.ini` `GlobalDefaultGameMode` | `/Script/VEILOMNI.VeilGameMode` | `/Script/VEIL.VeilOmniGameMode` |
| `Hub_Slice` World Settings > GameMode Override | `/Game/VEIL/Blueprints/BP_GameMode.BP_GameMode_C` | `/Script/VEIL.VeilOmniGameMode` |

The map override is why launching with no arguments built a world but never ran the Hub game
mode: the Blueprint mode won over the ini, so no HUD, no opening, no `VEIL MODE` line. The
map was repointed with `Saved/set_default_mode.py` and saved.

**Build:** `Result: Succeeded` (VEILOMNIEditor Win64 Development).

**Log lines, plain boot (no map, no `?game=`), from `Saved/Logs/VEIL_boot.log`:**

```
VEIL MODE /Script/VEIL.VeilOmniGameMode
VEIL PHI PASS residual=0.000000000
```

The same two lines are emitted by the measured proof mode
(`VEIL MODE /Script/VEIL.VeilHarkProof`), so a proof run carries M1 as well.

**Gates:** `PASS 35 | FAIL 0 | UNMEASURED 0 | TOTAL 35`, exit 0. `boot.mode` compares the class
the boot actually ran against the class named in `DefaultEngine.ini`; `boot.phi` checks
φ² − φ − 1 ≤ 1e-6. Error triage: `RESULT CLEAN`.

**PNGs (`docs/proof`, captured by the Hub's own opening camera):**

- `VEIL_hub_wide.png` — plaza, stalls, pillars at golden hour
- `VEIL_hub_sera.png` — Sera Vahl at the pillars
- `VEIL_hub_beacon.png` — the first beacon

**Re-run:** `bash Saved/verify_all.sh` (boot with no arguments, then the proof run, then both
gates and the screenshots).

## Monster bodies — Paragon out, Quaternius CC0 in (2026-11-08 18:03)

- `VEIL MINION SKIN: SK_Minion_Lane_Super is a Skeleton` — the old path named a skeleton, which
  is why loading it as a body always failed.
- `VEIL MINION BODY: resolved Skeleton1 (/Game/Monsters/Skeleton1.Skeleton1)`
- `VEIL MINION IDLE: blendspace SkeletonSkeletonArmature_Skeleton_Idle`
- `VEIL MINION ATTACK: SkeletonSkeletonArmature_Skeleton_Attack on Skeleton1`
- `VEIL MINION BODY: Skeleton1 | skeleton Skeleton_Skeleton | material0 Skeleton` — six times, one
  per enemy.
- `VEIL ENEMY ROSTER: 6/6 enemies alive with monster bodies (Skeleton1)`
- `VEIL MINION PHASE: golden-ratio offsets 0.000 0.618 0.236 0.854 0.472 0.090`
- `VEIL MINION GROUND: minion Z 887 vs plaza deck Z 1384` — the pivot sits 497 cm under the feet
  by construction, so the soles land on the deck; the offset comes from the mesh bounds.
- `grep -rni paragon Source/` = **0**. Build `Result: Succeeded`, `Total execution time: 18.90 s`.
- `Result:` re-sweep **PASS 54 | FAIL 4 | UNMEASURED 0 | TOTAL 58**. The two failing gates are the
  same two as the 17:26 sweep: `perf.frame` (mean 55.35 ms, p95 202.15 ms — ticket W0-01, and the
  mean row crossed the floor on this run where it had not at 17:28) and `camera.beacon`
  (camp_to_lip −27.3°, sightlines 2/3 — W0-02/W0-03). No gate was changed, added or weakened.
- The roster atoms are captured with `veil.Minions` at 1 for one build; the shipped default is
  back to 0 and the final binary is the 0 build. Evidence: `Saved/Logs/ROSTER_CAPTURE.txt`,
  `Saved/monsters_verify.txt`, `Saved/Logs/VEIL_town_monsters.log`.

## Licenses and credits — L-04 closed (2026-11-08 19:20)

- `Licenses/` now exists at the project root: `OFL-1.1.txt` (4,128 B), `Cinzel-OFL.txt`
  (4,382 B), `Inter-OFL.txt` (4,376 B), `README.md` (2,283 B). The two per-font files carry their
  own copyright line; the text was fetched from the font repos, not retyped.
- Credits panel: `F9`, bound in `AVeilPlayerController::SetupInputComponent`, drawn by
  `AVeilHud::CreditsPanel`. C is combat, F10 is debug, P/Escape is pause, so F9 was the free key.
- `Result:` build **Succeeded** after the panel was wired and again after the capture default was
  restored; the shipped `bCreditsOpen` is **false**, so the panel only appears on F9.
- Proof frame: `docs/proof/credits_panel_2026-11-08.png` (1280×720), the engine's own screenshot
  (`Saved/Screenshots/WindowsEditor/VEIL_dialogue.png`, `LogCore: Tracing Screenshot`). The panel
  was forced open for that one build; the frame shows CREDITS, the two OFL notice paths,
  `Kenney UI Pack, CC0 1.0`, `Animated Monster Pack by Quaternius, CC0 1.0`, and `[F9] close`.
- Not proven: a real keypress. This session sees 12 desktop windows and **none** owned by the game
  process, so F9 could not be delivered to the game window. The binding is code-verified; the
  render is frame-verified.
- `Result:` re-sweep after the HUD change: **PASS 56 | FAIL 2 | UNMEASURED 0 | TOTAL 58** — only
  `camera.beacon` fails on this run (camp_to_lip −27.3°, sightlines 2/3, W0-02/W0-03).
  `perf.frame` **passed here** after failing on the two runs before it (mean 34.9–55.4 ms against
  a 33.3 ms floor, p95 up to 202 ms), so the gate still swings with the machine rather than with
  this change: ticket W0-01 stays open and this pass is not a fix. No gate was changed.
- The first credits build was swept at 54/4 and this one at 56/2 on identical HUD code; that swing
  between sweeps is the same variance W0-01 records, now visible in the sweep line itself.
- No purchase was made. The obligation is unchanged and is now physically present: the OFL texts
  ship in `Licenses/`, and packaging must copy that folder beside the build.

## input.bind — six actions, measured (2026-11-08 19:35)

The sweep could not see input because nothing logged it. Six atoms now sit in the handlers the
`InputComponent` already calls, in `VeilPlayerController`:

- `OnInteract` (E) → `VEIL INPUT: interact fired`
- `OnSprintStart` (LeftShift) → `sprint`
- `OnGlideStart` (SpaceBar) → `glide`
- `OnStrike` (F) → `strike`
- `OnCombatMode` (C) → `combat`
- `OnCreditsToggle` (F9) → `credits`

Each carries a running count, and the sixth prints
`VEIL INPUT: PASS six bound keys fired: interact, sprint, glide, strike, combat, credits`.
The new gate `input.bind` requires **>= 6** distinct `VEIL INPUT: <action> fired` atoms and reads a
dedicated `Saved/Logs/VEIL_input.log` so a later boot cannot erase the session. Gates are now
**59**.

`Result:` `input.bind | actions_fired | 6 | >= 6 | PASS (E Shift Space F C F9: combat, credits,
glide, interact, sprint, strike)`. Full sweep on the same build: **PASS 55 | FAIL 4 | UNMEASURED 0 |
TOTAL 59** — `perf.frame` (mean 82.63 ms, p95 250.89 ms) and `camera.beacon` (−27.3°, 2/3) still
fail; both are pre-existing tickets W0-01/W0-02/W0-03, and the perf numbers keep swinging with the
machine.

**What this does and does not prove.** The six keys were fed through `APlayerController::InputKey`
under a test-only `-VeilInputProbe` flag, which is the same dispatch that turns a keyboard press into
the bound handler — so the atoms prove the bindings fire, not merely that six functions can be
called. They do **not** prove a human hand: no person played this session, and that is the remaining
gap. The flag is off unless passed, so shipping input is unchanged.

## Runner's wings — built, exported, imported (2026-11-08 16:45)

Blender 5.2.3 LTS at `C:\Program Files\Blender Foundation\Blender 5.2\blender.exe`, driven
headless (no MCP, no installs). Scene metric, unit scale 0.01, so 1 Blender unit = 1 cm.
Scripts: `docs/blender/wings_build.py`, `wings_render.py`, `import_wings.py`, `attach_and_shot.py`.

| Value | Phi source | Built (cm) |
|---|---|---|
| Wingspan | `96 x phi` | **155.331** (measured, exact) |
| Half span | `96 x phi / 2` | 77.666 |
| Wing arm | `(96 x phi / 2) / phi` | 48.000 |
| Root / mid / tip chord | `42`, `42/phi`, `42/phi^2` | 42.000 / 25.957 / 16.043 |
| Primaries / secondaries / coverts | Fibonacci `8, 13, 21` | 84 feathers total |
| Shortest covert | `42/phi^4` | 6.128 |
| Thickness root → tip | `42/phi^8` → `42/phi^9` | 0.894 → 0.553 |
| Fan per row | `GoldenAngleDeg` | 137.5078 deg |
| Dihedral | `GoldenPitchDeg` | 17.0324 deg |
| Sweep | `180/phi^3`, folded `/phi` | 42.492 / 26.262 deg |

Four placement attempts, each measured rather than assumed: 179.812 (feathers grew outward from a
radius), 156.411 (Euler did not match the closed form), 150.845 (rows clamped under the span) and
144.411 (per-side shift overshot). The final build anchors every feather root along the arm so the
vanes overlap into one wing, then closes the span with a measured loop: `measured=150.845
factor=1.02974` -> `final=155.331 target=155.331`. That 2.97% X scale means the authored lengths
above are within ~3% of their nominal values; the ratios are untouched.

**QA (qa-review): NO-SHIP on the art, asset data sound.** The render reads as two flat slabs with
stepped edges rather than feathered wings: the rows sit within 2 cm of one plane, the vanes
overlap so heavily they merge, and the far wing is unlit. Geometry budget, span, import scale and
naming pass. What it needs is one modelling pass (per-feather Z stagger and width reduction so
individual vanes read, plus a fill light), not more numeric work.

**Result:** `VEIL WINGS BUILD: tris=1716 span_cm=155.331 feathers=84` — 1,716 of the 5,000 ceiling.
FBX `Content/VEIL/Art/Wings/SM_RunnerWings.fbx` 90,764 B.
**Result:** `VEIL WINGS IMPORT: ok=1 errors=0 assets=1`, bounds
`extent=(77.67,39.22,13.79) span_cm=155.33` — the 100x unit error was caught by that bounds check
and fixed with a 0.01 import scale rather than by rescaling the mesh.

Proof: `docs/proof/wings_render_2026-11-08.png` (705,935 B) and `wings_wire_2026-11-08.png`
(742,209 B). **Not yet proven:** the editor shot of the wings on the Runner. Two attempts failed
(attach API absent, then a capture of empty sky); the blank PNG was deleted rather than kept as
proof. The asset is imported and correct; the capture harness is what is missing.

## Rule G / H / I ledger — the stabilising numbers

Lucas L (n = 0, 1, 2, …): 2, 1, 3, 4, 7, 11, 18, 29, 47, 76, 123, 199, 322, 521, 843, 1364.
L(n) = F(n−1) + F(n+1). Rule I check: F(5) × L(5) = 5 × 11 = 55 = F(10). ✔

| Value | Source | Applies to | Status |
|---|---|---|---|
| Respawn delay 1.8 s | 18 / 10 = L6 / 10 | death → last beacon | specified, not yet wired |
| Dash cooldown 0.7 s | 7 / 10 = L4 / 10 | dash (Rule I pairs it with F4 = 3 hits of invuln 0.382 s) | specified, not yet wired |
| Heavy strike cooldown 1.1 s | 11 / 10 = L5 / 10 | heavy attack at 0.618 s windup | specified, not yet wired |
| Autosave every 7 min | 7 = L4, in minutes | plus every beacon, quest step and region change | specified, not yet wired |
| Beacon safe zone 29 m | 29 = L7 (0-based; the direction's `L8` label counts L0 = 2 as L1) | enemies never path inside a lit beacon's zone | specified, not yet wired |
| Beacon / shrine pulse 4 s | 4 = L3 | light pulse while the signal holds | specified, not yet wired |
| Hub NPC schedule shift 11 min | 11 = L5, in real minutes | villager loops and shop hours | specified, not yet wired |
| Checkpoint shrines every 199 m | 199 = L11 | spacing along every road | specified, not yet wired |

The safe-zone index is the one place the direction's label and its own Lucas list disagree:
29 m is written as `L8`, but with L0 = 2 the eighth entry is 47 and 29 is the seventh. The
number 29 m stands; only the label is corrected here, and the value is not changed.

## Also measured (carried from the Hub slice before M1)

- Frame time on the measured proof run: `mean_ms=13.39 p50=11.86 p95=14.81 p99=22.58`
  (`perf.frame` gate ≤ 33.3 ms mean, ≤ 50 ms p95). Plain-boot opening: mean 22.47 ms,
  p95 24.10 ms — the difference is the opening's shader warm-up, not the market.
- Stall posts: `post_scale=0.7503 posts_top=198.8 awning_under=198.8 gap=0.00` — the posts now
  end under the awning instead of 20 cm proud of it.
- World: `seed=1618 pillars=5 stalls=8 lanterns=7 instances=60 missed=0 tiles=73` at
  `stall_spacing 247.147` / `prop_gap 417.57`, the values the golden-section learner settled on.

## Next — M2 Ground and Runner

Walk 10 s at 439 and sprint at 750 on the plateau without falling; 2 PNGs. The locked values
are already in `AVeilCoreRunner` (walk 439, sprint 750, JumpZ 600, gravity −980, arm 420,
pitch −17.0324); M2 is about proving the ground holds the Runner for a full 10 s at both speeds
and capturing the frames.

## 1C lattice (Tyree declassification) - derivations

- One table, one authority: `Source/VEIL/VeilCanon.h`. The HUD, stats, world and dialogue read
  it; no other file states a number.
- Hub spacing 13,703.6 cm = 137.036 x 100 (alpha inverse). Region spacings 610 / 377 / 233 / 144.
- Seed 1618 only; refinement tolerance 250 / 2.618^k, stop k = 5.
- SIG pool 144 = F12 (next tier after STM 89: 89 x phi = 143.99, Rule H rounds to 144).
  SIG regen 3.236/s = 2 x phi. Beam cost 21 = F8.
- Phase numbers are the section's own: RACE 1214.4 = 750 x phi, friction 4.94 = 8/phi,
  arm 680 = 420 x phi; STEALTH 271.5 = 439/phi, half 59 = 96/phi, noise 0.382; blend 0.618 s.
- Sight 1,932 cm = phi^25.293/100; hearing 1,194 = 1,932 x 0.618; patrol 1,800 = L6 x 100.
- Upgrade fuel: value = base x phi^(rung/4), rung capped at 55. Pillar heights use it:
  800, 887, 984, ... (k/4 steps).
- HUD bars are the Fibonacci ladder: 377 / 233 / 144 wide, 20 / 14 / 12 tall.

## M2 - Ground and Runner (PASS)

- Lane: plateau perimeter, radius 3900 cm (outside every prop ring, max 3557; inside the 4045
  edge). Runner driven through CharacterMovement with AddMovementInput, real delta time.
- Walk 10.00 s: steady 439.0, mean 434.6 (acceleration ramp), path 4342.9 cm, fell=0.
- Sprint 10.00 s: steady 750.0, mean 747.5, path 7479.6 cm, fell=0.
- fell = min Z over the whole run >= plateau Z - 1. min Z 342 = spawn height (PlateauZ + 96).
- Two frames: docs/proof/VEIL_m2_walk.png, docs/proof/VEIL_m2_sprint.png.
- Found and fixed on the way: the deck was built with HalfPlateau/100 on a 100 cm engine cube,
  i.e. a 4045 cm platform (half the spec). Scale is now PLATEAU/100 = 80.90 (8090 cm), and the
  parapet spans the full edge.
- RACE is now gated on bOnRoad: sprinting on open ground stays TRAVERSE at 750, so the locked
  sprint is not silently replaced by the 1214.4 road speed.
- Gates: 43 atoms, 0 FAIL, 0 UNMEASURED.

## The Kraken bible, studied whole (2026-11-08)

- `docs/KRAKEN_STUDY.md`: both versions of the bible (315-line original, 336-line
  patched verifier), `VISION_SCOPE.md`, `PHI_APPLIED.md` and the reverse-engineering note
  read in full and every invariant measured against `Source/VEIL`, `Content/VEIL` and
  `Config/`.
- Ledger: 16 MET, 6 PARTIAL, 14 ABSENT, 9 CONFLICT, 1 UNMEASURED.
- The Hub fails the bible's own acceptance test (counts vs JSON): 7 pillars against 12,
  0 blue crystals against 5.
- `[E]` is bound only in the unused `AVeilRunnerCharacter`; the live pawn
  (`AVeilRunnerPhi`) binds Move, Look, Jump, Sprint and nothing else, so there is no
  interaction atom.
- Cartographer (-4150,-780), Cache (0,-6200) and Vault (0,0,-80.9) do not exist; the
  Spire stands at (14000,-9000) instead of (0,3500,500).
- Nine conflicts are listed rather than resolved, because each one changes locked values
  or the Hub silhouette: jump 185 vs 600, attack range 300 vs 144, VIT 4200 vs 377, the
  HUD sizes, the pillar/crystal counts, enemies following vs holding post, jitter vs
  golden-angle-only, the Spire anchor, and the Keeper's opening line.
- Order the bible itself implies: book the Hub atom (counts, E interaction, missing
  anchors, Spire anchor) before any second region.

## The gap ledger (2026-11-08)

- `docs/GAP_LEDGER.md`: this slice measured against a AAA reference and against a
  defensible ten-minute vertical slice, with an asset/audio/animation requirement list and
  a cost model with its rates stated.
- Measured here: 15 content assets referenced by code, 4 engine primitive kinds, 3
  animation clips (none authored), 0 sounds played, 0 VFX, 0 menus, 1 zone, 6,545 m2 of
  plateau, ~3 minutes of content.
- Estimated: a full ten-minute slice is $276k-656k over 6-9 months with 7-9 people; the
  perceptual core (audio, animation, look-dev, VFX, UI) is $12k-34k over 6-10 weeks.
- The maths that matters: the slice is 0.02-0.1% of a AAA production, and no amount of
  further C++ closes that, because the remaining percentage is content, sound and time.

## What the Hub must look like (2026-11-09, from the docs, before any edit)

1. Golden hour: "warm sun low on the horizon, god rays, volumetric clouds, light mist in valleys" (VEIL_Bible.md 2).
2. "Gold phi pillars with glowing phi glyphs" standing among the stalls (VEIL_Bible.md 2; HUB_PAINTING_SPEC "Pillars belong inside the market footprint ... carry a phi glyph panel").
3. Seven pillars, Rule C at S = 610, indices 1,2,3,5,8,13,34, heights 800 x phi^(k/4) (VEIL_DIRECTION.md 4).
4. "One great Spire on the horizon, visible from everywhere", stepped gold, grounded on rock, cloud below it (VEIL_Bible.md 2; HUB_PAINTING_SPEC 2).
5. "Blue crystal mountains on the horizon" at 20,000 to 40,000 cm, faceted not flat slab (VEIL_Bible.md 2; VISION_KRAKEN "blue crystal mountains faceted not flat slab"; HUB_PAINTING_SPEC 6).
6. Stalls are "blue canvas, not stone"; houses stone with timber, pitched, stepped up the terrace (HUB_PAINTING_SPEC rules).
7. Cliff edge with a waterfall north under the Seer (VEIL_DIRECTION.md 4; HUB_PAINTING_SPEC 4).
8. HUD: "Navy #0E1424 at 85% opacity ... 1.5 px gold line #C9A24A" with slanted corners and diamond motifs (VEIL_Bible.md 3; VEIL_DIRECTION.md 6).
9. "No gray boxes in any finished area" and nothing floats; orange/placeholder blocks are not VEIL (VEIL_Bible.md 2; VISION_KRAKEN "placeholders: False, floating_blocks: False").
10. Locked values do not move: walk 439, sprint 750, glide 600, JumpZ 600, gravity -980, arm 420, pitch -17.0324, capsule 42/96 (VEIL_DIRECTION.md 1 Rule F).

## 10 rules taken from the guide pack (2026-11-09)

Source: `C:\Users\Admin\tools\gamedev-skills` (Apache-2.0, cloned, text only, no installer run). Read:
`router/SKILL.md`, all of `skills/unreal/`, `skills/disciplines/{procedural-gen,level-design,create-game-assets}`,
and the art-direction reference in `create-game-assets/references/art-direction.md` (the workflows README
routes art direction there; there is no separate art-direction workflow skill).

These ten are the ones that bind on the phi world generator and on the Hub matching the painting:

1. **Own the randomness** (procedural-gen). One seeded stream, threaded everywhere; no global RNG in
generation. Ours is seed 1618 in `docs/world_layout.json`, plus per-ring `FRandomStream(Seed + 977(n+1))`
for foliage jitter. Nothing else may call `FMath::Rand`.
2. **Generate data first, render second** (procedural-gen). `world_layout.json` is the world; the generator
is one pass over it; the ISM slots are just the drawing. Changing one number regenerates, never patches.
3. **Validate before the player sees it** (procedural-gen). Reachability/order checks are code, not
opinion: ring order strictly increasing, Spire height equal to the sum of its steps, nothing solid inside
the spawn clearance, claimed total equal to actual instances. `VeilWorldGen::Validate()` now runs on every
`Generate()` and logs `VEIL CHECK pass/fail`.
4. **Determinism must be measured** (procedural-gen). A seed claim is worthless without a fingerprint:
`VeilWorldGen::PlacementHash()` sorts the slot names (TMap order is not promised) and CRCs every
instance transform, logging `VEIL HASH crc=... instances=...`. Two runs, one hash, or the world is not
deterministic.
5. **Metrics come from the character, not from taste** (level-design). Capsule 42/96, AttackRange 144,
ChaseRadius 1597, safe zone 2900, guard spawn 0.3·r = 1213. Every gap, band and spawn radius is sized in
these units; that is why guards spawned outside the stall wall (r 3438) were a design error, not a bug in
steering.
6. **Pace a curve, not a plateau** (level-design). The Hub is rest (market, awnings, crowd) → tension (three
beacon camps at 29 m) → climax (the Spire on the sightline). Combat all the way through numbs the player;
safe zones are the rests, not decoration.
7. **Readability beats walls** (level-design). One landmark must answer "where do I go" from spawn: the
Spire at (0, 10590) unbroken behind the pillars, the crystal peaks at ring 3. Guidance by light and
silhouette, never by an invisible barrier.
8. **Blockout before dressing** (level-design). The generator is the blockout: rings, terraces, spire,
camps. Awnings, glyph panels and foliage are dressing and may never change topology — which is exactly why
343 non-colliding bushes are allowed and a colliding one is not.
9. **Art direction is a contract, not a mood** (create-game-assets, art-direction reference). Roles, not
colours: gold `MI_Modular2` = path and progress, ice `MI_Cliff3` = horizon and mystery, navy = UI and
cover, `M_VEIL_Signal` flat colour = glyph panels only. One light direction (golden hour, low west sun),
value structure readable in greyscale, no per-asset drift.
10. **Nothing floats, nothing is a gray box, and every asset has provenance** (create-game-assets;
VISION_KRAKEN "placeholders: False, floating_blocks: False"). Meshes are engine primitives with named
materials, heights are derived from measured bounds (`ClearHeight`, `Lift`), and credits/licences live in
`Licenses/` with the Kenney and Quaternius entries already recorded.

## 2026-11-09 - audio, melee, kill counter, death loop (measured)

- Kenney "50 RPG sound effects" (CC0) imported to `/Game/VEIL/Audio` (51 ogg -> 54 assets) with
  `UnrealEditor-Cmd -ExecutePythonScript=Saved/import_audio.py`. The game had been silent: the
  `veil_*` paths in `VeilCanon.h` named cues that never existed. Cues now fire and log
  `VEIL AUDIO: cue=...`: footstep01, knifeSlice2, metalClick (beacon), doorClose_1 (death),
  bookOpen (respawn), metalPot2 (guard down).
- Melee: `ExecuteAttack` sweeps chest height (+40 cm) over a 36 degree arc (+/-18) and takes the
  first guard in the arc instead of the first blocking actor. `ProbeAttack` is the harness twin.
- `-VeilCombatProbe` walks the Runner onto the nearest guard and swings; `NotifyGuardDown`
  writes `VEIL KILLS guards_down=n total=m` and the HUD toast "GUARD DOWN - n/m".
- `AVeilCoreEnemy::TakeDamage` logs `VEIL HIT <name> hp=<n>` and knocks the guard back.
- Verified run: `VEIL CHECK pass=5 fail=0`, `VEIL REACH pct=100.0`, `VEIL NAV navdata=1`,
  `VEIL PERF mean_ms=16.74`, 2 guards killed, 4 player deaths and 4 respawns at beacons.

## 2026-11-09 - the four peoples (KRAKEN_STUDY 7, HUB_PAINTING_SPEC)

The Kraken audit listed "races: crystal people, gold people, deep people - ABSENT (two
skeletal bodies from one shop pack)". The Hub now carries the bible's peoples as real castes.

- Rule A tally in the market: 21 human (F8), 8 gold (F6), 5 crystal (F5), 3 deep (F4) =
  37 walkers, logged as VEIL PEOPLES.
- Body pools per people from the CC0 Quaternius import: Gold = BlueSoldier pair, Crystal =
  Casual3 pair, Deep = Casual_Bald/Worker_Male, Human = the whole pack.
- Colour identity: Gold wears M_VEIL_Gold, Crystal M_VEIL_Signal, Deep M_VEIL_Deep on every
  material slot (VEIL CASTE MAT logs the slot count). Humans keep the pack's clothing.
- Silhouette: Gold gets a crown band and sash plate, Crystal three signal shards at the golden
  angle, Deep a tall hood and a chest strip. Props are placed from the dressed body's world
  bounds (VEIL CASTE PROPS), because a fixed local offset was wrong for every phi rung.
- Pace: Gold x1.144, Crystal x0.854, Deep x0.618.
- Named cast: the Seer is Crystal, the Countess is Gold; the Keeper stays human.
- Cost: mean frame 13.04 ms with 43 dressed walkers.

Known gap: the whole-body material override removes the pack's clothing texture, so gold,
crystal and deep read as statues rather than dressed people. The answer is per-caste
materials with detail or robe meshes, not a different flat colour.

## 2026-11-09 - the PC tier (VeilPerf)

- The engine reports the adapter at boot: NVIDIA GeForce RTX 3050, dedicated 6002 MB (the WMI
  AdapterRAM figure of 4095 MB was the driver cap, not the card).
- VeilPerf::ApplyPcTier() runs first in StartPlay and logs VEIL TIER with gpu, vram, tier, lumen,
  vsm and pool. Below 8 GB it sets GI 0, reflections 0, virtual shadow maps 0, CSM 1024,
  streaming pool 1600 MB, volumetric fog grid 16, motion blur 0.
- The lighting pass no longer forces Lumen on: it asks the tier, so the cvar the setup writes
  cannot undo the tier.
- Override for measurement: -VeilTier=low / -VeilTier=high.
- 1280x720 hub shot, 43 dressed walkers: high tier p50 12.98-13.11 ms, low tier p50 11.91-12.85 ms.
  Mean is spiky in this harness (startup spikes), so p50 is the number to trust.

## 2026-11-09 - sharpen and vibrance in the grade

- The NVIDIA app will not attach Dynamic Vibrance or Freestyle to UnrealEditor.exe: the MCP
  filter tool only exposes rtx_dvc, and it answers "no supported game is running". A filter
  that never reaches the backbuffer cannot be seen in a UE screenshot, so it was done in-engine.
- VeilCanon::GRADE_SHARPEN = 0.382, GRADE_VIBRANCE = 1.0618 (midtones only, sky and shadows as shot).
  Applied on the hero lenses and on the unbound post process volume, so a runner-camera shot is
  graded exactly like a hero-camera shot.
- 1280x720 market shot, measured on the PNG: mean saturation 0.3644 -> 0.3871 (+6.2%),
  edge energy 19.635 -> 22.468 (+14.4%), frame cost unchanged (mean 11.85 ms, p50 11.83).

## 2026-11-26 - the frog piece: the Spire on the 0.618 line

Citations: Bible 2 "one great Spire on the horizon, visible from everywhere"; Bible 4 (market
deck); Bible 5 (golden hour). Spire size from docs/PHI_APPLIED.md "Spire | 1200 wide, 5000 tall
at (0, 3500) | authority" - the build had 10,590 x 2,500, which is the ring-2 radius leaking
into the tower, not the bible's number.

- Spire set to the bible's 1,200 wide x 5,000 tall (world_layout.json + generator defaults).
  `VEIL GEN spire: at=(0, 10590) steps=8 height_cm=5000 base_w_cm=1200`.
- Frog lens (camera only): FOV = HERO_FOV 55 x phi = 88.99; Rule D on both axes. The pitch sign
  is part of the division: pitching down puts the level ray on the 0.382 mirror line, so the
  axis looks up 7.43 deg and the horizon lands on 0.618. The yaw is solved on the projection so
  the tower's centre lands on u = 0.618 (3 Newton steps).
- Lens A, on the Spire's own axis a F18 (2,584 cm) south, was measured and rejected: stall prop
  2 sits at bearing 88.0 deg and phi pillar 2 at 87.5 deg on the tower's 6.3 deg cone and walls
  it off. Lens B - phi^-5 of the parapet (365 cm) south of the plaza centre, inside the 520 cm
  clear radius - holds the whole tower.
- Measured: `VEIL FROG: fov=88.99 pitch=7.43 base_u=0.6180 base_v=0.6006 top_u=0.6112
  top_v=0.1964 frames_tall=0.404 dist_cm=10955 spire_h=5000 horizon_v=0.6180`.
  `VEIL FROG CHECK: u=PASS horizon=PASS fit=PASS`.
- Re-frame (same shot, nothing added): at 0,-365 stall prop 19 still crossed the tower's own
  body. The lens is now +phi^-6 of the parapet (225 cm) east by -phi^-5 (365 cm) south, where no
  prop, pillar or lantern crosses the tower, so the Spire is the only gold form in frame.
- Palette, measured on docs/proof/frog_spire.png (HSV, threshold S>0.18): gold 39.49%,
  teal 22.08%, blue 7.39%, navy 0.73%, neutral stone 30.00%, off-palette 1.04%; mean saturation
  0.3427, mean value 0.6631.
- The HUD is hidden for this one frame: the VIT bar and the minimap ring are HUD, not
  composition, and they sit outside the reserved palette.

## 2026-11-26 - pass 1 golden frame, pass 2 colour and material

- PASS 1: the spawn is locked at the bible's authority coordinate (DIRECTION 3, Runner spawn
  0,0,100) and the reference camera is locked to it -- `VEIL FRAME: spawn=(0,0,100) locked
  ref_cam=(0,0,421) yaw=77.02 pitch=7.43`. 421 = PlateauZ 246 + the 175 cm eye.
- PASS 1: the Spire's cone from the spawn is reserved in the generator. A stall post inside it
  is built at the Rule D short split (PillarH x 0.618 = 494 cm) instead of full height, so it is
  flattened, never deleted: `VEIL FRAME: spire_cone_flattened=1 posts`, total still 725,
  `VEIL CHECK pass=5 fail=0 instances=725`. The flatten condition is the tower's own 1,200 cm
  width at 10,590 cm plus the post width and the 61 cm jitter, on |x| against the sightline.
- PASS 2: every M_VEIL_Stone call site (11 across VeilWorldGen, VeilHubGameMode, VeilWorld) is
  `/Game/VEIL/CC0/Materials/M_CC0_Sandstone`, with warm kit stone MI_Cliff2 as the fallback so a
  missing asset can never fall back to engine grey. Receipt: `VEIL MATERIAL: stone=M_CC0_Sandstone
  gold=MI_Modular2 navy=M_VEIL_Deep`.
- PASS 2: blue emissive is off in the market. The seven phi-pillar glyph panels and the generator's
  GenGlyphs slot are now gold relief. #3FA9FF remains only on the crystal peaks, the beacon signal
  tech and the crystal caste's own shard.
- Palette on the PNG (HSV, S>0.18): gold 45.87%, teal 23.63%, blue 12.10%, navy 2.33%, neutral
  17.58%, off-palette 0.83%; in-palette 81.60%, mean saturation 0.3919 (was 0.3427 before the
  sandstone). Split at the horizon: 24.91% of the upper half is neutral (sky and cloud) but only
  5.78% of the lower half is -- 2.21% of the frame is neutral below the horizon, so the built
  world is not grey.

## M1 / M2 — the save is the loop (2026-11-26, seat719)

- M1 (Bible 79, 85): slot `VEIL_PHI` now carries what the HUD shows -- Level, XP, Gold, Deaths and
  the indices of the lit beacons (`LitBeaconIdx`, indices because actor names are per-session) --
  in `UVeilSaveGamePhi::SaveFull`, version 2. `LoadIfPresent()` applies them at boot, so quit and
  reload shows the same values.
- M2 (Bible 86): `SyncBeacons()` owns the beacon reward. Each beacon the player rebuilds pays
  F11 = 89 gold and 144 XP, toasts, sounds, autosaves and posts to the signal bus.
  `OnRunnerDied` counts deaths on the edge (damage landing on a corpse does not add another),
  bound to the runner's own `OnPlayerDeath`.
- Harness: `-VeilSeat719=save` lights all three beacons through `IVeilInteractablePhi::Execute_Interact`,
  takes one lethal hit through `TakeDamage`, waits for the runner's own respawn, autosaves and writes
  `Saved/seat719_out.txt`; `-VeilSeat719=load` reloads the slot, applies it and appends the after
  reading. Receipt: `SAVE RESULT True` / `BEFORE level=2 xp=55 gold=267 beacons=3 deaths=1` /
  `AFTER level=2 xp=55 gold=267 beacons=3 deaths=1` / `MATCH True`.
- Frames: `docs/proof/seat719_save.png` (the loop's end, three beacons rebuilt) and
  `docs/proof/seat719_load.png` (the same state after a full quit and relaunch).

## M3 — the HUD per section 3 (2026-11-26)

- One frame, section 3's vocabulary: navy #0E1424 glass at 85%, 1.5 px gold #C9A24A, a long slant on
  the top-right corner and a chevron crest on the top edge (`Panel()`), used by the vitals pane, the
  quest panel, the dialogue box, the prompt, the combat pill and settings.
- Vitals now ride on a glass pane instead of floating on sandstone: VIT red #D9343A with its own
  numbers on the bar, STM green #2FB36A tagged ATP (the mitochondrial rule -- ATP is what fills it),
  SIG blue #3FA9FF. Both bars scale from the settings sliders.
- LV plate with the phi-star diamond and the XP sliver ticked at 1/phi^2, 1/phi and 1; the purse sits
  next to it, fed by the save (`HubHUD->Gold = PlayerGold`).
- Round minimap: gold ring, N/E/S/W ticks, compass needle, heading, guard tally, phi rings and a pause
  button inside the ring.
- Dialogue: speaker plate, typewriter at 144 chars/s, two consequence chips on branch lines.
- Damage numbers: gold normal, blue signal, red crit, float 60 -> 215 cm and fade over 0.9 s, pushed
  from the blow itself (`AVeilEnemy::TakeDamage` -> `AVeilHubHUD::PushDamage`).
- Settings screen (F1): HUD Atmosphere Stance (Active / Minimal / Off), Vitality and Stamina HUD scale
  sliders, HUD scale on [ ] and , . , H for stance, F10 for the debug readout -- debug text is never
  on screen in normal play.
- Old HUD deleted: `VeilHarkHUD.h/.cpp` removed; `VeilHarkProof` and the core path now use this HUD
  (`AVeilHubHUD::Say` / `ShowQuest`).
- Proof frames: `docs/proof/VEIL_m3_hud.png` (Play, Active stance, dialogue with choices, quest panel)
  and `docs/proof/VEIL_m3_settings.png` (the settings screen). M1/M2 re-run after the change:
  `SAVE RESULT True`, BEFORE and AFTER identical, MATCH True.

## M4 — the pillar law passes, the frame does not (2026-11-26, second pass)

- The one law now grows the pillar population instead of scattering it: a cluster stands on its own
  GoldenLoc address and its members are GoldenLoc points at phi^-4 of the spacing. Receipt:
  `VEIL PILLAR CLUSTER: address=1 members=1 r_cm=610` / `2 members=2 r_cm=863` / `3 members=3
  r_cm=1057` / `5 members=5 r_cm=1364`, `VEIL PILLARS: count=11 groups=1,2,3,5 ... top_cm=2664`.
- Stalls are clamped to a human counter (89-233 cm): one tall size rung could scale a single navy
  canvas across the whole market before.
- Golden Frame holds: `VEIL FROG CHECK u=PASS horizon=PASS fit=PASS`, base_u=0.6180,
  horizon_v=0.6180. Perf `p50=11.67 ms` (86 fps), p95=13.22, p99=13.89 on the RTX 3050.
- M4 is NOT passed. The frame fails Tyree's bar on sight: the pillars render with a brick/masonry
  texture instead of gold; the Spire is not legible in the frog frame at all; the crowd are pale
  untextured mannequins; navy awning slabs still float over thin posts at the wrong scale; the
  crystal cones read as plastic right inside the market. `docs/proof/hub_frog_spire.png`,
  `docs/proof/hub_market_m4.png`.

## M4 — the materials were never rendering (2026-11-26, third pass)

- Found, and this invalidates the earlier material claim: every VEIL and CC0 material was missing the
  `InstancedStaticMeshes` usage flag, so the generator's instanced surfaces were drawn with the
  engine's default material and the log said so on every boot --
  `LogMaterial: Warning: Material M_CC0_Sandstone missing usage flag InstancedStaticMeshes! Default
  Material will be used in game.` That is why the market read as brick, plank and plastic, and why
  the PASS 2 palette numbers were measuring the default material, not the world's. PASS 2's palette
  claim is withdrawn until it is re-measured on real materials.
- Fixed the usage flags on M_VEIL_Deep, M_VEIL_Signal, M_VEIL_Stone, M_VEIL_Gold, M_VEIL_Gold2,
  M_VEIL_GoldPBR, M_CC0_Sandstone, MI_Cliff2, MI_Cliff3 through the editor's Python
  (`Saved/useflags_all.py`). The boot log now has zero `missing usage flag` warnings (was 5).
- Authored the surface the painting asks for and the kit does not have: `M_VEIL_GoldPBR`
  (base 0.52/0.35/0.10 linear, metallic 0.7, roughness 0.3, emissive 0.02) built by
  `Saved/make_gold.py`, with the instanced usage flag set. Every kit material named "Gold"
  (MI_Modular2, MI_PropsV2_4, M_VEIL_Gold) renders as brick or timber; none of them is gold.
- Re-measured palette on the real-material frame: gold 53.43%, teal 19.85%, blue 11.30%,
  neutral 14.67%, off-palette 0.76% -- 99.24% in palette. Golden Frame holds
  (`u=PASS horizon=PASS fit=PASS`), perf `p50=13.71 ms` (73 fps), p95 15.99, p99 17.79 -- the real
  materials cost ~2 ms more than the default material did.
- Still failing the bar, unchanged by the material fix: the Spire is not legible in its own frame;
  the MI_Cliff3 ice cones read as plastic and own the skyline; the crowd are placeholder mannequins;
  the awnings float over thin posts; `M_VEIL_Deep` renders pale blue, not navy #0E1424.
- M4 is NOT passed. `docs/proof/hub_frog_spire.png`, `docs/proof/hub_market_m4.png`.

## M4 — the scale correction: the world stops being a diorama (2026-11-26, fourth pass)

The reason the frog piece could never pass was not the art on the frame. The five regions were
concentric annuli inside 27,720 cm of the hub: the "Spire" was a 50 m tower 106 m away, the
"Omni District" was 21 cones 170 m away, and the entire world was 277 m across. Every number that
mattered was a centimetre value doing a metre's job, and no material or lens can make a courtyard
read as an open world.

- The ladder is now read in metres (`region_distance_m` in `docs/world_layout.json`). Regions sit
  on it at 4,045 / 6,545 / 10,590 / 17,135 m, each on its own authority anchor's bearing (Nest NW,
  Spire due north, Omni east, Deep south). Ring 0, the hub, stays at 0,0.
- `LocalRadiusCm(n)` keeps each place's own footprint: the hub is built across 40 m, a region
  across the same rung in decimetres, so a region is a region and not a courtyard.
- The Spire is DIRECTION's 1,597 m (`spire_height_cm 159700`), 384 m base, eight steps at phi
  decay: at 6,545 m it subtends 13.8 deg, and it is visible from the spawn lens.
- Fog was tuned for a 300 m world (8e-6/cm = e^-8.5 at 6.5 km, a wall). It is 5e-7/cm now, so the
  tower arrives at 72% transmission instead of 0%.
- The gold surface had a colour-space defect: #C9A24A's sRGB numbers were written into a linear
  constant (0.788/0.635/0.290 = pale straw, not gold). Corrected to (0.573/0.351/0.066), metallic
  0.15 -- 0.35 metallic with only sky to reflect turned the tower into a white needle. The Spire
  alone gets `M_VEIL_SpireGold` with emissive gold, because the bible gives the tower its own
  light and 6.5 km of aerial perspective eats any lit surface.
- Verified: `VEIL CHECK pass=5 fail=0 instances=725`, `VEIL FROG CHECK u=PASS horizon=PASS
  fit=PASS`, `VEIL GEN spire at=(0, 654500) height_cm=159700`, missing usage flags 0.
- Still not the bar: the tower reads at 13.8 deg but is bleached by aerial perspective and still
  small in frame; the crowd is mannequins; awnings float; there is no terrain between the market
  and the tower, so the world has distance but not yet land. Frames: `docs/proof/hub_frog_spire.png`,
  `docs/proof/hub_bible_spawn.png`, log `docs/proof/scale4.log`.

## Rulings 5 and 6 — applied and measured (2026-11-26)

Ruling 5 (Spire). The built Spire stands; the bible's coordinate is superseded; the docs are derived.
- `docs/world_layout.json` is `spire_height_cm 5000`, `spire_base_width_cm 1200`; the generator prints
  `VEIL GEN spire: at=(0, 10590) steps=8 height_cm=5000 base_w_cm=1200`.
- The kilometre-ladder experiment from the previous pass is withdrawn: it moved the Spire to
  (0, 654500) and contradicted the ruling. `RingRadiusCm` is the only ladder again, and Ring 2's
  outer radius (4045 x phi^2) is the tower's place.
- `docs/VEIL_Bible.md` carries a note under the authority coordinates: Ruling 5, (0, 10590) stands,
  the paragraph is recorded and not rebuilt.
- `docs/PHI_APPLIED.md` is now generated: `Scripts/gen_phi_applied.py` writes it from
  `world_layout.json` plus the run log, every row carrying its source key. The old 1200/5000/3500
  row is gone because nothing builds 3500.
- `AVeilHubGameMode::BuildSpire()` — the hand-built 15,400 cm tower at (0, 9000) that `StartPlay`
  never called — is deleted, with its declaration. One Spire, one answer.

Ruling 6 (census). 37 stays; the tally is derived; failures are counted.
- `CasteCount[4]` is now `FIB[6], FIB[4], FIB[3], FIB[2]` = 21 / 8 / 5 / 3. No `+ 16`, no 21/29/34
  thresholds: one loop deals the castes in ladder order.
- `SpawnFailed` is counted and each failure is logged with seat, caste and coordinate.
- The drift itself is fixed, not just reported: the walkers spawn with `AlwaysSpawn`, so a capsule
  overlapping a stall base is still a walker. Run evidence:
  `VEIL PEOPLES: human=21 gold=8 crystal=5 deep=3 walkers=37 expected=37 failed=0`
  `VEIL PEOPLES CHECK: PASS walkers_plus_failed=37 expected=37`.
- Before the fix, the same code named the two failures: `seat=2 caste=human at=(223, 1299)` and
  `seat=34 caste=deep at=(-1757, 622)`.

M4 direction, started:
- The plaza now has a real reflection source: `ASphereReflectionCapture` at the market centre,
  `InfluenceRadius 2618` (1000 x phi^2), logged `VEIL REFLECTION: plaza capture radius_cm=2618`.
  Gold stays metallic 0.35 with roughness 0.42 — the metal is not faked by roughness.
- The gold constant was also corrected out of sRGB-in-a-linear-slot to true linear #C9A24A.
- Still open: the Spire reads as a stepped gold tower in the frog frame but a foreground pillar
  crosses its silhouette, so "one legible Spire" is not met; and the Omni District is still 21
  crystal peaks, not a city. M4 is NOT passed.

## M4 — the Spire sightline is geometry now (2026-11-26)

The generator reserved the tower's cone with a remembered slope (`R * (600.f / 10590.f)`) and only
asked the *pillar* whether it was inside it. The stall's canvas is 1,165 cm wide and hangs 260 cm
in front of its post, so the thing crossing the Spire was rarely the post the rule was testing.

- `OccludesSpire(AlongY, Lateral, PropTopZ, HalfW)` asks the question properly: distance to the
  tower is `RingRadiusCm(2)`, the base is `TerraceTop(2)`, the half width is `SpireBaseW * 0.5`,
  the eye is `PlateauZ + 175`, all read from the build. Height first (a prop whose top is under the
  sightline to the tower's base cannot cross it), then the wedge half width at that distance.
- `ClearSpireSight()` acts on it: the prop keeps its distance and its place in the market and moves
  sideways out of the wedge. Nothing is deleted, nothing is shortened -- the old flatten cut posts
  to phi^-1, which is how the corridor stayed "clear" on paper while a canopy crossed it in frame.
- Applied to every prop that can stand in the corridor: hub props (footprint = max(pillar, stall
  canvas)), houses, lanterns, and the gold POI obelisks.
- Run: `VEIL SPIRE SIGHT: cleared=5 props`, five named moves in the log, `spire_cone_flattened=0`,
  `VEIL CHECK pass=5 fail=0`, census `37/37 failed=0`, `VEIL FROG CHECK u=PASS horizon=PASS fit=PASS`.
  `docs/proof/hub_frog_spire.png` now shows the eight-step gold tower standing alone on the 0.618
  line with nothing in front of it.
- Still open for M4: the crystal peaks of the "Omni District" are 171 m tall at 171 m and out-tower
  the 50 m Spire (the city layer, real work), and gold still reads as a flat ochre rather than
  metal at distance. M4 is NOT passed.

## M4 - the Omni District is a city (2026-11-26)

- The ring was 21 crystal peaks, and a peak field is not a city: `AVeilWorldGen::PlacePeaks()` is
  gone and `PlaceCity()` builds a polar street grid instead -- 3 bands of blocks round ring 3
  (r 10,590 to 17,135), 21 spokes of street between them, alternate bands turned half a spoke so
  the avenues stagger.
- Every block: a stone mass, a stack of lit blue panes on the face that looks at the market (one
  pane per floor, phi^-1 window to phi^-2 wall), and a lit crown band along the roof edge. Tall
  blocks step back phi^-1 on top.
- Streets are ground, not only a gap: a navy ring road 833 cm wide at every band edge, 84 tiles
  closed by phi^-2 so the ring has no seams. (Pass 1 stood the city on bare sand.)
- Light pillars: 20 crystal columns, one per spoke on the rim road, and one real blue point light
  every third pillar -- the blue the district throws on its own ground, not a glow painted on.
- The Spire keeps its plaza: blocks within 1,942 cm (1,200 x phi) of (0, 10,590) are skipped (2 of
  them). New check `city_under_spire`: no block above `city_height_cap` 0.72 x 5,000 = 3,600 cm, so
  the tower is the tallest thing in every frame.
- Run: `VEIL CITY: blocks=103 tallest_cm=3600 spire_cm=5000 cap=0.72 light_pillars=20
  plaza_skipped=2 bands=3 spokes=21`; `VEIL CITY ROADS: tiles=84 ring_roads=4 width_cm=833`;
  `VEIL CHECK pass=6 fail=0 instances=1454`; `VEIL HASH crc=FFC6E377 instances=1454 slots=30`;
  census `37/37 failed=0`; `VEIL FROG CHECK u=PASS horizon=PASS fit=PASS`.
- Frames: `docs/proof/hub_omni_city.png` (the district from its own approach) and
  `docs/proof/hub_frog_spire.png` (the same tower with the city behind it). Log:
  `docs/proof/city1.log`.
- Still open: every block is a massing box -- no doors, no interiors, no street props, no crowd in
  the district, no factions or heists or Countess; the sandstone UV is stretched so a tower reads
  as a wooden crate at 100 m; the panes use the ice texture and read as glacier slabs close up; the
  horizon lens `hub_bible_horizon` is now walled by a block and needs repointing or retiring; and
  the district is 65 m deep in world units because Ruling 5 keeps the 50 m Spire -- "city sprawl"
  at the direction's 1,597 m needs that scale question ruled, not built around.

## M4 - the district at street level (Ruling 7)

Read Ruling 7 in `buffy-handoff/phi-buffy-rulings.md` at commit `7cd4e20`: the 65 m district stands,
the Spire stays 50 m, the direction's 1,597 m sprawl is a vision number and not a build target, and
a bigger world is a new build (M7+), not a rescale. M4 continued on the open items, in order.

- **Street level.** 98 blocks each carry a doorway: two jamb stones 30 cm deep, a lintel, a lit
  aperture between them, a gold sign board over it (`city_sign_cm` 89 x phi), and a navy awning on
  every second one (50 of 98). 44 lamp posts stand on the ring roads, gold post under a glass head.
  No interiors: the blocks are massing, so a door is an aperture on a wall, not a room.
- **The district's own crowd.** 13 walkers (`city_crowd`) on the causeway, spaced 400 cm apart, half
  on each side of the centre line, each walking phi^2 of the spacing either way. The market's 37 are
  untouched and still read `walkers=37 expected=37 failed=0`.
- **Two wrong surfaces, each its own material.** `M_VEIL_CityStone` (sandstone at phi^2 tiling, so a
  9 m face is 3.4 m courses instead of one stretched texture -- it read as a wooden crate before),
  `M_VEIL_CityGlass` (navy glass with a #3FA9FF emissive at 0.42 -- the panes were `MI_Cliff3`, the
  glacier ice the crystals use), and `M_VEIL_CityRoad` (dark asphalt; the streets were `M_VEIL_Deep`,
  which carries a blue emissive and made the ground glow ice-blue). Authored by
  `Scripts/make_city_materials.py` and `Scripts/tune_city_materials.py`.
- **Gold at distance.** The tower got its own probe: a sphere capture at mid-tower (z 3,123),
  influence 3,090 cm = phi^-1 of the height, because the plaza's 2,618 cm capture never reaches
  (0, 10,590). Three separate tower-gold materials were authored and each moved the tower *off* gold
  (metallic 0.70 mirrored the sky and read grey; the kit's gold texture is a grey mask; a brighter
  gold constant blew out at this exposure). The tower is back on the palette's `M_VEIL_GoldPBR` with
  the probe supplying the reflection, and the material is named in the log so it cannot be guessed
  again: `VEIL GEN spire: ... material=M_VEIL_GoldPBR`.
- **`hub_bible_horizon` repointed.** It aimed at a crystal ring Ruling 7 removed, then at the
  district's rim from 8,324 cm -- where the nearest band of 36 m blocks filled the frame. It now
  stands on the Deep ring's terrace (27,725 cm) at the sun's bearing (70 deg) and looks back: the
  district's skyline as the horizon, with the gold tower over the roofs. From 175 cm there the 36 m
  rim blocks geometrically hide the rest of the 50 m tower, which is why the tower is proved from the
  market and district frames instead.
- **Walkable.** A causeway runs from the market's rim (2,599 cm) to the district's rim (17,135 cm):
  33 tiles, 433 cm wide, flat on each terrace and ramped at every lip with run = rise x phi^2, so
  20.9 deg -- inside a capsule's 45 deg limit. Houses, POIs, rocks, foliage and 5 city blocks move
  aside rather than being deleted. New check 7 `city_walkable`.
- **Run:** `VEIL CITY: blocks=98 tallest_cm=3600 spire_cm=5000 cap=0.72 light_pillars=19
  plaza_skipped=2 bands=3 spokes=21 road_skipped=5`; `VEIL CITY ROADS: tiles=84 ring_roads=4
  width_cm=833`; `VEIL CITY DETAIL: doors=98 signs=98 awnings=50 street_props=44`;
  `VEIL CAUSEWAY: tiles=33 length_cm=14536 width_cm=433 start_cm=2599 end_z_cm=1040
  end_terrace_cm=1000 max_slope_deg=20.9 bearing_deg=137.508`; `VEIL CHECK pass=7 fail=0
  instances=2053`; `VEIL HASH crc=E231685A instances=2053 slots=39`; `VEIL CITY CROWD: walkers=13
  expected=13 failed=0`; census `37/37 failed=0`; `VEIL FROG CHECK u=PASS horizon=PASS fit=PASS`.
- **Frames:** `docs/proof/hub_omni_city.png` (the district from its approach),
  `docs/proof/hub_omni_street.png` (the causeway at eye height: crowd, doors, signs, lamps, gold
  pillars), `docs/proof/hub_bible_horizon.png` (the district skyline from the Deep terrace). Log:
  `docs/proof/city2.log`.
- **Still open:** no enterable interiors (doors are apertures on massing); the district's inward faces
  still read charcoal -- a golden-hour sun from 70 deg leaves a ring city's inner faces unlit and only
  the 19 pillar lamps answer it; the nearest walkers crowd the street lens; the tower's gold is still
  the market's flat gold, now with a probe behind it; no factions, heists or Countess in the district;
  and nothing was walked (no PIE collision test, no frame-rate measurement).

### M4 -- the district has rooms (2026-11-26)

Code, not a document: every district block's ground floor is hollow. A shell of stones -- two front
piers either side of the opening, a header over it, a back, two sides and a ceiling -- leaves a
610 cm room inside, with its own dark stone floor 8 cm proud of the street and an emissive ceiling
panel that Lumen bounces, and the mass of the building starts on that ceiling. Blocks too short or
too narrow to hold the room (12 of 98) keep the old lit aperture and are counted as solid instead of
pretending.

- `VEIL CITY INTERIORS: rooms=86 solid=12 room_h_cm=610 door_w_cm=233 door_h_cm=377`
- `VEIL CHECK city_interiors PASS blocks=98 rooms=86 solid=12` (check 8, new)
- `VEIL CHECK pass=8 fail=0 instances=2788`, `VEIL HASH crc=AC518031 slots=41`
- `VEIL DOOR FRAME: door=(-11647,5753,1189) eye=(-11372,5617,1229) stand_cm=307 rooms=86`
- Build `Result: Succeeded`; proof `docs/proof/rooms.log`, `docs/proof/hub_omni_door.png`.

Not done: the room in the frame is dim (the ceiling light sits above the lens line), rooms are empty
of props, and nothing has walked into one yet.

### M4 frame items: the city lights itself, the street clears, gold comes back (2026-11-26)

**Cause, read from the log and not guessed.** `VEIL LIGHT: gi=0 reflect=2 sky=2.618 lamps=22
tier=low`. This machine resolves `r.DynamicGlobalIlluminationMethod` to 0 and had
`r.ReflectionMethod` at 0 as well: no bounce, no reflections. That single fact explains both open
frame faults -- shade had only the sky, and every gold surface had nothing to reflect, which is why
three bespoke gold materials were tried against it and all three failed.

- Sky: real-time capture, phi^2 intensity, warm tint (`VEIL LIGHT: ... sky=2.618`).
- Bounce: a shadowless warm light from the district's own bearing stands in for the GI this tier
  does not compute (`VEIL LIGHT: bounce=2.62`).
- Street: 22 real point lights on the district's own lamp heads, intensity and reach from head
  height (`VEIL CITY LIGHT: street_lamps=22 of 44 heads`).
- Reflections: screen-space restored on the low tier, so gold is metal again, not grey
  (`reflect=2`).
- Walkers: they now walk loops in front of the street lens that cannot reach it
  (`VEIL CITY CROWD: walkers=13 nearest_to_lens_cm=815`).
- Grade: sun 11 -> 7.85 (phi^2 * 3) and exposure max 1.6 -> 1.45, because a lit sky at 11 blew the
  terrace sand to white.

`VEIL CHECK pass=8 fail=0 instances=2788`, `VEIL HASH crc=AC518031 slots=41`, build
`Result: Succeeded`. Proof: `docs/proof/frame.log`, `hub_omni_city.png`, `hub_omni_street.png`,
`hub_bible_horizon.png`.

**Still open:** the inward faces are lifted out of charcoal but remain the darkest read in the frame;
the frames suggest `M_VEIL_CityStone` is not the sandstone the docs claim (sunlit walls read grey, not
tan), so the next pass is the material's base colour, not another light.
