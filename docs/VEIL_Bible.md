---
title: "VEIL OMNI: The Last Signal, Path of Light"
subtitle: "Build Bible for Buffy (DLAM hand)"
author: "Source: Tyree Jones"
date: "2026-11-02"
---

# 0. Who does what

- **Tyree** is the source. He decides. He reviews proof.
- **Buffy** builds in D:\\VEIL-OMNI. She reads this file from disk, not from chat.
- **Muse** writes briefs. She never claims a result she didn't see.
- AI never pays a COOK fee. Never delete Content or Saved. Never delete the game. No package, no cook, no exe until 100%.

# 1. The game in one breath

An open-world action RPG. You are the RUNNER, the last carrier of a broken signal. You walk, glide on phi wings, and fight across five regions to rebuild the signal and open the one door. One game, one save, one door. Target: 60-100 hours main story, side quests galore. Free will everywhere, but phi is always ordered.

**Phi constants (use everywhere):**

- phi = 1.618033988749895
- phi^2 = phi + 1 (check within 1e-6 on every boot)
- Golden angle = 137.5077640500378 degrees
- GoldenLoc(i): angle = i x 137.5077640500378, radius = Spacing x sqrt(i). Unreal units are centimetres.
- Proof values at Spacing 10: i=1 (-7.3737, 6.7549), i=2 (1.2364, -14.0880), i=5 (18.8669, -12.0016)

# 2. Art direction (match the reference painting)

