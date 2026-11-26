# VEIL OMNI — SCOPE, WHOLE

Source of truth: `docs/VEIL_Bible.md` (Bible) + the Kraken file (`docs/VISION_KRAKEN.txt`)
+ `docs/PHI` law. Where this file and the Bible disagree, the Bible wins.

## The game
One open world, one Runner, one save, one door.
**Target: 60–100 hours of main story**, side quests galore, across five regions.
Free will everywhere, phi always ordered.

## Regions and authority coordinates (never moved)
| # | Region | Coordinates | Look |
|---|---|---|---|
| 1 | Hub | (0, 0) | Cliffside market, gold phi pillars, stalls, lanterns, crowds |
| 2 | Nest | (-450, 780) | High cliffs, waterfalls, huts on ledges |
| 3 | Spire | (0, 3500, 500) | Great tower, gold glyphs, storm at the top |
| 4 | Omni District | (2800, 500) | Blue-lit city sprawl, light pillars |
| 5 | Megastructure Deep | (0, -5000) | Blue pit under the cliffs, machines, bridges |

Also fixed: Vault (0, 0, -80.9) · Keeper Sera Vahl (900, 0) · Seer (-450, 780)
· Cartographer (-4150, -780) · Countess (809, -309, 64.7) · Cache (0, -6200)
· Beacons (2800, 500) / (-2000, -900) / (600, -2100) · RUNNER spawn (0, 0, 100).

## Law
- phi = 1.618033988749895, phi^2 = phi + 1 within 1e-6 on every boot.
- Golden angle 137.5077640500378 degrees.
- `GoldenLoc(i)`: angle = i x golden angle, radius = Spacing x sqrt(i), Spacing = 100 phi.
- Seed 1618. No random. No invention. No placeholders. Nothing floats.

## Ground
The open world level is `Lv_Mountain_Tops_VT` (real Iceland landscape, 179 MB), already
in the project. The runtime builder traces the real land under the Hub and seats every
region, site and body on it. It never lays fake terrain over authored terrain.

## 50 games in 1
Fifty named features from the Kraken file, feature i on `phi_place(i)` around the Hub.
Each is a real lantern you can walk to, not a name in a list, and each grows into its
own system (trade, hunt, glide, climb, heist, watch, race) as the regions come online.

## Milestones (each proven before the next)
| M | Milestone | Proof |
|---|---|---|
| M1 | Save fix | reload shows same values |
| M2 | Core loop: Keeper count, 3 beacons, fight, die, respawn, save | save file reads back |
| M3 | HUD per Bible §3 | Play screenshot |
| M4 | Hub slice on the real landscape | Play screenshot, no gray boxes |
| M5 | Glide, fly, GODMODE with ALL OFF | toggle test |
| M6 | Clean: 0 errors, 0 warnings | build + log tail |
| M7+ | Nest, Spire, Omni, Deep, each to the same bar | per-region proof |
| M8 | Story and side quests to 60–100 hours | full playthrough log |

## Current state
- Build: VEILOMNIEditor Win64 Development, `Result: Succeeded`.
- Boot: all six `VEIL PHI BOOT` lines pass, identity delta 0.0.
- World: five regions, 50 sites, 4 named bodies, crowd, beacons, Spire, on the real landscape.
- Open: HUD, `[E]` dialogue, combat, mounts, menus, save/load, quest chain.
