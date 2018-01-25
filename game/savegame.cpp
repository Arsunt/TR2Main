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
#include "game/savegame.h"
#include "global/vars.h"

void __cdecl InitialiseStartInfo() {
	// skip initialise if bonus game started
	if( SaveGame.bonusFlag )
		return;

	for( int i=0; i<24; ++i ) {
		START_INFO *start = &SaveGame.start[i];

		ModifyStartInfo(i);
		start->flags &= ~SIF_Available; // make level not available
		memset(&start->statistics, 0, sizeof(STATISTICS_INFO));
	}

	SaveGame.start[0].flags |= SIF_Available; // Make assault available
	SaveGame.start[1].flags |= SIF_Available; // Make new game available
}

void __cdecl ModifyStartInfo(int levelIdx) {
	START_INFO *start = &SaveGame.start[levelIdx];

	start->flags |= SIF_HasPistols; // Lara has pistols
	start->gunType = LGT_Pistols; // current weapon is pistols
	start->pistolAmmo = 1000; // infinite pistols ammo

	switch( levelIdx ) {
		case 0 : // Assault (Lara's Home)
			start->flags &= ~(SIF_HasHarpoon|SIF_HasGrenade|SIF_HasM16|SIF_HasShotgun|SIF_HasUzis|SIF_HasMagnums|SIF_HasPistols); // remove 'weapon' flags
			start->flags |= SIF_Available; // add 'available flag
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
			start->flags &= ~(SIF_HasHarpoon|SIF_HasGrenade|SIF_HasM16|SIF_HasUzis|SIF_HasMagnums); // remove 'weapon' flags except 'pistols' and 'shotgun'
			start->flags |= (SIF_HasShotgun|SIF_Available); // add 'shotgun' and 'available' flag
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
	}

	// Bonus game activated and level is not Assault
	if( SaveGame.bonusFlag && levelIdx != 0 ) {
		start->flags |= (SIF_HasHarpoon|SIF_HasGrenade|SIF_HasM16|SIF_HasShotgun|SIF_HasUzis|SIF_HasMagnums|SIF_HasPistols|SIF_Available); // add all 'weapon' and 'available' flags
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
