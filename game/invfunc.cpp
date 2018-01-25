/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
 * Original game is written by Core Design Ltd. in 1997.
 * Lara Croft and Tomb Raider are trademarks of Square Enix Ltd.
 *
 * This file is part of TR2Main.
 *
 * TR2Main is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TR2Main is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TR2Main.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "global/precompiled.h"
#include "game/invfunc.h"
#include "game/items.h"
#include "specific/frontend.h"
#include "global/vars.h"

void __cdecl InitColours() {
	InvColours.black		= S_COLOUR(0x00, 0x00, 0x00);
	InvColours.gray			= S_COLOUR(0x40, 0x40, 0x40);
	InvColours.white		= S_COLOUR(0xFF, 0xFF, 0xFF);
	InvColours.red			= S_COLOUR(0xFF, 0x00, 0x00);
	InvColours.orange		= S_COLOUR(0xFF, 0x80, 0x00);
	InvColours.yellow		= S_COLOUR(0xFF, 0xFF, 0x00);
	InvColours.darkGreen	= S_COLOUR(0x00, 0x80, 0x00);
	InvColours.green		= S_COLOUR(0x00, 0xFF, 0x00);
	InvColours.cyan			= S_COLOUR(0x00, 0xFF, 0xFF);
	InvColours.blue			= S_COLOUR(0x00, 0x00, 0xFF);
	InvColours.magenta		= S_COLOUR(0xFF, 0x00, 0xFF);
}

