# RULINGS FOR BUFFY — VEIL OMNI

Tyree's direction (2026-11-26): "I want yall to do it all." These seven rulings are decided here, by TON 618 on that word. Tyree can override any of them.

## 1. Canon tree: C:\Users\Admin\HarkGame

The built, measured work lives there — the four castes, the PC tier, the grade, the HUD, every PIE shot. D:\VEIL-OMNI is the older Cowork-seated codebase (the repo's README_RESEAT describes it: "not compiled or tested here"). The bible's §0 path line naming D:\VEIL-OMNI is stale and gets corrected to C:\Users\Admin\HarkGame. The repo's Cowork tree stays as archive, not the build tree. No migration, no merge.

## 2. The three missing docs go to the repo — from the PC

VEIL_DIRECTION.md, KRAKEN_STUDY.md, HUB_PAINTING_SPEC.md exist only on the PC; no copy reaches this side. Buffy: read them from disk in C:\Users\Admin\HarkGame\docs\ and push them to the repo (docs/ folder, main branch) so every citation you are required to make is checkable. Do this first, before the verification pass.

## 3. HUD: keep VIT / CTM / FIG

The phase-shift directive already settles this — the HUD stays, it is not rebuilt or re-decided. The bible's §3 VIT/STM spec gets a note, not a revert: the live HUD uses VIT/CTM/FIG. Keep it as built.

## 4. Builder counts: reference only

637 trees, 34 stalls, 70 dwellings, the 25.2-unit spire, 1,364 lattice points — none of these appear in the bible, so none of them bind VEIL OMNI. They guide proportion; they are not canon. What IS canon from the builder side is the §5 bridge: the Mitochondrial engine, ATP feeds stamina and abilities. The builder stays a proportion source and proof-of-order, not a thing to import. The whole-in-one-read build and the one-piece-at-a-time frog rule are not in conflict: the builder proves order, the frog rule builds it.

## 5. Spire position: (0, 10590) stands

The built Spire is the generator's PlaceSpire: 5,000 tall × 1,200 wide at (0, 10590), 8 steps, from docs/world_layout.json. The bible's (0, 3500) coordinate is superseded by the built world. Three reasons it makes sense: (a) the bible's own art direction says the Spire stands "on the horizon, visible from everywhere" — at 35 m a 50 m tower would loom over the market, not sit on the horizon; at 105.9 m it does. (b) 10,590 = base_ring_cm 4045 × φ² — phi-derived, part of the ring layout (rings [21, 13, 8, 21, 55]); moving it to 3500 would put it inside the Hub ring and break the layout. (c) The real assets are built there — the generator, the rings, the placed Spire. PHI_APPLIED.md is a stale spec (2026-11-04, never regenerated since the generator took the world over) — regenerate it from the build; docs are derived, not the other way around. BuildSpire() is dead code (never called) — remove it so it stops giving a fourth answer. The bible's coordinate gets a note, not a rebuild (same treatment as Rulings 1 and 3).

## 6. Census: keep 37, derive it, count the failures

21/8/5/3 = 37 is Fibonacci (F8/F6/F5/F4) and the dressing is real — the counts stand. The loop must derive the tally instead of hardcoding it: no literal +16, no hardcoded caste thresholds (21/29/34). If the tally changes, the loop follows. Add failure accounting: a SpawnActor returning null gets counted, not silently skipped. The newest run read 35, not 37 — two walkers failed to spawn and the census reported the survivors. That drift is the bug; the count itself is not.

## 7. Scale: keep the 50 m Spire world

The district is 65 m deep (10,590 → 17,135 cm) because Ruling 5 keeps the 50 m Spire. The direction's "city sprawl" at 1,597 m is a vision number, not a build target — it is not in the bible, and the bible's coordinates rule. The world as built is phi-ordered end to end (rings at φ-scaled radii, the 0.72 height cap, the sightlines), and the machine runs it at 84 fps on 1,454 instances. Scaling 9× would break every derived number and need a rebuild of the generator, the rings, the caps, and the sightlines — and a streaming approach the project does not have. The 65 m district IS the city. If a bigger world ever comes, it is a new build (M7+ regions), not a rescale of this one.

## What follows from these rulings

- Verify the Seer and Countess placements against the §4 authority coordinates (Keeper 900,0 · Seer -450,780 · Cartographer -4150,-780 · Cache 0,-6200 · Beacons 2800,500 / -2000,-900 / 600,-2100 · Vault 0,0,-80.9 · Countess 809,-309,64.7 · RUNNER spawn 0,0,100). The coordinates rule; the placements get checked against them, not the reverse.
- The spawn shot's grey stone centre structure fails §2 ("No gray boxes in any finished area"). The frog piece re-frame fixes it: gold Spire the only focal structure on the 0.618 width line, horizon on the 0.618 height line, three named depth layers, palette by hue histogram, four readable caste silhouettes in the foreground, seed 1618. One "name the match" line plus pass/fail per check.
