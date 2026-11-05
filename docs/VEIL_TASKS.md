# VEIL TASKS — the Bible broken into closable work

Source of this file: `docs/VEIL_Bible.md`, 112 lines, read from disk on 2026-11-05.
Author of the Bible: Tyree Jones (with Muse). This breakdown: Buffy, branch `buffy-phi`.
**Hark is the only builder in `C:\Users\Admin\HarkGame` right now. This file is docs only.**

## The law, at every scale

One law, micro to macro: `phi^2 = phi + 1`. phi 1.618033988749895, golden angle
137.5077640500378 deg, GoldenPitch 17.0324, `GoldenLoc(i,S)`: angle `i x 137.5077640500378`,
radius `S x sqrt(i)`, centimetres. Nothing in the project may hard-code 1.618, 137.5077 or
137.5; everything reads `PHI_Constants`. Binary is always plus one; phi closes.

**No placeholders, no gray boxes, no boring panels.** A task is closed only by a build
`Result:` line and a real in-game PNG (Bible lines 105-107). No claim without both.

Status legend: **DONE** (verified in HarkGame), **PARTIAL**, **TODO**, **LEGACY** (refers to a
system that was never built), **BLOCKED** (a named blocker).

---

## TODAY — the next 8 smallest tasks

| # | ID | Task | Closes with |
|---|---|---|---|
| 1 | M0-06 | Port `Public/Systems/PHI_Constants.h` + `Private/Systems/PHI_Constants.cpp` into the HarkGame module so one constant set owns phi (Bible 21-25) | Build `Result: Succeeded` + `VEIL PHI BOOT ... PASS` |
| 2 | M3-02 | STM bar to gold `#C9A24A`, ATP to white, delete every red/green HUD value (brief overrides Bible 46-47) | Play PNG + `grep D9343A|2FB36A` = 0 |
| 3 | M3-03 | Delete red from combat pill and crit numbers: pill dot `#3FA9FF`, crits white with blue bloom (Bible 51-52 conflict) | Play PNG, no red pixel in HUD |
| 4 | M3-04 | Move debug/control text to an F10 toggle (Bible 53, 112) | PNG with HUD clean, second PNG with F10 on |
| 5 | M4-02 | Sun pitch to 17.0324 golden hour, volumetric cloud on, mist in the valley (Bible 29) | Log line `VEIL SKY: sun pitch 17.0324` + PNG |
| 6 | M4-03 | Muse Beacon: electric blue light at 0,0, never removable code path (brief) | Log `VEIL MUSE: beacon 0,0 lit` + PNG |
| 7 | M4-04 | 89 obelisks on `GoldenLoc`, gold on every 8th and 13th, market stalls between (brief) | Log count 89 / gold 23 + PNG |
| 8 | M6-05 | Stop the 15 engine `LogPython` errors from `EditorToolset`/`ToolsetRegistry` | `grep -c "LogPython: Error"` = 0 |

---

## Conflicts found (every red, yellow and gray), with the fix