BOOL __cdecl Inv_AddItem(GAME_OBJECT_ID itemID) {
	int i, j, items;
	int found = 0;
	GAME_OBJECT_ID optionID = Inv_GetItemOption(itemID);

	for( i = 0; i < InvMainObjectsCount; ++i ) {
		if( InvMainList[i]->objectID == optionID ) {
			found = 1;
			break;
		}
	}

	for( j = 0; j < InvKeyObjectsCount; ++j ) {
		if( InvKeysList[j]->objectID == optionID ) {
			found = 2;
			break;
		}
	}

	if( found == 1 ) {
		if( itemID == ID_FLARES_ITEM )
			InvMainQtys[i] += 6;
		else
			++InvMainQtys[i];

		return TRUE;
	}

	if( found == 2 ) {
		++InvKeysQtys[j];
		return TRUE;
	}

	switch( itemID ) {

		case ID_COMPASS_ITEM :
		case ID_COMPASS_OPTION :
			Inv_InsertItem(&InvCompassOption);
			return TRUE;

		case ID_PISTOL_ITEM :
		case ID_PISTOL_OPTION :
			Inv_InsertItem(&InvPistolOption);
			if( Lara_GunType == LGT_Unarmed )
				Lara_GunType = LGT_Pistols;
			return TRUE;

		case ID_SHOTGUN_ITEM :
		case ID_SHOTGUN_OPTION :
			items = Inv_RequestItem(ID_SHOTGUN_AMMO_ITEM);
			for( i = 0; i < items; ++i ) {
				Inv_RemoveItem(ID_SHOTGUN_AMMO_ITEM);
				ShotgunAmmo += 12;
			}
			ShotgunAmmo += 12;
			Inv_InsertItem(&InvShotgunOption);
			if( Lara_GunType == LGT_Unarmed )
				Lara_GunType = LGT_Shotgun;
			if( Lara_TorsoObject == ID_LARA )
				Lara_TorsoObject = ID_LARA_SHOTGUN;
			GlobalItemReplace(ID_SHOTGUN_ITEM, ID_SHOTGUN_AMMO_ITEM);
			return FALSE;

		case ID_MAGNUM_ITEM :
		case ID_MAGNUM_OPTION :
			items = Inv_RequestItem(ID_MAGNUM_AMMO_ITEM);
			for( i = 0; i < items; ++i ) {
				Inv_RemoveItem(ID_MAGNUM_AMMO_ITEM);
				MagnumAmmo += 40;
			}
			MagnumAmmo += 40;
			Inv_InsertItem(&InvMagnumOption);
			GlobalItemReplace(ID_MAGNUM_ITEM, ID_MAGNUM_AMMO_ITEM);
			return FALSE;

		case ID_UZI_ITEM :
		case ID_UZI_OPTION :
			items = Inv_RequestItem(ID_UZI_AMMO_ITEM);
			for( i = 0; i < items; ++i ) {
				Inv_RemoveItem(ID_UZI_AMMO_ITEM);
				UziAmmo += 80;
			}
			UziAmmo += 80;
			Inv_InsertItem(&InvUziOption);
			GlobalItemReplace(ID_UZI_ITEM, ID_UZI_AMMO_ITEM);
			return FALSE;

		case ID_HARPOON_ITEM :
		case ID_HARPOON_OPTION :
			items = Inv_RequestItem(ID_HARPOON_AMMO_ITEM);
			for( i = 0; i < items; ++i ) {
				Inv_RemoveItem(ID_HARPOON_AMMO_ITEM);
				HarpoonAmmo += 3;
			}
			HarpoonAmmo += 3;
			Inv_InsertItem(&InvHarpoonOption);
			GlobalItemReplace(ID_HARPOON_ITEM, ID_HARPOON_AMMO_ITEM);
			return FALSE;

		case ID_M16_ITEM :
		case ID_M16_OPTION :
			items = Inv_RequestItem(ID_M16_AMMO_ITEM);
			for( i = 0; i < items; ++i ) {
				Inv_RemoveItem(ID_M16_AMMO_ITEM);
				M16Ammo += 40;
			}
			M16Ammo += 40;
			Inv_InsertItem(&InvM16Option);
			GlobalItemReplace(ID_M16_ITEM, ID_M16_AMMO_ITEM);
			return FALSE;

		case ID_GRENADE_ITEM :
		case ID_GRENADE_OPTION :
			items = Inv_RequestItem(ID_GRENADE_AMMO_ITEM);
			for( i = 0; i < items; ++i ) {
				Inv_RemoveItem(ID_GRENADE_AMMO_ITEM);
				GrenadeAmmo += 2;
			}
			GrenadeAmmo += 2;
			Inv_InsertItem(&InvGrenadeOption);
			GlobalItemReplace(ID_GRENADE_ITEM, ID_GRENADE_AMMO_ITEM);
			return FALSE;

		case ID_SHOTGUN_AMMO_ITEM :
		case ID_SHOTGUN_AMMO_OPTION :
			if( Inv_RequestItem(ID_SHOTGUN_ITEM) != 0 )
				ShotgunAmmo += 12;
			else
				Inv_InsertItem(&InvShotgunAmmoOption);
			return FALSE;

		case ID_MAGNUM_AMMO_ITEM :
		case ID_MAGNUM_AMMO_OPTION :
			if( Inv_RequestItem(ID_MAGNUM_ITEM) != 0 )
				MagnumAmmo += 40;
			else
				Inv_InsertItem(&InvMagnumAmmoOption);
			return FALSE;

		case ID_UZI_AMMO_ITEM :
		case ID_UZI_AMMO_OPTION :
			if( Inv_RequestItem(ID_UZI_ITEM) != 0 )
				UziAmmo += 80;
			else
				Inv_InsertItem(&InvUziAmmoOption);
			return FALSE;

		case ID_HARPOON_AMMO_ITEM :
		case ID_HARPOON_AMMO_OPTION :
			if( Inv_RequestItem(ID_HARPOON_ITEM) != 0 )
				HarpoonAmmo += 3;
			else
				Inv_InsertItem(&InvHarpoonAmmoOption);
			return FALSE;

		case ID_M16_AMMO_ITEM :
		case ID_M16_AMMO_OPTION :
			if( Inv_RequestItem(ID_M16_ITEM) != 0 )
				M16Ammo += 40;
			else
				Inv_InsertItem(&InvM16AmmoOption);
			return FALSE;

		case ID_GRENADE_AMMO_ITEM :
		case ID_GRENADE_AMMO_OPTION :
			if( Inv_RequestItem(ID_GRENADE_ITEM) != 0 )
				GrenadeAmmo += 2;
			else
				Inv_InsertItem(&InvGrenadeAmmoOption);
			return FALSE;

		case ID_FLARES_ITEM :
		case ID_FLARES_OPTION :
			Inv_InsertItem(&InvFlareOption);
			for( i=0; i<5; ++i ) {
				Inv_AddItem(ID_FLARE_ITEM);
			}
			return TRUE;

		case ID_FLARE_ITEM :
			Inv_InsertItem(&InvFlareOption);
			return TRUE;

		case ID_SMALL_MEDIPACK_ITEM :
		case ID_SMALL_MEDIPACK_OPTION :
			Inv_InsertItem(&InvSmallMedipackOption);
			return TRUE;

		case ID_LARGE_MEDIPACK_ITEM :
		case ID_LARGE_MEDIPACK_OPTION :
			Inv_InsertItem(&InvLargeMedipackOption);
			return TRUE;

		case ID_SECRET1 :
			SaveGame.statistics.secrets |= 1;
			return TRUE;

		case ID_SECRET2 :
			SaveGame.statistics.secrets |= 2;
			return TRUE;

		case ID_SECRET3 :
			SaveGame.statistics.secrets |= 4;
			return TRUE;

		case ID_PICKUP_ITEM1 :
		case ID_PICKUP_OPTION1 :
			Inv_InsertItem(&InvPickup1Option);
			return TRUE;

		case ID_PICKUP_ITEM2 :
		case ID_PICKUP_OPTION2 :
			Inv_InsertItem(&InvPickup2Option);
			return TRUE;

		case ID_PUZZLE_ITEM1 :
		case ID_PUZZLE_OPTION1 :
			Inv_InsertItem(&InvPuzzle1Option);
			return TRUE;

		case ID_PUZZLE_ITEM2 :
		case ID_PUZZLE_OPTION2 :
			Inv_InsertItem(&InvPuzzle2Option);
			return TRUE;

		case ID_PUZZLE_ITEM3 :
		case ID_PUZZLE_OPTION3 :
			Inv_InsertItem(&InvPuzzle3Option);
			return TRUE;

		case ID_PUZZLE_ITEM4 :
		case ID_PUZZLE_OPTION4 :
			Inv_InsertItem(&InvPuzzle4Option);
			return TRUE;

		case ID_KEY_ITEM1 :
		case ID_KEY_OPTION1 :
			Inv_InsertItem(&InvKey1Option);
			return TRUE;

		case ID_KEY_ITEM2 :
		case ID_KEY_OPTION2 :
			Inv_InsertItem(&InvKey2Option);
			return TRUE;

		case ID_KEY_ITEM3 :
		case ID_KEY_OPTION3 :
			Inv_InsertItem(&InvKey3Option);
			return TRUE;

		case ID_KEY_ITEM4 :
		case ID_KEY_OPTION4 :
			Inv_InsertItem(&InvKey4Option);
			return TRUE;

		default:
			break;
	}
	return FALSE;
}

