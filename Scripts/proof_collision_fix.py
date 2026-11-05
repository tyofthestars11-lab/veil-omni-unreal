"""The fall proof and the strike proof must not choose the same body."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

if "\t\tif (It->IsHunter())\n" in s:
    s = s.replace("\t\tif (It->IsHunter())\n",
                  "\t\tif (It->IsHunter() && !It->IsFallen())\n", 1)
    print("proof: chooses a standing post")

old = "hostile post(s) inside the Hub, strikes suppressed there, nearest post %.0f of %.0f"
new = "hostile post(s) inside the Hub of %.0f, none disarmed, nearest post %.0f"
if old in s:
    s = s.replace(old, new, 1)
    print("sanctuary: wording matches the rule")

open(p, "w", encoding="utf-8", newline="").write(s)
print("proof collision fix done")
