#include "Systems/PHILivingWorld.h"

#include "Engine/World.h"
#include "Systems/VEILTime.h"
#include "Systems/VEILSociety.h"
#include "VEIL.h"
#include "Systems/VEILPhi.h"

namespace
{
	// Named with a suffix on purpose: a bare `Phi` at file scope shadows the
	// engine's own Chaos constants and the build treats that as an error.
	constexpr float PhiV = VeilPhi::PhiF;

	/** One day of living is worth 1/phi^3 of a rung: slow, but it never stops. */
	constexpr float DayGrowthRung = 1.f / (VeilPhi::PhiF * VeilPhi::PhiF * VeilPhi::PhiF);
	/** A kindness or a slight is worth more than a quiet day. */
	constexpr float DeedRung = 1.f / (VeilPhi::PhiF * VeilPhi::PhiF);

	/** Deterministic 0..1 from two integers: the same day never repeats a take. */
	float Hash01(uint32 A, uint32 B)
	{
		uint32 H = 2166136261u ^ A;
		H *= 16777619u; H ^= (H >> 13);
		H ^= B; H *= 16777619u; H ^= (H >> 15);
		return (float)(H & 0x00FFFFFFu) / (float)0x01000000u;
	}

	/** "CAPTAIN ODALYS CRANE" is how the ledger names her; a neighbour says CRANE. */
	FString ShortName(FName Id)
	{
		TArray<FString> Parts;
		VEILCast::NameOf(Id).ToString().ParseIntoArray(Parts, TEXT(" "), true);
		return Parts.Num() > 1 ? Parts.Last() : (Parts.Num() == 1 ? Parts[0] : Id.ToString());
	}

	struct FExchange { const TCHAR* A; const TCHAR* B; };

	const TArray<FExchange>& ExchangeTable()
	{
		// Short, grounded, and about the district they are standing in. Nobody
		// announces their own name, and nobody speaks in exposition: this is two
		// people at work who have worked together for years.
		static const TArray<FExchange> List = {
			{ TEXT("Third silence again. You hear it?"),            TEXT("Third silence. Third night nobody sleeps through it.") },
			{ TEXT("Ledger says two lamps. The tower says one."),     TEXT("Then the tower is lying and the ledger is late.") },
			{ TEXT("Move the crates before the dust does."),         TEXT("The dust was here before the crates. It can wait its turn.") },
			{ TEXT("Runner go past you yet?"),                       TEXT("Twice. Both times looking up, like the Keeper was a lamp.") },
			{ TEXT("You are standing in my light."),                 TEXT("Your light is a beacon. Everybody is standing in it.") },
			{ TEXT("If the Deep opens tonight, do not run."),        TEXT("I never run. I leave early, which looks the same from behind.") },
			{ TEXT("Ash in the water again."),                       TEXT("Then drink it standing up. It settles, and so do you.") },
			{ TEXT("You still keep count of the names?"),            TEXT("Eleven. You ask so you can hear me say eleven.") },
			{ TEXT("Court wants the plaza quiet by dark."),          TEXT("Then the Court can come and be quiet in it.") },
			{ TEXT("Rain in the Deep tonight."),                     TEXT("Rain does not reach the Deep. So that is not rain.") },
			{ TEXT("Eat something. You have been on that wall since the bell."), TEXT("The wall has been on me longer. We have an arrangement.") },
			{ TEXT("Whose turn is the gate?"),                       TEXT("Mine. It is always mine, and you always ask.") },
			{ TEXT("They lit the second lamp while you were under."), TEXT("Somebody has to be awake for the good news.") },
			{ TEXT("Sleep. I will take the quiet half."),             TEXT("The quiet half is where the listening happens. Wake me for it.") },
		};
		return List;
	}
}

void UPHILivingWorld::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	for (const FVEILCharacterDef& Who : VEILCast::Characters())
	{
		FPHILifeRecord& R = Records.FindOrAdd(Who.Id);
		R.Character = Who.Id;
		// Nobody starts at nothing: one rung of their own competence.
		R.PowerRung = 1.f;
		Order.Add(Who.Id);
	}

	UE_LOG(LogVEIL, Log, TEXT("PHI LIVING: %d lives are now on their own clocks - routine, memory, power."),
		Records.Num());
}