void __cdecl Inv_InsertItem(INVENTORY_ITEM *item) {
	int i, j;

	if( item->invPos < 100 ) {
		for( i = 0; i < InvMainObjectsCount; ++i ) {
			if( InvMainList[i]->invPos > item->invPos ) {
				for( j = InvMainObjectsCount; j >= i; --j ) {
					InvMainList[j+1] = InvMainList[j];
					InvMainQtys[j+1] = InvMainQtys[j];
				}
				break;
			}
		}
		InvMainList[i] = item;
		InvMainQtys[i] = 1;
		++InvMainObjectsCount;
	} else {
		for( i = 0; i < InvKeyObjectsCount; ++i ) {
			if( InvKeysList[i]->invPos > item->invPos ) {
				for( j = InvKeyObjectsCount; j >= i; --j ) {
					InvKeysList[j+1] = InvKeysList[j];
					InvKeysQtys[j+1] = InvKeysQtys[j];
				}
				break;
			}
		}
		InvKeysList[i] = item;
		InvKeysQtys[i] = 1;
		++InvKeyObjectsCount;
	}
}

int __cdecl Inv_RequestItem(GAME_OBJECT_ID itemID) {
	int i;
	int optionID = Inv_GetItemOption(itemID);

	for( i = 0; i < InvMainObjectsCount; ++i )
		if( InvMainList[i]->objectID == optionID )
			return InvMainQtys[i];

	for( i = 0; i < InvKeyObjectsCount; ++i )
		if( InvKeysList[i]->objectID == optionID )
			return InvKeysQtys[i];

	return 0;
}


