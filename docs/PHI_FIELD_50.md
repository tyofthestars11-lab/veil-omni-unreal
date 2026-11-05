# PHI OF THE FIELD — 50 open-world RPG / action-adventure games, studied

Source of truth for the game: `docs/VEIL_Bible.md`. This file is the study the Bible
asks for: what the field actually publishes, what it does not, where phi explains the
field, and exactly where VEIL sits relative to all of it.

Rule of this document, the same rule as `docs/PHI_APPLIED.md`: **phi is precise math,
it does not invent.** Nothing below is forced into phi. Where a number is not phi, it
is stated as not phi. Where a number is not published, it is stated as not published.

Evidence classes used in every row:
- **P** — publisher/developer published the figure.
- **E** — community estimate; methodology varies between sources.
- **U** — not published. Do not fill this in with a guess.

---

## 1. What the field publishes

Only two of the eight metric families a world needs are published at scale.

**World area** is the best-documented number in the field, and even it is mostly
community measurement.

| Game | Area | Class | Note |
|---|---|---|---|
| The Elder Scrolls IV: Oblivion | 41 km2 | **P** | Microsoft press release, via Wikipedia |
| Skyrim | 37 km2 | E | sources give 14.2 sq mi, 37 and 39 km2 |
| The Witcher 3 | 122-142 km2 | E | sources disagree by 20 km2 |
| Elden Ring | 79 km2 | E | surface only, underground omitted |
| GTA V | 81 km2 | E | includes water |
| Red Dead Redemption 2 | 75 km2 | E | 29 sq mi |
| Cyberpunk 2077 | 100-110 km2 | E | includes Badlands |
| Spider-Man (2018) | 11.9 km2 | E | quarter-scale Manhattan |
| Ghost of Tsushima | ~28 km2 | E | 11 sq mi |
| Rise of the Ronin | ~27 km2 | E | 10.4 sq mi |
| Horizon Zero Dawn | U | U | Guerrilla documented that they **shrank** the map |
| Breath of the Wild / Tears of the Kingdom | U | U | no official area |
| Dark Souls 1/3, Bloodborne, Sekiro | U | U | not published |
| Horizon Forbidden West | U | U | not published |
| Assassin's Creed Origins / Odyssey / Valhalla / Black Flag | U | U | not published |
| Far Cry 3 / 4 / 5 / 6 | U | U | not published |
| Just Cause 2 / 3 | U | U | not published |
| Dying Light 1 / 2 | U | U | not published |
| Days Gone | U | U | not published |
| God of War (2018) / Ragnarok | U | U | not published |
| Death Stranding | U | U | not published |
| Metal Gear Solid V | U | U | not published |
| Kingdom Come: Deliverance | U | U | not published |
| Dragon Age: Inquisition | U | U | not published |
| Mass Effect: Andromeda | U | U | not published |
| No Man's Sky | U | U | planet count published, per-planet area not |
| Starfield | U | U | not published |
| WoW / ESO / FFXIV | U | U | zone counts published, zone area not |
| Immortals Fenyx Rising | U | U | not published |
| Watch Dogs 2 | U | U | not published |
| Sleeping Dogs | U | U | not published |
| Saints Row (2022) | U | U | not published |
| Mad Max | U | U | not published |
| ARK: Survival Evolved | U | U | not published |
| Valheim | U | U | not published |
| Outer Wilds | U | U | not published |
| Subnautica | U | U | not published |
| Genshin Impact | U | U | not published |
| Xenoblade Chronicles 3 | U | U | not published |
| Monster Hunter: World | U | U | not published |
| NieR: Automata | U | U | not published |
| Hogwarts Legacy | U | U | not published |
| Baldur's Gate 3 | U | U | not published |
| Divinity: Original Sin 2 | U | U | not published |
| Dragon's Dogma 2 | U | U | not published |
| STALKER 2 | U | U | not published |
| Kingdoms of Amalur | U | U | not published |
| Morrowind / Fallout 3 / 4 / 76 | U | U | not published |
| GTA III / Vice City / San Andreas | U | U | not published |
| Red Dead Redemption (1) | U | U | not published |

**Draw distance, field of view, camera offsets, crowd caps, LOD ratios, and per-game
frame budgets are not published for any of the fifty.** Where these circulate, they
come from shipped INI files or datamining, which is a different evidence class from a
publisher figure and is labelled separately in this project.

Engine-side numbers that *are* published, and that every one of those fifty games
either uses or competes with (Epic documentation, read directly):

| Parameter | Published value | Class |
|---|---|---|
| World Partition cell size | 256 m = 25 600 cm | **P** |
| World Partition loading range | 768 m = 76 800 cm radius | **P** |
| Open World template map | 2 km x 2 km | **P** |
| Lumen budget at 30 fps | 8 ms at 1080p | **P** |
| Lumen budget at 60 fps | 4 ms at 1080p | **P** |
| Lumen far field | 1 km (near field 200 m) | **P** |
| Virtual shadow maps | 16k x 16k, 128 x 128 pages, clipmaps 6-22 (64 cm to ~40 km) | **P** |
| Texture pool tiers | 200 / 400 / 700 / 1000 MB | **P** |
| Tonemapper | ACES filmic, exposure bias 0, 2^n scaling | **P** |
| TSR in Fortnite on PS5 | 1.5 ms effective, 1.1 ms critical path | **P** |
| Nanite instance hard cap | 16 million | **P** |
| Frame interval at 60 fps | 16.6 ms | arithmetic |
| Golden ratio in an engine default | **none found** | negative finding |