void UPHILivingWorld::Tick(float DeltaTime)
{
	const UWorld* World = GetWorld();
	const UVEILTime* Clock = World ? World->GetSubsystem<UVEILTime>() : nullptr;

	// A day closes: everyone who lived through it is a little more than they
	// were. This happens with or without the runner - they are their own beings.
	if (Clock && Clock->GetDay() != LastDay)
	{
		LastDay = Clock->GetDay();
		for (TPair<FName, FPHILifeRecord>& Pair : Records)
		{
			Pair.Value.PowerRung += DayGrowthRung;
			++Pair.Value.PowerSteps;
		}

		UE_LOG(LogVEIL, Log, TEXT("PHI LIVING: day %d closes - %d people each carry 1/phi^3 more than yesterday."),
			LastDay, Records.Num());

		// One concrete example in the log, so the growth is visible and not a claim.
		if (const FVEILCharacterDef* Example = VEILCast::FindCharacter(FName(TEXT("sera_vahl"))))
		{
			UE_LOG(LogVEIL, Log, TEXT("PHI LIVING: %s -> %s"), *Example->Name.ToString(),
				*PowerOf(Example->Id));
		}
	}

	// A world line every 34 s: who is up, who is at rest, what the strongest is
	// carrying. The living part has to be audible in a log, not only on a screen.
	// THEY TALK TO EACH OTHER. The runner does not have to be in the conversation
	// for the district to have one.
	if (Order.Num() >= 2)
	{
		ExchangeAge += DeltaTime;
		if (ExchangeAge >= ExchangeInterval) { SpeakAmongThemselves(); }
	}

	LogTimer += DeltaTime;
	if (LogTimer >= 34.f)
	{
		LogTimer = 0.f;
		if (const FVEILCharacterDef* Who = VEILCast::FindCharacter(FName(TEXT("odalys_crane"))))
		{
			UE_LOG(LogVEIL, Log, TEXT("PHI LIVING: %s"), *StatusLine(Who->Id));
		}
	}
}

void UPHILivingWorld::SpeakAmongThemselves()
{
	ExchangeAge = 0.f;
	++Exchanges;

	const UWorld* World = GetWorld();
	const UVEILTime* Clock = World ? World->GetSubsystem<UVEILTime>() : nullptr;
	const int32 Day = Clock ? Clock->GetDay() : 1;

	// A golden stride, not a counter: consecutive exchanges are far apart in the
	// cast, so the same pair does not keep meeting and the same line does not
	// come back until the whole list has been round.
	const int32 N = Order.Num();
	const uint32 Stride = (uint32)(N * VeilPhi::InvPhiF) > 0u ? (uint32)(N * VeilPhi::InvPhiF) : 1u;
	const int32 IA = (int32)((Exchanges * Stride) % (uint32)N);
	const int32 IB = (int32)((Exchanges * Stride + 1u + (uint32)FMath::Max(1, N / 3)) % (uint32)N);
	const FName A = Order[(IA + N) % N];
	const FName B = Order[(IB + N) % N];
	if (A == B) { return; }

	const TArray<FExchange>& Set = ExchangeTable();
	const float Pick = Hash01(GetTypeHash(A), (uint32)Day * 31u + (uint32)Exchanges);

	// Two people with a deed on file do not talk about the weather first. Their
	// own last exchange with the runner is what they have in common tonight.
	const FPHILifeRecord* RecA = Records.Find(A);
	const bool bMachinePair = A.ToString().StartsWith(TEXT("unit_")) || B.ToString().StartsWith(TEXT("unit_"));

	FString LineA, LineB;
	if (RecA && !RecA->LastDeed.IsEmpty() && Pick < 0.30f)
	{
		LineA = FString::Printf(TEXT("Runner told me \"%s\". Promise or warning?"), *RecA->LastDeed);
		LineB = TEXT("Both. With runners it is always both.");
	}
	else if (bMachinePair && Pick < 0.55f)
	{
		LineA = TEXT("Tell the one about the dark again.");
		LineB = TEXT("You laugh wrong. That is exactly why I keep telling it.");
	}
	else
	{
		const FExchange& E = Set[(int32)(Pick * Set.Num()) % Set.Num()];
		LineA = E.A;
		LineB = E.B;
	}

	Overheard = FString::Printf(TEXT("%s: \"%s\"    %s: \"%s\""),
		*ShortName(A), *LineA, *ShortName(B), *LineB);

	UE_LOG(LogVEIL, Log, TEXT("PHI LIVING: overheard - %s"), *Overheard);
}

float UPHILivingWorld::OverheardAlpha() const
{
	// In over a second, out over the last two: a line you can miss is a line that
	// belongs to the world rather than to the interface.
	const float In = FMath::Clamp(ExchangeAge / 1.0f, 0.f, 1.f);
	const float Out = FMath::Clamp((ExchangeInterval - ExchangeAge) / 2.0f, 0.f, 1.f);
	return FMath::Min(In, Out);
}

FPHILifeRecord& UPHILivingWorld::Rec(FName Character)
{
	return Records.FindOrAdd(Character);
}