void __cdecl Inv_RemoveAllItems() {
	InvMainObjectsCount = 0;
	InvMainCurrent = 0;
	InvKeyObjectsCount = 0;
	InvKeysCurrent = 0;
}


BOOL __cdecl Inv_RemoveItem(GAME_OBJECT_ID itemID) {
	int i, j;
	GAME_OBJECT_ID optionID = Inv_GetItemOption(itemID);

	for( i = 0; i < InvMainObjectsCount; ++i ) {
		if( InvMainList[i]->objectID == optionID ) {
			if( --InvMainQtys[i] <= 0 ) {
				--InvMainObjectsCount;

				for( j = i; j < InvMainObjectsCount; ++j ) {
					InvMainList[j] = InvMainList[j+1];
					InvMainQtys[j] = InvMainQtys[j+1];
				}
			}
			return TRUE;
		}
	}

	for( i = 0; i < InvKeyObjectsCount; ++i ) {
		if( InvKeysList[i]->objectID == optionID ) {
			if( --InvKeysQtys[i] <= 0 ) {
				--InvKeyObjectsCount;

				for( j = i; j < InvKeyObjectsCount; ++j ) {
					InvKeysList[j] = InvKeysList[j+1];
					InvKeysQtys[j] = InvKeysQtys[j+1];
				}
			}
			return TRUE;
		}
	}

	return FALSE;
}


