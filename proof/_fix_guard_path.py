import io, os

VT = chr(11)
TARGET = "Tools\x0beil_d_drive_guard.ps1"
FIXED = "Tools" + chr(92) + "veil_d_drive_guard.ps1"

fixed = 0
for d in ('Tools', 'proof'):
    for name in sorted(os.listdir(d)):
        if not name.lower().endswith('.ps1'):
            continue
        p = os.path.join(d, name)
        with io.open(p, 'r', encoding='utf-8', newline='') as f:
            t = f.read()
        if VT in t:
            t = t.replace(TARGET, FIXED)
            with io.open(p, 'w', encoding='utf-8', newline='') as f:
                f.write(t)
            fixed += 1
print('fixed', fixed)
