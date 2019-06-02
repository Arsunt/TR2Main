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
#include "game/invtext.h"
#include "specific/file.h"
#include "specific/frontend.h"
#include "specific/game.h"
#include "global/vars.h"

// NOTE: there is no such function in the original code
int __cdecl GF_GetNumSecrets(DWORD levelID) {
	if( levelID >= GF_GameFlow.num_Levels ) {
		return 0;
	}
	__int16 *seq = GF_ScriptTable[levelID];
	int result = 3;

	while( *seq != GFE_END_SEQ ) {
		switch( *seq ) {
			case GFE_NUMSECRETS :
				result = seq[1];
				seq += 2;
				break;
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
			case GFE_ADD2INV :
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
				++seq;
				break;
			default :
				return result;
		}
	}
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

/*
 * Inject function
 */
void Inject_Gameflow() {
	INJECT(0x0041FA40, GF_LoadScriptFile);
	INJECT(0x0041FC30, GF_DoFrontEndSequence);
	INJECT(0x0041FC50, GF_DoLevelSequence);
	INJECT(0x0041FCC0, GF_InterpretSequence);

//	INJECT(0x004201A0, GF_ModifyInventory);
}
