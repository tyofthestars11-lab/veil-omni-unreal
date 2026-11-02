# VEIL OMNI — PACKAGING AND SIZE LAW

Measured, not estimated. Every number here came out of `Saved/StagedBuilds` on
2026-11-02, from the same cook that produced the build that was actually run.

## 1. What the package is

`Saved/StagedBuilds/Windows/VEIL.exe` plus `VEIL/Content/Paks/`. The exe is a
launcher; the game is the container.

| Piece | Size |
|---|---|
| `VEIL/Binaries/Win64/VEIL.exe` (the game binary) | 365 MB |
| `VEIL/Content/Paks/VEIL-Windows.ucas` (cooked content) | 1.6 GB |
| `VEIL-Windows.utoc` (container index) | 640 KB |
| `VEIL-Windows.pak` (non-IoStore leftovers) | 11 MB |
| `Engine/` (shader libs it needs) | 75 MB |
| **Total playable build** | **2.4 GB** |
| Source `Content/` on disk | 12 GB |

That is a **7.5 : 1** reduction from source content to shipped content, achieved
entirely by the cooker only packing what the maps and code reference.

## 2. The size goal

**Target 15–20 GB shipped. Hard ceiling 25 GB. Not 50, not 100.**

A 50–100 GB install is a consequence of hand-placing thousands of unique meshes
and 8K textures. VEIL's world is code: 10,201 terrain tiles, 236 composition
pieces, 40 species bodies, 24 mines, 50 activities, six acts — all of it from
2.6 MB of C++. Buying gigabytes would make the game smaller in spirit, not
bigger.

Budget for the full game, when art and audio exist:

| Bucket | Ceiling |
|---|---|
| Geometry + world art | 4 GB |
| Textures (4K on hero assets only) | 7 GB |
| Audio: music, ambience, SFX | 4 GB |
| Voice | 2 GB |
| Animation per species | 2 GB |
| Binary + shaders | 1 GB |
| Cinematics (optional) | 3 GB |

## 3. Compression: already Kraken-class

Sony's Kraken is **Oodle**, made by RAD. Oodle ships with Unreal Engine at no
extra cost and no royalty, and UE5's IoStore container is compressed with it by
default. The cook log proves it is in the pipeline:

```
LogTextureFormatOodle: Oodle Texture TFO init; latest sdk version = 2.9.16
LogTextureFormatOodle: Oodle Texture loading DLL: oo2tex_win64_2.9.16.dll
LogTextureEncodingSettings: RDO Off Lambda=0, Effort=Normal
```

So "kraken it like Sony" is not a future step — it is what produced the 1.6 GB
container above. There is no second compressor that will meaningfully beat the
cooker's own packing on this content, because the cooker already discards
everything the game does not reference.

Options if Oodle's licence is ever a problem (it is free inside UE, but it is not
open source):

- **Zstd** — open, in the engine, roughly 10–20% worse ratio, much faster.
- **Store-only** — no pak compression at all. Larger download, near-zero load
  cost. Only worth it on SSD-first distribution.

A third-party outer compressor (a "TON618" wrapper) can only compress the
*already packed* container, where the easy redundancy is gone; expect a few per
cent, not a transformation. It is worth doing only if it is genuinely theirs and
measured — never assumed.

## 4. Rules

1. Measure after every cook. `Tools/size_report.sh` prints staged size against
   source content.
2. Content that the world does not reference must not ship. The cooker enforces
   this; do not defeat it by adding "always cook" directories.
3. Never grow the binary to save content. 365 MB of code replaced gigabytes of
   pre-placed assets, and that trade is the whole point of the project.
4. The size ceiling is a **patching** rule: every shipped gigabyte is downloaded
   again on each content patch.
