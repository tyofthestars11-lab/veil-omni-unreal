"""The reseat block sits inside a per-body loop; run it once, log once."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old = ("\t{\n"
       "\t\tconst double Half = PHI_Constants::Reach(5.771) * 0.5;\n"
       "\t\tint32 OnDeck = 0;")
new = ("\t{\n"
       "\t\tstatic bool bCrowdSeated = false;\n"
       "\t\tif (!bCrowdSeated)\n"
       "\t\t{\n"
       "\t\tbCrowdSeated = true;\n"
       "\t\tconst double Half = PHI_Constants::Reach(5.771) * 0.5;\n"
       "\t\tint32 OnDeck = 0;")
if old not in s:
    raise SystemExit("MISS reseat head")
s = s.replace(old, new, 1)

old_tail = ('\t\tUE_LOG(LogVEILWorld, Display,\n'
            '\t\t\tTEXT("VEIL HUB CROWD: %d bodies stood on the authored deck at %.1f"), OnDeck, GPlazaZ);\n'
            '\t}')
new_tail = ('\t\tUE_LOG(LogVEILWorld, Display,\n'
            '\t\t\tTEXT("VEIL HUB CROWD: %d bodies stood on the authored deck at %.1f"), OnDeck, GPlazaZ);\n'
            '\t\t}\n'
            '\t}')
if old_tail not in s:
    raise SystemExit("MISS reseat tail")
s = s.replace(old_tail, new_tail, 1)

open(p, "w", encoding="utf-8", newline="").write(s)
print("crowd: reseat runs once, logs once")
