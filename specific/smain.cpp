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
#include "specific/smain.h"
#include "game/cinema.h"
#include "game/demo.h"
#include "game/gameflow.h"
#include "game/inventory.h"
#include "game/invfunc.h"
#include "game/missile.h"
#include "game/savegame.h"
#include "game/setup.h"
#include "game/sound.h"
#include "game/text.h"
#include "specific/display.h"
#include "specific/frontend.h"
#include "specific/game.h"
#include "specific/hwr.h"
#include "specific/init.h"
#include "specific/input.h"
#include "specific/option.h"
#include "specific/output.h"
#include "specific/registry.h"
#include "specific/setupdlg.h"
#include "specific/sndpc.h"
#include "specific/winmain.h"
#include "modding/background_new.h"
#include "global/vars.h"

#ifdef FEATURE_HEALTHBAR_IMPROVED
extern DWORD HealthBarMode;
extern bool PsxBarPosEnabled;
#endif // FEATURE_HEALTHBAR_IMPROVED

#ifdef FEATURE_FOV_FIX
extern bool PsxFovEnabled;
extern double GameGUI_Scale;
extern double InvGUI_Scale;
#endif // FEATURE_FOV_FIX

#ifdef FEATURE_BACKGROUND_IMPROVED
extern DWORD InvBackgroundMode;
extern DWORD PictureStretchLimit;
#endif // FEATURE_BACKGROUND_IMPROVED

#ifdef FEATURE_SHADOW_IMPROVED
extern DWORD ShadowMode;
#endif // FEATURE_SHADOW_IMPROVED

#ifdef FEATURE_SCREENSHOT_IMPROVED
extern DWORD ScreenshotFormat;
extern char ScreenshotPath[MAX_PATH];
#endif // FEATURE_SCREENSHOT_IMPROVED

#ifdef FEATURE_FOG_DISTANCE
extern double ViewDistanceFactor;
extern double FogBeginFactor;
extern double FogEndFactor;
extern double WaterFogBeginFactor;
extern double WaterFogEndFactor;
#endif // FEATURE_FOG_DISTANCE

#ifdef FEATURE_ASSAULT_SAVE
void SaveAssault() {
	OpenGameRegistryKey(REG_GAME_KEY);
	SetRegistryBinaryValue(REG_GAME_ASSAULT, (LPBYTE)&Assault, sizeof(ASSAULT_STATS));
	CloseGameRegistryKey();
}
#endif // FEATURE_ASSAULT_SAVE

