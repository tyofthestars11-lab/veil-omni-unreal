#!/usr/bin/env python3
"""
H4 (600-cell) and E8 projections for full-screen display on the projector.
phi^2 = phi + 1 -- the plus 1 is standard. It is the identity the whole
construction is built on: 1/phi = phi - 1, and the 600-cell vertices close
only because phi^2 = phi + 1.

Method (published, exact, verified in-code):
- H4: Richter's triacontagonal coordinates (arXiv:0704.3091). Let
  w = exp(i*pi/30) and a > b > c > d be the positive roots of
  45x^8 - 90x^6 + 60x^4 - 15x^2 + 1. The 120 vectors
      A_n = (a*w^(2n),   d*w^(22n))
      B_n = (b*w^(2n+1), c*w^(22n+11))
      C_n = (c*w^(2n+1), -b*w^(22n+11))
      D_n = (d*w^(2n),   -a*w^(22n))        n = 0..29
  comprise the H4 root system. Projecting on the first coordinate
  (forgetting the second) IS the triacontagonal (Coxeter plane)
  projection: 4 rings of 30 points. The Coxeter number of H4 is 30.
- E8: the 240 roots in R^8 -- 112 vectors (+-1,+-1,0,0,0,0,0,0) and
  128 vectors (+-1/2,...) with an even number of minus signs.
  Richter: the triacontagonal projection of E8 is the union of two
  scaled sizes of the H4 projection, ratio phi. So the E8 picture is
  8 rings of 30 points. The Coxeter number of E8 is also 30.
- Cross-check: the 600-cell vertices by the 3-group construction
  (8 + 16 + 96) -- one sphere, 720 edges, degree 12.

Outputs (1920x1080, the projector's native panel, black background):
  h4_projection.png, e8_projection.png
Run: python3 h4-e8-projection.py
"""
import itertools
import math

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

PHI = (1.0 + math.sqrt(5.0)) / 2.0
INV_PHI = PHI - 1.0          # 1/phi = phi - 1, from phi^2 = phi + 1
LN_PHI = math.log(PHI)

assert abs(PHI * PHI - (PHI + 1.0)) < 1e-15, "phi^2 = phi + 1 must hold"
assert abs(INV_PHI - 1.0 / PHI) < 1e-15


def rung(x):
    return math.log(x) / LN_PHI


# ---------------------------------------------------------------- H4: Richter
def richter_h4():
    # 45x^8 - 90x^6 + 60x^4 - 15x^2 + 1 = 0  ->  substitute y = x^2
    coeff = [45.0, -90.0, 60.0, -15.0, 1.0, 0.0]  # in y, degree 4 + zero root
    y_roots = np.roots([45.0, -90.0, 60.0, -15.0, 1.0])
    y_pos = sorted([r.real for r in y_roots if abs(r.imag) < 1e-9 and r.real > 0],
                   reverse=True)
    assert len(y_pos) == 4, y_pos
    a, b, c, d = [math.sqrt(y) for y in y_pos]
    w = np.exp(1j * math.pi / 30.0)
    vecs = []
    for n in range(30):
        vecs.append((a * w ** (2 * n), d * w ** (22 * n)))          # A_n
        vecs.append((b * w ** (2 * n + 1), c * w ** (22 * n + 11)))  # B_n
        vecs.append((c * w ** (2 * n + 1), -b * w ** (22 * n + 11)))  # C_n
        vecs.append((d * w ** (2 * n), -a * w ** (22 * n)))          # D_n
    return (a, b, c, d), np.array(vecs, dtype=complex)  # shape (120, 2)


