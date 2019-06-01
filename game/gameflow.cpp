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
#include "game/gameflow.h"
#include "game/invtext.h"
#include "specific/file.h"
#include "global/vars.h"

BOOL __cdecl GF_LoadScriptFile(LPCTSTR fileName) {
	GF_SunsetEnabled = 0;

	if( !S_LoadGameFlow(fileName) )
		return FALSE;

	GF_GameFlow.levelCompleteTrack = 41; // "level complete" track is hardcoded for some reason

	InvCompassOption.lpString		= GF_GameStringTable[GSI_InvItem_Statistics];

	InvPistolOption.lpString		= GF_GameStringTable[GSI_InvItem_Pistols];
	InvShotgunOption.lpString		= GF_GameStringTable[GSI_InvItem_Shotgun];
	InvMagnumOption.lpString		= GF_GameStringTable[GSI_InvItem_Magnums];
	InvUziOption.lpString			= GF_GameStringTable[GSI_InvItem_Uzis];
	InvHarpoonOption.lpString		= GF_GameStringTable[GSI_InvItem_Harpoon];
	InvM16Option.lpString			= GF_GameStringTable[GSI_InvItem_M16];
	InvGrenadeOption.lpString		= GF_GameStringTable[GSI_InvItem_Grenade];

	InvFlareOption.lpString			= GF_GameStringTable[GSI_InvItem_Flare];

	InvPistolAmmoOption.lpString	= GF_GameStringTable[GSI_InvItem_PistolAmmo];
	InvShotgunAmmoOption.lpString	= GF_GameStringTable[GSI_InvItem_ShotgunAmmo];
	InvMagnumAmmoOption.lpString	= GF_GameStringTable[GSI_InvItem_MagnumAmmo];
	InvUziAmmoOption.lpString		= GF_GameStringTable[GSI_InvItem_UziAmmo];
	InvHarpoonAmmoOption.lpString	= GF_GameStringTable[GSI_InvItem_HarpoonAmmo];
	InvM16AmmoOption.lpString		= GF_GameStringTable[GSI_InvItem_M16Ammo];
	InvGrenadeAmmoOption.lpString	= GF_GameStringTable[GSI_InvItem_GrenadeAmmo];

	InvSmallMedipackOption.lpString	= GF_GameStringTable[GSI_InvItem_SmallMedipack];
	InvLargeMedipackOption.lpString	= GF_GameStringTable[GSI_InvItem_LargeMedipack];

	InvPickup1Option.lpString		= GF_GameStringTable[GSI_InvItem_Pickup];
	InvPickup2Option.lpString		= GF_GameStringTable[GSI_InvItem_Pickup];

	InvPuzzle1Option.lpString		= GF_GameStringTable[GSI_InvItem_Puzzle];
	InvPuzzle2Option.lpString		= GF_GameStringTable[GSI_InvItem_Puzzle];
	InvPuzzle3Option.lpString		= GF_GameStringTable[GSI_InvItem_Puzzle];
	InvPuzzle4Option.lpString		= GF_GameStringTable[GSI_InvItem_Puzzle];

	InvKey1Option.lpString			= GF_GameStringTable[GSI_InvItem_Key];
	InvKey2Option.lpString			= GF_GameStringTable[GSI_InvItem_Key];
	InvKey3Option.lpString			= GF_GameStringTable[GSI_InvItem_Key];
	InvKey4Option.lpString			= GF_GameStringTable[GSI_InvItem_Key];

	InvPassportOption.lpString		= GF_GameStringTable[GSI_InvItem_Game];
	InvPhotoOption.lpString			= GF_GameStringTable[GSI_InvItem_LaraHome];

	InvDetailOption.lpString		= GF_SpecificStringTable[SSI_DetailLevels];
	InvSoundOption.lpString			= GF_SpecificStringTable[SSI_Sound];
	InvControlOption.lpString		= GF_SpecificStringTable[SSI_Controls];

	SetRequesterHeading(&LoadGameRequester, GF_GameStringTable[GSI_Passport_SelectLevel], 0, NULL, 0);
	SetRequesterHeading(&SaveGameRequester, GF_GameStringTable[GSI_Passport_SelectLevel], 0, NULL, 0);

	return TRUE;
}

BOOL __cdecl GF_DoFrontEndSequence() {
	return ( GF_EXIT_GAME == GF_InterpretSequence(GF_ScriptBuffer, GFL_NORMAL, 1) );
}

int __cdecl GF_DoLevelSequence(DWORD levelID, GF_LEVEL_TYPE levelType) {
	for( DWORD i = levelID; i < GF_GameFlow.num_Levels; ++i ) {
		int direction = GF_InterpretSequence(GF_ScriptTable[i], levelType, 0);

		if( GF_GameFlow.singleLevel >= 0 ||
			(direction & ~0xFFu) != GF_LEVEL_COMPLETE )
		{
			return direction;
		}
	}
	IsTitleLoaded = FALSE;
	return GF_EXIT_TO_TITLE;
}

/*
 * Inject function
 */
void Inject_Gameflow() {
	INJECT(0x0041FA40, GF_LoadScriptFile);
	INJECT(0x0041FC30, GF_DoFrontEndSequence);
	INJECT(0x0041FC50, GF_DoLevelSequence);

//	INJECT(0x0041FCC0, GF_InterpretSequence);
//	INJECT(0x004201A0, GF_ModifyInventory);
}
