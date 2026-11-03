# VEIL OMNI — AI handoff

Everything here was measured on Tyree's machine, not assumed. If a number is not in this
file and not in the source, it is unknown — say so instead of inventing it.

## The rules that actually govern the work

- Tyree's word is the design. Do not redesign his numbers; implement them.
- Phi is fixed and never guessed: `phi = 1.618033988749895`, `phi^2 = phi + 1`,
  rung = `ln(v) / ln(phi)` round-tripped, golden angle `137.5077640500378` (rung 10.2318),
  golden pitch `17.0324` (rung 5.8916, cos 0.9561, sin 0.2929) about **RightVector**,
  tetrahedral `109.471221`, `c = 299792458`.
- Never delete `Content/` or `Saved/`. No cook, no package, until the whole thing is done.
- Authority coordinates are fixed: RUNNER 0,0,100 · Keeper 900,0 · Seer −450,780 ·
  Cartographer −4150,−780 · Cache 0,−6200 · Vault 0,0,−80.9 · Countess 809,−309,64.7.

## Build and play (the only two commands that matter)

Build **`VEILEditor`**, never `VEIL`:

```
"/c/Program Files/Epic Games/UE_5.8/Engine/Build/BatchFiles/Build.bat" VEILEditor Win64 Development \
  -Project="D:\VEIL-OMNI\VEIL.uproject" -WaitMutex -FromMsBuild -MaxParallelActions=12 -NoUBA
```

The launched editor only loads the `VEILEditor` binaries. Building `VEIL` produces a
"successful" build that changes nothing on screen — this has already burned several runs.

Play proof (windowed, one shot, log to `temp_play.log`):

```
Start-Process UnrealEditor.exe 'D:\VEIL-OMNI\VEIL.uproject','/Game/Maps/VEILDistrict' `
  '-game','-windowed','-resx=1280','-resy=720','-phishot','-log','-abslog=D:\VEIL-OMNI\temp_play.log'