That last row matters for the whole thesis: **the golden ratio is not in the engine.**
1.33 / 1.77 / 2.39 are film presets, not 1.618. Phi is not a gift of Unreal; it has to
be authored, and this project authors it.

---

## 2. Where phi actually explains the field

Two relations survive contact with published numbers, and both come from the field,
not from invention.

**Frame budget and Lumen.** 16.6 ms divided by phi cubed is 3.92 ms. Epic publishes a
4 ms Lumen budget at 1080p for 60 fps. Frame budget / phi^3 is the published GI
budget to within 2 percent. That is a real relation, and it means the field is already
spending one phi cube on light, two on everything else.

**Streaming radius and cell size.** Loading range 76 800 cm over cell 25 600 cm is
exactly 3.0 cells: the field streams three cells in every direction. That is a
Cartesian constant, and it is not phi — which is exactly the point of section 4.

Area is scale-only: a world of area A has characteristic length L = sqrt(A).
Skyrim is 6.08 km across, GTA V is 9.00 km, Spider-Man is 3.45 km. Those are the real
numbers the field actually ships.

---

## 3. The phi ladder, and where VEIL actually sits

Every world can be put on one ladder:

```
L(n) = Spacing x phi^n        Spacing = 100 phi = 161.803 cm
```

| n | L(n) |
|---|---|
| 0 | 1.62 m |
| 2 | 4.24 m |
| 4 | 11.09 m |
| 6 | 29.03 m |
| 8 | 76.01 m |
| 9 | 122.99 m |
| 10 | 198.99 m |
| 12 | 521.0 m |
| 14 | 1364 m |
| 16 | 3571 m |
| 18 | 9349 m |

Now measure the Bible's own authority coordinates in that ladder. Hub (0, 0), Spire
(0, 3500), Omni (2800, 500), Deep (0, -5000), Cartographer (-4150, -780), Cache
(0, -6200). Unreal units are centimetres, so the whole world spans 100 m from the Hub
to the Cache, and the 50 sites sit inside 11.4 m.

**That is n = 8 to n = 9 on the ladder. Every open world in section 1 is n = 14 to
n = 16.** Skyrim at 6.08 km across is 44 times the span of the Bible coordinates.
VEIL as written is a plaza with region *names*, not an open world. That is the honest
measurement, and it is why the frame has been reading as a small diorama.

The +1 here is a single number, not a redesign: **the world scale exponent.** Scale
every authority coordinate and every GoldenLoc radius by phi^k with k = 9, and the
Bible's own lattice becomes an open world without moving one relative position:

- Hub to Cache: 100 m -> 12.30 km (Skyrim is 6.08 km across; GTA V is 9.00 km)
- Hub to Spire: 35 m -> 4.30 km (visible from everywhere, as the Bible requires)
- 50 sites: 11.4 m -> 1.40 km ring, each site a real walk
- Spacing: 161.803 cm -> 199 m between neighbouring seat rings

k is the only free parameter, and it belongs in `PHI_Constants` as `WorldScale`, so
phi still orders the whole world and the engine carries the scale. k = 9 is the
recommendation: it puts VEIL between Skyrim and GTA V in span while keeping the 60-100
hour story inside a walkable, streamable world.

---

## 4. The +1, stated plainly

Phi is the ordering law: golden angle placement, spacing x phi^n, golden-section lens,
one save, one door. Phi is *not* the engine, and the field does not use it.

The +1 is everything the field proved and the Bible already names as craft:

- **Light is a phi cube of the frame** (4 ms of 16.6 ms), which is Epic's own number.
- **Streaming is three cells**, which is the field's Cartesian 3.0, added on top.
- **ACES filmic, exposure bias 0**, so no frame blows out.
- **HLOD and Nanite** instead of hand-made LOD ratios, which nobody publishes.
- **World Partition 256 m cells**, so a 12.3 km world streams in 48-cell spans.
- **Texture pool 1000 MB at Epic**, held by budgets rather than hope.

So the claim "VEIL is those games but infinitely better" resolves to something
checkable rather than a boast:

1. Same span as Skyrim-to-GTA V, once k = 9 is applied (the field's scale, phi-exact).
2. Same light budget as Epic's own 60 fps target, spent as frame / phi^3.
3. Placement that no game in section 1 can reproduce: 50 named features on
   golden-angle seats, every distance a power of phi, deterministic from seed 1618.
4. No placeholder geometry, no gray boxes, no floating props: every piece traces the
   real ground under its own feet.

None of that is "infinitely better" by assertion. It is one ladder, one exponent, and
four published budgets, each of which can be read in a log line or a PNG.

---

## 5. Open contradictions to fix, in order

1. **World scale exponent k = 9 is not applied yet.** Until it is, VEIL is a plaza and
   the study predicts exactly what the frames show.
2. **Crowd AI**: 28 bodies exist, four peoples, real characters, ground-traced. They
   need NavMesh and a StateTree/behaviour layer before they read as a town rather than
   walkers on golden-angle bearings.
3. **Fight, quests, mounts, save/load** are Bible systems with no code yet. The 50
   sites are lanterns; each one has to grow its system.
4. **Bake** per Bible section 7 is a build of lighting, geometry and navigation on the
   authored map; it is not a cook and not a package.
