"""The sanctuary log printed the nearest post as the ring and vice versa."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")
old = "\t\tInside, MinD, Hold);"
new = "\t\tInside, Hold, MinD);"
if old in s:
    s = s.replace(old, new, 1)
    open(p, "w", encoding="utf-8", newline="").write(s)
    print("sanctuary: ring and nearest post in the right order")
else:
    print("args already ordered, or anchor changed")
