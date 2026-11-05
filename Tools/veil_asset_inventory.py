"""
VEIL asset inventory. Read-only. Run in the Unreal editor:
  Window > Developer Tools > Output Log, switch the Cmd dropdown to "Python", then:

      exec(open(r"<repo>\\Tools\\veil_asset_inventory.py").read())

Writes Saved/VEIL_asset_inventory.json and prints a summary of what actually exists
in /Game, grouped by the Bible §2/§4 needs (gold, crystal, rune, pillar, stall,
lantern, cliff, water, spire, foliage, rock, crowd, character).

This exists because placement code must use real asset paths. No text-file .uasset.
"""
import unreal
import json
import os
import time

GAME_ROOT = "/Game"
OUT_JSON = os.path.join(unreal.Paths.project_saved_dir(), "VEIL_asset_inventory.json")

NEEDS = {
    "gold_material": ["gold", "gilded", "brass", "phi"],
    "crystal_material": ["crystal", "shard", "gem", "blue"],
    "rune_material": ["rune", "glyph", "sigil", "emissive"],
    "pillar": ["pillar", "column", "obelisk", "monolith"],
    "market": ["stall", "market", "awning", "crate", "barrel", "cart"],
    "lantern": ["lantern", "lamp", "torch", "brazier", "candle"],
    "cliff_rock": ["cliff", "rock", "boulder", "crag", "stone"],
    "water": ["water", "river", "fall", "cascade", "pond"],
    "spire": ["spire", "tower", "steeple"],
    "foliage": ["tree", "bush", "grass", "fern", "palm", "vine", "plant"],
    "ground": ["ground", "terrain", "sand", "dirt", "floor", "path", "road"],
    "fabric": ["cloth", "banner", "flag", "rug", "sail", "rope"],
    "character": ["mannequin", "character", "human", "npc", "body"],
}

MESH_CLASSES = ["StaticMesh"]
MAT_CLASSES = ["Material", "MaterialInstanceConstant", "MaterialFunction"]
OTHER_CLASSES = MESH_CLASSES + MAT_CLASSES + ["Texture2D", "Blueprint", "StaticMeshActor",
                                              "SkeletalMesh", "NiagaraSystem", "Level"]


def _class_name(asset_data):
    try:
        return str(asset_data.asset_class_path.asset_name)
    except Exception:
        return str(asset_data.asset_class)


def main():
    started = time.time()
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    registry.scan_paths_synchronous([GAME_ROOT], force_rescan=False)

    filt = unreal.ARFilter(
        package_paths=[GAME_ROOT],
        recursive_paths=True,
        class_names=OTHER_CLASSES,
        recursive_classes=True,
    )
    try:
        assets = registry.get_assets(filt)
    except Exception as err:
        unreal.log_error("VEIL inventory: registry query failed: %s" % err)
        return

    rows = []
    for ad in assets:
        cls = _class_name(ad)
        path = str(ad.package_name) + "." + str(ad.asset_name)
        row = {"name": str(ad.asset_name), "path": path, "class": cls}
        if cls in MESH_CLASSES:
            try:
                mesh = ad.get_asset()
                box = mesh.get_bounding_box()
                row["size_cm"] = [round(box.max.x - box.min.x, 1),
                                  round(box.max.y - box.min.y, 1),
                                  round(box.max.z - box.min.z, 1)]
                row["materials"] = [str(m.get_path_name()).split(".")[-1]
                                    for m in mesh.get_editor_property("static_materials")]
            except Exception as err:
                row["error"] = str(err)
        rows.append(row)

    by_need = {}
    for need, keys in NEEDS.items():
        hits = [r for r in rows if any(k in r["name"].lower() for k in keys)]
        by_need[need] = sorted({r["path"] for r in hits})

    payload = {
        "generated_utc": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "project": unreal.Paths.project_dir(),
        "total_assets": len(rows),
        "by_class": {},
        "by_need": by_need,
        "assets": sorted(rows, key=lambda r: r["path"]),
    }
    for r in rows:
        payload["by_class"][r["class"]] = payload["by_class"].get(r["class"], 0) + 1

    with open(OUT_JSON, "w") as fh:
        json.dump(payload, fh, indent=2)

    unreal.log("=" * 70)
    unreal.log("VEIL ASSET INVENTORY -> %s" % OUT_JSON)
    unreal.log("total assets under %s: %d  (%.1fs)" % (GAME_ROOT, len(rows), time.time() - started))
    for cls, count in sorted(payload["by_class"].items(), key=lambda kv: -kv[1]):
        unreal.log("  %-28s %d" % (cls, count))
    unreal.log("-" * 70)
    for need in sorted(by_need):
        hits = by_need[need]
        unreal.log("  %-18s %3d  %s" % (need, len(hits), hits[0] if hits else "<none found>"))
    unreal.log("=" * 70)


main()
