"""UE_LOG verbosity is a compile-time constant; a ternary cannot choose it."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

old_start = s.find("\tUE_LOG(LogVEILWorld, bPhiBoot && bPhiWhole ? Display : Error,")
if old_start < 0:
    print("gate log already constant")
else:
    end = s.find(");", old_start) + 2
    new = """\tif (bPhiBoot && bPhiWhole)
\t{
\t\tUE_LOG(LogVEILWorld, Display,
\t\t\tTEXT("VEIL PHI GATE: ENFORCED AND PASSED | reach enemy %.1f | reach strike %.1f | sanctuary %.1f | fov %.3f"),
\t\t\tPHI_Constants::Reach(1.283), PHI_Constants::Reach(2.2), PHI_Constants::Reach(2.0),
\t\t\t100.0 / PHI_Constants::Phi);
\t}
\telse
\t{
\t\tUE_LOG(LogVEILWorld, Error,
\t\t\tTEXT("VEIL PHI GATE: FAILED | boot %d | whole %d | reach enemy %.1f"),
\t\t\tbPhiBoot ? 1 : 0, bPhiWhole ? 1 : 0, PHI_Constants::Reach(1.283));
\t}"""
    s = s[:old_start] + new + s[end:]
    open(p, "w", encoding="utf-8", newline="").write(s)
    print("gate: constant verbosity, two logs")
