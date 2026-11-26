# THE KRAKEN BIBLE, STUDIED WHOLE

Read in full, then measured against the live source (`Source/VEIL`, `Content/VEIL`,
`Config/`). Nothing here is quoted from memory; every status below was produced by a
grep or a log line on 2026-11-08.

## What the bible is

| file | lines | what it holds |
|---|---|---|
| `docs/canon/VISION_KRAKEN.orig.txt` | 315 | the original: prose, five Python files, and a boolean `WHOLE: PASS` |
| `docs/VISION_KRAKEN.txt` | 336 | the same file with every boolean verifier replaced by a measured-gate call |
| `docs/VISION_SCOPE.md` | — | regions, authority anchors, 60-100 h target, milestone table |
| `docs/PHI_APPLIED.md` | — | the number table and where "the +1" (engine craft) sits |
| `docs/PHI_AAA_REVERSE_ENGINEERED.md` | — | the reading: generator true, assertion unfalsifiable |

Authority order used below: **Rule 0** (phi states it, Unreal carries it out) and the
Rule F locked values in `VeilCanon.h` first; then the bible's anchors and prohibitions;
then the prose. Where two numbers collide, the collision is listed in §11 rather than
quietly resolved.

## 1. The law

- `phi = 1.618033988749895`, `phi^2 = phi + 1` within 1e-6 on every boot — **MET**
  (`VEIL PHI PASS residual=0.000000000`).
- Golden angle `137.5077640500378` — **MET** (`VeilCanon::GOLDEN_ANGLE`,
  `VeilPhi::GoldenLoc`).
- `seed 1618`, one seed for every procedural system — **MET** (`VeilCanon::SEED`, used by
  `FRandomStream` and the generator).
- `GoldenLoc(i)`: angle `i x golden`, radius `Spacing x sqrt(i)` — **MET**
  (`VeilPhi::GoldenLoc`, used for stalls, lanterns, crowd, pillars).
- No random — **CONFLICT**: the generator adds `Rng.FRandRange(-Jitter, Jitter)` with
  `Jitter = 61` cm and `-9..9` degrees of yaw to every placement (`VeilWorld.cpp:395,424`).
  Seeded is not the same as golden-angle-only.

## 2. The eleven authority anchors

| anchor | bible | code | status |
|---|---|---|---|
| Runner spawn | 0, 0, 100 | `VeilCanon::SPAWN`, placed at `PlateauZ + 100` | **MET** |
| Keeper Sera Vahl | 900, 0, 0 | `VeilCanon::KEEPER`, `AVeilKeeperNPC` | **MET** |
| Seer | -450, 780, 0 | `VeilCanon::SEER`, dressed body | **MET** |
| Countess | 809, -309, 64.7 | `VeilCanon::COUNTESS` + gold dais | **MET** |
| Beacon 1 | 2800, 500, 0 | `VeilCanon::BEACONS[0]` | **MET** |
| Beacon 2 | -2000, -900, 0 | `VeilCanon::BEACONS[1]` | **MET** |
| Beacon 3 | 600, -2100, 0 | `VeilCanon::BEACONS[2]` | **MET** |
| Cartographer | -4150, -780, 0 | none | **ABSENT** |
| Cache | 0, -6200, 0 | none | **ABSENT** |
| Vault | 0, 0, -80.9 | none (`80.9` appears only in a deck-scale comment) | **ABSENT** |
| Golden Spire | 0, 3500, 500 | `BuildSpire` places a gold cylinder at **14000, -9000, 0** | **CONFLICT** |

Seven of eleven are exact. The Spire is 15.2 km east and 12.5 km south of its anchor:
it is a horizon prop, not the anchor. Its scale (9, 9, 150) on a 100 cm cylinder is
1,400 cm of shaft, against the bible's 5,000 cm tall / 1,200 wide.

## 3. The five zones

| zone | bible centre | built | status |
|---|---|---|---|
| Hub | 0, 0 | 8,090 cm plateau, 7 pillars, 8 stalls, 7 lanterns, 21 crowd, 3 beacons | **PARTIAL** |
| Nest | -450, 780 | only a comment ("facing the Nest") | **ABSENT** |
| Spire | 0, 3500 | a distant cylinder, no zone | **ABSENT** |
| Omni District | 2800, 500 | none | **ABSENT** |
| Megastructure Deep | 0, -5000 | none | **ABSENT** |

