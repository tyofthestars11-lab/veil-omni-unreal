"""Repair the phi-whole gate so its formulas match the math, not the other way round."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


c = rd(P + "PHI_Constants.cpp")
before = c

# golden angle is 360 * (1 - 1/phi), which is 137.5077640500378
c = c.replace("360.0 - 360.0 / (Phi * Phi)", "360.0 * (1.0 - 1.0 / Phi)")
# the camera lens in VeilRunner is 100/phi = 61.803398874989, not 360/(phi+4)
c = c.replace("360.0 / (Phi + 4.0)", "100.0 / Phi")

# the respawn row was a number that exists nowhere in the game: drop it entirely
lines = c.split("\n")
lines = [ln for ln in lines if "respawn height" not in ln]
c = "\n".join(lines)

# the summary no longer claims a LAW count it never computed
c = c.replace('TEXT("VEIL PHI WHOLE: %d PASS, %d FAIL, %d LAW (Bible numbers phi does not govern)"),\n\t\tPass, Fail, 6);',
              'TEXT("VEIL PHI WHOLE: %d PASS, %d FAIL"), Pass, Fail);')
c = c.replace('TEXT("VEIL PHI WHOLE: %d PASS, %d FAIL"), Pass, Fail);',
              'TEXT("VEIL PHI WHOLE: %d PASS, %d FAIL"), Pass, Fail);')

wr(P + "PHI_Constants.cpp", c)
print("gate formulas corrected:", c != before)

# the FOV comment in the runner was the wrong formula; the number is right
r = rd(P + "VeilRunner.cpp")
rb = r
r = r.replace("// The lens is phi too: 360 / (phi + 4) = 61.803398874989 deg (golden field of view).",
              "// The lens is phi too: 100 / phi = 61.803398874989 deg (golden field of view).")
if r != rb:
    wr(P + "VeilRunner.cpp", r)
print("fov comment corrected:", r != rb)
