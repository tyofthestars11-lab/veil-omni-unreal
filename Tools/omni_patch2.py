# omni_patch2.py - exact-match edits: Nest fragment claims and the derived count.
import io

P = r"D:\VEIL-OMNI\Source\VEIL\Private\Systems\VEILOmniAll.cpp"
src = io.open(P, encoding="utf-8").read()
original = src
results = []


def rep(note, old, new):
    global src
    if old not in src:
        results.append(("MISS", note))
        return
    src = src.replace(old, new, 1)
    results.append(("OK", note))


rep("claim nearest nest fragment",
    "\t// 2. The nearest thing that answers the interact interface.\n",
    "\t// 1b. THE NEST'S OWN SHARDS, claimed by IDENTITY. The generic nearest-actor\n"
    "\t// search picked whichever interactable stood closest, so a beacon or a\n"
    "\t// shrine inside the reach could take the claim and the shard stayed standing:\n"
    "\t// the chain reached 2 of 3 no matter what the player did. A Nest fragment is\n"
    "\t// now claimed as a Nest fragment, and counted when it is.\n"
    "\t{\n"
    "\t\tAVEILFragment* NestShard = nullptr;\n"
    "\t\tfloat NestDist = VEILOmniAuthority::SpeakerReach;\n"
    "\t\tfor (TActorIterator<AVEILFragment> It(World); It; ++It)\n"
    "\t\t{\n"
    "\t\t\tAVEILFragment* Shard = *It;\n"
    "\t\t\tif (!Shard || Shard->RegionId != FName(TEXT(\"Nest\"))) { continue; }\n"
    "\t\t\tconst float D = FVector::Dist(At, Shard->GetActorLocation());\n"
    "\t\t\tif (D < NestDist) { NestDist = D; NestShard = Shard; }\n"
    "\t\t}\n"
    "\t\tif (NestShard)\n"
    "\t\t{\n"
    "\t\t\tIVEILVaultInteractable::Execute_Interact(NestShard, Runner);\n"
    "\t\t\tNestFragments = FMath::Min(NestFragmentsTotal, NestFragments + 1);\n"
    "\t\t\tUE_LOG(LogVEIL, Warning, TEXT(\"PHI OMNI INTERACT: Nest fragment claimed at %.0f cm - %d of %d.\"),\n"
    "\t\t\t\tNestDist, NestFragments, NestFragmentsTotal);\n"
    "\t\t}\n"
    "\t}\n\n"
    "\t// 2. The nearest thing that answers the interact interface.\n")

rep("skip fragments in generic search",
    "\t\tif (!Actor->GetClass()->ImplementsInterface(UVEILVaultInteractable::StaticClass())) { continue; }\n",
    "\t\tif (!Actor->GetClass()->ImplementsInterface(UVEILVaultInteractable::StaticClass())) { continue; }\n"
    "\t\tif (Actor->IsA(AVEILFragment::StaticClass())) { continue; } // claimed above, by identity\n")

rep("derived count only rises",
    "\tNestFragments = FMath::Max(0, NestFragmentsTotal - FragmentsAlive);\n",
    "\t// The measured count and the claimed count only ever rise: a shard that was\n"
    "\t// claimed and is still pending kill must not drop the chain back a step.\n"
    "\tNestFragments = FMath::Max(NestFragments, NestFragmentsTotal - FragmentsAlive);\n")

misses = [n for s, n in results if s == "MISS"]
for status, note in results:
    print("%-4s %s" % (status, note))
if misses:
    print("REFUSED to write: %d anchor(s) missing" % len(misses))
    raise SystemExit(1)
if src == original:
    print("source unchanged")
    raise SystemExit(1)
io.open(P, "w", encoding="utf-8", newline="\n").write(src)
print("WROTE %s (%d -> %d chars)" % (P, len(original), len(src)))