BOOL __cdecl GameMain() {
	__int16 gfOption, gfDirection, gfParameter;
	bool isFrontendFail, isLoopContinue;

	HiRes = 0;
	ScreenSizer = 1.0;
	GameSizer = 1.0;

	if( !S_InitialiseSystem() )
		return FALSE;

	if ( !GF_LoadScriptFile("data\\tombPC.dat") ) {
		S_ExitSystem("GameMain: could not load script file");
		return FALSE; // the app is terminated here
	}

	SOUND_Init();
	InitialiseStartInfo();
	S_FrontEndCheck();
	S_LoadSettings();
	HiRes = -1;

	// NOTE: this HWR init was absent in the original code, but must be done here
	if( SavedAppSettings.RenderMode == RM_Hardware ) {
		HWR_InitState();
	}

	GameMemoryPointer = (BYTE *)GlobalAlloc(GMEM_FIXED, 0x380000); // 3.5 MB
	if( GameMemoryPointer == NULL ) {
		lstrcpy(StringToShow, "GameMain: could not allocate malloc_buffer");
		return FALSE;
	}

	HiRes = 0;
	TempVideoAdjust(1, 1.0);
	S_UpdateInput();
	IsVidModeLock = true;
#ifdef FEATURE_BACKGROUND_IMPROVED
	if( !BGND2_LoadPicture("data\\legal.pcx", FALSE, FALSE) ) {
		BGND2_ShowPicture(30, 90, 10, 2, TRUE);
	}
#else // FEATURE_BACKGROUND_IMPROVED
	S_DisplayPicture("data\\legal.pcx", FALSE);
	S_InitialisePolyList(FALSE);
	S_CopyBufferToScreen();
	S_OutputPolyList();
	S_DumpScreen();
	FadeToPal(30, GamePalette8); // fade in 30 frames / 1.0 second (software renderer only)
	S_Wait(90 * TICKS_PER_FRAME, TRUE); // wait 90 frames / 3.0 seconds (enable keyboard)
	S_FadeToBlack(); // fade out 12 frames / 0.4 second (software renderer only)
#endif // FEATURE_BACKGROUND_IMPROVED
	S_DontDisplayPicture();
	IsVidModeLock = false;

	isFrontendFail = GF_DoFrontEndSequence();
	if( IsGameToExit ) {
		return TRUE;
	}
	if( isFrontendFail ) {
		lstrcpy(StringToShow, "GameMain: failed in GF_DoFrontEndSequence()");
		return FALSE;
	}

	S_FadeToBlack();
	IsTitleLoaded = FALSE;

	gfOption = GF_GameFlow.firstOption;
	isLoopContinue = true;

	while( isLoopContinue ) {
		gfDirection = gfOption & 0xFF00;
		gfParameter = gfOption & 0x00FF;
		switch( gfDirection ) {

			case GF_START_GAME :
				if( GF_GameFlow.singleLevel >= 0 ) {
					gfOption = GF_DoLevelSequence(GF_GameFlow.singleLevel, 1);
				} else {
					if( gfParameter > GF_GameFlow.num_Levels ) {
						wsprintf(StringToShow, "GameMain: STARTGAME with invalid level number (%d)", gfParameter);
						return FALSE;
					}
					gfOption = GF_DoLevelSequence(gfParameter, 1);
				}
				break;

			case GF_START_SAVEDGAME :
				S_LoadGame(&SaveGame, sizeof(SAVEGAME_INFO), gfParameter);
				if( SaveGame.currentLevel > GF_GameFlow.num_Levels ) {
					wsprintf(StringToShow, "GameMain: STARTSAVEDGAME with invalid level number (%d)", SaveGame.currentLevel);
					return FALSE;
				}
				gfOption = GF_DoLevelSequence(SaveGame.currentLevel, 2);
				break;

			case GF_START_CINE :
				StartCinematic(gfParameter);
				gfOption = GF_EXIT_TO_TITLE;
				break;

			case GF_START_DEMO :
				gfOption = DoDemoSequence(-1);
				break;

			case GF_LEVEL_COMPLETE :
				gfOption = LevelCompleteSequence();
				break;

			case GF_EXIT_TO_TITLE :
			case GF_EXIT_TO_OPTION :
				if( (GF_GameFlow.flags & GFF_TitleDisabled) != 0 ) {
					gfOption = GF_GameFlow.titleReplace;
					if( gfOption == GF_EXIT_TO_TITLE || gfOption < 0 ) {
						lstrcpy(StringToShow, "GameMain Failed: Title disabled & no replacement");
						return FALSE;
					}
				} else {
					gfOption = TitleSequence();
					IsTitleFinished = true;
				}
				break;

			case GF_START_FMV :
			case GF_EXIT_GAME :
			default :
				isLoopContinue = false;
				break;
		}
	}
	S_SaveSettings();
	ShutdownGame();
	return TRUE;
}