| ID | Flag | Where | Conflict | Fix |
|---|---|---|---|---|
| C-RED-01 | RED | Bible 46 vs brief | VIT bar is `Red #D9343A`; the brief forbids red and wants electric blue `#3FA9FF` | Brief wins. Amend Bible 46 to `#3FA9FF`, delete `#D9343A` from code. Tyree signs the amendment because the Bible is the authority. |
| C-RED-02 | RED | Bible 51-52 | Combat pill carries a red dot; crits are red | Pill dot `#3FA9FF`; crits white `#F2F2F2` with blue bloom. No red anywhere. |
| C-RED-03 | RED | Bible 47 vs brief | STM bar is `Green #2FB36A`; brief says STM gold and no green in UI | STM gold `#C9A24A`, ATP white; green is vegetation only. |
| C-YELLOW-01 | YELLOW | Bible 86, 107 | Proof demands `Saved\seat719_out.txt`; no seat719 save system exists in HarkGame | Proof becomes `Saved/VEIL_save.json` + log `VEIL SAVE: loaded=1`. |
| C-YELLOW-02 | YELLOW | Bible 64, 67 | Countess authority coordinate `809,-309,64.7`; no Countess actor, no heist system | Reserve the coordinate in the authority table; spawn only in M7-03. No invented actor meanwhile. |
| C-YELLOW-03 | YELLOW | Bible 77 | "Mitochondrial engine" exists twice: `Private/Systems/PHI_MitochondrialEngine.cpp` (repo) and `Source/VEILOMNI/VeilMitochondria.h` (HarkGame) | One owner: port `PHI_MitochondrialEngine`, delete the header-only duplicate at M2-06. |
| C-YELLOW-04 | YELLOW | Bible 67 vs brief rings | Beacons `2800,500 / -2000,-900 / 600,-2100` (r 2844/2186/2184) do not sit on `r(n)=3000xphi^n` | Authority coordinates win (Bible 67). Rings are scatter guidance only; add a tolerance check of `0.382 x r`. |
| C-YELLOW-05 | YELLOW | Bible 67 | Cache `0,-6200` is r 6200, ring n2 is 7854, a 1654 drift | Keep Cache as an inter-ring landmark and log it; do not move it to satisfy the ring. |
| C-YELLOW-06 | YELLOW | Bible 45 | Badge example says "LV 72"; the live HUD shows "LV 1" because no XP system exists | Badge text reads player level; stays 1 until M7-08. |
| C-YELLOW-07 | YELLOW | Bible 38 vs live | Bible says UMG; the live HUD is Canvas `AVEilHUD` | Canvas stays as proof/debug HUD only; `WBP_VeilHUD` in UMG closes M3-14. |
| C-GRAY-01 | GRAY | Bible gap | The Bible has no audio or music section at all, and a legit game needs both | Add Bible §10 Audio; tasks M4-16..M4-21 with licence-clear CC0/CC-BY sources. |
| C-GRAY-02 | GRAY | Bible 34 | "Megascans already in the project" — HarkGame has CC0 packs, the Sicka kit and Polyart, not Megascans | Run `Tools/veil_asset_inventory.py`; place only real `/Game` paths, never invented ones. |
| C-GRAY-03 | GRAY | Bible 99 | Bake Lighting/Geometry/Navigation is written as mandatory; the town is Lumen-dynamic | Record "dynamic lighting, no bake" as a deliberate deviation in §7. |
| C-GRAY-04 | GRAY | Bible 97 | Build command targets `UnrealBuildTool VEILOMNI`; the proven command in HarkGame is the editor target | Update §7 to `Build.bat VEILOMNIEditor Win64 Development -WaitMutex`. |
| C-GRAY-05 | GRAY | Bible 31 vs brief | Bible says "gold phi pillars", no count; brief fixes 89 obelisks with gold on 8th/13th | 89 is authoritative; add the count check to M4-04. |
| C-GRAY-06 | GRAY | Bible 29 vs brief | TON 618 disc, core overhead and 8 spiral light beacons appear only in the brief | Add to M4-13/M4-14 as art tasks; Bible §2 amended at Tyree's sign-off. |

---

## M0 — Law, constants, reporting (Bible 8-13, 21-25)

| ID | Bible | Task | Files | Proof | Depends on | Status |
|---|---|---|---|---|---|---|
| M0-01 | 21-25 | Single `PHI_Constants` with phi, golden angle, `GoldenLoc`, `VerifyPhiIdentity`, `VerifyBoot` | `Public/Systems/PHI_Constants.h`, `Private/Systems/PHI_Constants.cpp` | Boot log PASS on 4 checks | — | DONE on `buffy-phi` |
| M0-02 | 22 | `VerifyBoot()` once on boot, never per frame | `VeilGameMode.cpp:79` | One log line per boot | M0-01 | DONE (HarkGame) |
| M0-03 | 25 | GoldenLoc proof values at Spacing 10 within 1e-4 | `PHI_Constants.cpp` | `VEIL PHI PROOF ... i=1/2/5 PASS` | M0-01 | DONE on branch |
| M0-04 | 13 | No cook, package or exe script in the repo | `Scripts/`, `Tools/` | `grep -r "BuildCookRun\|RunUAT"` = 0 | — | DONE |
| M0-05 | 12 | One builder rule; Buffy writes docs and branch only | this file | no HarkGame code writes from Buffy | — | ACTIVE |
| M0-06 | 21-25 | Port the two constant files into HarkGame's module | `Source/VEILOMNI/` | Build + boot PASS | M0-01 | TODO |

