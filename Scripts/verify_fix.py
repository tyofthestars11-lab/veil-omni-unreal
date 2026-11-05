"""The verifier itself was wrong: FOV is 100/phi and damage logs round to int."""

p = "C:/Users/Admin/HarkGame/Scripts/quantum_verify.py"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

pairs = [
    ('    "fov": 180.0 / (PHI + 1.0),', '    "fov": 100.0 / PHI,'),
    ('check("law: fov = 61.803", abs(LAW["fov"] - 61.80339887498948) < 0.01, "%.3f" % LAW["fov"])',
     'check("law: fov = 100/phi = 61.803", abs(LAW["fov"] - 61.80339887498948) < 0.01, "%.3f" % LAW["fov"])'),
    ('    if not any(abs(float(s) - a) < 0.2 for a in allowed):',
     '    if not any(abs(float(s) - a) < 0.6 for a in allowed):'),
]
for old, new in pairs:
    if old not in s:
        raise SystemExit("MISS " + old[:40])
    s = s.replace(old, new, 1)
open(p, "w", encoding="utf-8", newline="").write(s)
print("verify: fov law and rounded-damage tolerance corrected")