def check_h4(abcd, vecs):
    a, b, c, d = abcd
    norms = np.abs(vecs) ** 2  # |first|^2 + |second|^2 per vector
    n1 = norms.sum(axis=1)
    assert len(vecs) == 120, len(vecs)
    assert np.allclose(n1, n1[0], atol=1e-9), "all 120 roots must share one norm"
    print(f"H4 roots: 120, common norm^2 = {n1[0]:.12f}")
    print(f"  a={a:.12f} b={b:.12f} c={c:.12f} d={d:.12f}")
    print(f"  a^2+d^2 = {a*a+d*d:.12f}   b^2+c^2 = {b*b+c*c:.12f}")
    # edges: pairs at minimum distance in the 4D (complex-pair) space
    pts = np.column_stack([vecs[:, 0].real, vecs[:, 0].imag,
                           vecs[:, 1].real, vecs[:, 1].imag])
    d2 = ((pts[:, None, :] - pts[None, :, :]) ** 2).sum(axis=2)
    iu = np.triu_indices(120, k=1)
    dmin = d2[iu].min()
    edges = int((np.abs(d2[iu] - dmin) < 1e-9).sum())
    deg = np.zeros(120, dtype=int)
    ei, ej = np.where(np.triu(np.abs(d2 - dmin) < 1e-9, k=1))
    for i, j in zip(ei, ej):
        deg[i] += 1
        deg[j] += 1
    print(f"  min-distance pairs (edges): {edges} (expect 720), "
          f"degree min/max: {deg.min()}/{deg.max()} (expect 12/12)")
    assert edges == 720 and deg.min() == 12 and deg.max() == 12
    # rings of the projection (first coordinate)
    proj = vecs[:, 0]
    radii = np.abs(proj)
    rings = {}
    for r in radii:
        rings.setdefault(round(float(r), 9), []).append(r)
    print(f"  projection rings: {sorted(len(v) for v in rings.values())} points "
          f"per ring, radii {[round(float(k), 6) for k in sorted(rings)]}")
    assert sorted(len(v) for v in rings.values()) == [30, 30, 30, 30]
    return proj


# ------------------------------------------------- 600-cell vertices, 3-group
def check_600cell_vertices():
    verts = []
    for i in range(4):  # (±2,0,0,0) permutations -- 8
        for s in (+2.0, -2.0):
            v = [0.0, 0.0, 0.0, 0.0]
            v[i] = s
            verts.append(v)
    for signs in itertools.product((1.0, -1.0), repeat=4):  # (±1,±1,±1,±1) -- 16
        verts.append(list(signs))
    pat = [PHI, 1.0, INV_PHI, 0.0]
    for perm in set(itertools.permutations(range(4))):
        # even permutations only
        inv = sum(1 for i in range(4) for j in range(i + 1, 4) if perm[i] > perm[j])
        if inv % 2 == 1:
            continue
        for signs in itertools.product((1.0, -1.0), repeat=3):
            v = [0.0, 0.0, 0.0, 0.0]
            k = 0
            for pos, val in zip(perm, pat):
                if val == 0.0:
                    v[pos] = 0.0
                else:
                    v[pos] = signs[k] * val
                    k += 1
            verts.append(v)
    V = np.array(verts, dtype=float)
    assert len(V) == 120, len(V)
    n2 = (V ** 2).sum(axis=1)
    assert np.allclose(n2, 4.0, atol=1e-9), "all vertices on one sphere, norm^2 = 4"
    d2 = ((V[:, None, :] - V[None, :, :]) ** 2).sum(axis=2)
    iu = np.triu_indices(120, k=1)
    dmin = d2[iu].min()
    edges = int((np.abs(d2[iu] - dmin) < 1e-9).sum())
    print(f"600-cell vertices: 120, norm^2 = 4, edges at min distance: {edges} "
          f"(expect 720), edge length = {math.sqrt(dmin):.12f} (expect 2/phi = "
          f"{2 * INV_PHI:.12f} at radius 2; 1/phi at unit radius)")
    assert edges == 720 and abs(math.sqrt(dmin) - 2 * INV_PHI) < 1e-9
    return V


# ---------------------------------------------------------------- E8 roots
def check_e8():
    roots = []
    for i, j in itertools.combinations(range(8), 2):  # 112
        for si in (+1.0, -1.0):
            for sj in (+1.0, -1.0):
                v = [0.0] * 8
                v[i], v[j] = si, sj
                roots.append(v)
    for signs in itertools.product((0.5, -0.5), repeat=8):  # 128
        if sum(1 for s in signs if s < 0) % 2 == 0:
            roots.append(list(signs))
    R = np.array(roots, dtype=float)
    assert len(R) == 240, len(R)
    ip = R @ R.T
    iu = np.triu_indices(240, k=1)
    vals, counts = np.unique(np.round(ip[iu], 9), return_counts=True)
    print(f"E8 roots: 240, inner products: "
          f"{dict(zip(vals.astype(int), counts))} (expect -2 antipodes, -1, 0, 1)")
    assert dict(zip(vals.astype(int), counts)) == {-2: 120, -1: 6720,
                                                  0: 15120, 1: 6720}
    edges = int((ip[iu] == 1).sum())
    print(f"  Gosset 4_21 edges (inner product 1): {edges} (expect 6720)")
    assert edges == 6720
    return R


