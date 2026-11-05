"""Small copies of the record frames, small enough to look at."""

import glob
import os

from PIL import Image

W = "C:/Users/Admin/.freebuff/worktrees/52bfbe8a/e80fea6afa42e7c3cd60a735da7e14aecc808ccd7f84fcdd/shots/"

for f in sorted(glob.glob(W + "frame*.png")):
    im = Image.open(f)
    im.thumbnail((720, 720))
    out = f.replace("frame", "view_")
    im.convert("RGB").save(out, quality=80)
    print(f"  {os.path.basename(out)}  {im.size[0]}x{im.size[1]}  {os.path.getsize(out) // 1024} KB")
