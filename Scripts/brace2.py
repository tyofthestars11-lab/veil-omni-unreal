"""Insert the one missing closing brace above WalkAnim."""

npc = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilNpc.cpp"
lines = open(npc, encoding="utf-8", newline="").read().replace("\r\n", "\n").split("\n")

i = next((k for k, l in enumerate(lines) if "WalkAnim = LoadObject<UAnimSequence>(nullptr, AnimWalk)" in l), -1)
if i < 0:
    raise SystemExit("MISS WalkAnim")
j = max(k for k in range(i) if lines[k].strip() == "}")
if lines[j + 1].strip().startswith("}"):
    print("already closed")
else:
    lines.insert(j + 1, "\t\t}")
    open(npc, "w", encoding="utf-8", newline="").write("\n".join(lines))
    print("inserted closing brace after line", j + 1)
