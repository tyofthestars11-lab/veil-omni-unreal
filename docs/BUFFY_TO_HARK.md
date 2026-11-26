# BUFFY_TO_HARK

Buffy writes here (newest at the bottom, with time). Hark reads it when Tyree says "check Buffy".


## 2026-11-05 09:32 MST — GO 1/5: photo defects closed

**GPU:** NVIDIA GeForce RTX 3050, 6144 MiB, driver 617.42, CUDA 13.0. ACE-Step base mode
fits in 6 GB; LoRA training will have to be small-batch.

**Result:** `Result: Succeeded` (UE 5.8, VEILOMNIEditor Win64 Development).
**Log:** `SNAP=0 PY=15 CEF=0` (no CEF/ssl handshake spam; the 15 LogPython errors are
Epic's own EditorToolset/ToolsetRegistry plugins, not project code).

### What was actually wrong

1. **The Runner was one centimetre tall.** `Reach(0.5)` is a phi power (1.27), not a
   height, so the body was scaled to ~1 cm and read as "mesh none" in every frame.
   Now `100 * phi^1.5` = 206 cm, and the proof log reads the real `Avatar` component
   instead of `ACharacter`'s empty default `Mesh`:
   `VEIL PROOF BODY: Avatar mesh SK_Garret_FullBody | size 105 x 105 x 205 | visible 1`.

2. **The camera actor never framed him.** Record frames spawned a detached
   `ACameraActor`, which the PlayerController's auto view-target kept discarding.
   All five record frames now shoot through the Runner's own over-the-shoulder rig,
   the same one the proof frame uses (`VEIL OTS EYE: ... on the Runner's own rig` x5).

3. **Every frame is inside the lit Hub.** The old anchors stood the Runner inside mesa
   rock at Spire/Nest/Omni (Z 3251 / 592 / 3244), which is why those frames came out
   top-down with black voids. All five now stand on the Hub deck and look across the
   plaza: `VEIL_1_Hub`, `VEIL_2_HubNorth`, `VEIL_3_HubEast`, `VEIL_4_HubWest`, `VEIL_5_Deep`.

4. **The palette was being double-encoded.** Canvas takes *linear* colour and the
   display re-encodes it, so `#C9A24A` typed as 0.788 landed on screen as `#E6D193` —
   the yellow/orange Hark kept seeing. Every constant is now declared as sRGB and
   converted once. Measured off the proof frame: the quest bar edge is exactly
   `(201, 162, 74)` = `#C9A24A`, not `#E6D193`.

5. **The line under the quest bar is gone.** The banner keeps its crown and its slanted
   ends; nothing is drawn beneath the words any more.

### Files
- `Source/VEILOMNI/VeilRunner.cpp` — real 206 cm height.
- `Source/VEILOMNI/VeilGameMode.cpp` — Hub-only anchors, Runner's own rig for every
  frame, proof log reads the Avatar, Runner spawned for any frame without a pawnowner.
- `Source/VEILOMNI/VeilHud.cpp` — sRGB palette via hex, banner bottom line removed.

### Shots
`Saved/Screenshots/WindowsEditor/`: `VEIL_1_Hub.png`, `VEIL_2_HubNorth.png`,
`VEIL_3_HubEast.png`, `VEIL_4_HubWest.png`, `VEIL_5_Deep.png`, `VEILOMNI_proof.png`.
Downscaled copies: `shots/proof.png`, `shots/z_*.png`.

**Next:** GO 2/5 — HUD v2 (vitals plate, phi star badge, compass, 8-slot hotbar,
damage numbers, 4 s fade), then Kokoro voices, ACE-Step theme, M2-05 loop.


## 2026-11-05 09:39 MST — GO 2/5: the Muse pawn is in, Default Pawn switched

**Result:** `Result: Succeeded` (after two link fixes: `EnhancedInput`, then `UMG`).
**Log:** `SNAP=0 PY=15 CEF=0`. Camera loop stopped as instructed at 09:40.

### Canon this implements
- Hark 09:40: "Use Muse's version, not the old Runner... Make the Muse player the
  Default Pawn class... Retire the old VeilRunner from Default Pawn (keep the file)."
- Hark 09:16 Kraken physics: walk 439, sprint 750, jump 185 cm (JumpZVelocity 600,
  which is exactly sqrt(2 * 980 * 185)), glide 600, gravity -980.

### Ported into Source/VEILOMNI (logic and values unchanged, macro renamed)
- `VEILCountessCharacter.h/.cpp` — the Muse player pawn: ATP engine, dash 12 /
  attack 18 / interact 8, double jump, auto-glide after a running leap, camera law
  (spring arm 420 default, clamp 0..420, golden pitch -17.0324, V/R3 420<->0),
  widget-only pause (no `SetPause`), health + `TakeDamage` hook.
- `PHI_MitochondrialEngine.h/.cpp` — ATP 100, NDna 45.482, golden pitch 17.0324,
  equilibrium -14.438, echo gap 25.293, coherence -62.160.
- `VEILInteractable.h` — the `Interact` interface (`VEIL_API` -> `VEILOMNI_API`).

### Glue changed
- `VEILOMNI.Build.cs`: + `EnhancedInput`, `UMG`.
- `VeilGameMode.cpp`: `DefaultPawnClass = AVEILCountessCharacter`; the record and
  proof frames now spawn the Muse Countess, not the old Runner. `VeilRunner.cpp/.h`
  stay on disk, untouched.
- `VeilPlayerController.cpp`: quest distance reads any possessed pawn, so the Keeper
  check works with the Muse pawn.
- Muse code assigns no mesh (README: BP_Countess was to carry SM_Countess, which does
  not exist here). The pawn now wears the real Polyart body
  `/Game/Polyart/ModularStylizedChars/Meshes/SkeletalMeshes/Characters/SK_Garret_FullBody`,
  so it is a real skeletal character, not an empty capsule.

### Evidence from the run
```
VEIL PAD PROOF: Enhanced Input path active (Move=0 Look=0 Jump=0 ...)
VEIL PAD PROOF: legacy axis fallback (no Enhanced Move/Look bound)
VEIL LivingClock: world ticking 21.0s / 26.0s / 31.1s
VEIL PROOF BODY: CharacterMesh0 mesh SK_Garret_FullBody | size 121 x 121 x 202 | visible 1
```
Enhanced Input actions are unassigned in code (no IA_* assets exist yet), so the pawn
falls back to the legacy axes, which `Config/DefaultInput.ini` does map (W/S, mouse).

### Shots
`Saved/Screenshots/WindowsEditor/VEILOMNI_proof.png` and the five frame PNGs;
downscaled `shots/m_*.png`.

**Next:** HUD v2 (gold ribbon, curved compass, animated vitals, ONE mic, quest
tracker, tutorial hints), per Hark 09:28 and 09:10.


## 2026-11-05 09:53 MST — GO 2/5: HUD v2, built to VEIL_concept_hud

**Task:** HUD v2. **Build:** `Result: Succeeded` (VEILOMNIEditor Win64 Development).
**Log:** `SNAP=0 PY=15 CEF=0`.
**PNG:** `Saved/Screenshots/WindowsEditor/VEILOMNI_proof.png` (also `VEIL_1_Hub.png` to
`VEIL_5_Deep.png`; downscaled copies `shots/v2_*.png`).

### Canon quoted
- 09:55: "Top-left: a large navy glass panel wrapped in a flowing, glowing gold ribbon
  (curved, animated sparkle). Inside: a gold diamond with a phi glyph, LV below, then
  VIT and STM bars with labels left and values right. VIT bar #3FA9FF, STM bar gold."
- 09:55: "Top-right: an ornate circular minimap compass ... gold N/E/S/W ... a gold
  arrow for the player ... Round pause button above it on the right."
- 09:55: "Bottom-left: an angled navy banner with a gold scroll edge ... QUEST: THE
  GOLDEN MIDST ... objective line."
- 09:55: "Bottom-center: four diamond ability icons on a thin gold line with counts
  below (5, 3, 2, 2)." plus 10:00: "The phi diamond on the HUD hotbar is the time skill slot."
- 09:55: "Bottom-right: an ornate gold-framed COMBAT MODE plaque (the dot is blue)."
- 09:10 / 09:35: fade to 30 percent out of combat after 4 s, debug on F10 only, ONE mic.

### Files changed
- `Source/VEILOMNI/VeilHud.h` — v2 declarations plus living state (fade, ghost bars,
  damage numbers, F10 flag).
- `Source/VEILOMNI/VeilHud.cpp` — full v2 rewrite in Canvas: glass plate and tapered
  ribbon with a travelling sparkle; rotating phi star inside the gold diamond with a
  blue XP edge; VIT / STM / ATP bars with white ghost bars that hold 0.5 s; circular
  compass drawn from real static-mesh geometry with 21 golden-angle ticks, blue fog
  rim, gold player arrow, objective diamonds and the pause button; angled quest
  banner; four diamond abilities with counts 5/3/2/2 and the glowing phi time slot;
  the ONE mic button; COMBAT MODE plaque; dialogue and [E] prompt; damage numbers
  white with blue bloom; whole HUD fades to 30 percent after 4 s; F10 toggles the
  only debug text.

### Measured off the frame (sampled, not claimed)
- `#C9A24A` = 2,859 px, `#3FA9FF` = 1,638 px, yellow-ish = 8 px.
- No red constants remain in `VeilHud.cpp` (grep: 0). The red pixels on the frame trace
  to the Polyart character textures (NPC outfits), not to the HUD.

### Two layout decisions, stated
- The art (09:55) shows four ability diamonds while 09:10 said eight. I followed the
  concept art, because 09:55 is later and says "build it exactly".
- 09:10 put the compass top-centre; the concept art puts the round compass top-right.
  I followed the art.

**Next:** GO 3/5 — the over-the-shoulder camera with the Runner in frame is already
proven in these PNGs; then Sera Vahl's tutorial (WASD, Shift, Space, [E], glide, mic;
hints 1.618 s apart, progress stored in the save).


## Recycle Bin recovery (Hark 09:55 / task 5) — copy only, nothing deleted

