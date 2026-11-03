"""Clear the veil around wherever he actually wakes, then stand him on the floor.

The fixed-point clear works at V(0,-1500) but the seat search wakes him at
V(-831,-138,105), where a purple sheet crosses eye height and fills the frame. Six
seconds in, this sweeps a column around the runner - the only point that is guaranteed to
be real - removes anything covering it, and seats him on the lowest floor under 200 cm.
"""

import io

P = "Source/VEIL/Private/Character/VEILCharacter.cpp"
src = io.open(P, encoding="utf-8", newline="").read()
NL = "\r\n" if "\r\n" in src else "\n"

OLD = """					if (GetActorLocation().Z > 200.f)
					{
						SetActorLocation(FVector(0.f, -1500.f, 120.f), false, nullptr, ETeleportType::TeleportPhysics);
						UE_LOG(LogVEIL, Warning,
							TEXT("PHI SETTLE: runner brought down to the plaza floor at V(0,-1500,120)."));
					}"""
NEW = """					// AROUND THE RUNNER, NOT A FIXED POINT. The seat search wakes him at
					// V(-831,-138,105); a clear anchored at V(0,-1500) never touches that
					// spot, and the purple sheet crossing his eye there is what fills the
					// frame. This sweeps the column he is actually in.
					const FVector Me = GetActorLocation();
					const FBox Column(Me - FVector(600.f, 600.f, 400.f), Me + FVector(600.f, 600.f, 900.f));
					int32 Gone = 0;
					for (TActorIterator<AActor> EyeIt(GroundWorld); EyeIt; ++EyeIt)
					{
						AActor* Eye = *EyeIt;
						if (!Eye || Eye == this) { continue; }
						if (Eye->Tags.Contains(FName(TEXT("PhiGround")))) { continue; }
						if (Eye->IsA(APawn::StaticClass())) { continue; }
						TInlineComponentArray<UPrimitiveComponent*> EyeComps;
						Eye->GetComponents(EyeComps);
						bool bCovers = false;
						for (UPrimitiveComponent* EyeComp : EyeComps)
						{
							if (!EyeComp) { continue; }
							if (EyeComp->GetCollisionEnabled() == ECollisionEnabled::NoCollision) { continue; }
							const FBox EyeBox = EyeComp->Bounds.GetBox();
							if (!EyeBox.Intersect(Column)) { continue; }
							bCovers = true;
							UE_LOG(LogVEIL, Warning,
								TEXT("PHI EYE CLEAR: %s (%s) at %s, extent %s."),
								*Eye->GetName(), *Eye->GetClass()->GetName(),
								*EyeBox.GetCenter().ToCompactString(), *EyeBox.GetExtent().ToCompactString());
							break;
						}
						if (bCovers) { Eye->Destroy(); ++Gone; }
					}
					FHitResult FloorHit;
					FCollisionQueryParams Sp(SCENE_QUERY_STAT(VeilSettle), false, this);
					if (GroundWorld->LineTraceSingleByChannel(FloorHit, Me + FVector(0.f, 0.f, 1500.f),
						Me - FVector(0.f, 0.f, 3000.f), ECC_Visibility, Sp))
					{
						if (FloorHit.ImpactPoint.Z <= 200.f)
						{
							SetActorLocation(FVector(Me.X, Me.Y, FloorHit.ImpactPoint.Z + 98.f),
								false, nullptr, ETeleportType::TeleportPhysics);
						}
					}
					UE_LOG(LogVEIL, Warning, TEXT("PHI EYE CLEAR: %d actor(s) cleared around the runner at %s, floor z=%.0f."),
						Gone, *Me.ToCompactString(), FloorHit.ImpactPoint.Z);"""
o, n = OLD.replace("\n", NL), NEW.replace("\n", NL)
if src.count(o) != 1:
    print("ABORTED - settle body count=%d" % src.count(o))
    raise SystemExit(1)
io.open(P, "w", encoding="utf-8", newline="").write(src.replace(o, n, 1))
print("WROTE %s - eye clear around the runner" % P)