```

Screenshots land in `Saved/Screenshots/WindowsEditor/HighresScreenshot*.png`. Downscale with
PowerShell `System.Drawing` to ~900 px wide JPEG (there is no ImageMagick here).

## Measured state at commit `ca66814`

- `Result: Succeeded` (VEILEditor).
- Boot clears 28–30 worn mesh shells around the spawn column, and the runner is seated on
  the **market deck** — the slab whose top face is 500 cm — via the open-ground walk
  (`PHI UNSTICK: … moved to open ground … 368 cm out`). Feet end at z ≈ 598.
- `PHI SMOOTH PROOF: forward ≈ 431 cm, zero = 0 frames, verdict = SMOOTH`.
- `PHI SAVE PROOF: slot VEILOMNI_Save0 written=1`, `crash=0`.
- HUD: one owner, `AVEILHudCanvas::DrawHUD`. Minimap diameter is
  `UPHIMITO_MAP_DIAM(H) = H / phi^3 * 0.70` → 178 px at 1080p (the spec's 180), 119 px at
  720p. Quest plate 420×110, one-line quest ribbon width `ClipX / phi^2`, panels 55%.

## Open problem (do not claim this is fixed)

A wide reflective sheet lies across the plaza at eye height and fills the middle of the
frame. It is not the HUD and not the ocean actor (`APhiOcean` lowering never logged
`PHI SEA`). Next step is to name the component under the eye point at the seat and sink or
remove exactly that actor.

## Traps that already cost runs

- `Fix-VEIL-Whole-Muse.py` must **not** be run whole. It builds target `VEIL`, moves
  `VEILPlayerController.*` (already in `Docs/golden_keep`), restores headers from the
  oldest `Backup_veil_omni_install_2026-10-3x` snapshots, and rewrites movement and the HUD.
  Its only verified insight — Count 5 of the golden-angle variant pick collapsing to two
  buckets — is already applied in `GoldenPick` (`VEILWorldBuilder.cpp`).
- `Source/VEIL/Public/Game/VEILPlayerController.h.muse-backup` and the `.cpp.muse-backup`
  beside it are leftovers; any audit that globs `VEILPlayerController.*` under `Source`
  will report a false duplicate.
- A separate pasted "whole" script once replaced `VEILGameMode.h` with a 3.8 KB stub and
  cut `VEILCharacter.cpp` to 66 KB. Recovery was `git checkout -- Source/`. Keep the
  working tree committed before running anything wholesale.
- Source files are CRLF. Edit through the count-checked scripts in `Tools/` (they abort
  without writing when an anchor does not match exactly once); `str_replace` is brittle here.
- One HUD owner per zone: `AVEILHudCanvas::DrawHUD` owns the play HUD, `VEILHudSpec::Draw`
  is not called, `VEILOSFace::Draw` only when `AVEILPlayerController::bShowDebug`.

## Where things live

- `Source/VEIL/Private/Character/VEILCharacter.cpp` — input (`Move`, golden pitch about
  RightVector), boot clear, seat, unstick, `PHI SMOOTH PROOF`, `PHI SAVE PROOF`.
- `Source/VEIL/Private/Game/VEILGameMode.cpp` — `SpawnDefaultPawnFor`, golden-angle seat,
  `PlaceAt` dressing.
- `Source/VEIL/Private/Game/VEILHudCanvas.cpp` — the whole HUD, minimap, bands, panels.
- `Source/VEIL/Private/World/VEILWorldBuilder.cpp` — `GoldenPick` variant law, stone/gold.
- `Source/VEIL/Private/World/VEILMapCapture.cpp` — live top-down capture for the minimap.
- `Tools/patch_*.py` — idempotent, count-checked edit scripts.
- `Docs/proof/*.jpg` — play frames used as evidence.

## Input: where it is decided

The pawn chooses its own input component in code - `AVEILCharacter::CreatePlayerInputComponent()`
returns a `UEnhancedInputComponent` - so no config file can downgrade the runner to a plain
`UInputComponent`. `SetupPlayerInputComponent` builds the nine built-in actions **before**
binding, so the first pass already reports `PHI INPUT BIND: 9 of 9`; it used to report
`0 of 9` at possession because the actions were built later in `BeginPlay`.

`Config/DefaultInput.ini` must use plain keys. `+DefaultInputComponentClass=...` is an array
add to the config parser and is dropped on a scalar class property, which is what produced
the zero-action pawn and, on an older build, the fatal
`Cast of InputComponent ... to EnhancedInputComponent failed` (Casts.cpp:10).


## Interior headroom (2026-11-03)

The played map builds no rooms at runtime - it is a saved world, so `BuildRoom`'s clear height
never runs there and the shells are placed actors. Shells are therefore tallened at boot, after
3.5 s, as one unit: a roof under 340 cm of measured floor gap rises by exactly its shortfall, and
every wall under that roof footprint is stretched to the roof's new bottom with its base pinned.
A blind raise was tried first and detached roofs from walls (min clear fell to 94 cm).

Measured on the last run: 14 roofs lifted, 92 walls stretched, 20 rooms probed, min clear 245 cm,
1 room under 300 cm, 0 fatal. `PHI CEILING` reports the roof over the runner (open sky at spawn).


## Placeholders (2026-11-03)

Measured: 45 call sites still load `/Engine/BasicShapes/Cube|Cylinder|Sphere|Cone`, and several
are components (crowd instances, agent bodies, statue parts, ocean skin, gear), which is why no
spawn-time fix ever covered them. `/Game/Sicka_EnvPack2/Sicka_Ancient_Kit` meshes load fine:
`SM_Floor1_Big`, `SM_Floor2_Big`, `SM_AncientColumn_Big`, `SM_HlafDome` (under `Mesh/Props`, NOT
`Mesh/Modular`), `SM_AncientMonolith`, `SM_HolderRoof`.

`VEILKit::Shape/ShapePath/Redress` (`World/VEILKitMesh.h`) is the one mapping. A boot pass walks
every actor's static mesh components, swaps each engine shape for its kit mesh and fits that mesh
to the exact volume the placeholder occupied. Last run: 356 components re-dressed, 0 refused,
0 engine primitives left, verdict NO PLACEHOLDER IN THE FRAME.

Known limit: fitting kit art into a placeholder's volume stretches it (a floor slab stretched to
a 3 m cube reads as a board), and `Cylinder -> SM_AncientColumn_Big` is right for a post and
wrong for a humanoid body. Those call sites need real bodies, not a better cube.
