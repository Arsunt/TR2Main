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
#include "game/cinema.h"
#include "game/demo.h"
#include "game/gameflow.h"
#include "game/health.h"
#include "game/invfunc.h"
#include "game/invtext.h"
#include "specific/file.h"
#include "specific/frontend.h"
#include "specific/game.h"
#include "global/vars.h"

// NOTE: there is no such function in the original code
static bool __cdecl GF_GetSequenceValue(DWORD levelID, GF_EVENTS event, __int16 *pValue, __int16 defValue) {
	if( levelID >= GF_GameFlow.num_Levels ) {
		return false;
	}
	__int16 *seq = GF_ScriptTable[levelID];
	__int16 operand = 0;
	bool result = false;

	if( pValue != NULL) {
		*pValue = defValue; // set default value just in case
	}

	while( *seq != GFE_END_SEQ ) {
		__int16 seqCode = *seq;
		switch( seqCode ) {
			case GFE_STARTLEVEL :
			case GFE_LOADINGPIC :
			case GFE_DEMOPLAY :
			case GFE_CUTANGLE :
			case GFE_CUTSCENE :
			case GFE_PLAYFMV :
			case GFE_PICTURE :
			case GFE_JUMPTO_SEQ :
			case GFE_SETTRACK :
			case GFE_NOFLOOR :
			case GFE_STARTANIM :
			case GFE_NUMSECRETS :
			case GFE_ADD2INV :
				operand = seq[1];
				seq += 2;
				break;
			case GFE_LEVCOMPLETE :
			case GFE_GAMECOMPLETE :
			case GFE_SUNSET :
			case GFE_DEADLY_WATER :
			case GFE_REMOVE_WEAPONS :
			case GFE_REMOVE_AMMO :
			case GFE_KILL2COMPLETE :
			case GFE_LIST_START :
			case GFE_LIST_END :
				operand = 0;
				++seq;
				break;
			default :
				return result;
		}

		if( seqCode == event ) {
			// the event is found in the sequence
			if( pValue == NULL) {
				// if we don't need operand value, just return here
				return true;
			} else {
				// if we need the value, search until the end, the last value is correct one
				result = true;
				*pValue = operand;
			}
		}
	}
	return result;
}

// NOTE: there is no such function in the original code
int __cdecl GF_GetNumSecrets(DWORD levelID) {
	__int16 result = 3;
	GF_GetSequenceValue(levelID, GFE_NUMSECRETS, &result, result);
	return result;
}

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

