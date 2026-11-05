"""What shapes does the kit actually contain? Name, size and count per mesh."""
import os

OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "ed_probe_meshes.txt")
open(OUT, "w", encoding="utf-8").close()


def w(line):
    with open(OUT, "a", encoding="utf-8") as f:
        f.write(str(line) + "\n")
        f.flush()


import unreal  # noqa: E402

ROOTS = ["/Game/Sicka_EnvPack2", "/Game/VEIL", "/Game/CC0"]
rows = []
for root in ROOTS:
    for path in unreal.EditorAssetLibrary.list_assets(root, recursive=True, include_folder=False):
        try:
            obj = unreal.load_asset(path)
        except Exception:  # noqa: BLE001
            continue
        cls = obj.get_class().get_name() if obj else "none"
        if cls == "StaticMesh":
            try:
                b = obj.get_bounds()
                e = b.box_extent
                v = b.sphere_radius
            except Exception:  # noqa: BLE001
                continue
            rows.append((str(obj.get_name()), e.x * 2, e.y * 2, e.z * 2, v, path))
        elif cls in ("Material", "MaterialInstanceConstant"):
            rows.append((str(obj.get_name()), 0, 0, 0, 0, path))

w("assets: %d" % len(rows))
w("%-34s %8s %8s %8s %8s  %s" % ("name", "dx", "dy", "dz", "radius", "path"))
for name, dx, dy, dz, v, path in sorted(rows, key=lambda r: r[0].lower()):
    w("%-34s %8.0f %8.0f %8.0f %8.0f  %s" % (name[:34], dx, dy, dz, v, path))
w("V MESHES DONE")
