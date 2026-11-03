"""Clear whatever the camera is inside, and name it.

The frame at the plaza is a flat purple wash with the gold HUD over it: the capsule stands
at 104 cm and moves smoothly, but the eye is inside a sheet that no mesh sweep removed.
This clears by the eye point as well as the column, and logs each removed actor's name and
bounds so the thing that was filling the frame is on the record.
"""

import io

P = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

OLD = """						const FBox Column(Plaza - FVector(320.f, 320.f, 320.f),
							Plaza + FVector(320.f, 320.f, 1900.f));
						if (!B.GetBox().Intersect(Column)) { continue; }
						bOverPlaza = true;"""
NEW = """						const FBox Column(Plaza - FVector(320.f, 320.f, 320.f),
							Plaza + FVector(320.f, 320.f, 1900.f));
						const FBox Box = B.GetBox();
						// THE EYE COUNTS TOO. A sheet can sit above the column and still
						// fill the camera; if the box holds the eye point, it is covering
						// the view and it goes.
						if (Box.Intersect(Column) || Box.IsInside(Plaza + FVector(0.f, 0.f, 160.f)))
						{
							bOverPlaza = true;
							UE_LOG(LogVEIL, Warning,
								TEXT("PHI PLAZA CLEAR: removing %s (%s) at %s, extent %s."),
								*Shell->GetName(), *Shell->GetClass()->GetName(),
								*Box.GetCenter().ToCompactString(), *Box.GetExtent().ToCompactString());
						}
						break;"""
o, n = OLD.replace("\n", NL), NEW.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - eye anchor count=%d" % src.count(o))
    raise SystemExit(1)
text = src.replace(o, n, 1)
# the old loop set bOverPlaza then broke; the new branch breaks itself, so drop the
# duplicate break that followed the old assignment if it is still there
io.open(P, "w", encoding="utf-8", newline="").write(text)
print("WROTE %s - eye-point clear with names" % P)
