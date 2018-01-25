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

#ifndef GAMEFLOW_H_INCLUDED
#define GAMEFLOW_H_INCLUDED

#include "global/types.h"

typedef enum {
	GSI_Heading_Inventory,
	GSI_Heading_Option,
	GSI_Heading_Items,
	GSI_Heading_GameOver,

	GSI_Passport_LoadGame,
	GSI_Passport_SaveGame,
	GSI_Passport_NewGame,
	GSI_Passport_RestartLevel,
	GSI_Passport_ExitToTitle,
	GSI_Passport_ExitDemo,
	GSI_Passport_ExitGame,
	GSI_Passport_SelectLevel,
	GSI_Passport_SavePosition,

	GSI_Detail_SelectDetail,
	GSI_Detail_High,
	GSI_Detail_Medium,
	GSI_Detail_Low,

	GSI_Keymap_Walk,
	GSI_Keymap_Roll,
	GSI_Keymap_Run,
	GSI_Keymap_Left,
	GSI_Keymap_Right,
	GSI_Keymap_Back,
	GSI_Keymap_StepLeft,
	GSI_Keymap_Reserved1,
	GSI_Keymap_StepRight,
	GSI_Keymap_Reserved2,
	GSI_Keymap_Look,
	GSI_Keymap_Jump,
	GSI_Keymap_Action,
	GSI_Keymap_DrawWeapon,
	GSI_Keymap_Reserved3,
	GSI_Keymap_Inventory,
	GSI_Keymap_Flare,
	GSI_Keymap_Step,

	GSI_InvItem_Statistics,
	GSI_InvItem_Pistols,
	GSI_InvItem_Shotgun,
	GSI_InvItem_Magnums,
	GSI_InvItem_Uzis,
	GSI_InvItem_Harpoon,
	GSI_InvItem_M16,
	GSI_InvItem_Grenade,
	GSI_InvItem_Flare,
	GSI_InvItem_PistolAmmo,
	GSI_InvItem_ShotgunAmmo,
	GSI_InvItem_MagnumAmmo,
	GSI_InvItem_UziAmmo,
	GSI_InvItem_HarpoonAmmo,
	GSI_InvItem_M16Ammo,
	GSI_InvItem_GrenadeAmmo,
	GSI_InvItem_SmallMedipack,
	GSI_InvItem_LargeMedipack,
	GSI_InvItem_Pickup,
	GSI_InvItem_Puzzle,
	GSI_InvItem_Key,
	GSI_InvItem_Game,
	GSI_InvItem_LaraHome,

	GSI_String_Loading,
	GSI_String_TimeTaken,
	GSI_String_SecretsFound,
	GSI_String_Location,
	GSI_String_Kills,
	GSI_String_AmmoUsed,
	GSI_String_Hits,
	GSI_String_SavesPerformed,
	GSI_String_DistanceTravelled,
	GSI_String_HealthPacksUsed,
	GSI_String_ReleaseVersion,
	GSI_String_None,
	GSI_String_Finish,
	GSI_String_BestTimes,
	GSI_String_NoTimesSet,
	GSI_String_NA,
	GSI_String_CurrentPosition,
	GSI_String_FinalStatistics,
	GSI_String_Of,
	GSI_String_StorySoFar,
} GAME_STRING_ID;

typedef enum {
	SSI_DetailLevels,
	SSI_DemoMode,
	SSI_Sound,
	SSI_Controls,
	SSI_Gamma,
	SSI_SetVolumes,
	SSI_UserKeys,
	SSI_SaveFileWarning,
	SSI_TryAgainQuestion,
	SSI_Yes,
	SSI_No,
	SSI_SaveComplete,
	SSI_NoSaveGames,
	SSI_NoneValid,
	SSI_SaveGameQuestion,
	SSI_EmptySlot,
	SSI_Off,
	SSI_On,
	SSI_SetupSoundCard,
	SSI_DefaultKeys,
	SSI_Dozy,
} SPECIFIC_STRING_ID;

/*
 * Function list
 */
BOOL __cdecl GF_LoadScriptFile(LPCTSTR fileName); // 0x0041FA40
BOOL __cdecl GF_DoFrontEndSequence(); // 0x0041FC30
int __cdecl GF_DoLevelSequence(DWORD levelID, int levelType); // 0x0041FC50

#define GF_InterpretSequence ((__int16(__cdecl*)(__int16 *,int)) 0x0041FCC0)
#define GF_ModifyInventory ((void(__cdecl*)(DWORD,BOOL)) 0x004201A0)

#endif // GAMEFLOW_H_INCLUDED
