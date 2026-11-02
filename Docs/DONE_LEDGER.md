# VEIL OMNI — DONE LEDGER

Checked 2026-11-02 against `Docs/VEIL_Bible.md`. The Bible wins on any conflict,
and it says: **no claim, screenshot or "done" until it was verified in Play.**
This file holds to that. Every line below is either a measured number, a log line
from the packaged build, or a file path.

**Verdict: NOT DONE.** Six things are proven, four are partial, three fail the
Bible's own bar. The list of what closes it is at the bottom.

---

## 1. Proven, with evidence

| # | Promise | Evidence |
|---|---|---|
| P1 | Builds clean | `VEILEditor Win64 Development` → `Result: Succeeded`, 0 errors, repeatedly today |
| P2 | Packages and runs | `BuildCookRun` → `BUILD SUCCESSFUL`, `AutomationTool exiting with ExitCode=0`, staged binary launched and ran |
| P3 | No fatal crash | Packaged run: **0** `Fatal error` / `Unhandled Exception` / `Signal 11` |
| P4 | World generates | `PHI TERRAIN: 10201 instanced tiles`; `PHI WILDS: 230 groves, 169 ruins, 107 boulders, 105 shards, 12 wayposts`; `PHI CONCEPT: 236 pieces`; `PHI CROWD` bound |
| P5 | φ AI decides, live | `PHI BRAIN` 2,683 lines + `PHI NPC BRAIN` 3,224 with phinary intent (e.g. `intent HUNT = 0.100001 phi`, rung 7.6020); `PHI ECOLOGY` 14 camps with per-camp digits |
| P8 | Mines are authored, not scattered | 24 named seams in `GMineBook`, each with an owner, an ore, a depth and a reserve; built as headframes (4 posts, 2 beams, shaft, seam), not cones |
| P7 | Log discipline is measured, not claimed | After the fix: `STAND DOWN` 6,198 -> 111, `ECOLOGY` 2,664 -> 153 in a comparable run, with orders still cycling |
| P6 | The 50-activity loop closes | `proof/simulate_run.py`: **50/50** activities, 110.8 min, 9,158 m walked, `beacons 3/3 and warden down -> COMPLETE`, `phi^2 - phi - 1 = 0.000e+00` |

Source scale: **79 UCLASSes, 26,495 lines of C++** across AI, Animation,
Character, Game, Story, Systems, World, UI, Tests.

## 2. Partial — implemented, not proven

| # | Thing | State |
|---|---|---|
| A1 | Save / load (Bible M1, M2) | Code exists and runs; the last packaged run logged `PHI HUD LOAD: loaded=0 - no save yet`. **A save→quit→reload cycle has never been observed.** M1's required `loaded=1` proof is absent. |
| A2 | Push-to-talk ONE | Bound to **B**, state machine boots `ready - hold B to speak`, mic opens on press via the Voice module. No key press has ever been observed, and **no speech-to-text exists**, so a hold produces a measured duration, not a sentence. |
| A3 | Tetrahedron chain | Solid is now real geometry (4 corners, 4 triangles, procedural mesh, cook reports 0 missing assets), but `PHI TETRA: chain placed` never appears — the spawn is gated behind a path that does not fire at boot. Compiled, not placed. |
| A4 | Species identity | Six species exist and differ by rung, champion flag and order behaviour. **All share one body mesh** (Polyart Garret). They are not visually distinct. |

## 3. Fails the Bible's own bar

| # | Bar | What actually happens |
|---|---|---|
| F1 | M6: "0 errors, 0 warnings… no per-frame log spam" | Two engine **ensures** fire in the packaged build: `PSOPrecache.cpp:296` (`!IsComponentPSOPrecachingEnabled() \|\| bPSOPrecacheCalled`) and `GPUScene.cpp:706` (light data memcmp). Neither is fatal; both are ours to fix or explain. |
| F2 | M6: log only on change | **Fixed and measured.** Before: `PHI STAND DOWN` **6,198** and `PHI ECOLOGY` **2,664** in a two-minute run. After hysteresis (an order must hold two decisions) and one-log-per-engagement, the same length of run gives **111** and **153** — a 98% and 94% cut — while the camps still cycle (65 HUNT / 39 HOLD / 35 ENCIRCLE). Remaining noise: `PHI BRAIN` 1,147 + `PHI NPC BRAIN` 1,344 per 120 s, which is per-decision rather than per-frame. |
| F3 | M3/M4: HUD and hub slice must match the painting | Never visually verified by the agent; the game has only been checked through its own log. |

## 4. Not started

- Speech-to-text for ONE (local Whisper or equivalent).
- Distinct species bodies.
- Weather/time systems proven in play.
- Marriage / jail / farming / building systems — code may exist, none is proven.
- 60–100 hours of story. Simulation says 1.8 h to close all 50 activities.

## 5. What closes this ledger

1. Re-cook with the hysteresis + log gating, run, and confirm `STAND DOWN` and
   `ECOLOGY` line counts drop by an order of magnitude.
2. Save → quit → reload in the packaged build; capture `loaded=1` and matching
   level/gold/beacons. That is Bible M1/M2 and it is the single biggest gap.
3. Fix or justify the two ensures.
4. Make the tetra chain place at boot and confirm `PHI TETRA` in the run log.
5. Give the six species distinct bodies.
6. Only then: **M3/M4 visual pass**, which requires eyes on the screen.

Nothing above is "almost". Each line is either a number that exists or a number
that does not exist yet.
