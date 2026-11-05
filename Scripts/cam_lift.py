"""The record camera was spawning inside the map's mountain meshes."""

GM = "C:/Users/Admin/HarkGame/Source/VEILOMNI/VeilGameMode.cpp"
s = open(GM, encoding="utf-8", newline="").read().replace("\r\n", "\n")
notes = []

# The camera must be movable so it can be lifted out of rock.
old_decl = ("\t\tconst FVector CamAt(Anchor[Idx].X - FMath::Cos(YawRad) * Back,\n"
            "\t\t\tAnchor[Idx].Y - FMath::Sin(YawRad) * Back,\n"
            "\t\t\tGroundZ + PHI_Constants::Reach(UpPow[Idx]));\n")
new_decl = ("\t\tFVector CamAt(Anchor[Idx].X - FMath::Cos(YawRad) * Back,\n"
            "\t\t\tAnchor[Idx].Y - FMath::Sin(YawRad) * Back,\n"
            "\t\t\tGroundZ + PHI_Constants::Reach(UpPow[Idx]));\n"
            "\n"
            "\t\t// The map is a field of huge mountain meshes. A record camera placed by\n"
            "\t\t// arithmetic alone ends up inside one and photographs rock. Lift it until\n"
            "\t\t// the aim point is actually visible, and log the lift so it is auditable.\n"
            "\t\t{\n"
            "\t\t\tconst FVector Aim(AimAt[Idx].X, AimAt[Idx].Y, GroundZ + PHI_Constants::Reach(1.5));\n"
            "\t\t\tdouble Lift = 0.0;\n"
            "\t\t\tfor (int32 Try = 0; Try < 16; ++Try)\n"
            "\t\t\t{\n"
            "\t\t\t\tFHitResult Block;\n"
            "\t\t\t\tif (!GetWorld()->LineTraceSingleByChannel(Block, Aim, CamAt, ECC_Visibility))\n"
            "\t\t\t\t{\n"
            "\t\t\t\t\tbreak;\n"
            "\t\t\t\t}\n"
            "\t\t\t\tLift += PHI_Constants::Reach(0.4);\n"
            "\t\t\t\tCamAt.Z += PHI_Constants::Reach(0.4);\n"
            "\t\t\t}\n"
            "\t\t\tif (Lift > 0.0)\n"
            "\t\t\t{\n"
            "\t\t\t\tUE_LOG(LogVEILWorld, Display,\n"
            "\t\t\t\t\tTEXT(\"VEIL SHOT LIFT: %s rose %.0f to see past the rock\"), Names[Idx], Lift);\n"
            "\t\t\t}\n"
            "\t\t}\n")

if "VEIL SHOT LIFT" in s:
    notes.append("already lifted")
elif old_decl in s:
    s = s.replace(old_decl, new_decl, 1)
    notes.append("camera lift installed")
else:
    notes.append("MISS camera block")

open(GM, "w", encoding="utf-8", newline="").write(s)
print("\n".join(notes))
