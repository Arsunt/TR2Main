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
//	INJECT(0x004312A0, InitialiseLaraInventory);
//	INJECT(0x00431610, LaraInitialiseMeshes);
}
