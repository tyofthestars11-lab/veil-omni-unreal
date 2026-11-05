"""My cut removed the brace that closes if (Mesh && Body). Put it back once."""

npc = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilNpc.cpp"
raw = open(npc, encoding="utf-8", newline="").read().replace("\r\n", "\n")
lines = raw.split("\n")

i = next((k for k, l in enumerate(lines) if "WalkAnim = LoadObject<UAnimSequence>(nullptr, AnimWalk)" in l), -1)
if i < 0:
    raise SystemExit("MISS WalkAnim")

# find the last closing brace above it
j = max(k for k in range(i) if lines[k].strip() == "}")
if lines[j - 1].strip().startswith("//") or lines[j].strip() != "}":
    pass

# count braces in Seat() up to WalkAnim to see if one is genuinely missing
head = next((k for k, l in enumerate(lines) if "Seat(" in l and l.rstrip().endswith("{")), -1)
bal = 0
for k in range(head, i):
    bal += lines[k].count("{") - lines[k].count("}")
print("brace balance from Seat() to WalkAnim:", bal)

if bal == 1:
    lines.insert(j + 1, "\t\t}")
    open(npc, "w", encoding="utf-8", newline="").write("\n".join(lines))
    print("restored the Mesh guard closing brace")
else:
    print("no change needed")
