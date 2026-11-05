"""Fetch real CC0 textures from Poly Haven with no account, into the project.

Poly Haven's API needs no key and its CDN needs no login. We pull the 1k diffuse
of each chosen asset so the art can be authored in-project without placeholders.
"""
import json
import os
import urllib.request

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
OUT = os.path.abspath(os.path.join(ROOT, "Assets", "CC0"))
TEX = os.path.join(OUT, "Textures")
os.makedirs(TEX, exist_ok=True)

UA = {"User-Agent": "VEIL-OMNI/1.0 (project asset fetch)"}


def get_json(url, timeout=45):
    req = urllib.request.Request(url, headers=UA)
    with urllib.request.urlopen(req, timeout=timeout) as r:
        return json.loads(r.read().decode("utf-8"))


def get_file(url, path, timeout=120):
    req = urllib.request.Request(url, headers=UA)
    with urllib.request.urlopen(req, timeout=timeout) as r:
        data = r.read()
    with open(path, "wb") as f:
        f.write(data)
    return len(data)


print("indexing polyhaven...")
assets = get_json("https://api.polyhaven.com/assets?type=textures")
print("textures available:", len(assets))

CATEGORIES = {
    "canvas": ["canvas", "cloth", "fabric", "linen"],
    "rope": ["rope", "string", "cable"],
    "timber": ["wood", "planks", "timber", "beam"],
    "sandstone": ["sandstone", "brick", "stone"],
    "cliff": ["rock", "cliff", "boulder"],
    "water": ["water", "foam", "river", "waterfall"],
    "gold": ["gold", "metal", "brass"],
    "crystal": ["ice", "crystal", "snow"],
}

chosen = {}
for label, words in CATEGORIES.items():
    picked = []
    for key, meta in assets.items():
        hay = (key + " " + " ".join(meta.get("tags") or [])).lower()
        if any(w in hay for w in words):
            picked.append(key)
        if len(picked) >= 2:
            break
    chosen[label] = picked
    print(f"  {label:10s} -> {picked}")

manifest = {"source": "https://polyhaven.com (CC0)", "assets": []}
total = 0
for label, ids in chosen.items():
    for asset in ids:
        try:
            files = get_json("https://api.polyhaven.com/files/" + asset)
        except Exception as exc:
            print("  files failed", asset, exc)
            continue
        diff = (files.get("Diffuse") or files.get("diffuse") or files.get("Diff") or {})
        url = None
        for res in ("1k", "2k"):
            entry = diff.get(res)
            if isinstance(entry, dict):
                for fmt in ("jpg", "png"):
                    if fmt in entry and isinstance(entry[fmt], dict) and entry[fmt].get("url"):
                        url = entry[fmt]["url"]
                        break
            if url:
                break
        if not url:
            print("  no 1k diffuse for", asset)
            continue
        folder = os.path.join(TEX, asset)
        os.makedirs(folder, exist_ok=True)
        path = os.path.join(folder, asset + "_diff_1k.jpg")
        try:
            size = get_file(url, path)
        except Exception as exc:
            print("  download failed", asset, exc)
            continue
        total += size
        print(f"  saved {label:10s} {asset:28s} {size/1024:7.0f} KiB")
        manifest["assets"].append({"category": label, "id": asset, "file": os.path.relpath(path, ROOT), "bytes": size})

with open(os.path.join(OUT, "manifest.json"), "w", encoding="utf-8") as f:
    json.dump(manifest, f, indent=2)

print()
print(f"downloaded {len(manifest['assets'])} real CC0 textures, {total/1024/1024:.1f} MiB")
print("manifest: Assets/CC0/manifest.json")
