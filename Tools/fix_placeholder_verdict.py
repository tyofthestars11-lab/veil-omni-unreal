"""Remove the duplicate sweep I inserted, and give the real pass the verdict the CLI looks for.

The pass was never missing: it lives in AVEILGameMode::BeginPlay and repeats at 1 Hz until two
idle sweeps, logging "PHI ENGINE SHAPE CLEANUP". Tools/veil-cli.sh only looked for the older
"PHI PLACEHOLDER SWAP" text, so `verify` could only ever answer INCOMPLETE - the measurement
ran the whole time and nothing read it. My earlier insert was a duplicate of that pass: it is
removed here, and the real pass now states its verdict in the words the tool checks.
"""
import io

P = "Source/VEIL/Private/Game/VEILGameMode.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"
START = "\t// NO PLACEHOLDER SURVIVES THE BOOT."

# My copies are the only ones whose first comment is followed by a bare "//" line.
key = START + N + "\t//" + N
removed = 0
while key in s:
    a = s.find(key)
    m = s.find("PHI PLACEHOLDER SWAP: %d component(s)", a)
    assert m > 0, "my log line not found after a copy"
    j = s.find(N + "\t}" + N, m)
    assert j > 0, "closing brace not found"
    s = s[:a] + s[j + len(N + "\t}" + N):]
    removed += 1
print("duplicate copies removed: %d" % removed)
assert "PHI PLACEHOLDER SWAP: %d component(s)" not in s

old = ('\t\t\t\tTEXT("PHI ENGINE SHAPE CLEANUP: %d engine-shape component(s) re-dressed with kit '
       'meshes, %d refused, %d retired as dust, %d visible /Engine/BasicShapes component(s) '
       'remain."),' + N + '\t\t\t\tSwapped, Refused, Retired, Still);')
new = ('\t\t\t\tTEXT("PHI ENGINE SHAPE CLEANUP: %d engine-shape component(s) re-dressed with kit '
       'meshes, %d refused, %d retired as dust, %d visible /Engine/BasicShapes component(s) '
       'remain - verdict %s."),' + N + '\t\t\t\tSwapped, Refused, Retired, Still,' + N +
       '\t\t\t\tStill == 0 ? TEXT("NO PLACEHOLDER IN THE FRAME") : TEXT("STILL PLACEHOLDERS"));')
assert s.count(old) == 1, "cleanup log line count=%d" % s.count(old)
s = s.replace(old, new, 1)
io.open(P, "w", encoding="utf-8", newline="").write(s)
print("cleanup line now states its verdict")

C = "Tools/veil-cli.sh"
c = io.open(C, encoding="utf-8", newline="").read()
a = '  "PHI PLACEHOLDER SWAP"' + "\n"
assert c.count(a) == 1
c = c.replace(a, a + '  "PHI ENGINE SHAPE CLEANUP"\n', 1)
b = 'still=$(grep -ahoE "PHI PLACEHOLDER SWAP: [0-9]+ component.*" "$LOG" 2>/dev/null | tail -1)'
assert c.count(b) == 1
c = c.replace(b, 'still=$(grep -ahoE "PHI (PLACEHOLDER SWAP|ENGINE SHAPE CLEANUP): [0-9]+ component.*" "$LOG" 2>/dev/null | tail -1)', 1)
io.open(C, "w", encoding="utf-8", newline="").write(c)
print("cli reads the real marker")