## M1 — Save fix (Bible 79, 85)

| ID | Bible | Task | Files | Proof | Depends on | Status |
|---|---|---|---|---|---|---|
| M1-01 | 79 | One slot + autosave: level, XP, gold, beacons, deaths, position, region, quests | `VeilSaveGame.h`, `VeilGameMode.cpp` | `VEIL SAVE: loaded=1` and equal values after reload | M0-06 | PARTIAL (`bHasSaved`, `RespawnAt` exist) |
| M1-02 | 85 | HUD reads loaded values, not defaults (the LV1/gold 0 bug) | `VeilHud.cpp`, `VeilPlayerController.cpp` | PNG after reload shows same LV/gold | M1-01 | TODO |
| M1-03 | 86, 107 | Replace the `seat719_out.txt` proof path | `docs/VEIL_TASKS.md`, save code | `Saved/VEIL_save.json` exists | M1-01 | LEGACY |
| M1-04 | 79 | Deaths counter persists | `VeilSaveGame.h` | Reload shows deaths+1 retained | M1-01 | TODO |
| M1-05 | 79 | Quit/reload harness that a headless run can drive | `Scripts/`, `VeilGameMode.cpp` | log `VEIL SAVE: roundtrip OK` | M1-01 | TODO |

## M2 — Core loop (Bible 75-79, 86)

| ID | Bible | Task | Files | Proof | Depends on | Status |
|---|---|---|---|---|---|---|
| M2-01 | 86 | Keeper counted | `VeilPlayerController.h` (`bKeeperCounted`) | Quest banner changes to beacons | — | DONE |
| M2-02 | 86 | Three beacons can be lit in world | `VeilGameMode.cpp` (Bible 67 coords) | `BeaconsLit 3/3` | M1-01 | PARTIAL |
| M2-03 | 75 | Combat: enemies target player first, light/heavy/dodge/signal | `VeilMind.cpp`, `VeilNpc.cpp` | Play PNG + damage log | M2-02 | TODO |
| M2-04 | 86 | Death and respawn at `RespawnAt` | `VeilRunner.cpp` | log `VEIL RESPAWN` at saved point | M1-01 | PARTIAL |
| M2-05 | 86 | Full loop: load, keeper, 3 beacons, fight, die, respawn, save, quit, reload | whole module | One PNG + save JSON before/after | M2-01..04 | TODO |
| M2-06 | 77 | ATP feeds stamina and abilities, one implementation only | `PHI_MitochondrialEngine.cpp`, delete `VeilMitochondria.h` duplicate | ATP drain log + build | M0-06 | LEGACY |

## M3 — New HUD (Bible 36-55, 87; brief palette)

