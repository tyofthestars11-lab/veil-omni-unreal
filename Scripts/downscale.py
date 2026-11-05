"""Shrink the rendered frames to an inspectable size."""

import glob
import os

W = "C:/Users/Admin/.freebuff/worktrees/52bfbe8a/e80fea6afa42e7c3cd60a735da7e14aecc808ccd7f84fcdd/shots/"
src = sorted(glob.glob(W + "frame*.png"))
try:
    from PIL import Image
except Exception as exc:  # noqa: BLE001
    print("PIL unavailable:", exc)
    raise SystemExit(1)

for f in src:
    im = Image.open(f)
    im.thumbnail((900, 900))
    out = f.replace("frame", "small_")
    im.convert("RGB").save(out, quality=82)
    print(f"  {os.path.basename(out)}  {im.size[0]}x{im.size[1]}  {os.path.getsize(out) // 1024} KB")
