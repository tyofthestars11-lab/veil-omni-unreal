# Copilot instructions — VEIL OMNI: THE LAST SIGNAL, PATH OF LIGHT

This file is the standing prompt for any Copilot session in this repository. Every rule below is
taken from Tyree's own words, already recorded in this repo — not invented here.

Sources: `Docs/TYREE-INTENT-LEDGER.md`, `Docs/PHI-PHASE-LOCK-TYREE-LINKS-JARVIS-ONE-FIX.md`,
`Docs/AI_HANDOFF.md`, `Docs/VEIL-CLI.md`, `Docs/VEIL-CODE-LOOP-2026-11-04.md`.

## 1. Never open the Unreal editor window

Work in code and through the project's own headless door only:

```
bash Tools/veil-cli.sh build              # compiles VEILEditor - the binary the editor loads
bash Tools/veil-cli.sh headless 60        # runs the real game offscreen, nothing on screen
bash Tools/veil-cli.sh verify             # PASS/FAIL from the game's own log, exit 0 = pass
bash Tools/veil-cli.sh bake Tools/x.py    # runs a project Python script headless
```

`Tools/veil_copilot.py` couples to the editor's MCP hand on `127.0.0.1:8000`; that requires
`UnrealEditor.exe` running, so it counts as opening the editor. Do not launch it unless Tyree says
that window is allowed in that session.

## 2. Build the target that is actually loaded

`VEILEditor`, never `VEIL`. Building `VEIL` prints `Result: Succeeded` and changes nothing on
screen — recorded in `Docs/AI_HANDOFF.md` as a trap that already burned runs.

## 3. Phi is fixed, never guessed

- `phi = 1.618033988749895`, `phi^2 = phi + 1 = 2.618033988749895`
- golden angle `137.5077640500378`, golden pitch `17.0324` (about RightVector, cos 0.9561, sin 0.2929)
- tetrahedral `109.471221`, `c = 299792458`, `rung = ln(v)/ln(phi)` round-tripped, seed `1618`
- no Cartesian grid under the plan, no random placement

## 4. Authority coordinates are law

RUNNER `0,0,100` · Keeper Sera Vahl `900,0` · Seer `-450,780` · Cartographer `-4150,-780` ·
Cache `0,-6200` · Vault `0,0,-80.9` · Countess `809,-309,64.7` · Spire `0,3500,500` ·
Nest `-450,780` · Omni District `2800,500` · Megastructure Deep `0,-5000` ·
Beacons `2800,500`, `-2000,-900`, `600,-2100` · map `/Game/Maps/VEILDistrict` · save `VEILOMNI_Omni0`

## 5. Hard stops

- **No cook, no package, no exe, no apk, no obb** without his explicit word.
- **Never delete `Content/` or `Saved/`.** `Saved/Backup/VEILDistrict_pre-sweep.umap` is the restore point.
- Dialogue is `[E]` only. One mic only. HUD is a gold ribbon; the centre 60% stays clear.

## 6. Never fake a read

Only report what a tool actually returned. If a tool is not connected, say exactly:

```
TOOL NOT CONNECTED, NO REAL READ
```

then stop. Do not invent actors, counts, file paths, configs, screenshots or keys.

## 7. How "done" is measured

`bash Tools/veil-cli.sh verify` must exit **0**. The decisive line is the placeholder pass in
`AVEILGameMode::BeginPlay`, which logs:

```
PHI ENGINE SHAPE CLEANUP: <n> engine-shape component(s) re-dressed with kit meshes, 0 refused,
  0 retired as dust, 0 visible /Engine/BasicShapes component(s) remain - verdict NO PLACEHOLDER IN THE FRAME.
```

Two passes are legitimately silent: `PHI PLACEHOLDER SWAP` (retired name) and `PHI TERRAIN` (runs in
the world builder's own path). Silence is not failure; only the placeholder verdict decides.

A green build is not progress. A frame the owner can see, or a verdict line the game printed, is.