| ID | Bible | Task | Files | Proof | Depends on | Status |
|---|---|---|---|---|---|---|
| M3-01 | 46 | VIT bar electric blue `#3FA9FF`, numbers right, slanted ends | `VeilHud.cpp` | PNG + `grep D9343A` = 0 | — | DONE (HarkGame) |
| M3-02 | 47 | STM gold, ATP white, no green in UI | `VeilHud.cpp` | PNG | M3-01 | TODO |
| M3-03 | 51-52 | No red: pill dot blue, crits white | `VeilHud.cpp` | PNG | M3-01 | TODO |
| M3-04 | 53, 112 | Debug text only on F10 | `VeilHud.cpp`, `VeilPlayerController.cpp` | Two PNGs | — | TODO |
| M3-05 | 42 | Panel navy `#0E1424` at 85%, 1.5 px gold frame, slanted top-right, chevron crest | `VeilHud.cpp` / UMG | PNG | — | PARTIAL (72% fill, no crest) |
| M3-06 | 44 | Cinzel headers, Rajdhani body, white `#F2F2F2`, gold `#E8C46A` labels | UMG fonts | PNG | M3-14 | TODO |
| M3-07 | 45 | Gold diamond level badge with phi star and plate | `VeilHud.cpp` | PNG | — | PARTIAL (diamond + "LV 1") |
| M3-08 | 48 | Round gold ring, N/E/S/W ticks, compass needle, pause button inside | `VeilHud.cpp` | PNG | — | DONE (real geometry minimap, clickable pause) |
| M3-09 | 49 | Quest panel bottom-left "QUEST: THE GOLDEN MIDST" | `VeilHud.cpp` | PNG | — | PARTIAL (top banner) |
| M3-10 | 50 | Hotbar diamonds, centre slot larger and glowing, stack counts | `VeilHud.cpp` | PNG | — | PARTIAL |
| M3-11 | 55 | Settings: HUD scale sliders, atmosphere stance, GODMODE and ASSISTS | UMG | PNG of settings | M3-14 | TODO |
| M3-12 | 38 | `WBP_VeilHUD` in UMG (Canvas becomes debug only) | UMG assets | PNG from UMG HUD | M3-01..11 | TODO |
| M3-13 | 78 | Full-colour world map, fog of war, markers | UMG + `VeilGameMode.cpp` | PNG | M3-12 | TODO |
| M3-14 | 53 | HUD auto-fades out of combat | `VeilHud.cpp` | Two PNGs | M3-12 | TODO |

## M4 — Hub slice (Bible 29-34, 61, 67, 88; brief)

| ID | Bible | Task | Files | Proof | Depends on | Status |
|---|---|---|---|---|---|---|
| M4-01 | 29 | Golden hour, god rays, volumetric cloud, valley mist | `VeilGameMode.cpp` light block | PNG | — | PARTIAL |
| M4-02 | 29 | Sun pitch exactly 17.0324 | light block | log + PNG | M4-01 | TODO |
| M4-03 | brief 2 | Muse Beacon electric blue at 0,0, never removed | `VeilGameMode.cpp` | log + PNG | — | TODO |
| M4-04 | brief 3 | 89 obelisks on `GoldenLoc`, gold every 8th and 13th, stalls between | `VeilGameMode.cpp` | count log 89 / 23 gold + PNG | M0-06 | TODO |
| M4-05 | 61 | Market: stalls, lanterns, crowds | `VeilGameMode.cpp`, `VeilNpc.cpp` | PNG | M4-04 | PARTIAL (8 stalls, 66 bodies) |
| M4-06 | 31 | Glowing phi glyphs on pillars | material + `VeilGameMode.cpp` | PNG | M4-04 | PARTIAL |
| M4-07 | 31 | Spire on the horizon, visible from everywhere | `VeilGameMode.cpp` | PNG from 2 regions | — | PARTIAL |
| M4-08 | 31 | Waterfalls at terrace lips | Water plugin, Niagara | PNG | — | BLOCKED (no waterfall mesh in packs) |
| M4-09 | 31 | Rope and stone bridges between terraces | kit meshes | PNG | M4-08 | TODO |
| M4-10 | 32 | Runner hooded navy, gold trim, phi wing membranes glowing blue | `VeilRunner.cpp`, materials | PNG in glide | M5-02 | PARTIAL |
| M4-11 | 34 | No gray boxes; palette audit gold/blue/navy/white only | level + materials | PNG + palette grep | — | PARTIAL (stone reads white/gray) |
| M4-12 | 30 | Palette lock: navy `#0E1424`, gold `#C9A24A`, labels `#E8C46A`, blue `#3FA9FF`, white `#F2F2F2` | materials | `Scripts/golden_eye.py` clean | — | PARTIAL |
| M4-13 | brief 8 | TON 618 disc and core overhead | sky | PNG | M4-01 | TODO |
| M4-14 | brief 8 | 8 light beacons on a rising golden spiral | `VeilGameMode.cpp` | log 8 + PNG | M4-04 | TODO |
| M4-15 | 88 | Fly-around PNG, no gray boxes, 60 fps target | whole level | PNG + fps log | M4-01..14 | TODO |
| M4-16 | C-GRAY-01 | Ambient loop (CC0 "Cathedral in the forest", OGA `forest_2_0.ogg`) | `Content/Audio/` | SoundWave + PNG playing | — | TODO |
| M4-17 | C-GRAY-01 | Beacon stinger (CC-BY 4.0 "Orchestral Stinger", attribution required) | `Content/Audio/` | Cue fired log | M4-16 | TODO |
| M4-18 | C-GRAY-01 | Footsteps stone/cloth (CC0 Fantozzi + CC0 leather/cloth/armor) | `Content/Audio/` | Footstep cue log | — | TODO |
| M4-19 | C-GRAY-01 | UI gold-bell confirm (CC0 "Bell Arpeggio 24", CC0 Kenney interface) | `Content/Audio/` | Cue on quest accept | — | TODO |
| M4-20 | C-GRAY-01 | Wind/high-altitude ambience (CC0 "wind whoosh loop", CC0 "Space Winds") | `Content/Audio/` | Ambience loop log | — | TODO |
| M4-21 | licensing | `LICENSES.txt` with verbatim licence names, Sonniss bundle line, attributions | repo root | file exists, licences listed | M4-16..20 | TODO |

