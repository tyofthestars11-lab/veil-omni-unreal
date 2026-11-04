"""The verdict line says "112 engine-shape component(s)", so the extractor's "[0-9]+ component"
never matched it. Take the whole line instead of guessing its shape.
"""
import io

C = "Tools/veil-cli.sh"
c = io.open(C, encoding="utf-8", newline="").read()
old = 'still=$(grep -ahoE "PHI (PLACEHOLDER SWAP|ENGINE SHAPE CLEANUP): [0-9]+ component.*" "$LOG" 2>/dev/null | tail -1)'
new = 'still=$(grep -ahoE "PHI (PLACEHOLDER SWAP|ENGINE SHAPE CLEANUP): .*" "$LOG" 2>/dev/null | tail -1)'
assert c.count(old) == 1, "extractor count=%d" % c.count(old)
c = c.replace(old, new, 1)
io.open(C, "w", encoding="utf-8", newline="").write(c)
print("extractor widened to the whole verdict line")