int __cdecl GF_InterpretSequence(__int16 *seq, GF_LEVEL_TYPE levelType, int seqType) {
	int result = GF_EXIT_TO_TITLE;
	int trackIndex = 0;
	char str[80];

	GF_NoFloor = 0;
	GF_DeadlyWater = 0;
	GF_SunsetEnabled = 0;
	GF_LaraStartAnim = 0;
	GF_Kill2Complete = 0;
	GF_RemoveAmmo = 0;
	GF_RemoveWeapons = 0;

	memset(GF_Add2InvItems, 0, sizeof(GF_Add2InvItems));
	memset(GF_SecretInvItems, 0, sizeof(GF_SecretInvItems));

	TrackIDs[0] = 2;
	CineTargetAngle = 0x4000;
	GF_NumSecrets = 3;

	while( *seq != GFE_END_SEQ ) {
		switch( *seq ) {
			case GFE_STARTLEVEL :
				if( seq[1] > GF_GameFlow.num_Levels ) {
					sprintf(str, "INVALID LEVEL %d", seq[1]);
					result = GF_EXIT_TO_TITLE;
				} else if( levelType != GFL_STORY ) {
					if( levelType == GFL_MIDSTORY ) {
						return GF_EXIT_TO_TITLE;
					}
					result = StartGame(seq[1], levelType);
					GF_StartGame = false;
					if( levelType == GFL_SAVED ) {
						levelType = GFL_NORMAL;
					}
					if( (result & ~0xff) != GF_LEVEL_COMPLETE ) {
						return result;
					}
				}
				seq += 2;
				break;

			case GFE_LOADINGPIC :
				seq += 2;
				break;

			case GFE_DEMOPLAY :
				if( levelType != GFL_SAVED && levelType != GFL_STORY && levelType != GFL_MIDSTORY ) {
					return StartDemo(seq[1]);
				}
				seq += 2;
				break;

			case GFE_CUTANGLE :
				if( levelType != GFL_SAVED ) {
					CineTargetAngle = seq[1];
				}
				seq += 2;
				break;

			case GFE_CUTSCENE :
				if( levelType != GFL_SAVED ) {
					sprintf(str, "CUTSCENE %d %s", seq[1], GF_CutsFilesStringTable[seq[1]]);
					__int16 storedLevel = CurrentLevel;
					int cine_ret = StartCinematic(seq[1]);
					CurrentLevel = storedLevel;
					if( cine_ret == 2 && (levelType == GFL_STORY || levelType == GFL_MIDSTORY) ) {
						return GF_EXIT_TO_TITLE;
					}
					if( cine_ret == 3 ) {
						return GF_EXIT_GAME;
					}
				}
				seq += 2;
				break;

			case GFE_PLAYFMV :
				if( levelType != GFL_SAVED ) {
					if( seq[2] == GFE_PLAYFMV ) {
						if( S_IntroFMV(GF_FmvFilesStringTable[seq[1]], GF_FmvFilesStringTable[seq[3]]) ) {
							return GF_EXIT_GAME;
						}
						seq += 2;
					} else {
						if( S_PlayFMV(GF_FmvFilesStringTable[seq[1]]) ) {
							return GF_EXIT_GAME;
						}
					}
				}
				seq += 2;
				break;

			case GFE_LEVCOMPLETE :
				if( levelType != GFL_STORY && levelType != GFL_MIDSTORY ) {
					if( LevelStats(CurrentLevel) ) {
						return GF_EXIT_TO_TITLE;
					}
					result = GF_START_GAME | (CurrentLevel + 1);
				}
				++seq;
				break;

			case GFE_GAMECOMPLETE :
				DisplayCredits();
				if( GameStats(CurrentLevel) ) {
					return GF_EXIT_TO_TITLE;
				}
				result = GF_EXIT_TO_TITLE;
				++seq;
				break;

			case GFE_PICTURE :
				if( levelType != GFL_SAVED ) {
					sprintf(str, "PICTURE %s", GF_PictureFilesStringTable[seq[1]]);
				}
				seq += 2;
				break;

			case GFE_JUMPTO_SEQ :
				sprintf(str, "JUMPSEQ %d", seq[1]);
				seq += 2;
				break;

			case GFE_SETTRACK :
				TrackIDs[trackIndex++] = seq[1];
				SetCutsceneTrack(seq[1]);
				seq += 2;
				break;

			case GFE_SUNSET :
				if( levelType != GFL_STORY && levelType != GFL_MIDSTORY ) {
					GF_SunsetEnabled = 1;
				}
				++seq;
				break;

			case GFE_DEADLY_WATER :
				if( levelType != GFL_STORY && levelType != GFL_MIDSTORY ) {
					GF_DeadlyWater = 1;
				}
				++seq;
				break;

			case GFE_NOFLOOR :
				if( levelType != GFL_STORY && levelType != GFL_MIDSTORY ) {
					GF_NoFloor = seq[1];
				}
				seq += 2;
				break;

			case GFE_STARTANIM :
				if( levelType != GFL_STORY && levelType != GFL_MIDSTORY ) {
					GF_LaraStartAnim = seq[1];
				}
				seq += 2;
				break;

			case GFE_NUMSECRETS :
				if( levelType != GFL_STORY && levelType != GFL_MIDSTORY ) {
					GF_NumSecrets = seq[1];
				}
				seq += 2;
				break;

			case GFE_ADD2INV :
				if( levelType != GFL_STORY && levelType != GFL_MIDSTORY ) {
					if( seq[1] < 1000 ) {
						++GF_SecretInvItems[seq[1]];
					} else if( levelType != GFL_SAVED ) {
						++GF_Add2InvItems[seq[1] - 1000];
					}
				}
				seq += 2;
				break;

			case GFE_REMOVE_WEAPONS :
				if( levelType != GFL_STORY && levelType != GFL_MIDSTORY && levelType != GFL_SAVED ) {
					GF_RemoveWeapons = 1;
				}
				++seq;
				break;

			case GFE_REMOVE_AMMO :
				if( levelType != GFL_STORY && levelType != GFL_MIDSTORY && levelType != GFL_SAVED ) {
					GF_RemoveAmmo = 1;
				}
				++seq;
				break;

			case GFE_KILL2COMPLETE :
				if( levelType != GFL_STORY && levelType != GFL_MIDSTORY ) {
					GF_Kill2Complete = 1;
				}
				++seq;
				break;

			case GFE_LIST_START :
			case GFE_LIST_END :
				++seq;
				break;

			default :
				return GF_EXIT_GAME;
		}
	}

	if( levelType == GFL_STORY || levelType == GFL_MIDSTORY ) {
		result = GF_START_GAME;
	}
	return result;
}

