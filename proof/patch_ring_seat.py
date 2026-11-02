# VEIL - RING SEAT PATCH
#
# The editor's text tools cannot resolve nested paths on this host, so the change is applied by an
# exact, verified replacement. CRLF is preserved byte-for-byte: the file is opened with newline=''
# and written back the same way, so no line ending is rewritten.
import io
import sys

PATH = r"D:\VEIL-OMNI\Source\VEIL\Private\Game\VEILGameMode.cpp"

OLD = (
    "\t\t\tconst FVector Loc(FMath::Cos(Angle) * 950.f,\r\n"
    "\t\t\t\tFMath::Sin(Angle) * 950.f, 240.f);\r\n"
    "\t\t\tif (AVEILFactionAgent* Agent =\r\n"
    "\t\t\t\tGetWorld()->SpawnActor<AVEILFactionAgent>(\r\n"
    "\t\t\t\t\tAVEILFactionAgent::StaticClass(), Loc,\r\n"
    "\t\t\t\t\tFRotator(0.f, -Angle * 180.f / PI, 0.f)))\r\n"
    "\t\t\t{\r\n"
    "\t\t\t\tAgent->SpawnDefaultController();\r\n"
    "\t\t\t}"
)

NEW = (
    "\t\t\tFVector Loc(FMath::Cos(Angle) * 950.f,\r\n"
    "\t\t\t\tFMath::Sin(Angle) * 950.f, 240.f);\r\n"
    "\r\n"
    "\t\t\t// THE RING IS NOT EMPTY ANY MORE.\r\n"
    "\t\t\t//\r\n"
    "\t\t\t// On a bare map the 950 cm ring is open ground and a blind seat is fine. In the\r\n"
    "\t\t\t// baked district that ring runs THROUGH the market walls, and a spawn that lands\r\n"
    "\t\t\t// inside geometry is not adjusted - it is refused, silently, and the agent is\r\n"
    "\t\t\t// never born (two of the three were lost that way on VEILDistrictWP). A seat is\r\n"
    "\t\t\t// therefore PROBED for the floor it stands on, and the spawn never refuses.\r\n"
    "\t\t\t{\r\n"
    "\t\t\t\tFHitResult FloorHit;\r\n"
    "\t\t\t\tFCollisionQueryParams SeatParams(SCENE_QUERY_STAT(PHIRingSeat), false);\r\n"
    "\t\t\t\tif (APawn* Runner = UGameplayStatics::GetPlayerPawn(GetWorld(), 0))\r\n"
    "\t\t\t\t{\r\n"
    "\t\t\t\t\tSeatParams.AddIgnoredActor(Runner);\r\n"
    "\t\t\t\t}\r\n"
    "\t\t\t\tif (GetWorld()->LineTraceSingleByChannel(FloorHit,\r\n"
    "\t\t\t\t\tFVector(Loc.X, Loc.Y, 3000.f), FVector(Loc.X, Loc.Y, -1000.f),\r\n"
    "\t\t\t\t\tECC_Visibility, SeatParams))\r\n"
    "\t\t\t\t{\r\n"
    "\t\t\t\t\tLoc.Z = FloorHit.ImpactPoint.Z + 98.f;\r\n"
    "\t\t\t\t}\r\n"
    "\t\t\t}\r\n"
    "\r\n"
    "\t\t\tFActorSpawnParameters RingParams;\r\n"
    "\t\t\tRingParams.SpawnCollisionHandlingOverride =\r\n"
    "\t\t\t\tESpawnActorCollisionHandlingMethod::AlwaysSpawn;\r\n"
    "\r\n"
    "\t\t\tif (AVEILFactionAgent* Agent =\r\n"
    "\t\t\t\tGetWorld()->SpawnActor<AVEILFactionAgent>(\r\n"
    "\t\t\t\t\tAVEILFactionAgent::StaticClass(), Loc,\r\n"
    "\t\t\t\t\tFRotator(0.f, -Angle * 180.f / PI, 0.f), RingParams))\r\n"
    "\t\t\t{\r\n"
    "\t\t\t\tAgent->SpawnDefaultController();\r\n"
    "\t\t\t\tUE_LOG(LogVEIL, Display, TEXT(\"PHI RING SEAT: agent %d at %s (floor probed, never refused).\"),\r\n"
    "\t\t\t\t\ti, *Loc.ToCompactString());\r\n"
    "\t\t\t}\r\n"
    "\t\t\telse\r\n"
    "\t\t\t{\r\n"
    "\t\t\t\tUE_LOG(LogVEIL, Warning, TEXT(\"PHI RING SEAT: agent %d was refused at %s even with AlwaysSpawn.\"),\r\n"
    "\t\t\t\t\ti, *Loc.ToCompactString());\r\n"
    "\t\t\t}"
)


def main():
    with io.open(PATH, "r", encoding="utf-8", newline="") as f:
        src = f.read()

    if NEW in src:
        print("PATCH: already applied")
        return 0
    if OLD not in src:
        print("PATCH: anchor not found - nothing written")
        return 1

    out = src.replace(OLD, NEW, 1)
    with io.open(PATH, "w", encoding="utf-8", newline="") as f:
        f.write(out)
    print("PATCH: applied, {} -> {} bytes".format(len(src), len(out)))
    return 0


sys.exit(main())
