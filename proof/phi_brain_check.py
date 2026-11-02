"""Check the phi brain's arithmetic exactly as PHI_NPCProcessor computes it.

This is the formula, not the runtime: the engine run is blocked by the frozen
world clock, so the decision layer is verified here on the same constants.
"""
import math

PHI = 1.618033988749895
LNPHI = 0.48121182505960347
INTENTS = ["Rest", "Patrol", "Investigate", "Hunt", "Fight", "Flee"]
ANCHORS = [-2, -1, 0, 1, 2, 3]


def rung(x):
    return math.log(x) / LNPHI if x > 0 else 0.0


def phipow(r):
    return PHI ** r


def breathe(seed, phase, offset, scale):
    """FPhiDynamic::Rung - rung of a seed, walked by a phi-clock wave."""
    wave = math.sin((phase + offset) * 2.0 * math.pi)
    return rung(seed) + phipow(-4) * scale * wave


def moods(phase):
    return (breathe(1.0, phase, 0.0, 0.5),          # aggression
            breathe(1.0, phase, phipow(-1), 0.75),   # curiosity
            breathe(1.0, phase, phipow(-2), 0.5))    # patience


def pull(idx, phase, personality, threat01, health01):
    agg, cur, pat = moods(phase)
    threat_rung = rung(1.0 + 2.0 * min(max(threat01, 0.0), 1.0))
    health_rung = rung(1.0 + 2.0 * min(max(health01, 0.0), 1.0))
    band = ((phase + personality) * (PHI - 1.0)) % 1.0
    drift = (band - 0.5) * phipow(1)
    situation = threat_rung - health_rung * phipow(-1) + drift

    shift = 0.0
    if idx == 4:      # Fight
        shift = -agg - cur * phipow(-2)
    elif idx == 3:    # Hunt
        shift = -agg * phipow(-1)
    elif idx == 2:    # Investigate
        shift = -cur * phipow(-1)
    elif idx in (0, 1):  # Rest, Patrol
        shift = -pat * phipow(-1)

    score = -abs(situation - (rung(phipow(ANCHORS[idx])) + shift))
    variety = ((band + personality) * (1.0 + idx)) % 1.0
    return score - variety * phipow(-1)


def choose(phase, personality, threat01, health01, has_target=True):
    best, best_score = 1, -1e30
    for idx in range(6):
        if not has_target and idx in (3, 4, 5):
            continue
        s = pull(idx, phase, personality, threat01, health01)
        if s > best_score:
            best, best_score = idx, s
    return best


def band_of(phase, personality):
    return ((phase + personality) * (PHI - 1.0)) % 1.0


# ---------------------------------------------------------------- round trip
worst = 0.0
for v in (0.5, 1.0, 8.0, 34.0, 340.0, 420.0, 2200.0, 2500.0):
    worst = max(worst, abs(phipow(rung(v)) - v))
print("round-trip |phi^Rung(v) - v| worst over the AI's own values: %.9f" % worst)

# ------------------------------------------------------- variety over time
dt, steps = 1.0 / 30.0, 4000
personality = rung(1.0 + 0.618 * (PHI - 1.0))
phase, bands, counts = 0.0, [], {i: 0 for i in range(6)}
for _ in range(steps):
    phase += dt / PHI
    bands.append(band_of(phase, personality))
    counts[choose(phase, personality, 0.5, 0.9)] += 1

uniq = len(set(round(b, 12) for b in bands))
print("clock band samples: %d, distinct: %d" % (len(bands), uniq))
print("smallest gap between neighbouring bands: %.10f" % min(
    abs(bands[i + 1] - bands[i]) for i in range(len(bands) - 1)))
print("min band %.10f  max band %.10f" % (min(bands), max(bands)))
print("intent mix for ONE fixed situation (threat 0.5, health 0.9):")
for i in range(6):
    print("   %-11s %4d samples  %5.1f%%" % (INTENTS[i], counts[i], 100.0 * counts[i] / steps))

# Responsiveness: the same brain must still answer differently when it matters.
print("\nsame brain, different situations (how a machine answers as things change):")
print("   %-22s %s" % ("situation", " ".join("%4s" % n[:4] for n in INTENTS)))
for label, threat, health in (("calm, whole", 0.05, 1.0),
                              ("seen, whole", 0.5, 0.9),
                              ("close, whole", 0.9, 0.9),
                              ("close, wounded", 0.9, 0.35),
                              ("close, dying", 0.9, 0.1)):
    p, c = 0.0, {i: 0 for i in range(6)}
    for _ in range(steps):
        p += dt / PHI
        c[choose(p, personality, threat, health)] += 1
    print("   %-22s %s" % (label, " ".join("%4.0f" % (100.0 * c[i] / steps) for i in range(6))))

# same situation, two creatures: different brains, same law
other = rung(1.0 + 0.377 * (PHI - 1.0))
phase2, counts2 = 0.0, {i: 0 for i in range(6)}
for _ in range(steps):
    phase2 += dt / PHI
    counts2[choose(phase2, other, 0.5, 0.9)] += 1
same = sum(1 for i in range(6) if counts[i] == counts2[i])
print("creature A rung %.6f vs creature B rung %.6f -> identical intent histogram? %s"
      % (personality, other, "yes" if same == 6 else "no"))