void __cdecl GF_ModifyInventory(int levelID, BOOL isSecret) {
	int i;
	START_INFO *start = &SaveGame.start[levelID];

	// Pistols
	if( !start->has_pistols && GF_Add2InvItems[ADDINV_PISTOL] ) {
		start->has_pistols = 1;
		Inv_AddItem(ID_PISTOL_ITEM);
	}

	// Shotgun
	if( Inv_RequestItem(ID_SHOTGUN_ITEM) ) {
		if( isSecret ) {
			Lara.shotgun_ammo += 12 * GF_SecretInvItems[ADDINV_SHOTGUN_AMMO];
			for( i = 0; i < GF_SecretInvItems[ADDINV_SHOTGUN_AMMO]; ++i ) {
				AddDisplayPickup(ID_SHOTGUN_AMMO_ITEM);
			}
		} else {
			Lara.shotgun_ammo += 12 * GF_Add2InvItems[ADDINV_SHOTGUN_AMMO];
		}
	} else if( (!isSecret && GF_Add2InvItems[ADDINV_SHOTGUN]) || (isSecret && GF_SecretInvItems[ADDINV_SHOTGUN]) ) {
		start->has_shotgun = 1;
		Inv_AddItem(ID_SHOTGUN_ITEM);
		if( isSecret ) {
			AddDisplayPickup(ID_SHOTGUN_ITEM);
			Lara.shotgun_ammo += 12 * GF_SecretInvItems[ADDINV_SHOTGUN_AMMO];
			/* TODO: it looks like the original game bug. There is missing additional loop
			for( i = 0; i < GF_SecretInvItems[ADDINV_SHOTGUN_AMMO]; ++i ) {
				AddDisplayPickup(ID_SHOTGUN_AMMO_ITEM);
			} */
		} else {
			Lara.shotgun_ammo += 12 * GF_Add2InvItems[ADDINV_SHOTGUN_AMMO];
		}
	} else {
		if( isSecret ) {
			for( i = 0; i < GF_SecretInvItems[ADDINV_SHOTGUN_AMMO]; ++i ) {
				Inv_AddItem(ID_SHOTGUN_AMMO_ITEM);
				AddDisplayPickup(ID_SHOTGUN_AMMO_ITEM);
			}
		} else {
			for( i = 0; i < GF_Add2InvItems[ADDINV_SHOTGUN_AMMO]; ++i ) {
				Inv_AddItem(ID_SHOTGUN_AMMO_ITEM);
			}
		}
	}

	// Magnums
	if( Inv_RequestItem(ID_MAGNUM_ITEM) ) {
		if( isSecret ) {
			Lara.magnum_ammo += 40 * GF_SecretInvItems[ADDINV_MAGNUM_AMMO];
			for( i = 0; i < GF_SecretInvItems[ADDINV_MAGNUM_AMMO]; ++i ) {
				AddDisplayPickup(ID_MAGNUM_AMMO_ITEM);
			}
		} else {
			Lara.magnum_ammo += 40 * GF_Add2InvItems[ADDINV_MAGNUM_AMMO];
		}
	} else if( (!isSecret && GF_Add2InvItems[ADDINV_MAGNUM]) || (isSecret && GF_SecretInvItems[ADDINV_MAGNUM]) ) {
		start->has_magnums = 1;
		Inv_AddItem(ID_MAGNUM_ITEM);
		if( isSecret ) {
			AddDisplayPickup(ID_MAGNUM_ITEM);
			Lara.magnum_ammo += 40 * GF_Add2InvItems[ADDINV_MAGNUM_AMMO]; // TODO: it looks like the original game bug. Here should be GF_SecretInvItems
			for( i = 0; i < GF_SecretInvItems[ADDINV_MAGNUM]; ++i ) {
				AddDisplayPickup(ID_MAGNUM_AMMO_ITEM);
			}
		} else {
			Lara.magnum_ammo += 40 * GF_Add2InvItems[ADDINV_MAGNUM_AMMO];
		}
	} else {
		if( isSecret ) {
			for( i = 0; i < GF_SecretInvItems[ADDINV_MAGNUM_AMMO]; ++i ) {
				Inv_AddItem(ID_MAGNUM_AMMO_ITEM);
				AddDisplayPickup(ID_MAGNUM_AMMO_ITEM);
			}
		} else {
			for( i = 0; i < GF_Add2InvItems[ADDINV_MAGNUM_AMMO]; ++i ) {
				Inv_AddItem(ID_MAGNUM_AMMO_ITEM);
			}
		}
	}

	// Uzis
	if( Inv_RequestItem(ID_UZI_ITEM) ) {
		if( isSecret ) {
			Lara.uzi_ammo += 80 * GF_SecretInvItems[ADDINV_UZI_AMMO];
			for( i = 0; i < GF_SecretInvItems[ADDINV_UZI_AMMO]; ++i ) {
				AddDisplayPickup(ID_UZI_AMMO_ITEM);
			}
		} else {
			Lara.uzi_ammo += 80 * GF_Add2InvItems[ADDINV_UZI_AMMO];
		}
	} else if( (!isSecret && GF_Add2InvItems[ADDINV_UZI]) || (isSecret && GF_SecretInvItems[ADDINV_UZI]) ) {
		start->has_uzis = 1;
		Inv_AddItem(ID_UZI_ITEM);
		if( isSecret ) {
			AddDisplayPickup(ID_UZI_ITEM);
			Lara.uzi_ammo += 80 * GF_SecretInvItems[ADDINV_UZI_AMMO];
			for( i = 0; i < GF_SecretInvItems[ADDINV_UZI]; ++i ) {
				AddDisplayPickup(ID_UZI_AMMO_ITEM);
			}
		} else {
			Lara.uzi_ammo += 80 * GF_Add2InvItems[ADDINV_UZI_AMMO];
		}
	} else {
		if( isSecret ) {
			for( i = 0; i < GF_SecretInvItems[ADDINV_UZI_AMMO]; ++i ) {
				Inv_AddItem(ID_UZI_AMMO_ITEM);
				AddDisplayPickup(ID_UZI_AMMO_ITEM);
			}
		} else {
			for( i = 0; i < GF_Add2InvItems[ADDINV_UZI_AMMO]; ++i ) {
				Inv_AddItem(ID_UZI_AMMO_ITEM);
			}
		}
	}

	// Harpoon
	if( Inv_RequestItem(ID_HARPOON_ITEM) ) {
		if( isSecret ) {
			Lara.harpoon_ammo += 3 * GF_SecretInvItems[ADDINV_HARPOON_AMMO];
			for( i = 0; i < GF_SecretInvItems[ADDINV_HARPOON_AMMO]; ++i ) {
				AddDisplayPickup(ID_HARPOON_AMMO_ITEM);
			}
		} else {
			Lara.harpoon_ammo += 3 * GF_Add2InvItems[ADDINV_HARPOON_AMMO];
		}
	} else if( (!isSecret && GF_Add2InvItems[ADDINV_HARPOON]) || (isSecret && GF_SecretInvItems[ADDINV_HARPOON]) ) {
		start->has_harpoon = 1;
		Inv_AddItem(ID_HARPOON_ITEM);
		if( isSecret ) {
			AddDisplayPickup(ID_HARPOON_ITEM);
			Lara.harpoon_ammo += 3 * GF_SecretInvItems[ADDINV_HARPOON_AMMO];
			for( i = 0; i < GF_SecretInvItems[ADDINV_HARPOON]; ++i ) {
				AddDisplayPickup(ID_HARPOON_AMMO_ITEM);
			}
		} else {
			Lara.harpoon_ammo += 3 * GF_Add2InvItems[ADDINV_HARPOON_AMMO];
		}
	} else {
		if( isSecret ) {
			for( i = 0; i < GF_SecretInvItems[ADDINV_HARPOON_AMMO]; ++i ) {
				Inv_AddItem(ID_HARPOON_AMMO_ITEM);
				AddDisplayPickup(ID_HARPOON_AMMO_ITEM);
			}
		} else {
			for( i = 0; i < GF_Add2InvItems[ADDINV_HARPOON_AMMO]; ++i ) {
				Inv_AddItem(ID_HARPOON_AMMO_ITEM);
			}
		}
	}

	// M16
	if( Inv_RequestItem(ID_M16_ITEM) ) {
		if( isSecret ) {
			Lara.m16_ammo += 40 * GF_SecretInvItems[ADDINV_M16_AMMO];
			for( i = 0; i < GF_SecretInvItems[ADDINV_M16_AMMO]; ++i ) {
				AddDisplayPickup(ID_M16_AMMO_ITEM);
			}
		} else {
			Lara.m16_ammo += 40 * GF_Add2InvItems[ADDINV_M16_AMMO];
		}
	} else if( (!isSecret && GF_Add2InvItems[ADDINV_M16]) || (isSecret && GF_SecretInvItems[ADDINV_M16]) ) {
		start->has_m16 = 1;
		Inv_AddItem(ID_M16_ITEM);
		if( isSecret ) {
			AddDisplayPickup(ID_M16_ITEM);
			Lara.m16_ammo += 40 * GF_SecretInvItems[ADDINV_M16_AMMO];
			for( i = 0; i < GF_SecretInvItems[ADDINV_M16]; ++i ) {
				AddDisplayPickup(ID_M16_AMMO_ITEM);
			}
		} else {
			Lara.m16_ammo += 40 * GF_Add2InvItems[ADDINV_M16_AMMO];
		}
	} else {
		if( isSecret ) {
			for( i = 0; i < GF_SecretInvItems[ADDINV_M16_AMMO]; ++i ) {
				Inv_AddItem(ID_M16_AMMO_ITEM);
				AddDisplayPickup(ID_M16_AMMO_ITEM);
			}
		} else {
			for( i = 0; i < GF_Add2InvItems[ADDINV_M16_AMMO]; ++i ) {
				Inv_AddItem(ID_M16_AMMO_ITEM);
			}
		}
	}

	// Grenade
	if( Inv_RequestItem(ID_GRENADE_ITEM) ) {
		if( isSecret ) {
			Lara.grenade_ammo += 2 * GF_SecretInvItems[ADDINV_GRENADE_AMMO];
			for( i = 0; i < GF_SecretInvItems[ADDINV_GRENADE_AMMO]; ++i ) {
				AddDisplayPickup(ID_GRENADE_AMMO_ITEM);
			}
		} else {
			Lara.grenade_ammo += 2 * GF_Add2InvItems[ADDINV_GRENADE_AMMO];
		}
	} else if( (!isSecret && GF_Add2InvItems[ADDINV_GRENADE]) || (isSecret && GF_SecretInvItems[ADDINV_GRENADE]) ) {
		start->has_grenade = 1;
		Inv_AddItem(ID_GRENADE_ITEM);
		if( isSecret ) {
			AddDisplayPickup(ID_GRENADE_ITEM);
			Lara.grenade_ammo += 2 * GF_SecretInvItems[ADDINV_GRENADE_AMMO];
			for( i = 0; i < GF_SecretInvItems[ADDINV_GRENADE]; ++i ) {
				AddDisplayPickup(ID_GRENADE_AMMO_ITEM);
			}
		} else {
			Lara.grenade_ammo += 2 * GF_Add2InvItems[ADDINV_GRENADE_AMMO];
		}
	} else {
		if( isSecret ) {
			for( i = 0; i < GF_SecretInvItems[ADDINV_GRENADE_AMMO]; ++i ) {
				Inv_AddItem(ID_GRENADE_AMMO_ITEM);
				AddDisplayPickup(ID_GRENADE_AMMO_ITEM);
			}
		} else {
			for( i = 0; i < GF_Add2InvItems[ADDINV_GRENADE_AMMO]; ++i ) {
				Inv_AddItem(ID_GRENADE_AMMO_ITEM);
			}
		}
	}

	if( isSecret ) {
		// Flares (secret bonus)
		for( i = 0; i < GF_SecretInvItems[ADDINV_FLARE]; ++i ) {
			Inv_AddItem(ID_FLARE_ITEM);
			AddDisplayPickup(ID_FLARE_ITEM);
		}
		// Medipacks (secret bonus)
		for( i = 0; i < GF_SecretInvItems[ADDINV_SMALL_MEDIPACK]; ++i ) {
			Inv_AddItem(ID_SMALL_MEDIPACK_ITEM);
			AddDisplayPickup(ID_SMALL_MEDIPACK_ITEM);
		}
		for( i = 0; i < GF_SecretInvItems[ADDINV_LARGE_MEDIPACK]; ++i ) {
			Inv_AddItem(ID_LARGE_MEDIPACK_ITEM);
			AddDisplayPickup(ID_LARGE_MEDIPACK_ITEM);
		}
		// Pickups (secret bonus)
		for( i = 0; i < GF_SecretInvItems[ADDINV_PICKUP1]; ++i ) {
			Inv_AddItem(ID_PICKUP_ITEM1);
			AddDisplayPickup(ID_PICKUP_ITEM1);
		}
		for( i = 0; i < GF_SecretInvItems[ADDINV_PICKUP2]; ++i ) {
			Inv_AddItem(ID_PICKUP_ITEM2);
			AddDisplayPickup(ID_PICKUP_ITEM2);
		}
		// Puzzles (secret bonus)
		for( i = 0; i < GF_SecretInvItems[ADDINV_PUZZLE1]; ++i ) {
			Inv_AddItem(ID_PUZZLE_ITEM1);
			AddDisplayPickup(ID_PUZZLE_ITEM1);
		}
		for( i = 0; i < GF_SecretInvItems[ADDINV_PUZZLE2]; ++i ) {
			Inv_AddItem(ID_PUZZLE_ITEM2);
			AddDisplayPickup(ID_PUZZLE_ITEM2);
		}
		for( i = 0; i < GF_SecretInvItems[ADDINV_PUZZLE3]; ++i ) {
			Inv_AddItem(ID_PUZZLE_ITEM3);
			AddDisplayPickup(ID_PUZZLE_ITEM3);
		}
		for( i = 0; i < GF_SecretInvItems[ADDINV_PUZZLE4]; ++i ) {
			Inv_AddItem(ID_PUZZLE_ITEM4);
			AddDisplayPickup(ID_PUZZLE_ITEM4);
		}
		// Keys (secret bonus)
		for( i = 0; i < GF_SecretInvItems[ADDINV_KEY1]; ++i ) {
			Inv_AddItem(ID_KEY_ITEM1);
			AddDisplayPickup(ID_KEY_ITEM1);
		}
		for( i = 0; i < GF_SecretInvItems[ADDINV_KEY2]; ++i ) {
			Inv_AddItem(ID_KEY_ITEM2);
			AddDisplayPickup(ID_KEY_ITEM2);
		}
		for( i = 0; i < GF_SecretInvItems[ADDINV_KEY3]; ++i ) {
			Inv_AddItem(ID_KEY_ITEM3);
			AddDisplayPickup(ID_KEY_ITEM3);
		}
		for( i = 0; i < GF_SecretInvItems[ADDINV_KEY4]; ++i ) {
			Inv_AddItem(ID_KEY_ITEM4);
			AddDisplayPickup(ID_KEY_ITEM4);
		}
		// Clear the array (secret bonus)
		memset(GF_SecretInvItems, 0, sizeof(GF_SecretInvItems));
	} else {
		// Flares (level start)
		for( i = 0; i < GF_Add2InvItems[ADDINV_FLARE]; ++i ) {
			Inv_AddItem(ID_FLARE_ITEM);
		}
		// Medipacks (level start)
		for( i = 0; i < GF_Add2InvItems[ADDINV_SMALL_MEDIPACK]; ++i ) {
			Inv_AddItem(ID_SMALL_MEDIPACK_ITEM);
		}
		for( i = 0; i < GF_Add2InvItems[ADDINV_LARGE_MEDIPACK]; ++i ) {
			Inv_AddItem(ID_LARGE_MEDIPACK_ITEM);
		}
		// Pickups (level start)
		for( i = 0; i < GF_Add2InvItems[ADDINV_PICKUP1]; ++i ) {
			Inv_AddItem(ID_PICKUP_ITEM1);
		}
		for( i = 0; i < GF_Add2InvItems[ADDINV_PICKUP2]; ++i ) {
			Inv_AddItem(ID_PICKUP_ITEM2);
		}
		// Puzzles (level start)
		for( i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE1]; ++i ) {
			Inv_AddItem(ID_PUZZLE_ITEM1);
		}
		for( i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE2]; ++i ) {
			Inv_AddItem(ID_PUZZLE_ITEM2);
		}
		for( i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE3]; ++i ) {
			Inv_AddItem(ID_PUZZLE_ITEM3);
		}
		for( i = 0; i < GF_Add2InvItems[ADDINV_PUZZLE4]; ++i ) {
			Inv_AddItem(ID_PUZZLE_ITEM4);
		}
		// Keys (level start)
		for( i = 0; i < GF_Add2InvItems[ADDINV_KEY1]; ++i ) {
			Inv_AddItem(ID_KEY_ITEM1);
		}
		for( i = 0; i < GF_Add2InvItems[ADDINV_KEY2]; ++i ) {
			Inv_AddItem(ID_KEY_ITEM2);
		}
		for( i = 0; i < GF_Add2InvItems[ADDINV_KEY3]; ++i ) {
			Inv_AddItem(ID_KEY_ITEM3);
		}
		for( i = 0; i < GF_Add2InvItems[ADDINV_KEY4]; ++i ) {
			Inv_AddItem(ID_KEY_ITEM4);
		}
		// Clear the array (level start)
		memset(GF_Add2InvItems, 0, sizeof(GF_Add2InvItems));
	}
}

/*
 * Inject function
 */
void Inject_Gameflow() {
	INJECT(0x0041FA40, GF_LoadScriptFile);
	INJECT(0x0041FC30, GF_DoFrontEndSequence);
	INJECT(0x0041FC50, GF_DoLevelSequence);
	INJECT(0x0041FCC0, GF_InterpretSequence);
	INJECT(0x004201A0, GF_ModifyInventory);
}
