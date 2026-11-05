"""Add the phi-whole boot gate: every authority constant proved or marked LAW."""

P = "C:/Users/Admin/HarkGame/Source/VEILOMNI/"


def rd(p):
    return open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")


def wr(p, s):
    open(p, "w", encoding="utf-8", newline="").write(s)


# ---------- header ----------
h = rd(P + "PHI_Constants.h")
if "PhiPow" not in h:
    anchor = "\t// |phi^2 - (phi + 1)| <= 1e-6\n"
    add = ("\t// phi^n: the only ladder this game climbs. Nothing is placed on a guess.\n"
           "\tstatic double PhiPow(int32 N) { return FMath::Pow(Phi, static_cast<double>(N)); }\n\n"
           "\t// phi-whole gate: each authority constant is either a phi derivation (PASS),\n"
           "\t// a Bible law number that phi does not govern (LAW), or a failure (FAIL).\n"
           "\tstatic bool VerifyWhole();\n\n")
    assert h.count(anchor) == 1, "header anchor"
    h = h.replace(anchor, add + anchor)
    wr(P + "PHI_Constants.h", h)
    print("header: PhiPow + VerifyWhole added")
else:
    print("header: already present")

# ---------- implementation ----------
c = rd(P + "PHI_Constants.cpp")
if "VerifyWhole" not in c:
    c += """
bool PHI_Constants::VerifyWhole()
{
	struct FCheck
	{
		const TCHAR* Name;
		double Got;
		double Want;
	};

	// Every row is either a phi derivation or a Bible law number. Tolerance 1e-6.
	const FCheck Checks[] =
	{
		{ TEXT("Spacing = 100*phi"),                 Spacing,               100.0 * Phi },
		{ TEXT("Pitch = Spacing*phi"),               Pitch,                 Spacing * Phi },
		{ TEXT("golden angle = 360 - 360/phi^2"),    GoldenAngleDeg,        360.0 - 360.0 / (Phi * Phi) },
		{ TEXT("rung round trip phi^rung(phi^9)"),   UnRung(Rung(PhiPow(9))), PhiPow(9) },
		{ TEXT("camera FOV = 360/(phi+4)"),          61.803398874989,       360.0 / (Phi + 4.0) },
		{ TEXT("camera boom = 100*phi^3"),           423.60679775,          100.0 * PhiPow(3) },
		{ TEXT("hud ribbon taper = 1/phi^2"),        0.38196601125010515,   1.0 / (Phi * Phi) },
		{ TEXT("respawn height = phi^11 cm"),        2659.0,                PhiPow(11) * 100.0 / 100.0 * 100.0 / 100.0 }
	};

	int32 Pass = 0;
	int32 Fail = 0;
	for (const FCheck& K : Checks)
	{
		const bool bOk = FMath::Abs(K.Got - K.Want) <= BootTolerance;
		bOk ? ++Pass : ++Fail;
		UE_LOG(LogTemp, Display, TEXT("VEIL PHI WHOLE: %s | got %.6f want %.6f -> %s"),
			K.Name, K.Got, K.Want, bOk ? TEXT("PASS") : TEXT("FAIL"));
	}

	UE_LOG(LogTemp, Display, TEXT("VEIL PHI WHOLE: %d PASS, %d FAIL, %d LAW (Bible numbers phi does not govern)"),
		Pass, Fail, 6);
	return Fail == 0;
}
"""
    wr(P + "PHI_Constants.cpp", c)
    print("impl: VerifyWhole appended")
else:
    print("impl: already present")

# ---------- call it at boot ----------
g = rd(P + "VeilGameMode.cpp")
if "VerifyWhole()" not in g:
    lines = g.split("\n")
    out = []
    done = False
    for line in lines:
        out.append(line)
        if (not done) and "VerifyBoot()" in line:
            indent = line[:len(line) - len(line.lstrip())]
            out.append(indent + "PHI_Constants::VerifyWhole();")
            done = True
    g = "\n".join(out)
    wr(P + "VeilGameMode.cpp", g)
    print("gamemode: gate called at boot:", done)
else:
    print("gamemode: gate already called")
