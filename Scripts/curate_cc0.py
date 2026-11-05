"""Curate real CC0 assets by exact tag, for the painting's materials and props.

The previous pass matched substrings ("uropean" contains "rope") and pulled chairs.
This pass requires an exact tag hit and rejects furniture words in the asset name.
"""
import json
import os
import urllib.request

ROOT = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), ".."))
BASE = os.path.join(ROOT, "Assets", "CC0")
UA = {"User-Agent": "VEIL-OMNI/1.0 (project asset fetch)"}
REJECT = ("chair", "cabinet", "stool", "lamp", "light", "searchlight", "sofa",
          "table", "shelf", "vase", "bottle", "book", "clock", "camera")


def get_json(url, timeout=60):
    req = urllib.request.Request(url, headers=UA)
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return json.loads(r.read().decode("utf-8"))


def get_file(url, path, timeout=180):
    req = urllib.request.Request(url, headers=UA)
    with urllib.request.urlopen(req, timeout=timeout) as r:
        data = r.read()
    with open(path, "wb") as f:
        f.write(data)
    return len(data)


def pick(index, words, limit=2):
    """Exact tag hit, name not furniture, then shortest name for stable choice."""
    hits = []
    for key, meta in index.items():
        tags = {t.lower() for t in (meta.get("tags") or [])}
        name = key.lower()
        if any(w in name for w in REJECT):
            continue
        if tags & set(words):
            hits.append((key, sorted(tags & set(words))))
    hits.sort(key=lambda kv: (-len(kv[1]), len(kv[0])))
    return hits[:limit]


def walk_urls(node, acc):
    if isinstance(node, dict):
        if isinstance(node.get("url"), str):
            acc.append(node["url"])
        for v in node.values():
            walk_urls(v, acc)
    elif isinstance(node, list):
        for v in node:
            walk_urls(v, acc)


print("=== textures by exact tag ===")
tex = get_json("https://api.polyhaven.com/assets?type=textures")
TEX = os.path.join(BASE, "Textures2")
os.makedirs(TEX, exist_ok=True)
TEXWANT = {
    "canvas": ["fabric", "cloth", "canvas"],
    "rope": ["rope"],
    "timber": ["wood", "planks", "wooden"],
    "sandstone": ["sandstone", "stone", "bricks"],
    "cliff": ["rock", "cliff"],
    "water": ["water", "foam", "waterfall", "river"],
}
log = {"textures": [], "models": []}
for label, words in TEXWANT.items():
    chosen = pick(tex, words, 2)
    print(f"  {label:10s} -> {[c[0] for c in chosen]}  tags={[c[1] for c in chosen]}")
    for asset, _tags in chosen:
        try:
            files = get_json("https://api.polyhaven.com/files/" + asset)
        except Exception as exc:
            print("    files failed", asset, exc)
            continue
        node = (files.get("Diffuse") or files.get("diffuse") or {}).get("1k")
        urls = []
        walk_urls(node, urls)
        if not urls:
            continue
        folder = os.path.join(TEX, asset)
        os.makedirs(folder, exist_ok=True)
        url = next((u for u in urls if u.endswith(".jpg")), urls[0])
        path = os.path.join(folder, os.path.basename(url.split("?")[0]))
        try:
            get_file(url, path)
        except Exception as exc:
            print("    download failed", asset, exc)
            continue
        print(f"    saved {asset:32s} -> {os.path.relpath(path, ROOT)}")
        log["textures"].append({"category": label, "id": asset, "file": os.path.relpath(path, ROOT)})

print()
print("=== models by exact tag ===")
mods = get_json("https://api.polyhaven.com/assets?type=models")
MOD = os.path.join(BASE, "Models2")
os.makedirs(MOD, exist_ok=True)
MODWANT = {
    "rock": ["rock", "stone", "boulder"],
    "cliff": ["cliff", "cave"],
    "plant": ["plant", "tree", "fern", "grass", "bush", "flowers"],
    "timber": ["wood", "log", "plank", "barrel"],
}
for label, words in MODWANT.items():
    chosen = pick(mods, words, 3)
    print(f"  {label:10s} -> {[c[0] for c in chosen]}")
    for asset, _tags in chosen:
        try:
            files = get_json("https://api.polyhaven.com/files/" + asset)
        except Exception as exc:
            print("    files failed", asset, exc)
            continue
        gltf = files.get("gltf") or {}
        node = gltf.get("1k") or (next(iter(gltf.values())) if gltf else None)
        urls = []
        walk_urls(node, urls)
        if not urls:
            continue
        folder = os.path.join(MOD, asset)
        os.makedirs(folder, exist_ok=True)
        got = 0
        for u in urls:
            name = u.split("?")[0].rstrip("/").split("/")[-1]
            try:
                get_file(u, os.path.join(folder, name))
                got += 1
            except Exception:
                pass
        print(f"    saved {asset:32s} {got} files")
        log["models"].append({"category": label, "id": asset, "folder": os.path.relpath(folder, ROOT), "files": got})

with open(os.path.join(BASE, "manifest_curated.json"), "w", encoding="utf-8") as f:
    json.dump(log, f, indent=2)
print()
print(f"curated {len(log['textures'])} textures and {len(log['models'])} meshes")
