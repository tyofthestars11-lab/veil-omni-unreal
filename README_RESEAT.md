# VEIL OMNI Unreal — re-seated Cowork files

Source: Cowork/Copilot Unreal pieces, downloaded 2026-10-30 (Vault 3:46, Loop 3:48, Waves+Enemy 4:01, Regions 4:07, Countess updated 3:46). All Cowork READMEs say "not compiled or tested here" — this re-seat does not claim a compile.

## What changed in this re-seat, and nothing else
1. Paths: flat `Source/VEIL/` → real project layout `Public/Character`, `Public/Systems`, `Private/Character`, `Private/Systems`. No code logic changed except item 2.
2. Countess health, added: MaxHealth 100, Health, TakeDamage override, GetHealth/GetMaxHealth, OnHealthChanged event. This is the missing hook — enemies ApplyDamage 8/14/35 did nothing before, because ACharacter has no health of its own.
3. Nothing else renamed, no values changed: ATP Dash 12 / Attack 18 / Interact 8 (100→88→70→62), Vault tetra 109.471221°, radius 900, height 700, Loop 10 fragments, Waves 3,5,8,13,21 + WARDEN PRIME, Regions WorldScale 100.

## Still needed on the PC, in the editor, before play
- BP_Countess: set Skeletal Mesh = SM_Countess + Anim Blueprint, Default Pawn = BP_Countess. Code does NOT assign SM_Countess.
- BP_VEILEnemy: set a mesh (drone/soldier/boss body). Code assigns no enemy mesh.
- Vault: set LatticeMesh = gold Vault mesh, AnchorMesh = crystal/sphere. Both are null in code — no gold Vault mesh asset exists yet.
- Fragment/Forge/Beacon: set a mesh + RegionId on each placed actor.
- Input: project uses Enhanced Input. Her Countess still uses legacy BindAxis/BindAction. Either keep legacy Input mappings (MoveForward/MoveRight/LookUp/Turn, Jump/Sprint/Dash/Attack/Interact) for this compile, or convert to Input Actions + Mapping Context in the next pass. Do NOT claim Enhanced Input works until the mapping is built and tested in play.
- Menu that builds first + pause + win screens: still to write (Cowork finalization).
- Mobile joystick/touch: see https://github.com/tyofthestars11-lab/veil-omni-mobile — build_mobile_3d_rpg_670_one_motion.py (Meta 670 spec, not Cowork).

## Proof ONLY in play
Character visible, Vault visible, walks on floor, NavMesh works, loop advances. Load true / save true is NOT proof. No package, no 1.0, until 100%.
