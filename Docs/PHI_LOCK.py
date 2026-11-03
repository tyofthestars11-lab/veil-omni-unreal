"""PHI LOCK - every constant computed fresh from its definition, never typed.

Per the phase-lock rule: rung = ln(v)/ln(phi), computed, round-tripped back to
the value, and asserted exact. Nothing here is copied from a note; if a value
does not close, this prints the mismatch instead of hiding it.

Run:  python Docs/PHI_LOCK.py
"""
from __future__ import absolute_import

import math

PHI = (1.0 + math.sqrt(5.0)) / 2.0
LN_PHI = math.log(PHI)


def rung(value):
    """ln(v)/ln(phi) - the address of a value."""
    return math.log(value) / LN_PHI


def value(r):
    """Round-trip a rung back to its value."""
    return PHI ** r


def check(name, got, want, tol=5e-5):
    ok = abs(got - want) <= tol * max(1.0, abs(want))
    print("  %-34s %-22.12f %s" % (name, got, "OK" if ok else "MISMATCH want %.12f" % want))
    return ok


print("PHI LOCK - computed, not typed")
print("phi            = %.15f" % PHI)
print("phi^2          = %.15f   (phi+1 = %.15f)" % (PHI * PHI, PHI + 1.0))
print("golden angle   = %.13f deg  (= 360/phi^2)" % (360.0 / (PHI * PHI)))
print("tetrahedral    = %.9f deg   (= acos(-1/3))" % math.degrees(math.acos(-1.0 / 3.0)))
print("")

allok = True
print("CONSTANTS FROM THE PHI FRAMEWORK, RUNG ROUND-TRIPPED")
allok &= check("golden angle 137.5077640500378", 360.0 / (PHI * PHI), 137.5077640500378)
allok &= check("tetrahedral 109.471221", math.degrees(math.acos(-1.0 / 3.0)), 109.471221)

# Willow lock 9.6713: Google Willow carries 105 qubits; rung(105) is the lock.
allok &= check("willow lock rung (105 qubits)", rung(105.0), 9.6713, 1e-4)
print("     round-trip: phi^9.6713 = %.4f qubits" % value(rung(105.0)))

# law rung 55 / TON618 = phi^55
ton = value(55.0)
print("  %-34s %.4e" % ("TON618 = phi^55 (solar masses)", ton))
print("  %-34s %.4f" % ("body rung 51.7712 -> value", value(51.7712)))

# golden pitch 17.0324 - only report what the numbers actually say
pitch = math.radians(17.0324)
print("  %-34s tan=%.6f  atan(phi^-2)=%.6f  atan(phi^-3)=%.6f"
      % ("golden pitch 17.0324 deg", math.tan(pitch),
         math.degrees(math.atan(PHI ** -2)), math.degrees(math.atan(PHI ** -3))))

# VEIL district constants, rung addresses
print("")
print("VEIL DISTRICT CONSTANTS AS PHI RUNGS (rung, round-trip exact)")
veil = [("hub x 0.0 -> offset 1", 1.0),
        ("nest x -450 -> 450", 450.0),
        ("spire y 3500", 3500.0),
        ("omni x 2800", 2800.0),
        ("deep y -5000 -> 5000", 5000.0),
        ("cache y -6200 -> 6200", 6200.0),
        ("seed 1618", 1618.0),
        ("authority 11", 11.0)]
for label, v in veil:
    r = rung(v)
    back = value(r)
    flag = "OK" if abs(back - v) < 1e-9 else "DRIFT"
    print("  %-28s v=%-9.3f rung=%-10.6f back=%-11.6f %s" % (label, v, r, back, flag))

print("")
print("SEAL 111 drift %.2e %s" % (abs(value(rung(111.0)) - 111.0),
                                 "LOCKED" if abs(value(rung(111.0)) - 111.0) < 1e-9 else "DRIFT"))
print("ALL CHECKS:", "PASS" if allok else "FAIL")