__int16 __cdecl TitleSequence() {
	T_InitPrint();
	TempVideoAdjust(1, 1.0);
	NoInputCounter = 0;

	if( !IsTitleLoaded ) {
		if( !InitialiseLevel(0, 0) )
			return GF_EXIT_GAME;
		IsTitleLoaded = TRUE;
	}

	S_DisplayPicture("data\\title.pcx", TRUE);

#ifdef FEATURE_BACKGROUND_IMPROVED
	// NOTE: title menu fade-in was absent in the original game
	BGND2_ShowPicture(15, 0, 0, 0, FALSE);
#endif // FEATURE_BACKGROUND_IMPROVED

	if( GF_GameFlow.titleTrack != 0 )
		S_CDPlay(GF_GameFlow.titleTrack, TRUE);

	Display_Inventory(INV_TitleMode);
#ifdef FEATURE_BACKGROUND_IMPROVED
	BGND2_ShowPicture(0, 0, 10, 2, FALSE);
#else // FEATURE_BACKGROUND_IMPROVED
	S_FadeToBlack();
#endif // FEATURE_BACKGROUND_IMPROVED
	S_DontDisplayPicture();
	S_CDStop();

	if( IsResetFlag ) {
		IsResetFlag = FALSE;
		return GF_START_DEMO;
	}

	if( InventoryChosen == ID_PHOTO_OPTION )
		return GF_START_GAME | 0;

	if( InventoryChosen == ID_PASSPORT_OPTION ) {
		if ( InventoryExtraData[0] == 0 ) {
			__int16 slotNumber = InventoryExtraData[1];

			Inv_RemoveAllItems();
			S_LoadGame(&SaveGame, sizeof(SAVEGAME_INFO), slotNumber);

			return GF_START_SAVEDGAME | slotNumber;
		}

		if( InventoryExtraData[0] == 1 ) {
			__int16 levelID = 1;

			InitialiseStartInfo();
			if( (GF_GameFlow.flags & GFF_SelectAnyLevel) != 0 )
				levelID = InventoryExtraData[1] + 1;

			return GF_START_GAME | levelID;
		}
	}
	return GF_EXIT_GAME;
}

void __cdecl CheckCheatMode() {
	static int mode = 0;
	static int turn = 0;
	static __int16 angle = 0;
	static bool isFlare = false;
	__int16 as = LaraItem->currentAnimState;

	// Cheat is disabled in Lara home and final level
	if( CurrentLevel == 0 || CurrentLevel == GF_GameFlow.num_Levels-GF_GameFlow.num_Demos-1 )
		return;

	switch( mode ) {

		case 0:
			// Any State -> Step forward
			if( as == AS_WALK )
				mode = 1;
			break;

		case 1 :
			// Check flare
			isFlare = ( Lara_CurrentGunType == LGT_Flare );
			// Step forward -> Stop
			if( as != AS_WALK )
				mode = ( as == AS_STOP ) ? 2 : 0;
			break;

		case 2 :
			// Stop -> Step back
			if( as != AS_STOP )
				mode = ( as == AS_BACK ) ? 3 : 0;
			break;

		case 3 :
			// Step back -> Stop
			if( as != AS_BACK )
				mode = ( as == AS_STOP ) ? 4 : 0;
			break;

		case 4 :
			// Stop -> Start turn left / right
			if( as != AS_STOP ) {
				turn = 0;
				angle = LaraItem->pos.rotY;
				mode = ( as == AS_TURN_L ) ? 5 : ( as == AS_TURN_R ) ? 6 : 0;
			}
			break;

		case 5 :
			// Continue turn left (required at least 518 degrees / 1.44 spins)
			if( as == AS_TURN_L || as == AS_FASTTURN ) {
				turn += (__int16)(LaraItem->pos.rotY - angle);
				angle = LaraItem->pos.rotY;
			} else {
				mode = ( turn < -0x17000 ) ? 7 : 0;
			}
			break;

		case 6 :
			// Continue turn right (required at least 518 degrees / 1.44 spins)
			if ( as == AS_TURN_R || as == AS_FASTTURN ) {
				turn += (__int16)(LaraItem->pos.rotY - angle);
				angle = LaraItem->pos.rotY;
			} else {
				mode = ( turn > 0x17000 ) ? 7 : 0;
			}
			break;

		case 7 :
			// Stop -> Start jump
			if( as != AS_STOP )
				mode = ( as == AS_COMPRESS ) ? 8 : 0;
			break;

		case 8 :
			// Start falling
			if( LaraItem->fallSpeed > 0 ) {
				// Check if jump interrupted
				if( as != AS_FORWARDJUMP && as != AS_BACKJUMP ) {
					// Finish cheat sequence with no action
				} // Check if flare is not active
				else if( !isFlare || Lara_CurrentGunType != LGT_Flare ) {
					// Explode Lara!
					ExplodingDeath(Lara_ItemNumber, 0xFFFFFFFF, 1);
					LaraItem->hitPoints = 0;
					LaraItem->flags |= 0x0100; // TODO: flags definition
				} // Check jump forward
				else if( as == AS_FORWARDJUMP ) {
					// Complete level
					IsLevelComplete = TRUE;
				} // Check jump backward
				else if( as == AS_BACKJUMP ) {
					// Give weapon
					Inv_AddItem(ID_SHOTGUN_ITEM);
					Inv_AddItem(ID_MAGNUM_ITEM);
					Inv_AddItem(ID_UZI_ITEM);
					Inv_AddItem(ID_HARPOON_ITEM);
					Inv_AddItem(ID_M16_ITEM);
					Inv_AddItem(ID_GRENADE_ITEM);

					// Give ammo
					ShotgunAmmo = 500;
					MagnumAmmo = 500;
					UziAmmo = 5000;
					HarpoonAmmo = 5000;
					M16Ammo = 5000;
					GrenadeAmmo = 5000;

					// Give medipacks and flares
					for( int i=0; i<50; ++i ) {
						Inv_AddItem(ID_SMALL_MEDIPACK_ITEM);
						Inv_AddItem(ID_LARGE_MEDIPACK_ITEM);
						Inv_AddItem(ID_FLARE_ITEM);
					}

					// Play SFX
					PlaySoundEffect(7, NULL, SFX_ALWAYS);
				}
				mode = 0;
			}
			break;

		default :
			mode = 0;
			break;
	}
}

