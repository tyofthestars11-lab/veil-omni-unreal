# HUB PAINTING SPEC — THE LAST SIGNAL

Read from the reference painting. This is now the composition authority for the Hub,
the Omni city and the Deep, alongside the coordinates in `VEIL_Bible.md`.

## What the painting actually shows

Reading it left to right, back to front:

1. **Runner in the air.** Navy cape, gold trim, two gold phi wings with a lit golden-ratio
   glyph on each membrane. Arms spread, feet clear of the ground: this is the glide, not a jump.
2. **Gold Spire, centre rear.** A stepped gold tower, five or more bands, narrow windows lit
   warm, standing *on* rock and passing through cloud. Fresh lightning behind it. It has a
   visible base on the landmass — it does not hang in the sky.
3. **Market terrace, foreground.** Gold square-section pillars with a glowing phi panel near
   the top, standing *among* the stalls, not in a ring far outside them. Blue-canvas awnings,
   timber posts, warm lantern light, crowds of small figures, stone paving, rope-and-plank
   spans between terraces.
4. **Cliff and water.** Tall white-blue waterfalls leaving the terrace edges into mist,
   several of them, in the left and centre ground.
5. **Blue city, mid right.** Dense rectangular blocks with warm gold and cold blue light,
   vertical light shafts rising out of the plazas, sitting on a plateau above a chasm.
6. **Crystal peaks, far right.** Faceted blue crystal mountains, bright, sharp, snow behind.
7. **The Deep, lower right.** A chasm directly under the city, with rectangular machine
   platforms, blue glow strips, walkways and falls dropping into it.
8. **Sky.** Golden hour on the left, storm and lightning on the right. Cloud sits *below* the
   Spire and *around* the city.

## Rules this gives us that the code did not have

- Pillars belong inside the market footprint, at plaza scale, and carry a phi glyph panel.
- Stalls are blue canvas, not stone.
- Houses are stone with timber, pitched, and stepped up the slope in terraces.
- Bridges are rope-and-plank spans between terraces, not free spans into air.
- Waterfalls are part of the composition, at the terrace lips.
- The city is a plateau mass with light shafts; the chasm and the Deep are *below* it.
- The Spire is grounded on rock, with cloud between it and the eye.
- Nothing in frame is a bare gray box, and nothing floats.

## State in code after this pass

Implemented and compiling (`VEILOMNIEditor Win64 Development`, Result: Succeeded):

- Hub on one flat 26 m stone terrace; 12 gold pillars, 8 houses, 8 blue-canvas stalls,
  lanterns, Vault at -80.9. Counts are logged against the Kraken JSON: 12 + 8 + 5 = 25,
  printed as `VEIL HUB COUNTS: PASS|FAIL`.
- 5 faceted crystal peaks at the Hub authority coordinate, signal-blue material.
- Spire seated on a real rock plinth: plinth on the ground, gold tower based at the
  authority +500, no floating crown.
- Four enemy varieties, one per region: Crystal Guardian, Gold Warrior, Deep Dweller,
  Beacon Shadow. Each holds its own post, reach 300, follow false, orbit false.
- Five peoples: Human, Gold, Crystal, Deep, plus the enemy bodies. Per-kind tallies logged.
- Full floating audit on every boot.

Still missing against this painting, in priority order:

1. Phi glyph panels on the pillars (needs a glyph material or a decal, authored in editor).
2. Waterfall meshes at the terrace lips. The current packs contain no waterfall mesh;
   this needs a real asset import, not a stretched plane.
3. Rope-and-plank bridge spans between terraces.
4. Canvas awning geometry that reads as cloth, not a dome.
5. Pitched stone-and-timber house bodies.
6. Runner's gold phi wings as visible geometry during glide.
7. The HUD gold ribbon and `[E]` dialogue.