- **Light:** golden hour, warm sun low on the horizon, god rays, volumetric clouds, light mist in valleys.
- **Palette:** warm gold and sandstone for the living world. Electric blue (#3FA9FF glow) for signal tech, crystals, and the Deep. Deep navy for shadow and UI.
- **Landmarks:** gold phi pillars with glowing phi glyphs; one great Spire on the horizon, visible from everywhere; blue crystal mountains; cliffs with waterfalls; rope and stone bridges.
- **Runner:** hooded navy cloak, gold trim, phi-glyph wing membranes that glow blue at the edges when gliding.
- **Unreal setup per map:** Lumen GI and reflections on. Directional Light (sun) + Sky Atmosphere + Sky Light (real-time capture) + Volumetric Cloud + Exponential Height Fog (volumetric on). Post Process: slight bloom, warm white balance, filmic tonemapper. This also clears the FastAerialPerspective warning.
- **Assets:** free Fab assets and Megascans already in the project, placed at the authority coordinates. No gray boxes in any finished area.

# 3. UI spec (no gray, no flat yellow, ever)

Style: dark glass panels, thin gold frames, slanted corners, diamond motifs. Built in UMG.

| Element | Spec |
|---|---|
| Panel fill | Navy #0E1424 at 85% opacity, soft blur behind |
| Frame | 1.5 px gold line #C9A24A, slanted top-right corner, small chevron crest on top edge |
| Text | Headers: Cinzel (free, OFL). Body: Inter or Rajdhani. White #F2F2F2, labels gold #E8C46A |
| Level badge | Gold diamond with phi star inside, "LV 72" plate below, top-left |
| VIT bar | Red #D9343A gradient, numbers right "3570 / 4200", slanted ends |
| STM bar | Green #2FB36A gradient under VIT, numbers right |
| Minimap | Round, gold ring with N/E/S/W ticks and compass needle, top-right; pause button inside the ring |
| Quest panel | Bottom-left, slanted frame: "QUEST: THE GOLDEN MIDST" then objective in smaller text |
| Hotbar | Row of gold-framed diamonds bottom-center, centre slot larger and glowing, stack counts in corners |
| Combat pill | "COMBAT MODE" in a gold-framed pill with red dot, bottom-right; hidden out of combat |
| Damage numbers | Gold for normal, blue for signal, red for crits; float up and fade |
| Rule | HUD auto-fades out of combat. No debug text on screen in normal play (move to a toggle, F10). |

Settings screen: sliders for Vitality and Stamina HUD scale, "HUD Atmosphere Stance" dropdown (Active / Minimal / Off), GODMODE and ASSISTS menus.

# 4. World and regions

| # | Region | Look (from the painting) | Role |
|---|---|---|---|
| 1 | Hub | Cliffside market, gold phi pillars, stalls, lanterns, crowds | Home, traders, quest board, the Keeper |
| 2 | Nest | High cliffs and waterfalls, wooden huts on ledges | Beacons, Seer, glide training |
| 3 | Spire | The great tower, gold glyphs, storm at the top | Signal fragments, vertical climb, boss |
| 4 | Omni District | Blue-lit city sprawl with light pillars | Factions, heists, the Countess |
| 5 | Megastructure Deep | Glowing blue pit under the cliffs, machines, bridges | Endgame, the one door |

**Authority coordinates (do not move):** Vault 0,0,-80.9. Countess 809,-309,64.7. RUNNER spawn 0,0,100. Keeper 900,0. Seer -450,780. Cartographer -4150,-780. Cache 0,-6200. Beacons 2800,500 / -2000,-900 / 600,-2100. Nest cliffs -450,780.

# 5. Systems (north star)

- **Movement:** sprint, climb, phi-wing glide, fly (unlocked), skiff and mounts.
- **GODMODE menu:** fly and noclip (separate), infinite health / ATP / stamina / ammo, damage x2 / x5 / x10 / one-hit, freeze all, kill all, teleport, time / weather / spawn, speed and jump, save anywhere. "GODMODE: ALL OFF" button. Cheats reset to off on restart.
- **Assists menu:** each assist toggleable, "ASSISTS: ALL OFF".
- **Dialogue:** branching, choices with consequence, skill checks (Witcher 3 / BG3 / Cyberpunk / New Vegas feel). NPCs remember help and harm.
- **Combat:** enemies target the player first. Light, heavy, dodge, signal abilities.
- **Freedom:** bond and marry, steal / jail / escape, hunt / fish / farm / tame, craft and build (persistent), dig / mine / chop.
- **Mitochondrial engine:** ATP is the energy stat that feeds stamina and abilities.
- **Maps:** full-colour world map and minimap, fog of war, markers.
- **Save:** one save slot plus autosave. Level, XP, gold, beacons, deaths, position, region, quests all persist.

# 6. Build order (each milestone must pass before the next)

| M | Milestone | Proof required |
|---|---|---|
| M1 | **Save fix.** HUD showed LV1, gold 0, beacons 0/3 while saves claimed level 4, gold 200. | Quit and reload shows the same values; log shows loaded=1 |
| M2 | **Core loop.** Load, Keeper count, 3 beacons, fight, die, respawn, save, quit, reload. | Saved\\seat719_out.txt reads SAVE RESULT True; level, gold, beacons, deaths persist |
| M3 | **New HUD** per section 3, old gray/yellow panels deleted. | Screenshot in Play matching the spec |
| M4 | **Hub slice** matching the painting: pillars, market, NPCs, cliffs, waterfalls, Spire on horizon, golden hour. | Fly-around screenshot, no gray boxes, 60 fps target |
| M5 | **Glide and fly** plus GODMODE menu with ALL OFF. | Play test of each toggle |
| M6 | **Clean.** 0 errors, 0 warnings in build and editor log. No per-frame log spam (log only on change). | Build line and log tail |
| M7+ | Nest, then Spire, then Omni District, then Megastructure Deep, each to the same bar. Then systems, then story to 60-100 hours. | Same proofs per region |

# 7. The loop (run it nonstop)

1. Edit code.
2. Build:\
`UnrealBuildTool.exe VEIL Win64 Development -Project="D:\VEIL-OMNI\VEIL.uproject" -WaitMutex -FromMsBuild -MaxParallelActions=12`\
Must end Succeeded, 0 errors, 0 warnings.
3. Bake the world: Build Lighting, Build Geometry, Build Navigation, Build All. Wait until done. A code build is not a world build. A bake is not a cook. A cook is not a package.
4. Play and test the milestone.
5. Fix in-flow. Same error twice means change approach. Never repeat a failed fix.

# 8. Reporting rules

- No claim, screenshot, or "done" until you verified it yourself in Play.
- No "looks done", "almost done", "should work", "in my head".
- Report only when a milestone passes: milestone number, build line, seat719_out.txt contents, save values before and after reload, one Play screenshot.

# 9. Known issues

- Editor log spams `ssl_client_socket_impl.cc handshake failed, net_error -100`. That is the editor's built-in browser, not VEIL code. Disable the Fab plugin (Edit > Plugins) if it keeps spamming.
- Debug text overlays clutter the screen. Move to an F10 toggle.