One of five. The one that exists is the base case the bible tells you to book first, and
even it does not match its own counts (§4).

## 4. Counts against the JSON — the Hub atom

`world_layout.json` (inside the bible) fixes the Hub at **12 phi pillars + 8 market houses
+ 5 blue crystals at (2800, 500) = 25 placements**. Measured in the build:
`pillars=7 stalls=8 lanterns=7 instances=62 crowd=21 beacons=3`, total placements 25 by
coincidence, composition wrong: 7 ≠ 12 pillars, 0 ≠ 5 blue crystals.

So "counts vs JSON" — the bible's own acceptance test for the Hub — **fails**, and it
fails on the pillar count, which is the one number the Hub's silhouette depends on.

## 5. Physics

| rule | bible | code | status |
|---|---|---|---|
| gravity | -980 | `VeilCanon::GRAVITY` | **MET** |
| walk | 439 | measured 439.0 cm/s over 10 s (`VEIL M2 WALK`) | **MET** |
| sprint | 750 | measured 750.0 cm/s over 10 s (`VEIL M2 SPRINT`) | **MET** |
| glide | 600, gold wings, drop -50 | `GLIDE = 600` constant; no glide behaviour, no wings geometry | **PARTIAL** |
| jump height | 185 | `JUMP = 600` (locked) | **CONFLICT** |
| mount speed | 900, grounded | no mount system | **ABSENT** |
| golden pitch | 17.0324 | `CAM_PITCH`, applied to the spring arm | **MET** |
| forward 0.9561 / pitch 0.2929 | component form of the pitch | not present as constants | **ABSENT** |
| enemies hold their post, never follow | follow false, orbit false | `AVeilEnemy` and `AVeilCoreEnemy` both `MoveToActor(Player)` inside 1597 cm | **CONFLICT** |
| enemy attack range | 300 | 144 | **CONFLICT** |
| capsule + floor collision, no clip | — | 42 / 96 capsule, `fell=0` over 10 s | **MET** |

## 6. Interface

- **Dialogue E only, never Space, never Enter** — **ABSENT in the live pawn.** The
  default pawn (`AVeilRunnerPhi`) binds Move, Look, Jump, Sprint and nothing else; `E` is
  bound only in the unused `AVeilRunnerCharacter`. `DefaultInput.ini` maps Jump to
  SpaceBar, which is allowed (it is not dialogue) but there is no E interaction to fall
  back on. The HUD draws an `E` keycap (`VeilHubHUD.cpp:457`) that nothing answers.
- **ONE prompt, mic symbol, click mic or type to One** — **ABSENT**: zero hits for
  mic/One in the source. There is a typewriter dialogue box instead.
- **Gold ribbon HUD, no boring text** — **PARTIAL**: the palette and ribbon geometry are
  real (`GOLD`, `GOLD_TEXT`, navy panels, φ-star badge, disc minimap, quest pips).
- **Objective 420 x 110 top-left** — **ABSENT** as a box; the quest panel is 372 x 136.
- **Minimap 180 x 180 at 70%** — **CONFLICT**: the disc is 124 cm across (radius 62).
- **Quest line 480 px** — **CONFLICT**: `QUEST_W = 372`.
- **Icon: gold V, golden spiral, feather wings, navy starfield tile** — **ABSENT**
  (`Content/VEIL/UI` holds one widget, no icon).

## 7. Cast and variety

| item | status |
|---|---|
| Keeper Sera Vahl, own post | **MET** (900, 0), but her line is not the bible's "Stand still. I count the living first." — the opening is a different five-line script |
| Seer, own post | **MET** |
| Countess, own post | **MET** |
| Cartographer | **ABSENT** |
| races: crystal people, gold people, deep people | **ABSENT** (two skeletal bodies from one shop pack) |
| enemy variety: Crystal Guardian / Gold Warrior / Deep Dweller / Beacon Shadow | **ABSENT in the live module**; those four names exist only in the retired `VEILOMNI` module. Live enemies are Shade / Brute / Warden |
| mounts: Gold Wing / Crystal / Market / Deep | **ABSENT** |

## 8. Menus and items