GAME_OBJECT_ID __cdecl Inv_GetItemOption(GAME_OBJECT_ID itemID) {
	switch( itemID ) {
	// Weapons
		case ID_PISTOL_ITEM :
		case ID_PISTOL_OPTION :
			return ID_PISTOL_OPTION;
		case ID_SHOTGUN_ITEM :
		case ID_SHOTGUN_OPTION :
			return ID_SHOTGUN_OPTION;
		case ID_MAGNUM_ITEM :
		case ID_MAGNUM_OPTION :
			return ID_MAGNUM_OPTION;
		case ID_UZI_ITEM :
		case ID_UZI_OPTION :
			return ID_UZI_OPTION;
		case ID_HARPOON_ITEM :
		case ID_HARPOON_OPTION :
			return ID_HARPOON_OPTION;
		case ID_M16_ITEM :
		case ID_M16_OPTION :
			return ID_M16_OPTION;
		case ID_GRENADE_ITEM :
		case ID_GRENADE_OPTION :
			return ID_GRENADE_OPTION;
	// Ammo
		// NOTE: It seems here could be Pistol Ammo Option, but it was deleted or commented by CORE
	/*	case ID_PISTOL_AMMO_ITEM :
		case ID_PISTOL_AMMO_OPTION :
			return ID_PISTOL_AMMO_OPTION; */
		case ID_SHOTGUN_AMMO_ITEM :
		case ID_SHOTGUN_AMMO_OPTION :
			return ID_SHOTGUN_AMMO_OPTION;
		case ID_MAGNUM_AMMO_ITEM :
		case ID_MAGNUM_AMMO_OPTION :
			return ID_MAGNUM_AMMO_OPTION;
		case ID_UZI_AMMO_ITEM :
		case ID_UZI_AMMO_OPTION :
			return ID_UZI_AMMO_OPTION;
		case ID_HARPOON_AMMO_ITEM :
		case ID_HARPOON_AMMO_OPTION :
			return ID_HARPOON_AMMO_OPTION;
		case ID_M16_AMMO_ITEM :
		case ID_M16_AMMO_OPTION :
			return ID_M16_AMMO_OPTION;
		case ID_GRENADE_AMMO_ITEM :
		case ID_GRENADE_AMMO_OPTION :
			return ID_GRENADE_AMMO_OPTION;
	// Flares, Medipacks
		case ID_SMALL_MEDIPACK_ITEM :
		case ID_SMALL_MEDIPACK_OPTION :
			return ID_SMALL_MEDIPACK_OPTION;
		case ID_LARGE_MEDIPACK_ITEM :
		case ID_LARGE_MEDIPACK_OPTION :
			return ID_LARGE_MEDIPACK_OPTION;
		case ID_FLARE_ITEM :
		case ID_FLARES_ITEM :
		case ID_FLARES_OPTION :
			return ID_FLARES_OPTION;
	// Puzzles
		case ID_PUZZLE_ITEM1 :
		case ID_PUZZLE_OPTION1 :
			return ID_PUZZLE_OPTION1;
		case ID_PUZZLE_ITEM2 :
		case ID_PUZZLE_OPTION2 :
			return ID_PUZZLE_OPTION2;
		case ID_PUZZLE_ITEM3 :
		case ID_PUZZLE_OPTION3 :
			return ID_PUZZLE_OPTION3;
		case ID_PUZZLE_ITEM4 :
		case ID_PUZZLE_OPTION4 :
			return ID_PUZZLE_OPTION4;
	// Pickups
		case ID_PICKUP_ITEM1 :
		case ID_PICKUP_OPTION1 :
			return ID_PICKUP_OPTION1;
		case ID_PICKUP_ITEM2 :
		case ID_PICKUP_OPTION2 :
			return ID_PICKUP_OPTION2;
	// Keys
		case ID_KEY_ITEM1 :
		case ID_KEY_OPTION1 :
			return ID_KEY_OPTION1;
		case ID_KEY_ITEM2 :
		case ID_KEY_OPTION2 :
			return ID_KEY_OPTION2;
		case ID_KEY_ITEM3 :
		case ID_KEY_OPTION3 :
			return ID_KEY_OPTION3;
		case ID_KEY_ITEM4 :
		case ID_KEY_OPTION4 :
			return ID_KEY_OPTION4;
		default :
			break;
	}
	return ID_NONE;
}

/*
 * Inject function
 */
void Inject_InvFunc() {
	INJECT(0x00423B10, InitColours);

//	INJECT(0x00423C20, RingIsOpen);
//	INJECT(0x00423D90, RingIsNotOpen);
//	INJECT(0x00423E20, RingNotActive);
//	INJECT(0x00424290, RingActive);

	INJECT(0x004242D0, Inv_AddItem);
	INJECT(0x00424AE0, Inv_InsertItem);
	INJECT(0x00424C10, Inv_RequestItem);
	INJECT(0x00424C90, Inv_RemoveAllItems);
	INJECT(0x00424CB0, Inv_RemoveItem);
	INJECT(0x00424DC0, Inv_GetItemOption);

//	INJECT(0x00424FB0, RemoveInventoryText);
//	INJECT(0x00424FE0, Inv_RingInit);
//	INJECT(0x004250F0, Inv_RingGetView);
//	INJECT(0x00425150, Inv_RingLight);
//	INJECT(0x00425190, Inv_RingCalcAdders);
//	INJECT(0x004251C0, Inv_RingDoMotions);
//	INJECT(0x00425300, Inv_RingRotateLeft);
//	INJECT(0x00425330, Inv_RingRotateRight);
//	INJECT(0x00425360, Inv_RingMotionInit);
//	INJECT(0x004253D0, Inv_RingMotionSetup);
//	INJECT(0x00425400, Inv_RingMotionRadius);
//	INJECT(0x00425430, Inv_RingMotionRotation);
//	INJECT(0x00425460, Inv_RingMotionCameraPos);
//	INJECT(0x00425490, Inv_RingMotionCameraPitch);
//	INJECT(0x004254B0, Inv_RingMotionItemSelect);
//	INJECT(0x00425510, Inv_RingMotionItemDeselect);
}