97 items in the bin. Copied every VEIL/OMNI/Muse/phi/HUD/Kraken/Bible/vision file
whose type is html/pdf/png/jpg/webp/txt/py/json/md/h. Skipped two 24 GB archive
folders (`VEIL-OMNI-TYREE`, `VEIL-OMNI-ARCHIVE`), .zip/.bat/.lnk and the D:\OMNI-UE
folder shells. **49 files recovered** into `docs\concept
ecovered\`:

- `BUFFY-COMMAND-PROMPT-ENTIRE-GAME.txt` — 3.0 KB
- `BUFFY-DLAM-EDITOR-PROMPT.txt` — 2.4 KB
- `BUFFY-PHASE-LOCK-TYREE-LINKS-JARVIS-ONE-FIX.txt` — 6.0 KB
- `Fix-VEIL-Whole-Muse.py` — 51.5 KB
- `VEIL-Jarvis-Setup.txt` — 1.8 KB
- `VEIL-OMNI-ALL-IMAGES-SHOWN.html` — 1973.0 KB
- `VEIL-OMNI-ALL-IN-ONE.html` — 17586.0 KB
- `VEIL-OMNI-ALL.py` — 4.5 KB
- `VEIL-OMNI-ENTIRE.py` — 4.3 KB
- `VEIL-OMNI-ENTIRE__1_.py` — 4.3 KB
- `VEIL-OMNI-ENTIRE__2_.py` — 4.5 KB
- `VEIL-OMNI-FORM-CODE.py` — 1.6 KB
- `VEIL-OMNI-GODMODE-ALL.html` — 736.1 KB
- `VEIL-OMNI-HUGE-KRAKEN.html` — 898.1 KB
- `VEIL-OMNI-HUGE-KRAKEN__1_.html` — 898.1 KB
- `VEIL-OMNI-HUGE.html` — 7824.8 KB
- `VEIL-OMNI-HUGE.py` — 12.8 KB
- `VEIL-OMNI-JARVIS-MODE.html` — 738.9 KB
- `VEIL-OMNI-ONE-WHOLE.py` — 5.5 KB
- `VEIL-OMNI-PHI-PROOF-KRAKEN.pdf` — 2269.5 KB
- `VEIL-OMNI-PHI-PROOF-KRAKEN__1_.pdf` — 2269.5 KB
- `VEIL-OMNI-PHI-PROOF.pdf` — 8592.2 KB
- `VEIL-OMNI-PHYSICS-BLUEPRINTS.py` — 3.0 KB
- `VEIL-OMNI-START-MENU-ALL-ITEMS-50-IN-1.html` — 734.1 KB
- `VEIL-OMNI-VISION-GAME.py` — 5.2 KB
- `VEIL-OMNI-WHOLE.py` — 3.3 KB
- `VEIL-OMNI-World.png` — 81.8 KB
- `VEIL-PYTHON-HELPS-MAKE-VEIL.py` — 2.8 KB
- `VEIL-Whole-Phi-Intent.py` — 4.4 KB
- `VEILOmniCore.h.h` — 1.4 KB
- `VEIL_Bible_429_6fn0.pdf` — 184.6 KB
- `World-Gen-Simulation-Phi-Whole.py` — 3.7 KB
- `buffy-check-everything-not-random-2026-11-02.md` — 3.9 KB
- `buffy-gold-column-collision-fix-2026-11-02.md` — 1.7 KB
- `buffy-nest-cleanup-hark-2026-11-02.md` — 2.3 KB
- `buffy-nest-hub-cleanup-2026-11-02.md` — 3.5 KB
- `media-generation-all-in-one-big-image-veil-omni-0-0c1471d9-00b9-48de-b783-e7ba4482c656.webp` — 991.3 KB
- `media-generation-generate-my-open-world-vision-0-02e837a0-9776-4b2a-a410-fe37d5e67b00.webp` — 856.6 KB
- `media-generation-generate-what-you-envision-wit-0-583cb093-d8f5-4d40-80f4-fa392158daaa.webp` — 751.0 KB
- `media-generation-render-more-veil-omni-world-hu-0-e84afab4-3da5-4b0b-be77-2a49e248e39d.webp` — 930.2 KB
- `media-generation-render-more-veil-omni-world-me-0-e82ae349-75d4-4098-998a-13ced9b29481.webp` — 711.6 KB
- `media-generation-render-more-veil-omni-world-ne-0-ddbca13f-b1e9-4d9d-888f-1493ce8817b8.webp` — 843.9 KB
- `media-generation-veil-omni-different-races-game-0-69456f94-8514-449f-a514-f46099714319.webp` — 586.2 KB
- `media-generation-veil-omni-game-hud-and-menus-w-0-d2ce4a18-5f6f-4bf4-a355-cc2606987a1c.webp` — 573.3 KB
- `media-generation-veil-omni-game-menu-options-wh-0-edb89e13-b78f-482e-a211-25b09780c657.webp` — 650.9 KB
- `media-generation-veil-omni-game-menus-inventory-0-f1d3563d-1114-425e-ac71-35ba4a78cf1a.webp` — 669.4 KB
- `media-generation-veil-omni-game-races-character-0-a53329ca-2c59-4c75-84b9-cd5d86d2482e.webp` — 675.3 KB
- `media-generation-veil-omni-races-individual-por-0-b49fe784-335f-4381-b4d3-ff6dc6b446ba.webp` — 653.8 KB
- `tyree-phi-framework-entire-2026-10-29.pdf` — 207.9 KB

Muse images recovered (13): `VEIL-OMNI-World.png`, `media-generation-all-in-one-big-image-veil-omni-0-0c1471d9-00b9-48de-b783-e7ba4482c656.webp`, `media-generation-generate-my-open-world-vision-0-02e837a0-9776-4b2a-a410-fe37d5e67b00.webp`, `media-generation-generate-what-you-envision-wit-0-583cb093-d8f5-4d40-80f4-fa392158daaa.webp`, `media-generation-render-more-veil-omni-world-hu-0-e84afab4-3da5-4b0b-be77-2a49e248e39d.webp`, `media-generation-render-more-veil-omni-world-me-0-e82ae349-75d4-4098-998a-13ced9b29481.webp`, `media-generation-render-more-veil-omni-world-ne-0-ddbca13f-b1e9-4d9d-888f-1493ce8817b8.webp`, `media-generation-veil-omni-different-races-game-0-69456f94-8514-449f-a514-f46099714319.webp`, `media-generation-veil-omni-game-hud-and-menus-w-0-d2ce4a18-5f6f-4bf4-a355-cc2606987a1c.webp`, `media-generation-veil-omni-game-menu-options-wh-0-edb89e13-b78f-482e-a211-25b09780c657.webp`, `media-generation-veil-omni-game-menus-inventory-0-f1d3563d-1114-425e-ac71-35ba4a78cf1a.webp`, `media-generation-veil-omni-game-races-character-0-a53329ca-2c59-4c75-84b9-cd5d86d2482e.webp`, `media-generation-veil-omni-races-individual-por-0-b49fe784-335f-4381-b4d3-ff6dc6b446ba.webp`.

Nothing in the Recycle Bin was emptied, moved or deleted. Hark can read these directly.


---

## 2026-11-05 - HER RUNNER + PHI QUANTUM ECHOES REVERSAL MATRIX

**Task:** Hark 09:52/09:53 - her Runner instead of Garret, hood + cape + gold phi wings; plus Tyree's phi quantum echoes reversal matrix as the anti-random rule.

**Files:** `Source/VEILOMNI/VEILCountessCharacter.h/.cpp`, `Source/VEILOMNI/VeilGameMode.cpp` (DefaultPawnClass), `Source/VEILOMNI/VeilHud.cpp`.

**Result:** Succeeded (`Build.bat VEILOMNIEditor Win64 Development`).
**Counts:** SNAP=0 PY=15 CEF=0.
**Proof lines:** `VEIL PROOF BODY: CharacterMesh0 mesh SK_Elara_FullBody | size 103 x 103 x 188 | visible 1` and `VEIL PROOF BODY: RunnerHood mesh SK_FemaleHood | size 46 x 46 x 26 | visible 1`.
**PNG:** `Saved/Screenshots/WindowsEditor/VEILOMNI_proof.png` (copy: `shots/w3_VEILOMNI_proof.png`).

### What changed
- **Body:** Polyart `SK_Elara_FullBody`, every material slot navy `M_VEIL_Deep`. Garret is gone from the module (source refs: 0).
- **Hood:** Polyart `SK_FemaleHood` driven by the body skeleton through master pose, navy.
- **Cape:** navy plane off the shoulders at 97 deg so it drapes instead of standing out like a board.
- **Wings:** 13 blades per wing, sizes in phi steps (`phi^(i*0.12)`), 137.507764 deg between blades, folded 18 deg on the ground, opened 80 deg in a glide, gold `M_VEIL_Gold`, radiating from the shoulder instead of stacking at one point.
- **Phi quantum echoes reversal matrix:** `PhiEcho(i) = sin(i * 137.507764 deg)`, `PhiReversal(i)` reverses the sign every other step. The only unseeded random in the module (HUD damage-number jitter) now uses it, so the same input always draws the same frame. `grep -rn "FMath::Rand|FRandRange" Source/VEILOMNI` now returns nothing.

### Root cause of the two broken builds
The append script died on `%` formatting inside `Printf` before it wrote the file, so `BuildRunnerLook`/`PoseWingsForShot` were declared but never defined (`LNK2019`) and the ctor edit (Garret -> Elara) was lost with it. The pawn therefore stayed Garret while the log still reported `SK_Garret_FullBody`. Fix: append with string replacement, never `%` formatting; then set the mesh path in the ctor and call `BuildRunnerLook()` right after `SetSkeletalMeshAsset`.

### Still open
- Task 2 (Hub rebuild from `docs/canon/VEIL-OMNI-World.json.txt`) not started.
- Hark 09:52 items 1, 3, 4: CEF spam is gone in the run (CEF=0), minimap still blocky gold, left shadows still dark.
- `LogPython: Error` = 15 (Epic EditorToolset/ToolsetRegistry) unchanged, per Hark not chased.


## 2026-11-05 - REAL WING GEOMETRY (procedural mesh), not scaled cubes

**What Hark asked (09:53):** a gold phi wing pair built as geometry, feathers at the golden angle 137.508 deg, 13 per wing, sized in phi steps, on a back root, folded on the ground and opened during glide.

**Files:** new `Source/VEILOMNI/VEILWingsComponent.h/.cpp`; `VEILCountessCharacter.h/.cpp`; `VEILOMNI.Build.cs` (+ProceduralMeshComponent), `VEILOMNI.uproject` (plugin enabled).

**Result:** Succeeded. Counts: SNAP=0 PY=15 CEF=0.
**PNG:** `Saved/Screenshots/WindowsEditor/VEILOMNI_proof.png` (copy `shots/ww_VEILOMNI_proof.png`), `VEIL_1_Hub.png` (`shots/ww_VEIL_1_Hub.png`).

### What is real now
- `UVEILWingsComponent` is a `UProceduralMeshComponent` that cuts 26 tapered feather quads: `Len = 24 * phi^(i*0.12)` per feather, golden angle used for the fan stagger, spread 0 folded and spread 1 open, rebuilt only when spread moves.
- The 26 scaled `Cube` feathers are gone from `BuildRunnerLook`. The pawn holds the component on her back and `UpdateWings` drives `SetSpread` from `bGliding` (or the proof override).
- Body is Polyart Elara, hood is Polyart `SK_FemaleHood` on her skeleton, cape is a navy plane. No Garret, no red on her.

### Build errors met and fixed (one at a time, each root-caused)
1. `UHT: Unable to find class UVEILWingsComponent` - UPROPERTY cannot reflect a type from a plugin-module header; the member is now a plain owned-component pointer.
2. `C1083 VEILWingsComponent.h not found` - the files had been written into the worktree instead of `C:/Users/Admin/HarkGame`; copied across.
3. `C2512 no default constructor` - `UProceduralMeshComponent` only has the `FObjectInitializer` form; subclass ctor now takes it.
4. `C4459 Phi hides global declaration` - engine Chaos headers own `Phi`; renamed ours `PhiRatio`.

### Honest limitation - wings are built but not yet seen
In both proof frames she reads as a dark hooded silhouette and the gold feather wings are **not visible**; a dark slab at her side is the cape plane. So the geometry exists and compiles, and the pawn drives it, but I have not proven it renders in frame. Next suspicion, in order: the component section is created in BeginPlay after the material was set, her mesh-space offset (-16, 0, 34) may sit inside her body, and the proof camera is behind but low. Nothing is claimed as done here until a PNG shows them.

### Still open
- Task 2, the Hub rebuild from `docs/canon/VEIL-OMNI-World.json.txt`, is untouched.
- Cape is still a plane; Hark wants it skinned to the spine with cloth.
- Hark 09:44 names Epic's free Game Animation Sample for motion-matching movement; it is not in Content yet (only Polyart locomotion clips: MF_Idle, MM_Walk_InPlace, MM_Run, MM_Jump, MM_Fall_Loop, MM_Land, BS_WalkRun).


## 2026-11-05 - UNUSED ASSETS WIRED: the Polyart locomotion set (42 anim assets, 6 used)

**Task (Tyree):** add every unused asset that matches the game or that the game needs to be whole.

**Files:** `Source/VEILOMNI/VEILCountessCharacter.h/.cpp` (locomotion state), `VEILWingsComponent.cpp` (instrumentation).

**Result:** Succeeded. Counts: SNAP=0 PY=15 CEF=0.
**PNG:** `Saved/Screenshots/WindowsEditor/*.png` (copies `shots/an_*.png`).

### Now used (were unread before this run)
`MF_Idle`, `MM_Walk_InPlace`, `MM_Run`, `MM_Jump`, `MM_Fall_Loop`, `MM_Land` - loaded from Polyart's own paths and chosen from real velocity: air up = jump, air down = fall, speed > 500 = run with rate clamped to speed/600, walk otherwise with rate speed/439, idle at rest. Log proves all six resolve: `VEIL ANIM SET: idle=1 walk=1 run=1 jump=1 fall=1 land=1`, and the state machine fires: `VEIL ANIM: MM_Fall_Loop at speed 0 rate 1.00` (she spawns falling, which also explains the odd proof framing).

### Wing instrumentation (the honest part)
`VEIL WINGS: verts=104 tris=312 sections=1 spread=0.00..0.68 registered=1` - the procedural wing geometry exists, is registered, and now actually opens (spread climbs during the shot). What the previous entries claimed without proof is now proven at the data level, and still **not** proven on screen: the frames still read her as a dark silhouette. The 26 static-mesh blades are equally invisible, so the shared cause is placement or material, not the mesh type.

### Still unused and needed
- `BS_WalkRun` (blend space), `ABP_FemaleCharacter` (animation blueprint), `ABP_FemaleHood`, `ABP_FemaleSkirt` - need an editor-authored anim blueprint to blend; a C++ state switch plays sequences without blending.
- `SK_FemaleSkirt` and the Polyart cloak/cape parts - candidate for the skinned cape Hark wants instead of a plane.
- `Sicka_EnvPack2` and `Iceland_Environment` are referenced by the town builder but not placed to canon; the Hub rebuild is still untouched.


## 2026-11-05 - GREY STATUE FIXED + FLUID LOCOMOTION VIA BS_WalkRun

**Task (Tyree):** the main char looks like a grey statue, not fluid; use the addons that make it feel like a game.

**Files:** `VEILCountessCharacter.h/.cpp`.

**Result:** Succeeded. Counts: SNAP=0 PY=15 CEF=0.
**Proof lines:** `VEIL ANIM SET: idle=1 walk=1 run=1 jump=1 fall=1 land=1`, `VEIL ANIM BLEND: BS_WalkRun loaded, position driven by speed`, `VEIL PROOF BODY: CharacterMesh0 mesh SK_Elara_FullBody | 78 x 78 x 185 | visible 1`.
**PNG:** `Saved/Screenshots/WindowsEditor/*.png` (copies `shots/bn_*.png`).

### Root cause of "grey statue"
`BuildRunnerLook` was doing `GetMesh()->SetMaterial(i, Navy)` on all eight slots, i.e. painting skin, face and clothes with one flat navy material. That is literally a grey statue. The override is deleted; her Polyart body materials stay, and navy is now only on the hood and cape.

### Fluidity
The unused `BS_WalkRun` blend space is now driven from C++: the mesh runs in `AnimationSingleNode`, the blend space is set as its asset, and the blend position is the real 2D speed (`min(speed, 600)`), so idle-walk-run interpolate continuously instead of hard-switching clips. Verified the API against the engine header on this machine (`SetBlendSpacePosition`, `SetAnimationAsset`), after two wrong guesses (`SetBlendSpace`, `SetBlendSpaceInput`).

### Fab
`LauncherInstalled.dat` exists and Epic is signed in on this PC, but Fab content still has to be added to the project by a click in the Epic Launcher / Fab library; nothing free can be pulled in headlessly. What Epic already installed here is only the engine plus what is in `Content` (Polyart, Sicka_EnvPack2, Iceland_Environment, VEIL). The free Fab packs that would help most for movement are Game Animation Sample (motion matching) and Niagara Examples (wing VFX); both need that one click before I can place their paths.

### Still open
- Wings: data-level proof only (`sections=1, verts=104, spread` climbs) - still not visible in frame.
- Hub rebuild from the canon JSON: not started.


## 2026-11-05 - HONEST COMPLETION AUDIT (Tyree: "not even 1 percent")

**Statement accepted.** Here is the measured state, not a claim.

### What exists (measured this run)
- `Source/VEILOMNI`: 28 files, ~4.6k lines total including backups. Playable code is roughly 2.6k lines: pawn, HUD canvas, player controller, game mode town builder, water, NPC, wings component, PHI constants.
- Content: Polyart (stylized characters + 42 anim assets), Sicka_EnvPack2, Iceland_Environment, VEIL (materials + one map). One playable map: `Lv_Veil_Town`.
- Working: one pawn (Polyart Elara + hood + procedural wings + point light), idle/walk/run blend via `BS_WalkRun`, jump/fall/land clips, canvas HUD v2, quest banner, a hand-built town, a proof-shot harness.
- Plugins enabled and verified: GeometryScripting, MeshModelingToolset, ChaosCloth, PCG, MotionWarping (+ Niagara, Water, MetaHuman, AnimationSharing already present). Build Succeeded, run SNAP=0 PY=15 CEF=0, plugin errors 0.

### What does not exist at all (Bible M1-M7 scope)
- No inventory, no item or class DataTables (09:55 canon asks for ARMOR/BELT/BOOTS/GLOVES/WARDEN slots).
- No enemies, no wave spawner, no combat beyond a damage number on the HUD.
- No quest system, no 11 authority coordinates, no beacons, no win state.
- No time-skill tree (Slow, Haste, Echo/Rewind, Stillpoint, Fibonacci Step) - only the HUD slot that would hold it.
- No world memory, no NPC schedules, no Ollama/Kokoro dialogue, no audio at all.
- No region streaming; the Hub is hand-placed boxes, not the canon layout.
- The wings, the one thing most visible, still do not render in frame.
- One map, one district; canon calls for 50 regions.

### Why "1 percent" is a fair number
Counts: 1 map of ~50 regions, 1 district of the canon Hub, 0 of 5 time skills, 0 of 11 authority coordinates, 0 of 6 item slots, 0 enemies, 0 quests, 0 audio, 0 saved-world memory. Systems that exist are scaffolding (HUD, pawn, town builder), not gameplay.

### The ordered plan from here (each step verified by build + PNG)
1. Wings render correctly (blocking every "she is the character" judgement).
2. Canon Hub via PCG from `VEIL-OMNI-World.json.txt`: 12 pillars, 8 houses, 5 crystals, river, bridge, Spire.
3. Inventory + item DataTables (empty schemas per 09:55, nothing invented).
4. Quest system + the 11 authority coordinates + beacons + win screen.
5. Enemies + wave spawner (3, 5, 8, 13, 21 + WARDEN PRIME per 09:40).
6. Time-skill tree on global/custom time dilation with the position-history rewind buffer.
7. World memory + NPC schedules + dialogue/voice.
8. Region streaming toward the 50 games.


## 2026-11-05 - WHOLE TEST ITEM 1: WINGS MEASURED, THEN SIZED

**Task:** item 1 of the Whole Test - her wings must exist, folded on the ground and open in glide.

**Files:** `VEILCountessCharacter.h/.cpp`, `VEILWingsComponent.cpp`.

**Result:** Succeeded. Counts: SNAP=0 PY=15 CEF=0.
**PNG:** `Saved/Screenshots/WindowsEditor/*.png` (copies `shots/big_*.png`).

### What the new placement log proved (before this change)
- `VEIL WING PLACE: loc=(-31.8, -31.8, 1219.7) scale=(1,1,1) mesh=(0,0,1071.7) capsule=(0,0,1159.7)`
- `VEIL WING BOUNDS: origin=(-12.2, -12.2, 1186.8) extent=(29.0, 29.0, 53.0) material=M_VEIL_Gold`

So the geometry, the registration and the material were never the problem. The wings measured **58 x 58 x 106 cm** - narrower than her shoulders - which is why they never read as wings on screen.

### Fix applied
- Attachment moved from mesh space to the capsule (`GetRootComponent`), 40 cm behind, 95 cm up, yawed 180 so the pair opens backwards.
- Component scale 1.8, feather base length 55 -> 78 cm in phi steps.
- New bounds: `extent=(72.0, 72.0, 135.2)` = **144 x 144 x 270 cm** of wing, gold material confirmed.
- Frame evidence: gold pixels in her half of the proof frame rose **12,355 -> 16,738**.

### Still open on item 1
Whether the wings now read correctly in the image is being verified frame by frame; the folded/open spread values in the log are 0.00 at rest and climb to 1.00 in glide. Items 2-8 of the Whole Test are untouched, and the Fab packs and Muse's recovered art are still waiting on Tyree.


## 2026-11-05 - WHOLE TEST ITEM 1: WINGS NOW FAN (golden angle divided across 13)

**Task:** item 1 - wings folded on the ground, opened in glide, gold phi feathers.

**Files:** `VEILWingsComponent.cpp`.

**Result:** Succeeded. Counts: SNAP=0 PY=15 CEF=0.
**PNG:** `Saved/Screenshots/WindowsEditor/*.png` (copies `shots/fan_*.png`).

### What changed and why
First time the wings rendered at all, they rendered as a starburst of needles: each blade was yawed by `i * 137.507764 deg` in 3D, so the pair scattered in every direction instead of fanning. The golden angle is now divided across the 13 feathers (`FanStep = 137.507764 / 13 = 10.5775 deg`), so the whole wing spans exactly the golden angle as a fan, feather lengths step in phi (`46 * phi^(i*0.06)`), and each blade is 30 percent as wide as it is long. Yaw only carries the fold-to-open spread (12 to 38 deg).

Frame evidence: gold pixels in her half of the proof frame 16,738 -> 14,424 with the fan (fewer, broader blades instead of many thin spikes), bounds `144 x 144 x 270 cm`, material `M_VEIL_Gold`.

### Honest status against the Whole Test
- Item 1: wings exist, are gold, fold at rest (spread 0.00) and open in glide (1.00) - but they are built from procedural quads anchored to the capsule, not a skinned wing on a spine socket as 09:53 asks. Visual quality is still short of the concept art's membrane-plus-gold-trim look.
- Items 2-8: untouched. Blended movement exists through BS_WalkRun in single-node mode, not an AnimBP as the Whole Test specifies.
- Waiting on Tyree: Game Animation Sample + Niagara Examples from Fab, and Muse's recovered race/area art.


## 2026-11-05 - FAB SESSION LIVE, COUNTESS PACK IN THE PROJECT

**Fab (eyes on the real UI):** the attached browser is signed in now. `Game Animation Sample by Epic Games` shows **"Saved in My Library"** on its listing page (`fab.com/listings/880e319a-a59e-4ed2-b268-b32dac7fa016`), UE 5.4-5.8, 500+ animations, motion matching. Fab's website has no download control - files only arrive through the Epic Games Launcher, which I cannot drive headlessly. So: owned, not yet on disk.

**What is on disk and now in the project:** the launcher's vault already held the account's downloads, and I copied `ParagonCountess` in - **2,727 assets, 1.8 GB**, keeping the `/Game/ParagonCountess/...` paths intact. The gold skin the target asks for is real:
`/Game/ParagonCountess/Characters/Heroes/Countess/Skins/Tier1/Count_Gold/Meshes/SM_CountessGilded`
plus `SM_Countess`, `S_Countess_Skeleton`, `Countess_AnimBlueprint`, `CountessPlayerCharacter` and a full `Animations/` folder.

Other library items already downloaded in the vault and available the same way: `ParagonGreystone` (2.2 GB), `ParagonMurdock` (2.1 GB), `ParagonRevenant` (1.7 GB), `ParagonMinions` (4.8 GB), SICKA pack, the modular character creator, mountain tops, lava material, underwater test.

**Verified:** build `Result: Succeeded`; run clean, log counters unchanged (SNAP 0, LogPython 15 from Epic's editor toolset, CEF 0).

**To get Game Animation Sample + Niagara Examples onto disk:** in the Epic Games Launcher open Fab -> Library, click Download on each; once they are in `VaultCache` I can copy them in exactly like the Countess, with no further clicks needed from Tyree.


## 2026-11-05 - REAL SPAWN BUG FIXED + GOLD COUNTESS AS THE NPC BODY

**Task:** use the Epic assets already on this PC; stop losing the pawn to a collision.

**Files:** `VEILCountessCharacter.cpp`, `VeilNpc.cpp`, `VEILOMNI.uproject`.

**Result:** Succeeded. Counts: SNAP=0 PY=15 CEF=0, `SpawnDefaultPawnAtTransform` failures **0** (was 2 per run).

### Two bugs the log finally named
1. `LogSpawn: Warning: SpawnActor failed because of collision at the spawn location [0,0,300] for [VEILCountessCharacter]` and `SpawnDefaultPawnAtTransform: Couldn't spawn Pawn`. The player pawn was being refused at the world origin, which is why proof frames kept showing odd framing and a stray second pawn. Fixed with `SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn` in her constructor. Verified: 0 failures this run.
2. `Unable to load Paragon_Proto_Retarget ... its class (Rig) does not exist`. Enabled `ControlRig` and `RigLogic` (plugin list now 43). Still 5 occurrences of that warning - the retarget rig needs an editor-side asset resave, so it is **not** resolved yet.

### Countess
`VeilNpc.cpp` spawned Polyart bodies by path. Its female and male body paths now point at the real gold Paragon mesh:
`/Game/ParagonCountess/Characters/Heroes/Countess/Skins/Tier1/Count_Gold/Meshes/SM_CountessGilded`
World builder log confirms placement: `VEIL WORLD OMNI: 12 blue block(s), 3 light pillars, Countess at (809, -309)`.

### Frame data
`VEIL PROOF BODY: CharacterMesh0 mesh SK_Elara_FullBody | 50 x 60 x 185 | visible 1`, `RunnerHood ... visible 1`, `VEIL WING BOUNDS extent=(47.4, 73.2, 83.2) material=M_VEIL_Gold`, `VEIL ANIM BLEND: BS_WalkRun loaded`.

### Honest gap
The Countess mesh is assigned to the NPC class, but the frames still do not prove a gold-armoured figure standing at her post - that needs a shot aimed at (809,-309). Motion matching is still unavailable: Game Animation Sample is in the Fab library but has not been downloaded by the launcher, so no files exist to copy.


## 2026-11-05 - FIVE EPIC PACKS IN THE PROJECT (real fantasy bodies, all free)

**Task:** add the real Epic assets from the account library instead of placeholders.

**What went in (copied from the launcher vault, user's own free library):**
| pack | assets | size |
|---|---|---|
| ParagonCountess (gold skin for the Countess) | 2,727 | 1.8 GB |
| ParagonMinions (enemy bodies) | 2,105 | 4.8 GB |
| ParagonGreystone (armoured hero) | 1,733 | 2.2 GB |
| ParagonMurdock (hero) | 1,896 | 2.1 GB |
| ParagonRevenant (hero) | 1,621 | 1.7 GB |
| **total** | **10,082** | **12.6 GB** |

Content root now: Iceland_Environment, ParagonCountess, ParagonGreystone, ParagonMinions, ParagonMurdock, ParagonRevenant, Polyart, Sicka_EnvPack2, VEIL. Free space 172 GB.

**Verified:** build `Result: Succeeded`; run counters recorded above; `VEIL WORLD OMNI: 12 blue block(s), 3 light pillars, Countess at (809, -309)`.

**What this unlocks:** ParagonMinions gives the wave spawner real enemy meshes (waves 3/5/8/13/21 per 09:40) instead of cubes; Greystone, Murdock and Revenant give grounded NPCs of different builds; CountessGilded is the gold NPC at her post. These are Epic's own assets, licensed free to this account.

**Still blocked on the launcher:** Game Animation Sample and Niagara Examples are in the Fab library but never downloaded, so there are no files to copy. Launcher -> Fab -> Library -> Download is the gate.


## 2026-11-05 - REMAINING FREE VAULT PACKS ADDED (Bible-driven choice)

**Rule followed:** only what the Bible needs - the Runner is a **male or female** choice, and the Hub needs **terraced cliffs**. So: the modular character creator (extra bodies/parts) and the Iceland mountain package (cliffs), not random props.

**Added this pass:** `StarterContent` (264 assets, 608 MB) plus the sample project's `__ExternalActors__` (41) and `__ExternalObjects__` (6) folders that came inside the same Modular pack. `Polyart` and `Iceland_Environment` were already present and were skipped, not overwritten.

**Content inventory now:**
| folder | assets | size |
|---|---|---|
| ParagonMinions | 2,105 | 4.8 GB |
| ParagonCountess | 2,727 | 1.8 GB |
| ParagonGreystone | 1,733 | 2.2 GB |
| ParagonMurdock | 1,896 | 2.1 GB |
| ParagonRevenant | 1,621 | 1.7 GB |
| Sicka_EnvPack2 | 567 | 3.2 GB |
| Polyart | 574 | 630 MB |
| StarterContent | 264 | 608 MB |
| Iceland_Environment | 78 | 1.2 GB |
| VEIL | 43 | 50 MB |
| **total** | **~11,600** | **~18 GB** |

**Risk flagged, not hidden:** the `__ExternalActors__` / `__ExternalObjects__` folders are One-File-Per-Actor data from someone else's sample project. They are not ours and could confuse this project's actor serialisation. I copied them by following the pack's own Content folder, which is exactly the kind of mistake I should not repeat; they are candidates for being moved aside (not deleted) once someone confirms nothing references them.

**Verified:** build `Result: Succeeded`; run counters above; `VEIL WORLD OMNI: 12 blue block(s), 3 light pillars, Countess at (809, -309)`.

**Bible-relevant next use:** ParagonMinions -> wave spawner (3/5/8/13/21 + WARDEN PRIME); Greystone/Murdock/Revenant -> grounded, non-red NPCs; StarterContent + Iceland -> Hub cliffs and terraces; Polyart + modular parts -> male/female Runner bodies.


### Correction, same session
Moved the foreign `__ExternalActors__` / `__ExternalObjects__` folders out of `Content` to `Saved/OFPABackup_20261105/` (moved, not deleted, per the no-delete rule). They were One-File-Per-Actor data from the Modular sample project and had no business in this project's Content root. Re-ran the game after the move: `VEIL WORLD OMNI: 12 blue block(s), 3 light pillars, Countess at (809, -309)` still builds the world.

## 2026-11-05 - FAB LIBRARY INVENTORY (answering the 11:52 FAB section)

**What I did, on this PC, with no guessing:** read the launcher's own Fab database
(`C:/ProgramData/Epic/EpicGamesLauncher/VaultCache/FabLibrary/listings_v1.db`) and wrote every
listing to `docs/FAB_LIBRARY.md`.

**Result: 53 items in the library, 61 acquisitions, 10 downloaded to disk.** Every listing in the
database is already owned, so none of the 53 needs claiming - they only need **Add to Project**
(or a copy from the vault, for the 10 already on disk).

### Already on disk, ready to copy straight into Content (no clicks needed)
| Item | Vault path |
|---|---|
| Paragon: Countess | `VaultCache/ParagonCountess/data/Content/ParagonCountess` |
| Paragon: Minions | `VaultCache/ParagonMinions/data/Content/ParagonMinions` |
| Paragon: Greystone | `VaultCache/ParagonGreystone/data/Content/ParagonGreystone` |
| Paragon: Revenant | `VaultCache/ParagonRevenant/data/Content/ParagonRevenant` |
| Mountain Tops (5.3-5.7) | `VaultCache/...` |
| Stylized Character Creator - Adventurer | `VaultCache/...` |
| Underwater test | `VaultCache/...` |
(Exact paths are in FAB_LIBRARY.md; six of these predate the launcher naming them in `local_listing`,
so they show as "(title only in launcher cache)" in the table - their folders are the 6 named above.)

### Free, already owned, directly useful to the Whole Test - Add to Project targets
- **Free Niagara Pack**, **[18] Free Spells Niagara**, **Fantasy Flower VFX (Free Sampler)** -> wing glow and phi echoes.
- **Free Animation Library**, **Free Animations Pack**, **Free Anims Mix Pack**, **Free Crawl Animation**, **Free Interaction Animation**, **[FREE] 2025 Trendy Dance Collection Mocap Pack** -> locomotion variety.
- **UNIBLOCKS FREE**, **Midgardr FREE**, **DOCKS VOL.2**, **City Park Environment LITE**, **Environment - Rock Collection 04**, **Fantasy Flower VFX** -> the phi Hub build.
- **StylizedWeather**, **Dynamic Sky & Light Manager**, **MW Landscape Auto Material**, **Time Warper Free** -> the 17.0324 degree golden-hour sun.

### Not in the account at all (so they must be claimed, and claiming is a UI action)
Game Animation Sample, Content Examples (Niagara), Paragon Serath, Aurora, Phase, Shinbi.
They are absent from every table of the launcher database, which is why nothing about them could be
downloaded or added by me.

### Honest limits of this pass
- Claiming a Fab listing requires pressing **Add to Library / Personal licence** on the listing page
  while signed in. I did not drive those clicks, so **nothing new was claimed** and
  **no new pack was added to VEILOMNI** in this pass.
- Nothing was bought, no payment screen was touched, no account setting changed.
- No pack over 20 GB was fetched; City Sample is not in the account.
- The project was not rebuilt or re-run for this pass, so the last proven run still stands:
  `SNAP=0 PY=15 CEF=0 PAWNFAIL=0`, `VEIL WORLD OMNI: 12 blue block(s), 3 light pillars, Countess at (809, -309)`.

### Fab storefront session - what I found by actually opening it
- Opened `https://www.fab.com/library` in the signed-in browser. It renders a My Library with **24 items**
  (Specular Placement Tool, Weather System, Prototyping Basic Assets, Money VFX, Loot Drops VFX,
  Free Dissolve Shader, WAFU/HomkaH audio packs, Safe Pawn, ...). No overlap with the launcher's 53.
- Tyree confirmed (asked directly) that this browser session is the **same Epic account** as the launcher.
  So the 24 vs 53 gap is Fab-web bookings vs legacy Marketplace bookings in the launcher, not two accounts.
- Tried Fab's search endpoint directly from the page to read listings compactly; `/i/search` returns HTML,
  not JSON, so no programmatic claim/price read is possible. Claiming and downloading are per-item UI clicks
  on the storefront, plus Add to Project inside the launcher - neither is drivable from this shell.
- Nothing claimed, nothing downloaded, nothing bought in this step. Free/Personal only, as instructed.

## 12:0x - FAB ACCOUNT CHECK + ADD-TO-PROJECT BLOCKER
- **Account check:** every acquisition in the launcher's Fab database belongs to one Epic account,
  `ce1557c9b4394f10933e9506099f2c16` (61 acquisitions / 53 listings). The per-row `user_uid` values that
  look like many accounts are the *sellers'* ids, not different buyers. Tyree confirmed the fab.com session
  is the same account, so the Launcher Fab Library (53) is the source of truth, as the 12:02 section says.
- **Downloaded this pass: 0 GB.** `VaultCache` newest entries are still FabLibrary, Sicka, ParagonMinions,
  ParagonRevenant - nothing new arrived, because "Add to Project" and the download button are Launcher UI
  actions and I cannot drive a native window from this shell.
- **Skipped, and why (step 3's filter):** Car Dealer, Car Paint, Drivable Mars Rover, Top-Down Car_Crashed,
  Military Low Poly, Realistic Asphalt, Ceramic/Stone Tiles, Scifi Material, Mounts And Vehicles, Vehicle Game,
  Car Dealer-style templates -> modern/vehicle/sci-fi, no fit for VEIL. Audio-only packs (WAFU, HomkaH,
  Pocket Keys) are kept as a maybe for SFX but not added.
- **Not done this pass, stated plainly:** no pack was added to VEILOMNI, no scratch project was created in
  C:\Users\Admin\Fab, nothing was migrated into /Game/VEIL/, there is no new build Result: line and no new
  -VeilShot PNG. Last proven run remains SNAP=0 PY=15 CEF=0 PAWNFAIL=0.

## 12:0x - STEP A DONE: THE 10 VAULT PACKS ARE IN Content
Verified by file count against the vault (vault vs project):
- Iceland_Environment 80/80, Polyart 578/578, StarterContent 267/267, ParagonCountess 2731/2731,
  ParagonGreystone 1735/1735, ParagonMinions 2107/2107, ParagonMurdock 1900/1900,
  ParagonRevenant 1623/1623, Sicka_EnvPack2 580/580, and **LavaMat 45/45 copied this pass**.
- Content now **11,690 files / 20 GB**; 161 GB free on C:. Folder names unchanged, nothing renamed into /Game/VEIL.
- `__ExternalActors__` (41) and `__ExternalObjects__` (6) from the ModularS sample were deliberately **not** copied:
  they are that sample project's level-actor deltas and were already relocated to Saved/OFPABackup_20261105 to
  stop them attaching to VEIL levels. Copying them back would recreate that bug.
- Nothing over 20 GB was requested; nothing was bought.

## 12:1x - BUILD LINE, AND THE MOUNTAIN TOPS / STYLIZED FINDING
**Build proof (step A):** log `/tmp/vb990.log` (*not* in the project - copy below), command:
`Build.bat VEILOMNIEditor Win64 Development -Project=C:\Users\Admin\HarkGame\VEILOMNI.uproject -WaitMutex`
Result line, verbatim:
```
Build exit: 0
Result: Succeeded
error C count: 0
```
**Mountain Tops and Stylized Adventurer are NOT in VaultCache.** Full inventory of the vault's pack folders:
`FREEMounc1f4bef6b908V1/Iceland_Environment`, `LavaMate692205dab066V1/LavaMat`,
`ModularS30eeb54ea3e4V1/{Polyart, StarterContent, __ExternalActors__, __ExternalObjects__}`,
`ParagonCountess`, `ParagonGreystone`, `ParagonMinions`, `ParagonMurdock`, `ParagonRevenant`,
`SickaEnv8e38c1c18a06V1/Sicka_EnvPack2`. That is 10 packs and they are all in `Content` already, file counts matched.
The launcher's `download_meta` table lists Mountain Tops and Stylized Adventurer as downloaded, but no files
exist under those paths any more - so they need a fresh **Download** click in the Launcher before anything can
be copied. Nothing was fabricated in their place.
**B started:** `pywinauto 0.6.9` installed via `pip install --user pywinauto`. No Blender on the machine yet
(`C:\Program Files\Blender Foundation` absent), so step C has no authoring tool installed.

## 12:1x - B BLOCKED AT STEP 1: THE LAUNCHER EXPOSES NO CONTROL TREE
- pywinauto 0.6.9 attaches: window `Epic Games Launcher`, handle 12259938.
- **UIA:** window found, children **0**, descendants **0**. Tree dumped to `docs/LAUNCHER_TREE.txt` (1 line).
- **win32 backend:** root class is **`UnrealWindow`** (the Launcher is a UE Slate app, not Electron/CEF),
  children **0**.
So there are no buttons named "Download" or "Add to Project" for a script to press. This is not a permissions
problem and not a missing package - Slate only publishes its accessibility tree when a screen reader / UIA
client is detected, and this build is not publishing it.
- **Unblock, unverified, next honest attempt:** enable Windows accessibility so Slate turns on its
  accessibility bridge (start Narrator, or set the UIA "accessibility active" flag), then re-dump the tree.
  If that still yields 0, the only remaining route is screenshot + coordinate clicking, which I will not do
  blind on a storefront that can show a payment dialog - one mis-click at the wrong coordinates is unrecoverable.
- Nothing downloaded, nothing added, no dialog was clicked, no money involved. Blender not installed (no time
  this turn; `winget install BlenderFoundation.Blender` is the command, ~350 MB).

## 12:1x - QUEUE RUN, ONE-LINE BLOCKERS
- **Narrator a11y try:** started narrator.exe, waited 6 s, re-probed: UIA descendants still **0**, win32 children **0**. BLOCKED - Slate still not publishing its tree.
- **winget Blender:** `winget install BlenderFoundation.Blender` resolved to Blender 5.2.3 MSI and the CDN returned **403 Forbidden (0x80190193)**. Not installed. Trying the LTS package next.
- **Blender portable zip:** download.blender.org returns **403 Forbidden** to both the winget CDN and python urllib from this machine, so the release index can't even be read. BLOCKED on this network path.

## 12:2x - QUEUE RUN: BLENDER INSTALLED, MESHES MADE, IMPORTED, PROVEN
**Blender:** winget CDN and download.blender.org both 403. The **dotsrc mirror** serves the same file:
`https://mirrors.dotsrc.org/blender/release/Blender4.5/blender-4.5.5-windows-x64.zip` -> HTTP 200,
398,872,872 bytes, unzipped to `C:\Users\Admin\Tools\Blender\blender-4.5.5-windows-x64\blender.exe`.
`blender -b --version` -> **Blender 4.5.5 LTS (hash 836beaaf597a)**.
**Meshes, scripted (no hand modelling), `Saved/veil_meshes.py`:**
- `VEIL_WingMembrane`: 196 verts / 49 faces, 13 ribs on a 100-degree fan, phi length falloff,
  3 material slots - `VEIL_Frame_Gold` (0.788,0.635,0.290), `VEIL_Membrane_Navy` (0.055,0.078,0.141),
  `VEIL_Edge_Blue` (0.247,0.663,1.000, emission 6.0).
- `VEIL_Cloak`: 121 verts / 96 faces, 4-ring skirt + hood dome, vertex group **pin** on the top ring for cloth.
- FBX: `Content\VEIL\Runner\Source\VEIL_Runner_Wings.fbx` (24,012 bytes).
**Import (headless, no GUI):** `UnrealEditor-Cmd -run=pythonscript -script=Saved\veil_import.py -nullrhi`.
Created in `/Game/VEIL/Runner/`: `VEIL_WingMembrane.uasset`, `VEIL_Cloak.uasset`, `VEIL_Frame_Gold.uasset`,
`VEIL_Membrane_Navy.uasset`, `VEIL_Edge_Blue.uasset`.
**Proof:** build `Result: Succeeded`, exit 0, `error C` 0 (log `Saved/build_latest.log`).
Play `-VeilShot`: exit 0, **SNAP=0**, PY=15, CEF=0, PAWNFAIL=0,
`VEIL WORLD OMNI: 12 blue block(s), 3 light pillars, Countess at (809, -309)`.
PNG: `Saved/Screenshots/WindowsEditor/VEILOMNI_proof.png` (plus HubEast/HubWest/Deep).
**Still open:** the new mesh is imported but not yet swapped into the Runner's wing component; Minions are not
yet spawned as enemies; Hub is not yet dressed with Sicka/Iceland. Those are the next wire steps.

## GOAL 1
Goal 1 done.
- Root cause found: every Enhanced Input action was null (`Move=0 Look=0 Jump=0`), and the project used
  Enhanced player-input classes, so WASD delivered nothing. Fixed by pointing the loads at the real Polyart
  input assets and switching `DefaultInput.ini` back to `Engine.PlayerInput` / `Engine.InputComponent`
  (backup `DefaultInput.ini.bak.goal1`).
- Camera over the shoulder: arm 300, socket offset (0,58,64), lag 12, FOV 90, pitch 17.0324.
- Headless movement proof, `-VeilMoveTest`, driving the same `MoveForward` handler the W axis calls:
  `speed=439.0`, `jump pressed`, `falling=1`, `RESULT distance=345.7 cm moved=1`.
- Build `Result: Succeeded`. SNAP=0. PNG `docs/proof/goal1.png`.

## PARAGON MINIONS: ROOT CAUSE FOUND AND FIXED
- `SK_Minion_Lane_Super.uasset` is **15,362 bytes - a redirector, not a mesh**. That is why every
  `LoadObject<USkeletalMesh>` on it returned null; the content was fine, the path pointed at a stub.
- The real meshes carry no `SK_` prefix: `Minion_Lane_Super_Dawn`, `Minion_Lane_Melee_Dawn`,
  `Minion_Lane_Super_Core_Dawn`, with skeletons `Minion_Lane_Core_Skeleton` and `Minion_Lane_Siege_Skeleton`
  already present. Using `Minion_Lane_Super_Dawn` logs
  `VEIL MINION BODY: ... loaded=1`.
- Wired into the crowd (1 seat in 5) and spawned as six enemy bodies in the Omni district.

## PHI AI, ANIMATION AND GROUNDING ON THE MINIONS
- Idle sequence `Animations/Melee/NonCombat_Idle` plays on all six; playback phase is
  `frac(i * 0.6180339887) * clipLength`, the golden fraction, so no two minions share a frame.
  The earlier attempt used `Idle2Run_Super`, a *blendspace*, which `PlayAnimation` cannot take -
  that was the 6 missing-object errors.
- Motion: 20 Hz timer walking a golden spiral, angle = t*18 + k*137.5077 deg, radius from
  `PHI_Constants::Reach`, so the swarm never repeats an even division.
- Grounding: minion Z = ground - (Bounds.Origin.Z - Bounds.BoxExtent.Z), so the lowest vertex
  sits on the ground regardless of mesh pivot.
- Build `Result: Succeeded`; Play SNAP=0; MISSING=0; PNG docs/proof/alive.png.

## MINION FOLLOW-UPS CLOSED
- Idle now uses the Super lane's own sequence when the pack ships one (falls back to Melee
  `NonCombat_Idle` if the Super folder only has blendspaces - it did only have blendspaces at last check).
- Phi radius clamped to 320..900 cm so the spiral stays inside the Omni district instead of drifting.
- Re-verified: build green, SNAP=0, MISSING=0, PNG docs/proof/alive.png.

## MINION STATE MACHINE (IDLE <-> ATTACK)
- Two states only: 0 idle, 1 attack. Distance to the Runner decides, evaluated at the existing 20 Hz step -
  no extra tick, no behaviour tree.
- Aggro range is `PHI_Constants::Reach(2.792) * 2.0` (phi-derived, ~540 cm), not a round number.
- On a state change the body plays `Animations/Super/Idle` or `Animations/Super/Attack_A`, phase-offset by
  `frac(k * 0.6180339887) * clipLength` so a group never snaps to the same frame.
- Proved with `-VeilAggroTest`, which places the Runner inside the swarm: log shows
  `VEIL AGGRO TEST: Runner placed inside the swarm` then `VEIL MINION STATE: switched to attack on proximity`.
- Build `Result: Succeeded`, SNAP=0, MISSING=0, PNG docs/proof/aggro.png.

## AGGRO PROOF AND CALIBRATION
- First attempt never fired: aggro range was `Reach(2.792) * 2.0` (~540 cm) while the swarm orbits at
  320-900 cm, so the Runner standing at the centre was always outside it. Widened to `Reach(3.0) * 4.0`
  (~1120 cm), which covers the whole orbit band.
- With `-VeilAggroTest` the log now ends: `VEIL AGGRO TEST: Runner placed inside the swarm` ->
  `VEIL MINION STATE: switched to attack on proximity`. So the swap idle -> Attack_A is real, not theoretical.
- Correction: the run WITHOUT the flag also logs one switch. The Runner spawns ~2.9 km away, so that
  switch must come from the proof-shot camera moving the pawn into the district. Either way the state
  machine fires from real proximity, not from the test hook - the hook only makes it deterministic.

## GOAL: Hub frame vs canon — minion radius, camera, lighting
### Canon check (docs/canon/)
- Authority present: golden angle 137.5077640500378, **golden pitch 17.0324**, golden ratio 1.618.
- No sun/lux/exposure JSON exists in docs/canon/, so light *levels* have no canonical number.
  The only canonical constraints that apply to lighting/camera are chromatic (navy/gold/blue) and
  the pitch, both of which are now verifiably in code:
  - `SpringArm->SetRelativeRotation(FRotator(-17.0324f, 0.f, 0.f))` (VEILCountessCharacter.cpp:83)
  - `GoldenPitch = 17.0324` (PHI_MitochondrialEngine.cpp:7)

### Changed (all phi-derived, none decorative)
1. Exposure — `S.AutoExposureBias` was **+2.0** while the comment beside it said the stop must be
   -phi. That contradiction was the blown-white ground in the first frame. Now -0.9 (measured:
   black floor above -1.6, clipping below -0.2).
2. Sun — 130000 lux, (1, .74, .52) -> **95000 lux, (1, .78, .55)**: gold key without clipping.
3. Cool counter-light added — ADirectionalLight, -52 pitch / 214 yaw, 9000 lux, #3FA9FF
   (0.247, 0.663, 1.0). API note: ADirectionalLight has no SetIntensity; use GetLightComponent().
4. Minion spiral — radius clamp 320..900 -> **260..620** cm, orbit 18 -> **26** deg/s, so the swarm
   fills the near frame instead of orbiting as a distant carousel. Aggro reach (~1120 cm) still
   covers the whole band.
5. Proof camera — removed the 85 cm lateral bias (she sat in the bottom-left corner), back
   259 -> **301 cm**, up 191 cm, aim 40 cm down: centred, and clears the 7-cliff dress ring.
6. Proof stand point — was y = -2600, which *is* the dress-ring radius, so the lens started inside
   a cliff. Now (-60, 240) on open hub floor.

### Verification
- Build: `Result: Succeeded`, exit 0, 0 `error C`.
- Play: exit 0, **SNAP=0**, **MISSING=0**, `VEIL SHOT NEAR: 0 piece(s) within 10 m of the lens`.
- Frame inspected each pass (5 renders). Regressions caught and reverted: skylight 6.5 + open lower
  hemisphere turned the world black; membrane rotated 90 deg sprayed wing blades across the lens;
  membrane at 2.4x sat between camera and head.

### Still open (honest)
- The district *does not yet read as alive*. The frame is dominated by unlit rock occupying the
  forward view: gold key is fully occluded there and the 9000-lux blue fill produced no measurable
  change, which points at the material/render path, not the light level.
- Found in the log: `M_VEIL_Deep missing usage flag SkeletalMesh! Default Material will be used in
  game.` (13 material warnings). The Runner is also drawing default material as a result.
  Next step is setting `bUsedWithSkeletalMesh` on the VEIL materials and re-shooting, plus rotating
  the proof yaw to face the open plaza rather than the rock wall.

## GOAL: settle the idle -> attack transition
### What the engine actually allows here
`USkeletalMeshComponent::PlayAnimation(UAnimationAsset*, bool bLooping)` has no blend parameter, and
these minions run in `AnimationSingleNode` mode with no anim blueprint, so there is no blend space and
no montage slot to blend through. `UAnimSingleNodeInstance` exposes play rate, position and looping -
not pose weights. So the transition cannot be a pose cross-fade without adding an ABP or a montage
slot asset, neither of which exists in this project yet.

### What was implemented instead (and it does remove the snap in time)
1. **Settle ramp.** On entering a clip the minion starts at play rate 0.15 and each 20 Hz tick closes
   1/phi^2 (0.3819660113) of the remaining gap to 1.0 - about 0.15 s to full speed. The attack winds
   up from its first frame instead of arriving already mid-swing at full rate.
2. **Entry frame.** Attack now starts at t = 0. Idle keeps its golden-fraction phase
   (`frac(k * 0.6180339887) * clipLength`) so the swarm stays desynced.
3. **Hysteresis.** State enters attack at the phi reach and only leaves at `reach * phi` (1.618x), so
   distance noise on the orbit can no longer flip state and re-cut the clip every tick.

### Verification
- Build: `Result: Succeeded`, exit 0, 0 `error C`.
- Run with `-VeilShot -VeilAggroTest`: exit 0, SNAP=0, MISSING=0.
- Log proof the path executed: `VEIL MINION BLEND: settle ramp 0.15 -> 1.0 at 1/phi^2 per 20 Hz tick`,
  `VEIL AGGRO TEST: Runner placed inside the swarm`, `VEIL MINION STATE: switched to attack on proximity`.
- PNG: docs/proof/VEILOMNI_proof.png (single still; a rate ramp's smoothness cannot be judged from one
  frame - it needs a short capture to confirm by eye).

### Still open (honest)
- This is a timing settle, not a pose cross-fade. The skeleton still swaps clip instantly; only the
  speed and the entry frame are eased. A true blend requires a montage with BlendInTime played through
  a slot, i.e. an AnimBlueprint for the minions.
    Next step: create ABP_Minion (idle<->attack state machine with a 0.2 s cross-fade rule) and play
    through it; the play-rate ramp then becomes unnecessary.

## GOAL: minions converge on the Runner along the phi spiral (not just orbit)
### Design
`VeilStepMinions` used to place every minion at `centre + radius(angle)` with the angle derived from
the shared clock and the radius from a fixed clamp - so the swarm only ever orbited a fixed band. It
now carries per-minion integrated state:

- `GVeilMinionAngle[k]` - degrees, accumulated. Angle must be integrated, never recomputed as
  `T * rate`: the moment the rate changes between idle and attack, a recomputed angle teleports.
- `GVeilMinionRadius[k]` - cm from that minion's spiral centre.
- `GVeilMinionCenter[k]` - the orbit centre itself. It eases onto the Runner while attacking
  (0.12/tick) and back to the pack centre when idle (0.05/tick), so the hand-off drifts instead of
  snapping.

### The approach path is an actual golden spiral
Attack motion is `r = r0 * e^(-b*theta)` with `b = ln(phi)/(pi/2)`, i.e. **one quarter turn strips
exactly one phi of radius** - the definition of the golden (logarithmic) spiral, not a straight-line
lerp with a golden label. Angle rate rises to `26 * phi^2` (~68 deg/s) on the way in, which shortens
the run-in without changing the curve's shape; the radius is clamped at `Reach(1.0)` = 262 cm standoff
so they close on her without standing inside her. Idle eases the radius back to that minion's own
`Clamp(Reach(2.4 + (k%6)*0.28), 260, 620)` band. Attacking minions now face the Runner; idle ones face
along their orbit.

### Verification
- Build: `Result: Succeeded`, exit 0, 0 `error C`.
- Run `-VeilShot -VeilAggroTest`: exit 0, SNAP=0, MISSING=0.
- Log proves the radial decay actually reaches the Runner:
  `VEIL MINION CHARGE: golden log-spiral approach, b=ln(phi)/(pi/2), standoff 262 cm`
  then `VEIL MINION CHARGE: minion 4 spiralled in to the 262 cm standoff`
  alongside `VEIL AGGRO TEST: Runner placed inside the swarm` and `switched to attack on proximity`.
- PNG: docs/proof/VEILOMNI_proof.png.

### Note
Reached standoff in the run, so the spiral completes in well under the test's capture window. Motion
still needs an eye-on video to judge feel; the still only proves a minion arrived.

## GOAL: Hub frame vs canon - minion spiral radius, camera, lighting (pass 2)
### Canon check
`docs/canon/` holds no lighting asset: the only canonical numbers are the golden angle
137.5077640500378, the golden ratio 1.618 and **golden pitch 17.0324**. Grep for sun/lux/exposure/fov
in `VEIL-OMNI-ALL-TOGETHER.cpp.txt` returns only the title line, so light levels have no canonical
value to match - the pitch and the navy/gold/blue palette are the whole authority here.

### The real cause of the black district (two bugs, both structural)
1. **The sun was pointing at the sky.** `FRotator(PHI_Constants::GoldenPitchDeg, 223.f, 0.f)` is
   +17.0324 pitch, and a directional light travels along its forward vector, so a positive pitch
   aims it UPWARD. Every upward-facing surface was unlit; the bright floor in every earlier frame was
   the three point-light pillars, not the sun. The canon value is an *elevation*, so it now reads
   `FRotator(-PHI_Constants::GoldenPitchDeg, 223.f, 0.f)`.
2. **The swarm stood 495 cm below the floor.** The new diagnostic proves it:
   `VEIL MINION GROUND: minion Z 889 vs plaza deck Z 1384`. `GVeilMinionZ` is derived from mesh
   bounds, not from the deck, so the whole swarm was under the plaza the camera was shooting. The
   step call now passes `FMath::Max(GVeilMinionZ, GPlazaZ)`.

### Adjustments made
- Swarm recentred from the Omni block cluster at (2800, 500) - 28 m outside the Hub frame - to the Hub
  itself, and its idle band widened from 260..620 to **620..1500 cm** so the ring is legible on screen.
- Sky fill raised one phi step (`PhiPow(2)` -> `PhiPow(3)`) now that the key actually reaches the rock.
- Proof camera no longer trusts a fixed offset: it walks a phi ladder of six
  back/up combinations (`x(1 + 0.618*i)` back, `x(1 + 0.809*i)` up) and keeps the first with a clear
  `LineTraceTestByChannel` to the Runner. If none is clear it relocates to the open Hub floor.

### Verification
- Build `Result: Succeeded`, exit 0, 0 `error C`. Play exit 0, SNAP=0, MISSING=0.
- Logs: `VEIL MINION GROUND: minion Z 889 vs plaza deck Z 1384`,
  `VEIL MINION CHARGE: minion 0 spiralled in to the 262 cm standoff`,
  `VEIL PROOF CAM: no clear line from the pawn, shot moved to the open Hub floor`.
- Frame inspected: docs/proof/VEILOMNI_proof.png plus docs/proof/hub_all_six.jpg (all six shots).

### Still open (honest)
- The district now reads as geometry on the deck with minions standing on it, but it is still dark:
  the Hub corridor the Runner spawns in is lit only at the deck. Brightness is close to the ceiling
  of what exposure can do without washing out the gold key.
- **Record cameras 2-5 are inside the Runner's own mesh** - the montage shows the inside of her hand
  and head. `TakeShot()` places them off the `Anchor[]` array without any clearance or trace, so four
  of six proof frames are unusable. That is the next fix: give each record camera a clearance test the
  same way the proof camera now has one.
- `M_VEIL_Deep` still reports `missing usage flag SkeletalMesh` (13 warnings) and falls back to the
  default material, which is why the Runner renders untextured white.

## GOAL: minion swarm state changes - Super Idle <-> Attack_A on approach
### What was already there
The swap existed: `GVeilMinionState[k]` 0/1, `GVeilMinionIdle` =
`/Game/ParagonMinions/.../Animations/Super/Idle.Idle`, `GVeilMinionAttack` =
`/Game/ParagonMinions/.../Animations/Super/Attack_A.Attack_A`, driven inside the existing 20 Hz step
timer with a settle ramp and hysteresis.

### The defect that made it inert
`const double GVeilAggroRange = PHI_Constants::Reach(3.0) * 4.0;` is about 2,700 cm, but the swarm now
rings the Hub at 620..1500 cm. Every minion was inside the aggro radius from the moment it spawned, so
the state could only ever be attack - Super Idle was unreachable and the swarm never "reacted" to
anything, it just started angry. Now:

    const double GVeilAggroRange = PHI_Constants::Reach(2.0) * PHI_Constants::Phi;   // 685 cm

That sits inside the orbit band, so idle is the default and approach is what flips it. Release uses
the existing hysteresis at `threshold * phi` = 1108 cm, so a minion at the band edge cannot chatter.

### Verification (one run, `-VeilShot -VeilAggroTest`)
Build `Result: Succeeded`, exit 0, no `error C`. Play exit 0, SNAP=0, MISSING=0.
Transitions logged with the distance that caused them:

    minion 2 swaps Super Idle -> Attack_A at 487 cm (threshold 685)
    minion 4 swaps Super Idle -> Attack_A at 674 cm (threshold 685)
    minion 1 swaps Super Idle -> Attack_A at 685 cm (threshold 685)
    ...
    minion 0 swaps Attack_A -> Super Idle at 1277 cm (threshold 685)
    minion 1 swaps Attack_A -> Super Idle at 1275 cm (threshold 685)
    minion 2 swaps Attack_A -> Super Idle at 1409 cm (threshold 685)

Counts for the run: **13 idle -> attack, 9 attack -> idle**. Both directions exercised, at the
expected thresholds, in the same session.

### Note
This is a logic/state change, proven from the log rather than from the still frame: a still cannot show
a transition. The clip swap itself still eases in via the settle ramp (0.15 -> 1.0 play rate at 1/phi^2
per tick), because single-node playback has no pose blend.

## GOAL: six minions patrol the Omni district on phi paths
### Change
Idle is no longer a fixed-radius orbit around the Hub. The patrol centre is the Omni district at
(2800, 500) - the step call, the spawn positions and the band all moved there - while the attack
centre still follows the Runner, so a minion that gets close still spirals in.

- Patrol path: the radius rides a **golden ladder**, `HomeRad * (1 + 1/phi^3 * sin(angle * phi))`,
  with the angle integrated per minion. The trail is therefore a phi-petalled arc rather than a
  circle, and each minion starts at `k * 137.5077640500378` degrees so the six are spread.
- Patrol ring: `Clamp(Reach(5.2 + (k%6)*0.18), 1900, 3200)` cm - outside the 12-block cluster, so
  they walk around the district instead of through it.
- Pace: 26 deg/s was an orbit at 4.4 m/s, measured. Now 4.8 deg/s, about 1.9 m/s at the 2200 cm ring.

### Verification
- Build `Result: Succeeded`, exit 0, no `error C`. Play exit 0, SNAP=0, MISSING=0.
- `VEIL MINION PATROL: six minions on a phi-ladder patrol around the Omni district`
- Position samples for minion 0 from the run (see the speed line printed by the test), both at the
  district ground Z 889, with the radius oscillating as the petal turns - i.e. moving, not standing.

### Note
Two samples prove displacement and speed; they cannot show the feel of the path. The petal amplitude
(1/phi^3) and the 1900..3200 ring are phi-derived but not yet eye-checked against the block cluster,
so a minion could still clip a block corner.

## GOAL: golden-ratio idle phase offsets so the six stop animating in sync
### Problem found
The offset existed, but in **two** places, each recomputing `Frac(k * 0.6180339887f)` independently -
once in the spawn loop (index `m`) and once in the idle re-entry (index `k`). Duplicated maths of that
kind drifts the moment one site changes, and nothing verified the result: no log ever showed whether
the six actually ran out of step.

### Change
- `TArray<float> GVeilMinionPhase` holds one golden-ratio fraction per minion, seeded once at spawn as
  `frac(m / phi)` and reused on every re-entry to Super Idle, with the formula kept only as a fallback
  if the array is short.
- The fraction is applied as a position inside the clip: `phase * Idle->GetPlayLength()`.

### Verification (real clip positions, not just the seeds)
Build `Result: Succeeded`, exit 0, no `error C`. Play exit 0, SNAP=0, MISSING=0.

    VEIL MINION PHASE: golden-ratio offsets 0.000 0.618 0.236 0.854 0.472 0.090
    VEIL MINION PHASE: live idle clip position per minion 0.77 0.39 0.01 0.62 0.48 0.86

Six distinct seeds, and six distinct live positions read back from
`USkeletalMeshComponent::GetPosition() / GetPlayLength()` during the run - not in lockstep. The live
values differ from the seeds because each minion has been running (and one, minion 4, took a round trip
through Attack_A: `swaps Super Idle -> Attack_A at 685 cm`, `swaps Attack_A -> Super Idle at 1166 cm`)
while the clip kept advancing, which is the point: the offsets survive state changes.

### Note
Clip phase is verified numerically. Whether the stagger *reads* as natural movement still needs eyes on
motion; a still cannot show it.

## GOAL: minions sitting on the ground and facing their travel in the Omni district
### Grounding - the bug
`GVeilMinionZ` was derived **once** from `Mini->GetBounds()`, i.e. the bind pose. Animation moves the
lowest vertex, so the feet were off the floor by an amount nobody measured, and the step call also
aimed them at that bind-pose offset instead of the district floor (`GHubGroundZ`, where the twelve
blocks stand).

### Grounding - the fix
Every tick, per minion, measure the actor origin to the lowest CURRENT vertex:
`FootOff = ActorZ - (Bounds.Origin.Z - Bounds.Extent.Z)`, clamped to +/-500 cm, and place the actor at
`GroundZ + FootOff`. That is a direct measurement of the running pose rather than an assumption about
the bind pose, and the target is now `GHubGroundZ` itself.

### Facing - the fix
Patrol yaw comes from the actual displacement since the tick-start snapshot,
`atan2(Trav.Y, Trav.X)`, with the orbit tangent as the fallback when stationary; attacking minions
still face the Runner.

### Two bugs in my own instrumentation (both found and fixed here)
1. The first audit compared against `GVeilMinionLast[j]`, which is refreshed **mid-loop**, so every
   minion except the last read zero displacement and reported `0.00` alignment. That looked like a
   facing bug and was not one.
2. My first correction wrote the post-move position back into the snapshot array, re-creating the same
   zero. The snapshot is now taken for all six at the top of the tick, before anything moves.

### Measured output (this run, -VeilShot)
VEIL MINION PATROL: t=2.5 minion 0 at (4864, 948, 896), radius 2112
VEIL MINION AUDIT: feet vs floor (cm) 0 0 0 0 0 0
VEIL MINION AUDIT: forward . travel 1.00/89 1.00/-105 1.00/28 1.00/139 1.00/23 1.00/64
VEIL MINION PATROL: t=8.0 minion 0 at (4662, 1976, 896), radius 2376

`feet vs floor` of **0 cm on all six**: the feet sit exactly on the district floor with whichever pose
is playing. `forward . travel` of **1.00 on all six**: each minion faces the direction it actually
walks, with the patrol yaw values shown per minion (89, -105, 28, 139, 23, 64 degrees).

### Still open (honest)
This proves the ACTOR faces travel. It cannot prove the visible mesh does: if the Paragon minion mesh
is authored facing +Y rather than +X, the model will look 90 degrees off while every number above
reads 1.00. That needs eyes on a frame containing the district - the proof shot still frames the Hub,
so it does not show this.

## GOAL: real idle animation from the pack's Super blendspaces
### Correction to my own earlier finding
I reported twice that `Super` contained no blendspaces and that the pack's blendspaces lived only under
Melee/Ranged/Siege. That was wrong on both counts:
- my `grep super` was truncated at 20 lines, so it never showed the Super blendspace;
- the runtime asset-registry scan returned **0 assets scanned**, because the registry is not populated
  in `-game`, and I read that empty result as "the folder has no blendspaces".

The pack does ship one: `/Game/ParagonMinions/.../Animations/Super/Blendspaces/Idle2Run_Super`.

### What is implemented now
Idle plays a real blendspace from the pack, evaluated at its default parameters (the resting end of
Idle->Run). Paths come from the pack's actual on-disk listing (186 assets, 6 blendspaces), not from the
registry, and a candidate is accepted only if `Asset->GetSkeleton() == Mesh->GetSkeleton()`, so a
blendspace from the wrong lane can never be applied. Preference order is idle-named Super first, then
the rest; `Super/Idle` remains the fallback.

### Measured output (this run, -VeilShot)
VEIL MINION IDLE: blendspace Idle2Run_Super
VEIL MINION POSE: bone pelvis moved 10.7 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 12.8 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 4.3 cm in 1.0 s (0 = bind pose)
VEIL MINION AUDIT: feet vs floor (cm) 0 0 0 0 0 0
VEIL MINION AUDIT: forward . travel 1.00/89 1.00/-105 1.00/28 1.00/139 1.00/23 1.00/64
VEIL MINION POSE: bone pelvis moved 1.4 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 10.6 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 14.7 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 3.4 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 1.9 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 10.3 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 14.6 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 3.4 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 1.6 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 10.2 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 14.5 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 3.7 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 1.3 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 11.9 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 12.5 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 3.2 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 5.1 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 12.7 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 7.1 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 2.6 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 7.2 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 1.2 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 6.8 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 3.5 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 6.1 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 6.3 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 5.3 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 4.3 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 6.5 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 12.2 cm in 1.0 s (0 = bind pose)
VEIL MINION POSE: bone pelvis moved 4.5 cm in 1.0 s (0 = bind pose)

The chosen asset is the Super blendspace `Idle2Run_Super`; the pelvis moves 1.4 - 12.8 cm per second in
**component space** (actor patrol motion excluded), which is what distinguishes a playing clip from bind
pose - bind pose would read 0.0. Feet remain 0 cm off the district floor.

### Earlier probe bug fixed here
The first pose probe used world-space bone locations and reported ~190 cm/s, which was just the patrol
speed - it would have "passed" even with the mesh frozen. It now samples
`EBoneSpaces::ComponentSpace`.

### Still open (honest)
Numeric proof that a clip is playing is not proof that it looks right. Whether `Idle2Run_Super` at zero
parameters reads as a natural idle stance for these six, and whether the Paragon mesh's authored forward
matches the actor forward, still needs eyes on a frame containing the district - the proof shot frames
the Hub.

# M4 - the Omni District at street level (Ruling 7)

**Task.** Read Ruling 7 (commit `7cd4e20`) and continue M4 from the city layer's open items in the
order given: massing details, the two wrong surfaces, gold at distance, the walled horizon lens,
walkability, the tower's reflection probe.

**Ruling quoted.** "The district is 65 m deep (10,590 -> 17,135 cm) because Ruling 5 keeps the 50 m
Spire. The direction's 'city sprawl' at 1,597 m is a vision number, not a build target ... The 65 m
district IS the city. If a bigger world ever comes, it is a new build (M7+ regions), not a rescale of
this one."

**Build.** `Result: Succeeded` (only the pre-existing C4996 Cast warning).

**Files changed.** `Source/VEIL/VeilCity.cpp` (new: the district's street layer and PlaceCauseway),
`Source/VEIL/VeilWorldGen.cpp/.h` (city materials, layout keys, causeway clearance, check 7, spire
material log), `Source/VEIL/VeilHubGameMode.cpp/.h` (horizon lens, tower probe, district crowd,
street lens), `docs/world_layout.json` (`city_door_cm`, `city_door_height_cm`, `city_sign_cm`,
`city_causeway_tile_cm`, `city_crowd`), `docs/PHI_APPLIED.md` (regenerated),
`Scripts/make_city_materials.py`, `Scripts/tune_city_materials.py`, `Scripts/tune_tower_gold*.py`,
`Scripts/probe_tower_gold.py`, `Scripts/m4_city_*.py`, `Scripts/gen_phi_applied.py`.

**Proof log** (`docs/proof/city2.log`):

```
VEIL CITY: blocks=98 tallest_cm=3600 spire_cm=5000 cap=0.72 light_pillars=19 plaza_skipped=2 bands=3 spokes=21 road_skipped=5
VEIL CITY ROADS: tiles=84 ring_roads=4 width_cm=833
VEIL CITY DETAIL: doors=98 signs=98 awnings=50 street_props=44
VEIL CAUSEWAY: tiles=33 length_cm=14536 width_cm=433 start_cm=2599 end_z_cm=1040 end_terrace_cm=1000 max_slope_deg=20.9 bearing_deg=137.508
VEIL CHECK city_walkable PASS tiles=33 end_z_cm=1040 terrace_cm=1000 slope_deg=20.9
VEIL CHECK pass=7 fail=0 instances=2053
VEIL HASH crc=E231685A instances=2053 slots=39 seed=1618
VEIL CITY CROWD: walkers=13 expected=13 failed=0
VEIL PEOPLES: human=21 gold=8 crystal=5 deep=3 walkers=37 expected=37 failed=0
VEIL REFLECTION: tower capture radius_cm=3090 at_z=3123 at=(0,10590)
VEIL GEN spire: at=(0, 10590) steps=8 height_cm=5000 base_w_cm=1200 material=M_VEIL_GoldPBR
VEIL FROG CHECK: u=PASS horizon=PASS fit=PASS | fg_cm=1276 mid_cm=4045 bg_cm=10590
```

**PNGs.** `docs/proof/hub_omni_city.png`, `docs/proof/hub_omni_street.png`,
`docs/proof/hub_bible_horizon.png`, `docs/proof/hub_frog_spire.png` (unchanged).

**Errors.** 15 `LogPython: Error` lines, all from the engine's disabled `EditorToolset` plugin
(`init_unreal.py` importing `editor_toolset.skills`); none from project scripts. No `VEIL SNAP` line
is printed in this `-game` shot run, so it is not claimed.

**What the frames show, and what they do not.** The street frame is a street: paved causeway, the
district crowd on it, gold sign boards, blue-lit doorway panes, gold pillars and the skyline at the
end. The city frame shows the grid, the crowns and the blue panes, but the inward faces are charcoal
-- the sun comes from 70 deg and a ring city presents its shade side to any lens inside it, and only
the 19 pillar lamps answer it. The horizon frame is the district's skyline from the Deep terrace,
with the gold tower over the roofs.

**Whole Test** (unchanged by this pass): Runner grounded/over-shoulder (not re-shot), blended
movement (not re-shot), HUD v2 shown in the horizon frame, hub from json 37/37 with 7/7 checks,
golden hour, Sera Vahl (not re-shot), 13 district walkers spawn and walk, `VEIL SNAP` not printed in
a `-game` run.

**Not pushed.** `C:/Users/Admin/HarkGame` has no `.git`; the repo seat
(`Saved/repo-seat/veil-omni-unreal`, now at `7cd4e20`) has none of this work. Push on your word.

## Scope: mobile and 2D are out (Tyree, 2026-11-26)

**Task.** Take the vision update at `46dba60e` as binding: mobile and 2D are irrelevant, not built
from, not cited, not carried.

**Where the commit is.** `46dba60e` is already an ancestor of `origin/main` (`7cd4e20`), so the copy
of `buffy-handoff/phi-veil-omni-vision.md` this build reads is the updated one. What it changed: the
mobile faction set deleted from the factions record, the browser edition's XP ladder deleted from the
economy record, and the two "it exists, it is not the game" paragraphs (mobile specs, arena detail)
replaced by one line each -- "irrelevant ... not carried in this document". The web builder stays, but
only as proof of the law (ruling 4), not as the game.

**Checked against the build (no changes needed).**
- Targets: `Source/VEILOMNI.Target.cs` and `Source/VEILOMNIEditor.Target.cs` only. No Android, iOS or
  HTML5 target, no platform extension, no packaging script for either.
- No mobile or 2D project files exist under `C:/Users/Admin/HarkGame` (no Godot project, no
  `index.html`, no `package.json`, no `*.tscn`).
- The only mobile strings in `Config/` are stock UE defaults that ship with any project:
  `DefaultTouchInterface=/Engine/MobileResources/HUD/DefaultVirtualJoysticks` with
  `bAlwaysShowTouchInterface=False` and `bUseMouseForTouch=False`, plus the engine's own
  `OculusTouch_*` axis rows in `DefaultInput.ini`. None is a mobile build and none is read by the
  Hub. Left alone: removing engine defaults buys nothing and risks the input mapping.
- `_Parked/` holds Paragon mesh copies (`ParagonCountess`, `ParagonMinions`, ...), not mobile or 2D
  work.
- Docs: no local doc cites the mobile game or the browser edition as canon. The word "browser" in
  `docs/VEIL_Bible.md` §9, `docs/HARK_TO_BUFFY.md` and `docs/VISION_JARVIS.txt` means the editor's own
  CEF window, Fab access, and Jarvis's Web Speech -- three different things, none of them the browser
  edition.

**Recorded.** The rule now sits under "Rules that do not bend" in `docs/VEIL_TASKS.md`, so it
outlives this thread: nothing from `veil-omni-mobile` or `veil-browser`, one line each in the vision
doc and none here.

**Build.** No C++ or content changed, so nothing was rebuilt; the last build of this tree stands at
`Result: Succeeded` with `VEIL CHECK pass=7 fail=0 instances=2053`.

## M4 piece: rooms, not boxes (2026-11-26)

**What changed in code.** `PlaceCity` builds a ground-floor shell per block instead of one solid
cube: front piers + header + back + two sides + ceiling, a floor slab on the `GenCityFloors` slot, and
an emissive ceiling panel on the glass slot, with the mass starting at `CityDoorH * PHI = 610` cm.
A 233 x 377 doorway is now an opening you can walk through, not a lit pane on a wall. New check 8
`city_interiors`: rooms + solid must equal blocks, and rooms must be non-zero.

**One frame, one check.** `hub_omni_door` stands 307 cm off the nearest room door to the causeway and
looks at the inner corner: jambs, threshold, dark floor, two walls. It took three passes to get the
lens right -- the first stood 600 cm out inside a neighbour's wall and photographed a wall; the lens
now traces its own standing room first.

**Values.** `VEIL CHECK pass=8 fail=0 instances=2788`; `VEIL HASH crc=AC518031 slots=41`;
`rooms=86 solid=12`; build `Result: Succeeded`.

**Ruling followed.** No job files, no launcher runs beyond the single shot check, no editor session
to look around: code first, the shot is the check.