`ARMOR`, `BELT`, `BOOTS`, `GLOVES`, `WARDEN` — **ABSENT**. Zero hits for inventory,
equipment or item UI. The ten `WARDEN` hits are the enemy class `AVeilWarden`. There is
no menu system of any kind in the live module.

## 9. The prohibitions

| prohibition | status |
|---|---|
| no placeholders | **PARTIAL** — the 8,090 plateau, the Spire and the beacon pillars are `/Engine/BasicShapes` primitives; the market counters/awnings/columns are the Sicka kit |
| nothing floats | **MET** for props (ground traces, `missed=0`), **MET** for the Runner (`fell=0`) |
| no duplicates | **UNMEASURED** — no audit compares instances for repetition |
| grounded everything | **MET** |
| vision only, no fake claims | **MET** since the verifier patch: the file reports gate failures instead of printing PASS |

## 10. Build order and proof

The bible's order is stated and unambiguous: **Hub first, one zone at a time, counts vs
JSON, play plus screenshot proof, no random, seed 1618.** The project's record follows it
(M1 mode/phi, M2 gait, both with logs and PNGs) but has not yet passed the Hub's own
counts test, which is the gate the bible puts at the door.

## 11. Conflicts that need a ruling (not silently resolved)

| # | two numbers | where each lives | cost of choosing wrong |
|---|---|---|---|
| 1 | jump 185 vs jump 600 | bible / `VeilCanon::JUMP` (Rule F) | changes every gap and ledge in the world |
| 2 | attack range 300 vs 144 | bible / `AVeilEnemy::AttackRange` | 300 makes the market a killing floor; 144 makes guards inert |
| 3 | VIT 4200, STM 180, LV 72 vs 377 / 89 / 89 cap | bible HUD sample / Rule F pools | the whole bar ladder is built on 377 |
| 4 | HUD 420x110, 180x180, 480 px vs 372x136, 124, 372 | bible / canon | re-laying out the HUD |
| 5 | 12 pillars + 5 crystals vs 7 pillars + 0 crystals | bible JSON / generator | the Hub silhouette and its count gate |
| 6 | enemies follow within 1597 cm vs hold post | code / bible | the market's danger model |
| 7 | seeded jitter ±61 cm vs golden angle only | generator / bible | placement purity; the min-gap gate moves |
| 8 | Spire (0, 3500, 500) vs (14000, -9000) | bible / code | whether the Spire is a landmark you walk to or a horizon prop |
| 9 | Keeper's line: "Stand still. I count the living first." vs the five-line opening | bible / canon | the game's first ten seconds |

## 12. What the bible itself says to do next

Its recursion is `region(n) = region(n-1) + region(n-2)`, with the `+1` being one loop
that plays. The Hub is the `1`. It is not booked yet, because:

1. its counts do not match its JSON (7 pillars, no crystals);
2. `[E]` does not exist in the live pawn, so there is no interaction atom at all;
3. Cartographer, Cache and Vault — three of eleven anchors — are missing;
4. the Spire is not at its anchor.

The correct order is therefore: **fix the Hub atom to its own counts, give it the E
interaction the bible specifies, seat the three missing anchors, move the Spire to its
anchor — then Nest.** Fifty sites, mounts, races, items and four more regions are `phi`
applied to that atom, and multiplying before the atom is true is the exact failure mode
the reverse-engineering note describes.

## 13. Ledger

| status | count | items |
|---|---|---|
| MET | 16 | phi identity, golden angle, seed, GoldenLoc, 7 anchors, gravity, walk, sprint, pitch, capsule/floor, grounded, quest ribbon, Seer, Countess, Keeper post |
| PARTIAL | 6 | Hub zone, glide, HUD ribbon, placeholders, minimap, Spire-as-prop |
| ABSENT | 14 | Nest, Spire zone, Omni, Deep, Cartographer, Cache, Vault, E-dialogue, ONE mic prompt, icon, races, enemy cast, mounts, items/menus |
| CONFLICT | 9 | §11 |
| UNMEASURED | 1 | duplicates |

One of five zones, zero of five menus, zero mounts, zero of four enemy archetypes in the
live module, and an interaction key that does not exist. That is the honest state of the
bible against the build: the law is real and measured, the world is one atom of the five,
and the atom does not yet pass its own count gate.