void UPHILivingWorld::NoteDeed(FName Character, bool bHelped, const FString& Words)
{
	if (Character.IsNone()) { return; }

	FPHILifeRecord& R = Rec(Character);
	if (bHelped) { ++R.Favours; } else { ++R.Harm; }
	R.LastDeed = Words;
	const UWorld* World = GetWorld();
	const UVEILTime* Clock = World ? World->GetSubsystem<UVEILTime>() : nullptr;
	R.LastDeedDay = Clock ? Clock->GetDay() : 1;
	R.PowerRung += bHelped ? DeedRung : DeedRung / PhiV; // being defied teaches you something too
	++R.PowerSteps;
	++TotalDeeds;

	UE_LOG(LogVEIL, Log, TEXT("PHI LIVING: %s remembers \"%s\" (favours %d, harm %d, power rung %.4f)."),
		*Character.ToString(), *Words, R.Favours, R.Harm, R.PowerRung);
}

bool UPHILivingWorld::HasMemory(FName Character) const
{
	const FPHILifeRecord* R = Records.Find(Character);
	return R && (R->Favours > 0 || R->Harm > 0) && !R->LastDeed.IsEmpty();
}

FString UPHILivingWorld::MemoryLineFor(FName Character) const
{
	const FPHILifeRecord* R = Records.Find(Character);
	if (!R || R->LastDeed.IsEmpty()) { return FString(); }

	const UWorld* World = GetWorld();
	const UVEILTime* Clock = World ? World->GetSubsystem<UVEILTime>() : nullptr;
	const int32 DaysAgo = Clock ? FMath::Max(0, Clock->GetDay() - R->LastDeedDay) : 0;
	const FString When = DaysAgo == 0 ? TEXT("earlier this same day")
		: DaysAgo == 1 ? TEXT("yesterday")
		: FString::Printf(TEXT("%d days ago"), DaysAgo);

	// The greeting is not the authored one anymore: it is the one that remembers.
	if (R->Harm > 0 && R->Favours == 0)
	{
		return FString::Printf(TEXT("You told me \"%s\", %s. I wrote it down. I write everything down."),
			*R->LastDeed, *When);
	}
	if (R->Favours > 0 && R->Harm == 0)
	{
		return FString::Printf(TEXT("You said \"%s\", %s. I have been carrying that around since."),
			*R->LastDeed, *When);
	}
	return FString::Printf(TEXT("You have stood on both sides of me - \"%s\" was the last of it, %s. Which are you today?"),
		*R->LastDeed, *When);
}

float UPHILivingWorld::PowerRungOf(FName Character) const
{
	const FPHILifeRecord* R = Records.Find(Character);
	return R ? R->PowerRung : 1.f;
}

FString UPHILivingWorld::PowerOf(FName Character) const
{
	const FPHILifeRecord* R = Records.Find(Character);
	if (!R) { return FString(); }

	FString Signature = TEXT("keeps their own counsel");
	for (const FName& ArchId : VEILSociety::ArchetypesOf(Character))
	{
		if (const FVEILArchetype* Arch = VEILSociety::FindArchetype(ArchId))
		{
			Signature = Arch->Signature;
			break;
		}
	}

	return FString::Printf(TEXT("%s  //  rung %.4f  //  grown %d"), *Signature, R->PowerRung, R->PowerSteps);
}

float UPHILivingWorld::RegardOf(FName Character) const
{
	const FPHILifeRecord* R = Records.Find(Character);
	if (!R) { return 0.f; }
	const int32 Net = R->Favours - R->Harm;
	return FMath::Clamp((float)Net / 3.f, -1.f, 1.f);
}

bool UPHILivingWorld::IsOffDuty() const
{
	const UWorld* World = GetWorld();
	const UVEILTime* Clock = World ? World->GetSubsystem<UVEILTime>() : nullptr;
	return Clock ? !Clock->IsDutyBeat() : false;
}

FString UPHILivingWorld::RoutineOf(FName Character) const
{
	return VEILSociety::ActivityNow(Character, !IsOffDuty());
}

FString UPHILivingWorld::StatusLine(FName Character) const
{
	const FVEILCharacterDef* Who = VEILCast::FindCharacter(Character);
	const FString Name = Who ? Who->Name.ToString() : Character.ToString();
	return FString::Printf(TEXT("%s  %s  //  %s  //  %s"),
		*Name, IsOffDuty() ? TEXT("[AT REST]") : TEXT("[ON POST]"),
		*RoutineOf(Character), *PowerOf(Character));
}

TStatId UPHILivingWorld::GetStatId() const
{
	return TStatId();
}
