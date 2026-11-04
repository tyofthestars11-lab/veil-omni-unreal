# HANDOFF — Buffy → Copilot, 2026-11-04

Written on the same machine (D:\VEIL-OMNI), same branch (`veil-whole-local`). Everything
below was checked tonight with a command, not remembered. Where I have not seen something,
it says so.

## Repo state, read from git

- Branch `veil-whole-local`, HEAD `b8c73d7`.
- `origin/veil-whole` is at `b8c73d7` — every commit is on the remote. `git log
  origin/veil-whole..HEAD` is empty.
- `origin/veil-whole-local` is at `e21b79f`; local is **10 commits ahead of it**. The work
  is safe remotely, just on `veil-whole` rather than the same-named branch. Nothing lost.
- `main` untouched (`22532a2`).

## The door, and the rules that are not negotiable

- One build/run/verify door: `Tools/veil-cli.sh` — verbs `build`, `headless`, `bake`,
  `verify`, `shot`, `log`, `cmd`.
- Build **`VEILEditor`**, never `VEIL`. Building `VEIL` prints `Result: Succeeded` and
  changes nothing the editor loads (see `Docs/AI_HANDOFF.md`).
- `headless` needs **75 s**; 45 s returns `INCOMPLETE - 2 of 10 passes spoke`.
- **No Unreal window, no cook, no package, no exe, no apk** without Tyree's word. No
  deleting `Content/` or `Saved/`.
- Never claim progress without a measured frame, and name the file you measured. Never
  pipe a grep through `head` when proving something is absent — that produced a false
  "no call site" claim earlier (corrected in `2652ecf`).
- The proof frame is the **eye-level** one (`Docs/shots/_cli_shot.png`), not the 4 km
  vista `Saved/Screenshots/hub_view.png` (mean 178, and it hides everything).

## The headline: every palette material is Unlit

`Tools/bake_palette.py:63` — verified, still there tonight:

    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)

All 11 palette materials (`/Game/Materials/Palette/VEIL_*`) are Unlit with the owner's hex
wired into **Emissive**. Those are the materials the district actually wears — the
occlusion grid names `VEIL_Cream` on the walls and `VEIL_Brown` on the cliff. Unlit outputs
emissive only: base colour, roughness, normal and shadow maps are dead inputs, so no
surface can take the sun, take the sky light, or show a corner.

That is the "flat slabs" look, and it is the reason the light work below moved the frame by
points instead of stopping the problem.

**I have rewritten `make_material()` to Default Lit** — glow kept only for `VEIL_Crystal`
and `VEIL_Gold`, roughness 0.72 on the rest. **I have NOT run it**, because the script
deletes and rebuilds those 11 assets and Tyree's rule is no deleting Content without his
word. When he says go:

    bash Tools/veil-cli.sh bake Tools/bake_palette.py
    bash Tools/veil-cli.sh build
    bash Tools/veil-cli.sh shot 80

then measure `Docs/shots/_cli_shot.png` (mean, near-black, near-white) and compare against
the table below.

## What I measured tonight, same camera each time

| state | mean | near-black | near-white |
|---|---|---|---|
| start of session | 16 | 80.6% | 0% |
| skylight 6.85 → 18 | 24 | 64.6% | 0% |
| seat out of the roofed room, sun 2,200 → 6,400 lux | 56 | 51.6% | 0.2% |
| sun 8,000 | 65 | 51.1% | 1.1% |
| sun 8,800, sky fill 100 | 75 | 50.5% | 1.6% |

Two real fixes are in that path, plus one honest dead end:

1. **The seat stood inside a room.** The eye-beat occlusion probe showed nine of nine rays
   on `SM_Wall1_Big`/`SM_Wall3_Big`, plus the spawn room's own `SM_HolderRoof` ceiling in
   `VEIL_CrystalDeep`, and **not one ray to sky**. `VEILCharacter.cpp` (the `PHI GROUND
   STAND` block) now leaves the roofed room along a golden-angle spiral from the anchor
   until it finds floor under the feet and nothing overhead, and logs the step:
   `PHI GROUND STAND: ... now V(X=-516, Y=473, Z=188), open-sky step 1`.
2. **The post-process is `AEM_Manual` with zero bias**, so nothing compensated for a
   2,200-lux dusk sun. Sun is now 8,800 lux, sky light 100.
3. **Dead end, recorded so it is not repeated:** hiding the enclosing shell
   (`StaticMeshActor_448`) and turning its shadows off both measured **exactly nothing**
   (24 → 24 mean). They are still in the code as evidence; they are not the fix.

## Still open, in priority order

1. **Palette shading model** (above). Changes what he actually sees.
2. **Black slab masses.** 50.5% of the eye frame is near-black: large flat black shapes
   on the upper-left and right of `Saved/Screenshots/proof_small.png`. The `PHI OCCLUDE`
   lines name the surfaces; re-measure *after* the material fix before touching them,
   because unlit-black and unlit-navy read the same today.
3. **Duplicate runner body.** Two figures stand in the proof frame — the real one at the
   camera and a second capped body to his left. The panel-hide sweep in `VEILCharacter.cpp`
   (`PHI BODY PANEL SWEEP`) is supposed to catch it and does not.
4. **Dead authority tables.** `BuildPhiWhole` returns at `VEILWorldBuilder.cpp:3547` when
   `vision_whole.json` loads, so the authority stones, the zone dress and the Spire-at-Hub
   entry never run. The live Spire is `BuildPhiForms` at (0, 3500) and is correct. Someone
   has to decide with Tyree: delete the dead tables, or remove the early return.
5. **Your `IsSpireAtHub` guard** sits on that unreachable path. It is correct; it just never
   executes. Recorded in `Docs/VEIL-CODE-LOOP-2026-11-04.md`.

## What I have not seen

I have not seen the game in a window — that is forbidden without his word. Every number
above is from a headless frame plus its log. **If his window shows something that
contradicts these numbers, his screen wins** and that is the next thing to chase, not the
measurement.

Do not run the bake, do not bake lighting, do not cook, do not package.