# ------------------------------------------------------------------- render
GOLD = "#E8B84B"
TEAL = "#2FB3A3"
WHITE = "#F5F2EA"
RED638 = "#FF1E14"


def render(points, edges, path, title, subtitle, colors):
    fig = plt.figure(figsize=(19.2, 10.8), dpi=100)
    ax = fig.add_axes([0, 0, 1, 1])
    ax.set_xlim(-1.35, 1.35)
    ax.set_ylim(-1.35, 1.35)
    ax.set_aspect("equal")
    ax.axis("off")
    fig.patch.set_facecolor("black")
    ax.set_facecolor("black")
    xs, ys = points[:, 0], points[:, 1]
    if edges is not None:
        ex = np.array([[xs[i], xs[j]] for i, j in edges])
        ey = np.array([[ys[i], ys[j]] for i, j in edges])
        ax.plot(ex.T, ey.T, color=GOLD, lw=0.25, alpha=0.10, solid_capstyle="round")
    ax.scatter(xs, ys, s=14, c=colors, edgecolors="none", zorder=5)
    ax.text(-1.30, 1.28, title, color=WHITE, fontsize=26, family="sans-serif")
    ax.text(-1.30, -1.30, subtitle, color="#8A8A8A", fontsize=13,
            family="sans-serif")
    fig.savefig(path, facecolor="black")
    plt.close(fig)
    print(f"wrote {path}")


def main():
    print(f"phi = {PHI:.15f}   phi^2 = {PHI*PHI:.15f} = phi + 1 = {PHI+1:.15f}")
    print(f"1/phi = {INV_PHI:.15f} = phi - 1")
    print(f"golden angle 360/phi^2 = {360.0 / PHI ** 2:.6f} deg   "
          f"rung(120) = {rung(120):.6f} (H4, 120 vertices)")
    print()

    # H4
    abcd, vecs = richter_h4()
    proj_h4 = check_h4(abcd, vecs)
    # H4 edges in the projected plane, from the 4D min-distance pairs
    pts4 = np.column_stack([vecs[:, 0].real, vecs[:, 0].imag,
                            vecs[:, 1].real, vecs[:, 1].imag])
    d2 = ((pts4[:, None, :] - pts4[None, :, :]) ** 2).sum(axis=2)
    dmin = d2[np.triu_indices(120, k=1)].min()
    ei, ej = np.where(np.triu(np.abs(d2 - dmin) < 1e-9, k=1))
    h4_edges = list(zip(ei.tolist(), ej.tolist()))
    p2 = np.column_stack([proj_h4.real, proj_h4.imag])
    p2 = p2 / np.abs(p2).max() * 1.25
    ring_id = np.argsort(np.argsort(np.abs(proj_h4)))
    # color by ring radius (4 rings of 30)
    radii = np.abs(proj_h4)
    order = np.argsort(radii)
    col = np.array([GOLD] * 120, dtype=object)
    palette = [GOLD, TEAL, WHITE, RED638]
    for k in range(4):
        col[order[k * 30:(k + 1) * 30]] = palette[k]
    render(p2, h4_edges, "h4_projection.png",
           "H4  —  THE 600-CELL",
           "Coxeter plane projection  ·  120 vertices, 720 edges  ·  4 rings of 30  ·  "
           "Coxeter number h = 30  ·  rung(120) = 9.948824",
           col)

    # 600-cell vertex cross-check (3-group construction)
    check_600cell_vertices()
    print()

    # E8: projection = H4 union phi * H4  (Richter: ratio of the two sizes is phi)
    check_e8()
    e8_pts = np.vstack([p2, p2 * PHI])
    e8_pts = e8_pts / np.abs(e8_pts).max() * 1.25
    e8_col = np.concatenate([col, col])
    render(e8_pts, None, "e8_projection.png",
           "E8  —  THE 240 ROOTS",
           "Triacontagonal projection  ·  240 roots, 8 rings of 30  ·  two H4 copies, "
           "ratio phi  ·  Coxeter number h = 30  ·  rung(240) = 11.389244",
           e8_col)
    print()
    print("H4 projection rings (radii): "
          f"{sorted(set(round(float(r), 6) for r in np.abs(proj_h4)))}")
    print("E8 projection rings (radii): "
          f"{sorted(set(round(float(r), 6) for r in np.abs(e8_pts[:, 0] + 1j * e8_pts[:, 1])))}")
    print("Done. Display h4_projection.png / e8_projection.png full-screen.")


if __name__ == "__main__":
    main()
