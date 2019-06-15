/*
 * Copyright (c) 2017-2019 Michael Chaban. All rights reserved.
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
#include "game/laramisc.h"
#include "game/invfunc.h"
#include "game/items.h"
#include "game/larafire.h"
#include "game/sound.h"
#include "global/vars.h"

void __cdecl UseItem(__int16 itemID) {
	if( itemID <= ID_NONE || itemID >= ID_NUMBER_OBJECTS )
		return;

	switch( itemID ) {
		case ID_PISTOL_ITEM:
		case ID_PISTOL_OPTION:
			Lara.request_gun_type = LGT_Pistols;
			break;

		case ID_MAGNUM_ITEM:
		case ID_MAGNUM_OPTION:
			Lara.request_gun_type = LGT_Magnums;
			break;

		case ID_UZI_ITEM:
		case ID_UZI_OPTION:
			Lara.request_gun_type = LGT_Uzis;
			break;

		case ID_SHOTGUN_ITEM:
		case ID_SHOTGUN_OPTION:
			Lara.request_gun_type = LGT_Shotgun;
			break;

		case ID_HARPOON_ITEM:
		case ID_HARPOON_OPTION:
			Lara.request_gun_type = LGT_Harpoon;
			break;

		case ID_M16_ITEM:
		case ID_M16_OPTION:
			Lara.request_gun_type = LGT_M16;
			break;

		case ID_GRENADE_ITEM:
		case ID_GRENADE_OPTION:
			Lara.request_gun_type = LGT_Grenade;
			break;

		case ID_FLARES_ITEM:
		case ID_FLARES_OPTION:
			Lara.request_gun_type = LGT_Flare;
			break;

		case ID_SMALL_MEDIPACK_ITEM:
		case ID_SMALL_MEDIPACK_OPTION:
			if( LaraItem->hitPoints > 0 && LaraItem->hitPoints < 1000 ) {
				LaraItem->hitPoints += 500;
				if( LaraItem->hitPoints > 1000 )
					LaraItem->hitPoints = 1000;
				Inv_RemoveItem(ID_SMALL_MEDIPACK_ITEM);
				PlaySoundEffect(116, NULL, 2);
				++SaveGame.statistics.mediPacks;
			}
			break;

		case ID_LARGE_MEDIPACK_ITEM:
		case ID_LARGE_MEDIPACK_OPTION:
			if( LaraItem->hitPoints > 0 && LaraItem->hitPoints < 1000 ) {
				LaraItem->hitPoints += 1000;
				if( LaraItem->hitPoints > 1000 )
					LaraItem->hitPoints = 1000;
				Inv_RemoveItem(ID_LARGE_MEDIPACK_ITEM);
				PlaySoundEffect(116, NULL, 2);
				SaveGame.statistics.mediPacks += 2;
			}
			break;
	}
}

void __cdecl LaraCheatGetStuff() {
	Inv_AddItem(ID_HARPOON_ITEM);
	Inv_AddItem(ID_M16_ITEM);
	Inv_AddItem(ID_GRENADE_ITEM);
	Inv_AddItem(ID_SHOTGUN_ITEM);
	Inv_AddItem(ID_UZI_ITEM);
	Inv_AddItem(ID_MAGNUM_ITEM);
	Inv_AddItem(ID_PISTOL_ITEM);

	for( int i = 0; i < 10; ++i ) {
		Inv_AddItem(ID_FLARES_ITEM);
		Inv_AddItem(ID_SMALL_MEDIPACK_ITEM);
		Inv_AddItem(ID_LARGE_MEDIPACK_ITEM);
	}

	Lara.magnum_ammo = 1000;
	Lara.uzi_ammo = 2000;
	Lara.shotgun_ammo = 300;
	Lara.harpoon_ammo = 300;
	Lara.m16_ammo = 300;
	Lara.grenade_ammo = 300;
}

void __cdecl InitialiseLaraInventory(int levelID) {
	int i;
	START_INFO *start = &SaveGame.start[levelID];

	Inv_RemoveAllItems();

	if( GF_RemoveWeapons ) {
		start->has_pistols = 0;
		start->has_magnums = 0;
		start->has_uzis = 0;
		start->has_shotgun = 0;
		start->has_m16 = 0;
		start->has_grenade = 0;
		start->has_harpoon = 0;
		start->gunType = LGT_Unarmed;
		start->gunStatus = LGS_Armless;
		GF_RemoveWeapons = 0;
	}

	if( GF_RemoveAmmo ) {
		start->m16Ammo = 0;
		start->grenadeAmmo = 0;
		start->harpoonAmmo = 0;
		start->shotgunAmmo = 0;
		start->uziAmmo = 0;
		start->magnumAmmo = 0;
		start->pistolAmmo = 0;
		start->flares = 0;
		start->largeMedipacks = 0;
		start->smallMedipacks = 0;
		GF_RemoveAmmo = 0;
	}

	// NOTE: additional weapon availability checks not presented in the original game
	if( !Objects[ID_PISTOL_OPTION].loaded ) {
		start->has_pistols = 0;
	}
	if( !Objects[ID_UZI_OPTION].loaded ) {
		start->has_uzis = 0;
	}
	if( !Objects[ID_MAGNUM_OPTION].loaded ) {
		start->has_magnums = 0;
	}
	if( !Objects[ID_SHOTGUN_OPTION].loaded ) {
		start->has_shotgun = 0;
	}
	if( !Objects[ID_GRENADE_OPTION].loaded ) {
		start->has_grenade = 0;
	}
	if( !Objects[ID_M16_OPTION].loaded ) {
		start->has_m16 = 0;
	}
	if( !Objects[ID_HARPOON_OPTION].loaded ) {
		start->has_harpoon = 0;
	}

	// NOTE: additional ammo availability checks not presented in the original game
	if( !start->has_pistols && !Objects[ID_PISTOL_AMMO_OPTION].loaded ) {
		start->pistolAmmo = 0;
	}
	if( !start->has_uzis && !Objects[ID_UZI_AMMO_OPTION].loaded ) {
		start->uziAmmo = 0;
	}
	if( !start->has_magnums && !Objects[ID_MAGNUM_AMMO_OPTION].loaded ) {
		start->magnumAmmo = 0;
	}
	if( !start->has_shotgun && !Objects[ID_SHOTGUN_AMMO_OPTION].loaded ) {
		start->shotgunAmmo = 0;
	}
	if( !start->has_grenade && !Objects[ID_GRENADE_AMMO_OPTION].loaded ) {
		start->grenadeAmmo = 0;
	}
	if( !start->has_m16 && !Objects[ID_M16_AMMO_OPTION].loaded ) {
		start->m16Ammo = 0;
	}
	if( !start->has_harpoon && !Objects[ID_HARPOON_AMMO_OPTION].loaded ) {
		start->harpoonAmmo = 0;
	}

	// NOTE: additional start gun type availability checks not presented in the original game
	switch( start->gunType ) {
		// if the current weapon is absent then fallthrough
		case LGT_Pistols	: if( start->has_pistols ) break;
		case LGT_Magnums	: if( start->has_magnums ) break;
		case LGT_Uzis		: if( start->has_uzis ) break;
		case LGT_Shotgun	: if( start->has_shotgun ) break;
		case LGT_M16		: if( start->has_m16 ) break;
		case LGT_Grenade	: if( start->has_grenade ) break;
		case LGT_Harpoon	: if( start->has_harpoon ) break;
			start->gunType = start->has_pistols ? LGT_Pistols : LGT_Unarmed;
			break;
		default :
			break;
	}

	// Statistics
	Inv_AddItem(ID_COMPASS_ITEM);

	// Pistols
	Lara.pistol_ammo = 1000;
	if( start->has_pistols ) {
		Inv_AddItem(ID_PISTOL_ITEM);
	}

	// Magnums
	if( start->has_magnums ) {
		Inv_AddItem(ID_MAGNUM_ITEM);
		Lara.magnum_ammo = start->magnumAmmo;
		GlobalItemReplace(ID_MAGNUM_ITEM, ID_MAGNUM_AMMO_ITEM);
	} else {
		for( i = 0; i < start->magnumAmmo / 40; ++i ) {
			Inv_AddItem(ID_MAGNUM_AMMO_ITEM);
		}
		Lara.magnum_ammo = 0;
	}

	// Uzis
	if( start->has_uzis ) {
		Inv_AddItem(ID_UZI_ITEM);
		Lara.uzi_ammo = start->uziAmmo;
		GlobalItemReplace(ID_UZI_ITEM, ID_UZI_AMMO_ITEM);
	} else {
		for( i = 0; i < start->uziAmmo / 80; ++i ) {
			Inv_AddItem(ID_UZI_AMMO_ITEM);
		}
		Lara.uzi_ammo = 0;
	}

	// Shotgun
	if( start->has_shotgun ) {
		Inv_AddItem(ID_SHOTGUN_ITEM);
		Lara.shotgun_ammo = start->shotgunAmmo;
		GlobalItemReplace(ID_SHOTGUN_ITEM, ID_SHOTGUN_AMMO_ITEM);
	} else {
		for( i = 0; i < start->shotgunAmmo / 12; ++i ) {
			Inv_AddItem(ID_SHOTGUN_AMMO_ITEM);
		}
		Lara.shotgun_ammo = 0;
	}

	// Grenade
	if( start->has_grenade ) {
		Inv_AddItem(ID_GRENADE_ITEM);
		Lara.grenade_ammo = start->grenadeAmmo;
		GlobalItemReplace(ID_GRENADE_ITEM, ID_GRENADE_AMMO_ITEM);
	} else {
		for( i = 0; i < start->grenadeAmmo / 2; ++i ) {
			Inv_AddItem(ID_GRENADE_AMMO_ITEM);
		}
		Lara.grenade_ammo = 0;
	}

	// M16
	if( start->has_m16 ) {
		Inv_AddItem(ID_M16_ITEM);
		Lara.m16_ammo = start->m16Ammo;
		GlobalItemReplace(ID_M16_ITEM, ID_M16_AMMO_ITEM);
	} else {
		for( i = 0; i < start->grenadeAmmo / 10; ++i ) {
			Inv_AddItem(ID_M16_AMMO_ITEM);
		}
		Lara.m16_ammo = 0;
	}

	// Harpoon
	if( start->has_harpoon ) {
		Inv_AddItem(ID_HARPOON_ITEM);
		Lara.harpoon_ammo = start->harpoonAmmo;
		GlobalItemReplace(ID_HARPOON_ITEM, ID_HARPOON_AMMO_ITEM);
	} else {
		for( i = 0; i < start->harpoonAmmo / 3; ++i ) {
			Inv_AddItem(ID_HARPOON_AMMO_ITEM);
		}
		Lara.harpoon_ammo = 0;
	}

	// Flares
	for( i = 0; i < start->flares; ++i ) {
		Inv_AddItem(ID_FLARE_ITEM);
	}

	// Small medipacks
	for( i = 0; i < start->smallMedipacks; ++i ) {
		Inv_AddItem(ID_SMALL_MEDIPACK_ITEM);
	}

	// Large medipacks
	for( i = 0; i < start->largeMedipacks; ++i ) {
		Inv_AddItem(ID_LARGE_MEDIPACK_ITEM);
	}

	Lara.gun_status = LGS_Armless;
	Lara.last_gun_type = start->gunType;
	Lara.gun_type = start->gunType;
	Lara.request_gun_type = start->gunType;
	LaraInitialiseMeshes(levelID);
	InitialiseNewWeapon();
}

/*
 * Inject function
 */
void Inject_LaraMisc() {
//	INJECT(0x00430380, LaraControl);
//	INJECT(0x00430A10, AnimateLara);

	INJECT(0x00430D10, UseItem);
	INJECT(0x00430ED0, LaraCheatGetStuff);

//	INJECT(0x00430F90, ControlLaraExtra);
//	INJECT(0x00430FB0, InitialiseLaraLoad);
//	INJECT(0x00430FE0, InitialiseLara);

	INJECT(0x004312A0, InitialiseLaraInventory);

//	INJECT(0x00431610, LaraInitialiseMeshes);
}
