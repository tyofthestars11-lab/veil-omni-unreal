"""Teach Tools/veil-cli.sh the marker the pass actually logs.

The script is CRLF, and the first attempt matched a bare "\n", so nothing was replaced. Every
pattern here is built with the file's own newline.
"""
import io

C = "Tools/veil-cli.sh"
c = io.open(C, encoding="utf-8", newline="").read()
M = "\r\n" if "\r\n" in c else "\n"

add = '  "PHI PLACEHOLDER SWAP"' + M
if '"PHI ENGINE SHAPE CLEANUP"' not in c:
    assert c.count(add) == 1, "verdict pattern line count=%d" % c.count(add)
    c = c.replace(add, add + '  "PHI ENGINE SHAPE CLEANUP"' + M, 1)
    print("pattern added")

old = 'still=$(grep -ahoE "PHI PLACEHOLDER SWAP: [0-9]+ component.*" "$LOG" 2>/dev/null | tail -1)'
if old in c:
    c = c.replace(old, 'still=$(grep -ahoE "PHI (PLACEHOLDER SWAP|ENGINE SHAPE CLEANUP): [0-9]+ component.*" "$LOG" 2>/dev/null | tail -1)', 1)
    print("verdict grep widened")

assert 'PHI (PLACEHOLDER SWAP|ENGINE SHAPE CLEANUP)' in c, "verdict grep not widened"
io.open(C, "w", encoding="utf-8", newline="").write(c)
print("CLI WRITTEN")
