#include "Story/VEILItems.h"

namespace VeilItems
{
	// THE SHEET, LINE FOR LINE. The first fifteen are its TYPE table in order with the WHAT and
	// WHO columns kept intact; the rest are the materials it lists underneath.
	static const TArray<FEntry>& Table()
	{
		static const TArray<FEntry> Rows = {
			{ TEXT("ARMOR"),    ESlot::Armor,      EWho::All,        TEXT("Chest, gold-trimmed, VIT +") },
			{ TEXT("BELT"),     ESlot::Belt,       EWho::All,        TEXT("Utility, shard slots x5") },
			{ TEXT("BOOTS"),    ESlot::Boots,      EWho::RunnerOnly, TEXT("Speed, sprint, glide landing") },
			{ TEXT("GLOVES"),   ESlot::Gloves,     EWho::CastOnly,   TEXT("Grip, combat, craft") },
			{ TEXT("WARDEN"),   ESlot::Warden,     EWho::WardenOnly, TEXT("Shield, guard, beacon ward") },
			{ TEXT("CAST"),     ESlot::Cast,       EWho::CastOnly,   TEXT("Staff, fragment magic") },
			{ TEXT("FRAGMENT"), ESlot::Fragment,   EWho::All,        TEXT("Phi shard, power, signal") },
			{ TEXT("CACHE"),    ESlot::Cache,      EWho::All,        TEXT("Storage, deep cache 0,-6200") },
			{ TEXT("MACHINE"),  ESlot::Machine,    EWho::All,        TEXT("Craft, machine parts") },
			{ TEXT("RUNNER"),   ESlot::Runner,     EWho::RunnerOnly, TEXT("Wings, cape, gold phi") },
			{ TEXT("WEAPON"),   ESlot::Weapon,     EWho::All,        TEXT("Sword, bow, spear, gold") },
			{ TEXT("POTION"),   ESlot::Potion,     EWho::All,        TEXT("Vigor, Aether, heal") },
			{ TEXT("FOOD"),     ESlot::Food,       EWho::All,        TEXT("Camp cook, stamina") },
			{ TEXT("MOUNT"),    ESlot::Mount,      EWho::All,        TEXT("Horse, crystal beast, glider") },
			{ TEXT("KEY"),      ESlot::Key,        EWho::All,        TEXT("Vault key, Spire key, Cache key") },

			// ---- "Also:" on the sheet ----
			{ TEXT("SWORD"),        ESlot::Sword,       EWho::All, TEXT("Gold blade, one hand") },
			{ TEXT("BOW"),          ESlot::Bow,         EWho::All, TEXT("Signal shot, drawn string") },
			{ TEXT("SPEAR"),        ESlot::Spear,       EWho::All, TEXT("Reach, throw, haft") },
			{ TEXT("STAFF"),        ESlot::Staff,       EWho::CastOnly, TEXT("Fragment magic focus") },
			{ TEXT("SHIELD"),       ESlot::Shield,      EWho::All, TEXT("Guard, beacon ward") },
			{ TEXT("SHARD"),        ESlot::Shard,       EWho::All, TEXT("Phi shard, forge fuel") },
			{ TEXT("MAP"),          ESlot::Map,         EWho::All, TEXT("Deep map, cartographer's hand") },
			{ TEXT("SCROLL"),       ESlot::Scroll,      EWho::All, TEXT("Lore, seam, rumour") },
			{ TEXT("GEM"),          ESlot::Gem,         EWho::All, TEXT("Trade, socket, value") },
			{ TEXT("CRYSTAL"),      ESlot::Crystal,     EWho::All, TEXT("Blue faceted, mountain born") },
			{ TEXT("CLOTH"),        ESlot::Cloth,       EWho::All, TEXT("Cape, wrap, sail") },
			{ TEXT("LEATHER"),      ESlot::Leather,     EWho::All, TEXT("Belt, boot, saddle") },
			{ TEXT("METAL"),        ESlot::Metal,       EWho::All, TEXT("Gold trim, plate, rivet") },
			{ TEXT("WOOD"),         ESlot::Wood,        EWho::All, TEXT("Haft, bridge plank, stall") },
			{ TEXT("MACHINE PART"), ESlot::MachinePart, EWho::All, TEXT("Cache machine, deep works") },
			{ TEXT("MOUNT SADDLE"), ESlot::Saddle,      EWho::All, TEXT("Ride, carry, dismount clean") },
			{ TEXT("WINGS"),        ESlot::Wing,        EWho::RunnerOnly, TEXT("Gold phi wings") },
			{ TEXT("CAPE"),         ESlot::Cape,        EWho::RunnerOnly, TEXT("Dark cape") },
			{ TEXT("RING"),         ESlot::Ring,        EWho::All, TEXT("Court ring, quiet bond") },
			{ TEXT("AMULET"),       ESlot::Amulet,      EWho::All, TEXT("Signal charm, worn close") },
			{ TEXT("HELMET"),       ESlot::Helmet,      EWho::All, TEXT("Gold brow, sight slit") },
		};
		return Rows;
	}

	const TArray<FEntry>& All() { return Table(); }
	int32 Count() { return Table().Num(); }

	const TCHAR* SlotName(ESlot In)
	{
		for (const FEntry& E : Table()) { if (E.Slot == In) { return E.Name; } }
		return TEXT("NONE");
	}

	const TCHAR* WhoName(EWho In)
	{
		switch (In)
		{
		case EWho::RunnerOnly: return TEXT("RUNNER");
		case EWho::WardenOnly: return TEXT("WARDEN");
		case EWho::CastOnly:   return TEXT("CAST");
		default:               return TEXT("ALL");
		}
	}

	FString ForWho(EWho In)
	{
		FString Out;
		for (const FEntry& E : Table())
		{
			if (E.Who != EWho::All && E.Who != In) { continue; }
			if (!Out.IsEmpty()) { Out += TEXT(", "); }
			Out += E.Name;
		}
		return Out.IsEmpty() ? FString(TEXT("nothing on this page")) : Out;
	}

	FString Summary()
	{
		// Count the TYPES the sheet's table names (the first fifteen rows), not the materials.
		int32 Types = 0;
		for (const FEntry& E : Table())
		{
			if ((uint8)E.Slot <= (uint8)ESlot::Key) { ++Types; }
		}
		return FString::Printf(TEXT("%d types / %d lines"), Types, Table().Num());
	}
}
