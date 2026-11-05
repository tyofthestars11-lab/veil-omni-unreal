"""Point the authoring and dress scripts at a map and height from the environment."""

FILES = {
    "C:/Users/Admin/HarkGame/Scripts/ed_hub_v6.py": [
        ('MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"',
         'MAP = os.environ.get("VEIL_MAP", "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT")'),
        ("ROCK_LOW = 182041.0", 'ROCK_LOW = float(os.environ.get("VEIL_ROCKLOW", "182041.0"))'),
        ("ROCK_HIGH = 189158.0", 'ROCK_HIGH = float(os.environ.get("VEIL_ROCKHIGH", "189158.0"))'),
        ("DECK = 197600.0", 'DECK = float(os.environ.get("VEIL_DECK", "197600.0"))'),
        ("BASE = 181800.0", 'BASE = float(os.environ.get("VEIL_BASE", "181800.0"))'),
    ],
    "C:/Users/Admin/HarkGame/Scripts/ed_dress.py": [
        ('MAP = "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT"',
         'MAP = os.environ.get("VEIL_MAP", "/Game/Iceland_Environment/Example_Map/Lv_Mountain_Tops_VT")'),
        ("DECK = 197600.0", 'DECK = float(os.environ.get("VEIL_DECK", "197600.0"))'),
    ],
}

for path, pairs in FILES.items():
    s = open(path, encoding="utf-8", newline="").read().replace("\r\n", "\n")
    out = []
    for old, new in pairs:
        if new in s:
            out.append("already: " + old[:34])
        elif old in s:
            s = s.replace(old, new, 1)
            out.append("patched: " + old[:34])
        else:
            out.append("MISS: " + old[:34])
    open(path, "w", encoding="utf-8", newline="").write(s)
    print(path.rsplit("/", 1)[-1])
    for line in out:
        print("   " + line)
