# The code loop, 2026-11-04 — headless, no editor window

Every number here comes from a `UnrealEditor-Cmd -run=pythonscript` run against
`/Game/Maps/VEILDistrict`, or from `Build.bat`. No Unreal editor window was opened in this pass.

## Defects found in code, fixed, and measured

### 1. The hub was never saved into the level
`AVEILGoldenHub::Part()` created components with `NewObject` + `RegisterComponent` and stored no
reference to them. A component held only by the actor's component list is not serialized, so the
composition existed for one session only.

- bake log said `HUB parts: 1061`
- the level reloaded and reported `HUB present: True  parts: 0`
- `AVEILHubMarket` had a `UPROPERTY() TArray<UStaticMeshComponent*> Parts` all along, which is why
  its parts survived
- fix: the hub now holds `Parts`, the same way
- after fix: `HUB present: True  parts: 1061`

### 2. The 25-site market built nothing
`AVEILHubMarket::GroundAt` used one `ECC_Visibility` trace; in a level loaded headless it answered
nothing, so every site was skipped and the market logged `0 carved columns, 0 arch stalls,
0 blue monolith peaks; 25 of 25 sites had no ground`.

- fix: the query tries `ECC_Visibility` then `ECC_WorldStatic`, and if neither answers the site
  stands on the plaza floor (z = 0, which is where the hub's paving is) with a log line saying so
- after fix: `seed 1618, 12 carved columns, 8 arch stalls, 5 blue monolith peaks; 0 of 25 sites had
  no ground`, and `MARKET parts: 45` persisted through a reload

### 3. The grey was a hardcoded starter material
`VEILWorldBuilder` swapped every modular wall onto `M_Brick_Cut_Stone` (neutral grey) in three
places, and also used it as `StoneMat` when demoting gold to masonry. Measured before:
`M_Brick_Cut_Stone 5094`.

- fix: every brick path in the builder now points at `/Game/PHI/Palette/M_PHI_CreamStone`
- after: `M_PHI_CreamStone 5094` (before the palette pass), 0 brick paths left in the builder

### 4. The grey/black tint constants are now the vision's palette
`Stone` was `(0.46,0.44,0.40)` grey and `CapTint` `(0.28,0.24,0.21)` near-black. They are now cream
terrace stone, deep blue roof and a green ground band, converted from the art's hex values, and
`LayTile` lays the ground in the green band instead of the wall colour.

## Current measured state of `VEILDistrict`

```
HUB present: True   parts: 1061
MARKET present: True parts: 45     (12 pillars x2, 8 stalls x2, 5 peaks)
static meshes 6534   flat 78.2%   building-scale (>=300 cm) 10.8%   median height 0 cm
material VEIL_Green        5063
material VEIL_Cream         413
material VEIL_Brown         397
material VEIL_Gold          347
material VEIL_CrystalDeep   268
saved -> True
```

The 78.2% flat figure is the ground paving (tiles are meant to be thin); the 705 pieces at building
scale are the architecture. This is the number that has to rise for the district to read as a city
rather than a floor with buildings on it — and the composition work (market town at the base,
terraces above, Spire behind, crystal range at the horizon) is what raises it.

## Rules kept

- no cook, no package, no exe, no apk, no obb
- no Unreal editor window opened
- nothing under `Content/` or `Saved/` deleted; `Saved/Backup/VEILDistrict_pre-sweep.umap` intact
- every change is in `Source/VEIL` or `Tools/`, and every claim above is a log line

## Tools added this pass

- `Tools/bake_hub_complete.py` — compose hub + market into the level, audit actor and material counts
- `Tools/audit_massing.py` — height distribution of every static mesh (flat vs building scale)
- `Tools/verify_final.py` — one pass: hub parts, market parts, massing, materials, save
- `Tools/sweep_generated.py` — the tagged sweep, kept with its warning (see the state doc: it deleted
  the world's mesh field once and the level had to be restored from backup)

## The second loop: intent first, then the CLI, still no editor window

Working from `Docs/TYREE-INTENT-LEDGER.md` and `Docs/PHI-PHASE-LOCK-TYREE-LINKS-JARVIS-ONE-FIX.md`
as the authority, this pass used `Tools/veil-cli.sh` — the project's own headless door — and never
opened Unreal.

### What was actually broken in the proof path (not in the game)

1. **`build` refused to compile.** `[ -x "$BUILD_BAT" ]` — a `.bat` is never executable to MSYS, so
   the verb printed "no Build.bat at ..." on a path that exists. Now `[ -f ]`.
2. **`shot` could never render.** Its command line contained a literal `\n` where a line
   continuation belonged, so the render invocation broke and the rest ran as a separate command.
   Fixed to a real continuation.
3. **`verify` could never pass.** It looked for `PHI PLACEHOLDER SWAP`, a marker the current pass
   no longer logs. The pass had been rewritten to repeat at 1 Hz until two idle sweeps, skip the
   composed market, and log `PHI ENGINE SHAPE CLEANUP`. The measurement ran every session and
   nothing read it — `VERDICT: INCOMPLETE`, exit 2, forever. The patterns now name the real marker,
   and the cleanup line states its verdict explicitly.

### What the pass measured, in the game's own log

```
PHI ENGINE SHAPE CLEANUP: 112 engine-shape component(s) re-dressed with kit meshes, 0 refused,
  0 retired as dust, 0 visible /Engine/BasicShapes component(s) remain - verdict NO PLACEHOLDER IN THE FRAME.
PHI ENGINE SHAPE CLEANUP: 265 engine-shape component(s) re-dressed with kit meshes, 0 refused,
  0 retired as dust, 0 visible /Engine/BasicShapes component(s) remain - verdict NO PLACEHOLDER IN THE FRAME.
PHI MARKET KEPT: 1 composed market actor(s) left exactly as authored.
```

`VEIL_NULLRHI` not set: the run is offscreen 320x180 with a real local player, nothing on screen.

### Still silent, and honest about it

- `PHI PLACEHOLDER SWAP` — retired name; the cleanup line above replaced it.
- `PHI TERRAIN` — belongs to the world builder's own path, not to this sweep.

Both are listed as silent by design; only the placeholder verdict decides PASS.

## Third loop: GitHub tested first, Copilot opened, zone geography fixed

### GitHub, tested exactly as Tyree wrote it (HTTPS, public, no token, no SSH)

```
git ls-remote https://github.com/tyofthestars11-lab/veil-omni-unreal.git HEAD
  22532a2d10e5b4eef03855786ffa2164e36ec6df    HEAD        exit 0
git clone ... -> clone exit 0
git status   -> On branch main, up to date with 'origin/main', working tree clean
git log      -> 22532a2 Re-seat Cowork Unreal files to Public/Private layout, add Countess health hook
```

So **GitHub is connected**. What the repo contains, measured: **18 files, 178 KB**, laid out as
`Private/`, `Public/`, `README_RESEAT.md` — **no `.uproject`, no `Source/`, no `Content/`, no map.**
It is the Cowork/Copilot re-seat, and its own README says "not compiled or tested here".

Local repo: branch `main`, HEAD `cef0590`, **0 remotes before this pass**, 400 changed/untracked
entries. `origin` was added (`https://github.com/tyofthestars11-lab/veil-omni-unreal.git`);
**nothing was pushed** — `git merge-base` between `cef0590` and `22532a2` returns nothing, so the two
histories are unrelated and reconciling them is Tyree's call, not a silent force-push.

### GitHub Copilot CLI — opened, signed in, and it found a real bug

```
C:\Users\Admin\AppData\Local\GitHubCopilotCLI\copilot    v1.0.105
smoke test -> "COPILOT ONLINE"  exit 0
session in D:\VEIL-OMNI, model Copilot Fusion, "Loading: 1 instruction, 2 skills"
(.github/copilot-instructions.md is that instruction)
```

Asked to map the reference onto the Unreal source without guessing, it returned file:line evidence
(HUD selection `VEILGameMode.cpp:104-111`; drawing `VEILHudCanvas.cpp:335+`, minimap `661-665`,
centre-safe rule `1266-1269`, ONE/mic `1807-1863`; `VEILHudMockup.cpp` exists but is never called;
physics `VEILCharacter.cpp:58-96`, `2045-2053`, header `223-258`; zones `VEILWorldBuilder.cpp:3560-3690`)
and then the finding that mattered:

> zone classification does not match the reference geography — the subsystem centres Hub at
> `(900,0)`, Omni at `(600,-2100)` and Deep at `(0,0)`; the reference puts them at `(0,0)`,
> `(2800,500)` and `(0,-5000)`.

It edited no files. It ran `bash Tools/veil-cli.sh headless 60` itself, headless.

### The fix, and the proof

`Source/VEIL/Private/Systems/VEILRegionSubsystem.cpp` — the five centres now read:

| Zone | Before | Now (Tyree's reference) |
|---|---|---|
| Hub | `(900,0)` | **`(0,0)`** |
| Nest | `(-450,780)` | `(-450,780)` unchanged |
| Spire | `(0,3500)` | `(0,3500)` unchanged |
| Omni District | `(600,-2100)` | **`(2800,500)`** |
| Megastructure Deep | `(0,0)` | **`(0,-5000)`** |

Measured immediately after, same headless door:

```
before:  VEIL REGION -> The Nest at V(X=-484.01, Y=106.56, Z=425.59)     <- Hub labelled as Nest
after:   VEIL REGION: start = Hub at V(Y=-1500.00, Z=100.00)
Result: Succeeded
VERDICT: PASS - no placeholder stood at the end of the sweep.   verify exit 0
UnrealEditor.exe running: 0
```

### Open, and not claimed as fixed

- The region pass now starts the runner in **Hub**, but its reported start position is
  `(0,-1500,100)`, not the `(0,0,100)` the seat law states. `OnWorldBeginPlay` reads the pawn before
  the seat pass moves it. This is a real discrepancy and needs measuring, not explaining away.
- The runner then logs `VEIL REGION -> The Nest at V(301.76, 1506.65, 162.29)` — nearest centre maths
  agrees with that point, but whether the district geometry puts a Nest there is unverified.
- HUD conformance is partial evidence only: `VEILHudCanvas.cpp:375` holds `PW = 420.f` with the
  `420 x 110` spec written above it; physics holds `JumpZVelocity = sqrt(2*980*185) = 602.2` for a
  185 cm rise, `GravityScale = 1`, `MaxWalkSpeed = WalkSpeed`. The reference's 439 / 750 / −50 and
  the 180×180 minimap were not each re-measured in the frame this pass.

### Rules held in this loop

- GitHub tested before anything else, exactly as written: HTTPS, public, no token, no SSH
- Copilot was the first agent to touch the question, and it ran headless
- no Unreal editor window: `UnrealEditor.exe running: 0`
- no cook, no package, no exe/apk/obb
- no push, no force, no history rewrite

## Fourth loop: Copilot reached directly, and the seat stops lying

### The direct Copilot door, found by checking the logs

`~/.copilot/` holds `chats/`, `logs/`, `agents/`, `mcp-config.json`. The direct, non-interactive door is:

```
copilot -p "<task>" --trust-folder --allow-all-paths      # exits after completion
```

No PTY, no permission prompt to race. First direct call, exit 0, 1m 6s, 1.01 credits.

The project's own MCP wiring (`.mcp.json`, `.codex/config.toml`, `.cursor/mcp.json`, `.vscode/mcp.json`,
`.gemini/settings.json`) all point at one server: **`unreal-mcp`, http://127.0.0.1:8000/mcp** — the
Unreal editor bridge, not Copilot. Measured this pass: `port 8000: CLOSED (timed out)`,
`initialize: WinError 10061`. It only exists while the editor is open, which is the window Tyree
forbade, so the bridge is correctly idle.

### What Copilot found in one direct call

The seat law `VEILProgression.h:21` says `RunnerX=0, RunnerY=0, RunnerZ=100`. Six sites seeded the
runner at **Y = -1500**, and the log beside one of them printed `V(0,0,100)` anyway:

| File:line | Before |
|---|---|
| `VEILGameMode.cpp:3086` | `Seat = FVector(0.f, -1500.f, 100.f);` |
| `VEILGameMode.cpp:3112` | `Authority(..., FVector(0.f, -1500.f, 100.f))` |
| `VEILGameMode.cpp:5132` | respawn `SetActorLocation(FVector(0.f, -1500.f, 100.f))` |
| `VEILCharacter.cpp:598` | authority seat, log says V(0,0,100) |
| `VEILCharacter.cpp:606` | spawn-column clear centred on (0,-1500) |
| `VEILCharacter.cpp:1518` | ground-stand seat `(0,-1500,0)` |
| `VEILCharacter.cpp:1642` | boxed-in rescue `(0,-1500,100)` |

All seven now read `VEILAnchors`, and the log prints the numbers it uses. The floor is safe:
nine `SM_Floor1_Big` tiles, step 8000, 80 m each — `(0,0)` is inside the centre tile.

Measured, same headless door:

```
PHI SEAT LAW: runner spawns on V(0,0,100) over PHI GROUND, not on PlayerStart_0.
PHI RUNNER SPAWN: VEILCharacter at V(Z=100.00) (open ground ahead 235 cm; 261 shell(s) ignored)
VEIL REGION: start = Hub at V(Z=100.00)
VERDICT: PASS - no placeholder stood at the end of the sweep.     verify exit 0
```

Two honest notes: a 45 s headless window returns `INCOMPLETE - 2 of 10 passes spoke` because the boot
plus sweep now needs ~75 s (75 s run: PASS, exit 0); and four `PHI UNSTICK` lines still fire, whose
positions are recorded above rather than argued away. `-1500` remains in plaza geometry and pad
coordinates (`VEILGameMode.cpp:1113, 2067, 2208`, `VEILCharacter.cpp:667-668`) — those are scenery
addresses, not the runner's seat, and were deliberately left alone.

### Pushed

`cef0590..3efdfa9` then this seat fix, branch `veil-whole`, with `main` untouched.

## Fifth loop: Copilot writing in the side terminal, and a dead path caught by measurement

### How it was reached

Typing `copilot` in the side terminal is the whole connection: it opens in `D:\VEIL-OMNI`, loads
`.github/copilot-instructions.md` as its one instruction, runs `Copilot Fusion` (it selected
`GPT-6 Luna`), and asks for approval before touching anything
(`Approve read-only inspection` / edits). No wrapper, no flags needed.

### What Copilot wrote

Asked to stop planting a Spire marker at the Hub (`vision_whole.json` carries `"Spire": [0,0,500]`
while the tower is built at `(0,3500)`), it added to `VEILWorldBuilder.cpp` (25 insertions, 3 deletions):

- `IsSpireAtHub(Name, X, Y)` — true only for `Name == "Spire" && X == 0 && Y == 0`
- the guard in **both** authority-block paths, with `PHI AUTHORITY: skipped Spire block at Hub centre`,
- `AuthorityPlaced` now counts what was actually placed instead of `AuthorityCount`.

Build `Succeeded`; headless run: `PHI SEAT LAW: runner spawns on V(0,0,100)`, `PHI UNSTICK` **0**,
`VERDICT: PASS` (exit 0). Pushed as `8f0488c` on `veil-whole`.

### The measurement that changes the conclusion

The skip never fired, and the reason is not the guard:

```
grep -rn "BuildPhiWhole" Source/VEIL/   ->  only the definition in VEILWorldBuilder.cpp
GameMode calls Builder->BuildWorld() at VEILGameMode.cpp:1763 and :1771
log: no "PHI AUTHORITY", no "address(es) placed", no "PHI ZONE DRESS", no "PHI VISTA"
```

**`BuildPhiWhole()` is never called.** The authority block that carries the Spire at `[0,0,500]` is
dead code, so the doubled Spire does not exist in the running world — the live Spire is the tower at
`(0,3500)`, which matches the reference. Copilot's edit is correct and harmless, and it proves nothing
about the live district. Ghost work, caught because the log was read instead of the diff.

Physics measured in the same frame, for the record: `gravity -980, walk 439, sprint 750,
leap 602 cm/s = 185 cm, glide 600 with drop -50, capsule r 42 half-height 96`.
