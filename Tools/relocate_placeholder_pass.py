"""Keep exactly ONE placeholder sweep, and keep it inside AVEILGameMode::BeginPlay.

Boundaries are taken from the pass's own code, not from a neighbouring comment: a copy starts at
its first comment line and ends at the closing brace after its "2.2f, false);" timer line. Every
copy is removed, the enclosing function of each is printed, and one copy is inserted after
BeginPlay's opening brace, where the other measured passes run.
"""
import io
import re

P = "Source/VEIL/Private/Game/VEILGameMode.cpp"
s = io.open(P, encoding="utf-8", newline="").read()
N = "\r\n" if "\r\n" in s else "\n"
START = "\t// NO PLACEHOLDER SURVIVES THE BOOT."
TAIL = "2.2f, false);"


def enclosing(text, pos):
    best = "<none>"
    for m in re.finditer(r"AVEILGameMode::(\w+)\s*\(", text[:pos]):
        best = m.group(1)
    return best


spans = []
at = 0
while True:
    a = s.find(START, at)
    if a < 0:
        break
    c = s.find(TAIL, a)
    assert c > 0, "timer line not found after copy at %d" % a
    j = s.find("\t}", c)
    assert j > 0, "closing brace not found after timer line"
    end = j + len("\t}")
    if s[end:end + 2] == N:
        end += len(N)
    spans.append((a, end, enclosing(s, a)))
    at = end

print("copies found: %d" % len(spans))
for a, e, fn in spans:
    print("  copy at char %d (line %d) inside %s()" % (a, s[:a].count(N) + 1, fn))
assert spans, "the pass is not in the file"

block = s[spans[0][0]:spans[0][1]]
assert block.count(TAIL) == 1, "block holds %d timer lines" % block.count(TAIL)
assert block.count(START) == 1, "block holds %d start markers" % block.count(START)

for a, e, fn in reversed(spans):
    s = s[:a] + s[e:]

anchor = "void AVEILGameMode::BeginPlay()" + N + "{" + N
assert s.count(anchor) == 1, "BeginPlay anchor count=%d" % s.count(anchor)
i = s.index(anchor) + len(anchor)
s = s[:i] + block + s[i:]

assert s.count(START) == 1, "post-insert start count=%d" % s.count(START)
line = s[:s.index(START)].count(N) + 1
print("one copy now, at line %d, inside %s()" % (line, enclosing(s, s.index(START))))
io.open(P, "w", encoding="utf-8", newline="").write(s)
print("WRITTEN")