void __cdecl S_SaveSettings() {
	OpenGameRegistryKey(REG_GAME_KEY);
	SetRegistryDwordValue(REG_MUSIC_VOLUME, MusicVolume);
	SetRegistryDwordValue(REG_SOUND_VOLUME, SoundVolume);
	SetRegistryDwordValue(REG_DETAIL_LEVEL, DetailLevel);
	SetRegistryFloatValue(REG_GAME_SIZER, GameSizer);
	SetRegistryBinaryValue(REG_GAME_LAYOUT, (LPBYTE)Layout[CTRL_Custom].key, sizeof(UINT16)*14);
	CloseGameRegistryKey();

#ifdef FEATURE_FOG_DISTANCE
	OpenGameRegistryKey(REG_VIEW_KEY);
	SetRegistryFloatValue(REG_DRAW_DISTANCE, ViewDistanceFactor);
	SetRegistryFloatValue(REG_FOG_BEGIN, FogBeginFactor);
	SetRegistryFloatValue(REG_FOG_END, FogEndFactor);
	SetRegistryFloatValue(REG_UW_FOG_BEGIN, WaterFogBeginFactor);
	SetRegistryFloatValue(REG_UW_FOG_END, WaterFogEndFactor);
	CloseGameRegistryKey();
#endif // FEATURE_FOG_DISTANCE
}

