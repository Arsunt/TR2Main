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
#include "game/savegame.h"
#include "global/vars.h"

void __cdecl InitialiseStartInfo() {
#if 0 // NOTE: this original check is removed, because it breaks game+ logic in case of any level selection
	// skip initialise if bonus game started
	if( SaveGame.bonusFlag )
		return;
#endif

	for( int i=0; i<24; ++i ) {
		START_INFO *start = &SaveGame.start[i];

		ModifyStartInfo(i);
		start->available = 0; // make level not available
		memset(&start->statistics, 0, sizeof(STATISTICS_INFO));
	}

	SaveGame.start[0].available = 1; // make assault available
	SaveGame.start[1].available = 1; // make new game available
}

void __cdecl ModifyStartInfo(int levelIdx) {
	START_INFO *start = &SaveGame.start[levelIdx];

	start->has_pistols = 1; // Lara has pistols
	start->gunType = LGT_Pistols; // current weapon is pistols
	start->pistolAmmo = 1000; // infinite pistols ammo

	switch( levelIdx ) {
		case 0 : // Assault (Lara's Home)
			start->available = 1; // make level available
			start->has_pistols = 0; // Lara has no weapons
			start->has_magnums = 0;
			start->has_uzis = 0;
			start->has_shotgun = 0;
			start->has_m16 = 0;
			start->has_grenade = 0;
			start->has_harpoon = 0;
			start->gunType = LGT_Unarmed; // Lara doesn't have weapons
			start->gunStatus = LGS_Armless; // Lara has no weapons in hands

			start->pistolAmmo	= 0;
			start->magnumAmmo	= 0;
			start->uziAmmo		= 0;
			start->shotgunAmmo	= 0;
			start->m16Ammo		= 0;
			start->grenadeAmmo	= 0;
			start->harpoonAmmo	= 0;

			start->flares = 0;
			start->smallMedipacks = 0;
			start->largeMedipacks = 0;
			break;

		case 1 : // Regular New Game
			start->available = 1; // make level available
			start->has_pistols = 1; // Lara has pistols and shotgun
			start->has_magnums = 0;
			start->has_uzis = 0;
			start->has_shotgun = 1;
			start->has_m16 = 0;
			start->has_grenade = 0;
			start->has_harpoon = 0;
			start->gunStatus = LGS_Armless; // Lara has no weapons in hands

			start->magnumAmmo	= 0;
			start->uziAmmo		= 0;
			start->shotgunAmmo	= 12;
			start->m16Ammo		= 0;
			start->grenadeAmmo	= 0;
			start->harpoonAmmo	= 0;

			start->flares = 2;
			start->smallMedipacks = 1;
			start->largeMedipacks = 1;
			break;

		// NOTE: there was no 'default' in the original game, so new game with level selection was broken
		default : // New Game from any other level
			start->available = 1; // make level available
			start->has_pistols = 1; // Lara have just pistols
			start->has_magnums = 0;
			start->has_uzis = 0;
			start->has_shotgun = 0;
			start->has_m16 = 0;
			start->has_grenade = 0;
			start->has_harpoon = 0;
			start->gunStatus = LGS_Armless; // Lara has no weapons in hands

			start->magnumAmmo	= 0;
			start->uziAmmo		= 0;
			start->shotgunAmmo	= 0;
			start->m16Ammo		= 0;
			start->grenadeAmmo	= 0;
			start->harpoonAmmo	= 0;

			start->flares = 0;
			start->smallMedipacks = 0;
			start->largeMedipacks = 0;
			break;
	}

	// Bonus game activated and level is not Assault
	if( SaveGame.bonusFlag && levelIdx != 0 ) {
		start->available = 1; // make level available
		start->has_pistols = 1; // Lara have all weapons
		start->has_magnums = 1;
		start->has_uzis = 1;
		start->has_shotgun = 1;
		start->has_m16 = 1;
		start->has_grenade = 1;
		start->has_harpoon = 1;
		start->gunType = LGT_Grenade; // current weapon is grenade launcher

		start->uziAmmo		= 10001;
		start->magnumAmmo	= 10001;
		start->shotgunAmmo	= 10001;
		start->m16Ammo		= 10001;
		start->grenadeAmmo	= 10001;
		start->harpoonAmmo	= 10001;

		start->flares = 255;
	}
}

/*
 * Inject function
 */
void Inject_SaveGame() {
	INJECT(0x00439190, InitialiseStartInfo);
	INJECT(0x00439200, ModifyStartInfo);

//	INJECT(0x004392E0, CreateStartInfo);
//	INJECT(0x004394F0, CreateSaveGameInfo);
//	INJECT(0x00439A20, ExtractSaveGameInfo);
//	INJECT(0x0043A280, ResetSG);
//	INJECT(0x0043A2A0, WriteSG);
//	INJECT(0x0043A2F0, ReadSG);
}
