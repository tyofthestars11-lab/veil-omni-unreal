import io, os, re, sys

HEADER = [
    "# NON-DELETE GUARD: the ENTIRE D: drive is protected. Tyree's word only lifts it.\n",
    ". (Join-Path (Split-Path $PSScriptRoot -Parent) 'Tools\veil_d_drive_guard.ps1')\n",
    "Assert-VeilDDriveSafe -Action Write -Path $PSCommandPath -Script $PSCommandPath\n",
    "\n",
]

def inject(path):
    with io.open(path, 'r', encoding='utf-8-sig') as f:
        text = f.read()
    if 'veil_d_drive_guard' in text:
        return 'already'
    lines = text.splitlines(keepends=True)
    i = 0
    n = len(lines)
    while i < n:
        s = lines[i].strip()
        if s == '' or s.startswith('#'):
            i += 1
            continue
        break
    if i >= n:
        return 'empty'
    # leading param block?
    if re.match(r'(?i)^\s*param\s*\(', lines[i]):
        depth = 0
        j = i
        while j < n:
            for ch in lines[j]:
                if ch == '(':
                    depth += 1
                elif ch == ')':
                    depth -= 1
            if depth <= 0:
                break
            j += 1
        insert_at = j + 1
    else:
        insert_at = i
    out = lines[:insert_at] + HEADER + lines[insert_at:]
    with io.open(path, 'w', encoding='utf-8', newline='') as f:
        f.write(''.join(out))
    return 'ok'

changed = skipped = 0
for d in ('Tools', 'proof'):
    for name in sorted(os.listdir(d)):
        if not name.lower().endswith('.ps1'):
            continue
        p = os.path.join(d, name)
        if name == 'veil_d_drive_guard.ps1':
            continue
        r = inject(p)
        if r == 'ok':
            changed += 1
        else:
            skipped += 1
print('injected', changed, 'skipped', skipped)