void __cdecl S_LoadSettings() {
	DWORD soundVol = 0;
	DWORD musicVol = 0;

	OpenGameRegistryKey(REG_GAME_KEY);
	GetRegistryDwordValue(REG_MUSIC_VOLUME, &musicVol, 10); // NOTE: There was bug in the original code. 165 instead of 10.
	GetRegistryDwordValue(REG_SOUND_VOLUME, &soundVol, 10);
	GetRegistryDwordValue(REG_DETAIL_LEVEL, &DetailLevel, 1);
	GetRegistryFloatValue(REG_GAME_SIZER, &GameSizer, 1.0);
	GetRegistryBinaryValue(REG_GAME_LAYOUT, (LPBYTE)Layout[CTRL_Custom].key, sizeof(UINT16)*14, NULL);

#ifdef FEATURE_HEALTHBAR_IMPROVED
	GetRegistryDwordValue(REG_HEALTHBAR_MODE, &HealthBarMode, 0);
	GetRegistryBoolValue(REG_PSXBARPOS_ENABLE, &PsxBarPosEnabled, false);
#endif // FEATURE_HEALTHBAR_IMPROVED

#ifdef FEATURE_FOV_FIX
	GetRegistryBoolValue(REG_PSXFOV_ENABLE, &PsxFovEnabled, false);
	GetRegistryFloatValue(REG_GAME_GUI_SCALE, &GameGUI_Scale, 1.0);
	GetRegistryFloatValue(REG_INV_GUI_SCALE, &InvGUI_Scale, 1.0);
	CLAMP(GameGUI_Scale, 1.0, 2.0);
	CLAMP(InvGUI_Scale, 1.0, 2.0);
#endif // FEATURE_FOV_FIX

#ifdef FEATURE_BACKGROUND_IMPROVED
	GetRegistryDwordValue(REG_INVBGND_MODE, &InvBackgroundMode, 1);
	GetRegistryDwordValue(REG_PICTURE_STRETCH, &PictureStretchLimit, 10);
#endif // FEATURE_BACKGROUND_IMPROVED

#ifdef FEATURE_SHADOW_IMPROVED
	GetRegistryDwordValue(REG_SHADOW_MODE, &ShadowMode, 0);
#endif // FEATURE_SHADOW_IMPROVED

#ifdef FEATURE_SCREENSHOT_IMPROVED
	GetRegistryDwordValue(REG_SCREENSHOT_FORMAT, &ScreenshotFormat, 0);
	GetRegistryStringValue(REG_SCREENSHOT_PATH, ScreenshotPath, sizeof(ScreenshotPath), ".\\screenshots");
#endif // FEATURE_SCREENSHOT_IMPROVED

#ifdef FEATURE_ASSAULT_SAVE
	GetRegistryBinaryValue(REG_GAME_ASSAULT, (LPBYTE)&Assault, sizeof(ASSAULT_STATS), NULL);
#endif // FEATURE_ASSAULT_SAVE

	CloseGameRegistryKey();

	// NOTE: There was no such call in the original code, which produces control configuration bugs
	DefaultConflict();

	// NOTE: There was no such volume range check in the original game
	SoundVolume = (soundVol > 10) ? 10 : soundVol;
	MusicVolume = (musicVol > 10) ? 10 : musicVol;
	S_SoundSetMasterVolume(6 * SoundVolume + 4);	// 4,  10,  16,  22,  28,  34,  40,  46,  52,  58,  64
	S_CDVolume(MusicVolume ? MusicVolume*25+5 : 0);	// 0,  30,  55,  80, 105, 130, 155, 180, 205, 230, 255

#ifdef FEATURE_FOG_DISTANCE
	OpenGameRegistryKey(REG_VIEW_KEY);
	GetRegistryFloatValue(REG_DRAW_DISTANCE, &ViewDistanceFactor, 1.0);
	GetRegistryFloatValue(REG_FOG_BEGIN, &FogBeginFactor, 0.6);
	GetRegistryFloatValue(REG_FOG_END, &FogEndFactor, 1.0);
	GetRegistryFloatValue(REG_UW_FOG_BEGIN, &WaterFogBeginFactor, 0.6);
	GetRegistryFloatValue(REG_UW_FOG_END, &WaterFogEndFactor, 1.0);
	CloseGameRegistryKey();

	CLAMP(ViewDistanceFactor, 1.0, 6.0);
	CLAMP(FogEndFactor, 0.0, ViewDistanceFactor);
	CLAMP(FogBeginFactor, 0.0, FogEndFactor);
	CLAMP(WaterFogEndFactor, 0.0, FogEndFactor);
	CLAMP(WaterFogBeginFactor, 0.0, FogBeginFactor);
#endif // FEATURE_FOG_DISTANCE

#if defined (FEATURE_FOV_FIX) || defined (FEATURE_FOG_DISTANCE)
	setup_screen_size();
#endif // FEATURE_FOV_FIX || FEATURE_FOG_DISTANCE
}

/*
 * Inject function
 */
void Inject_SMain() {
	INJECT(0x00454B10, GameMain);
	INJECT(0x00454DE0, TitleSequence);
	INJECT(0x00454EF0, CheckCheatMode);
	INJECT(0x00455250, S_SaveSettings);
	INJECT(0x004552D0, S_LoadSettings);
}