## M5 — Glide, fly, GODMODE (Bible 71-73, 89)

| ID | Bible | Task | Files | Proof | Depends on | Status |
|---|---|---|---|---|---|---|
| M5-01 | 71 | Sprint | `VeilPlayerController.cpp` | key works in Play | — | DONE (Shift) |
| M5-02 | 71 | phi-wing glide with visible wings | `VeilRunner.cpp` | PNG mid-glide | M4-10 | PARTIAL (glide toggles, no wings) |
| M5-03 | 71 | Climb | `VeilRunner.cpp` | PNG on wall | M5-02 | TODO |
| M5-04 | 71 | Fly (unlocked) | `VeilRunner.cpp` | PNG airborne | M5-02 | TODO |
| M5-05 | 71 | Skiff and mounts | new pawns | PNG riding | M5-04 | TODO |
| M5-06 | 72 | GODMODE: fly, noclip, infinite vit/atp/stm/ammo, damage x2/x5/x10/one-hit, freeze, kill all, teleport, time/weather/spawn, speed/jump, save anywhere, ALL OFF, reset on restart | UMG + `VeilPlayerController.cpp` | Play test each toggle, restart proves OFF | M3-11 | TODO |
| M5-07 | 73 | Assists menu with ALL OFF | UMG | Play test | M3-11 | TODO |
| M5-08 | 89 | Proof of every toggle | Play | PNG per toggle set | M5-06..07 | TODO |

## M6 — Clean (Bible 90, 105-112; brief)

| ID | Bible | Task | Files | Proof | Depends on | Status |
|---|---|---|---|---|---|---|
| M6-01 | 90 | Build 0 errors, 0 warnings | module | `Result: Succeeded`, 0 warnings | all | PARTIAL |
| M6-02 | 90 | Editor log 0 warnings, no per-frame spam | `VeilGameMode.cpp`, `VeilNpc.cpp` | log tail | M6-01 | TODO |
| M6-03 | 111 | SS1/CEF handshake spam gone (Fab disabled) | `VEILOMNI.uproject` | `grep handshake` = 0 | — | DONE (HarkGame) |
| M6-04 | 112 | Debug overlay behind F10 | `VeilHud.cpp` | PNG | M3-04 | TODO |
| M6-05 | brief | `LogPython: Error` = 0 (engine mounts EditorToolset/ToolsetRegistry despite the project flag) | `VEILOMNI.uproject`, editor plugin state | `grep -c "LogPython: Error"` = 0 | — | BLOCKED (engine-side, 15 errors) |
| M6-06 | 90 | Log only on change | all modules | no repeated lines per frame | M6-02 | PARTIAL |

