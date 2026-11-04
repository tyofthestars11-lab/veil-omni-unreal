// THE ITEM BOOK, FROM THE OWNER'S OWN SHEET.
//
// VEIL-OMNI-GODMODE-ALL.html and VEIL-OMNI-START-MENU-ALL-ITEMS-50-IN-1.html both carry the
// panel "ITEM TYPES - ALL OF IT": a table of 15 TYPES with WHAT each is and WHO may carry it,
// followed by the full material list (Swords, Bows, Spears, Staves, Shields, Potions, Food,
// Keys, Shards, Maps, Scrolls, Gems, Crystals, Cloth, Leather, Metal, Wood, Machine Parts,
// Mount Saddles, Wings, Capes, Rings, Amulets, Boots, Belts, Gloves, Armor, Helmets).
//
// Measured before this file existed: ARMOR, BELT, BOOTS and GLOVES appeared only inside the
// 50-item ledger string on the title screen, and POTION, FOOD, HELMET, AMULET and SADDLE
// appeared in ZERO source files. The inventory page could not name a single thing it held.
//
// Nothing here spawns, spends or saves. It is the vocabulary the inventory, shop, craft and
// arena pages read, so all five speak the same names as the sheet.
#pragma once

#include "CoreMinimal.h"

namespace VeilItems
{
	/** The sheet's TYPE column, in its own order. */
	enum class ESlot : uint8
	{
		Armor, Belt, Boots, Gloves, Warden, Cast, Fragment, Cache, Machine, Runner,
		Weapon, Potion, Food, Mount, Key,
		// The materials the sheet lists after the table.
		Sword, Bow, Spear, Staff, Shield, Shard, Map, Scroll, Gem, Crystal,
		Cloth, Leather, Metal, Wood, MachinePart, Saddle, Wing, Cape, Ring, Amulet, Helmet,
		Count
	};

	/** The sheet's WHO column. */
	enum class EWho : uint8
	{
		All,
		RunnerOnly,
		WardenOnly,
		CastOnly
	};

	struct FEntry
	{
		const TCHAR* Name;
		ESlot Slot;
		EWho Who;
		const TCHAR* What;
	};

	/** Every line of the sheet, in the sheet's order. */
	VEIL_API const TArray<FEntry>& All();

	/** How many lines the book holds. */
	VEIL_API int32 Count();

	/** "ARMOR", "MOUNT SADDLE", ... */
	VEIL_API const TCHAR* SlotName(ESlot In);

	/** "ALL", "RUNNER", "WARDEN", "CAST" - the sheet's WHO words. */
	VEIL_API const TCHAR* WhoName(EWho In);

	/** One line naming everything a carrier may hold, for the inventory page. */
	VEIL_API FString ForWho(EWho In);

	/** "15 types / 36 lines" - what the boot log prints once. */
	VEIL_API FString Summary();
}
