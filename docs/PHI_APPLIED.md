# PHI APPLIED — generated from the build

**This file is derived.** It is written by `Scripts/gen_phi_applied.py` from
`docs/world_layout.json` and the run log; it is not a spec and nothing is copied into it by
hand. Ruling 5 (2026-11-26): the build is the authority, the docs follow it. The previous
hand-written version of this file claimed "1200 wide, 5000 tall at (0, 3500)" — the size was
right by luck, the coordinate was the bible's and the generator never built it.

Generated 2026-11-26 from `m4_city9.log`.

## The world ladder

| number | value | source |
|---|---|---|
| seed | 1618 | `seed` |
| phi | 1.618033988749895 | `phi` |
| golden angle | 137.5077640500378 deg | `golden_angle_deg` |
| base ring | 4045 cm | `base_ring_cm` |
| rings | 5 | `ring_count` |
| ring names | Hub, Nest, Spire, Omni, Deep | `ring_names` |
| ring counts | [21, 13, 8, 21, 55] | `counts` |

- ring 0: r_cm=4045 inner_cm=520 count=21 placed=29
- ring 1: r_cm=6545 inner_cm=4045 count=13 placed=13
- ring 2: r_cm=10590 inner_cm=6545 count=8 placed=8
- ring 3: r_cm=17135 inner_cm=10590 count=21 placed=98
- ring 4: r_cm=27725 inner_cm=17135 count=55 placed=55

## The Omni District (M4 city layer)

| number | value | source |
|---|---|---|
| blocks | 98 | `VEIL CITY` |
| tallest block | 3600 cm | `VEIL CITY` |
| height cap vs Spire | 0.72 x 5000 cm | `city_height_cap` |
| light pillars | 19 | `city_light_pillars` |
| blocks skipped for the Spire plaza | 2 | `VEIL CITY` |
| bands / spokes | 3 / 21 | `city_bands`, `city_spokes` |
| ring roads | 4 x 833 cm | `VEIL CITY ROADS` |
| blocks skipped for the causeway | 5 | `VEIL CITY` |
| doors / signs / awnings / lamp posts | 98 / 98 / 50 / 44 | `VEIL CITY DETAIL` |
| causeway | 33 tiles, 14536 cm long, 433 cm wide | `VEIL CAUSEWAY` |
| causeway start / end | 2599 cm / 1040 cm | `VEIL CAUSEWAY` |
| causeway ramp slope | 20.9 deg (walkable) | `VEIL CAUSEWAY` |
| district crowd | 13 walkers, 13 expected, 0 failed | `VEIL CITY CROWD` |

The district is a polar street grid: bands of blocks round the ring, spokes of street between
them, a navy ring road at every band edge, a lit pane stack per floor, and one light pillar per
spoke on the rim road. It is not 21 crystal peaks: peaks have no streets, and the street is what
makes a city read as a city. No block may exceed `city_height_cap` x the Spire's height, so the
tower stays the tallest thing in every frame this world makes (checked as `city_under_spire`).

M4 adds the street layer: a doorway with light behind it, jambs and a gold sign board on
every block, a navy awning on every second one, lamp posts down every ring road, and 13
district walkers on the main street. The block massing, the window panes, the road surface
and the tower's gold each have their own material (`M_VEIL_CityStone`, `M_VEIL_CityGlass`,
`M_VEIL_CityRoad`, `M_VEIL_TowerGold`, authored by `Scripts/make_city_materials.py`).

**Walkable.** The district sits 754 cm above the market behind three terrace walls (144, 233
and 377 cm). A causeway runs from the market's rim (`base_ring / phi` = 2,500 cm) to the
district's rim, flat on each terrace and ramped at every lip with run = rise x phi^2:
20.9 deg, which is within a capsule's 45 deg step limit. The `city_walkable` check reads the
built road, not the intent: tiles exist, the deck ends level with the district, no ramp is
steeper than 45 deg.

## The Spire (Ruling 5)

| number | value | source |
|---|---|---|
| position | (0, 10590) cm | `VEIL GEN spire` |
| height | 5000 cm | `spire_height_cm` |
| base width | 1200 cm | `spire_base_width_cm` |
| steps | 8 | `spire_steps` |

10,590 = 4,045 x phi^2: the Spire stands on its own ring's outer radius. The bible's
(0, 3500) is superseded and carries a note, not a rebuild. `AVeilHubGameMode::BuildSpire()`,
the hand-built tower at (0, 9000) that was never called, is deleted.

## What the run printed

| check | result |
|---|---|
| generator validate | `pass=7 fail=0 instances=2053` |
| golden frame | `u=PASS horizon=PASS fit=PASS | fg_cm=1276 mid_cm=4045 bg_cm=10590` |
| census | `human=21 gold=8 crystal=5 deep=3 walkers=37 expected=37 failed=0` |
| census accounting | `PASS walkers_plus_failed=37 expected=37` |
| placement hash | `crc=E231685A instances=2053` |
| plaza reflection | `plaza capture radius_cm=2618 at_z=300` |

Materials: `stone=/Game/VEIL/CC0/Materials/M_CC0_Sandstone.M_CC0_Sandstone gold=/Game/VEIL/Materials/M_VEIL_GoldPBR.M_VEIL_GoldPBR navy=/Game/VEIL/Materials/M_VEIL_Deep.M_VEIL_Deep crystal=/Game/Sicka_EnvPack2/Sicka_Ancient_Kit/Materials/MI_Cliff3.MI_Cliff3`

## Not in this document

Motion, lens and HUD numbers live in `Source/VEIL/VeilCanon.h`, where the game reads them.
They are not repeated here: a second copy is the failure this file just had.