## M7 — Regions, systems, story (Bible 62-65, 71-79, 91)

| ID | Bible | Task | Files | Proof | Depends on | Status |
|---|---|---|---|---|---|---|
| M7-01 | 62 | Nest: cliffs, waterfalls, huts, Seer at -450,780, glide training | level + NPCs | PNG + Seer dialogue | M5-02 | TODO |
| M7-02 | 63 | Spire: vertical climb, signal fragments, storm top, boss | level + boss | PNG + boss defeat log | M7-01 | TODO |
| M7-03 | 64 | Omni District and the Countess at 809,-309,64.7 | level + NPC | PNG + dialogue | M7-02 | LEGACY (no Countess exists) |
| M7-04 | 65 | Megastructure Deep and the one door | level | PNG + door open | M7-03 | TODO |
| M7-05 | 67 | Authority coordinates in one header, never scattered | `VeilAuthority.h` | one source, grep clean | — | PARTIAL (statics on the controller) |
| M7-06 | 74 | Branching dialogue, choices with consequence, NPC memory | dialogue system | PNG + memory log | — | PARTIAL (single line) |
| M7-07 | 76 | Bond/marry, steal/jail/escape, hunt/fish/farm/tame, craft/build, dig/mine/chop | many | PNG per system | M7-06 | TODO |
| M7-08 | 79 | Level, XP, gold economy | save + HUD | PNG + save JSON | M1-01 | TODO |
| M7-09 | 91 | 60-100 hour main story and side quests | quest data | quest count + hours target | M7-04 | TODO |

---

## What is already done in HarkGame (as of 2026-11-05)

- `VeilRunner`: in-place walk clip, strike cue, pause/resume driven from `TakeShot`.
- Hub town: 322 actors, 230 authored pieces, 92 paving slabs, 24 rim walls, 4 gates,
  12 gold columns + capitals, 8 houses, 8 shrines, 10 stair flights, 5 spire tiers.
- Golden-angle pillars: 12 columns and capitals placed on `GoldenLoc` maths, seed 1618.
- `-VeilShot`: five record frames per run, including an over-the-shoulder proof frame.
- Real minimap in Canvas HUD: level geometry sampled into a north-up disc, gold ring,
  N/E/S/W, needle, rim-pinned objectives, clickable pause button.
- VIT bar electric blue `#3FA9FF`; zero red in the HUD source.
- 66 bodies seated by line trace with Real Polyart skeletal meshes; 0 floating.
- 0 `VEIL SNAP` warnings, 0 culled pieces, 0 CEF handshake lines.
- `Scripts/golden_eye.py` and `Scripts/quantum_verify.py` as phi audits.

## What is legacy or not built (a name without a system)

- **seat719 save** (Bible 86, 107): no such save path exists in HarkGame; the live save path
  is `VeilSaveGame.h`. Proof must move to `Saved/VEIL_save.json`.
- **Countess** (Bible 64, 67): coordinate reserved, no actor, no heist, no Omni District.
- **Mitochondrial engine** (Bible 77): two partial implementations, no single owner.
- **Megascans** (Bible 34): not in this project; CC0 packs, Sicka kit and Polyart are.
- **Bake steps** (Bible 99): Lumen-dynamic town means no bake; record the deviation.

## Rules that do not bend

- One builder at a time in HarkGame; Buffy writes docs and the branch only.
- No cook, no package, no exe until 100%. Never delete Content, Saved, Source or the game.
- Never touch `C:\HarkBridge`, keys or tokens.
- Same error twice means change approach and say so, with the commit hash.
