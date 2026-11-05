"""The inventory landed outside TakeShot; put it where CamAt exists."""

p = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(p, encoding="utf-8", newline="").read().replace("\r\n", "\n")

# 1. cut the misplaced block
start = s.find("\t\t{\n\t\t\t// What is actually in front of the lens")
endmark = ("\t\t\tUE_LOG(LogVEILWorld, Display, TEXT(\"VEIL SHOT NEAR: %d piece(s) within 10 m of the lens\"), Near);\n"
           "\t\t}\n")
if start < 0 or endmark not in s:
    raise SystemExit("MISS misplaced block")
e = s.find(endmark, start)
if e < 0:
    raise SystemExit("MISS misplaced block end")
s = s[:start] + s[e + len(endmark):]
print("shot: misplaced inventory removed")

# 2. insert it inside TakeShot, after CamAt is declared
cam = s.find("\t\tconst FVector CamAt(")
if cam < 0:
    raise SystemExit("MISS CamAt")
pawn = s.find("\t\tif (APawn* Pawn = PC->GetPawn())", cam)
if pawn < 0:
    raise SystemExit("MISS pawn in TakeShot")

near = '''		{
			// What is actually in front of the lens, named, with real bounds.
			int32 Near = 0;
			for (TActorIterator<AStaticMeshActor> It(GetWorld()); It; ++It)
			{
				const double D = FVector::Dist(It->GetActorLocation(), CamAt);
				if (D < 1000.0)
				{
					FVector O, E;
					It->GetActorBounds(true, O, E);
					UE_LOG(LogVEILWorld, Display,
						TEXT("VEIL SHOT NEAR: %s at %.0f | bounds %.0f x %.0f x %.0f"),
						*It->GetName(), D, E.X * 2.0, E.Y * 2.0, E.Z * 2.0);
					++Near;
				}
			}
			UE_LOG(LogVEILWorld, Display, TEXT("VEIL SHOT NEAR: %d piece(s) within 10 m of the lens"), Near);
		}
'''
s = s[:pawn] + near + s[pawn:]
open(p, "w", encoding="utf-8", newline="").write(s)
print("shot: inventory placed inside TakeShot")
