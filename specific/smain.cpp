/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
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

#ifdef FEATURE_HUD_IMPROVED
extern DWORD DemoTextMode;
extern DWORD JoystickButtonStyle;
extern DWORD SavegameSlots;
extern DWORD InvTextBoxMode;
extern DWORD HealthBarMode;
extern bool PsxBarPosEnabled;
extern bool JoystickHintsEnabled;
extern double GameGUI_Scale;
extern double InvGUI_Scale;
#endif // FEATURE_HUD_IMPROVED

#ifdef FEATURE_BACKGROUND_IMPROVED
static char PictureSuffix[32];
extern DWORD InvBackgroundMode;
extern DWORD StatsBackgroundMode;
extern DWORD PauseBackgroundMode;
extern DWORD PictureStretchLimit;
extern bool LoadingScreensEnabled;
extern bool RemasteredPixEnabled;
#endif // FEATURE_BACKGROUND_IMPROVED

#ifdef FEATURE_VIDEOFX_IMPROVED
extern DWORD ShadowMode;
extern DWORD AlphaBlendMode;
extern DWORD ReflectionMode;
extern DWORD PickupItemMode;
extern bool CustomWaterColorEnabled;
#endif // FEATURE_VIDEOFX_IMPROVED

#ifdef FEATURE_SCREENSHOT_IMPROVED
extern DWORD ScreenshotFormat;
extern char ScreenshotPath[MAX_PATH];
#endif // FEATURE_SCREENSHOT_IMPROVED

#ifdef FEATURE_INPUT_IMPROVED
#include "modding/joy_output.h"
extern bool WalkToSidestep;
extern bool JoystickVibrationEnabled;
extern bool JoystickLedColorEnabled;
#endif // FEATURE_INPUT_IMPROVED

#ifdef FEATURE_MOD_CONFIG
extern bool BarefootSfxEnabled;
#endif // FEATURE_MOD_CONFIG

#ifdef FEATURE_AUDIO_IMPROVED
extern double InventoryMusicMute;
extern double UnderwaterMusicMute;
#endif // FEATURE_AUDIO_IMPROVED

#ifdef FEATURE_VIEW_IMPROVED
extern bool PsxFovEnabled;
extern double ViewDistanceFactor;
extern double FogBeginFactor;
extern double FogEndFactor;
extern double WaterFogBeginFactor;
extern double WaterFogEndFactor;
#endif // FEATURE_VIEW_IMPROVED

#ifdef FEATURE_GAMEPLAY_FIXES
extern bool IsRunningM16fix;
extern bool IsLowCeilingJumpFix;
#endif // FEATURE_GAMEPLAY_FIXES

#ifdef FEATURE_GOLD
extern bool IsGold();
#endif

#ifdef FEATURE_ASSAULT_SAVE
void SaveAssault() {
	OpenGameRegistryKey(REG_GAME_KEY);
	SetRegistryBinaryValue(REG_GAME_ASSAULT, (LPBYTE)&Assault, sizeof(ASSAULT_STATS));
	CloseGameRegistryKey();
}
#endif // FEATURE_ASSAULT_SAVE

BOOL __cdecl GameMain() {
	__int16 gfOption, gfDirection, gfParameter;
	bool isFrontendFail, isLoopContinue, bonusFlag;

	HiRes = 0;
	ScreenSizer = 1.0;
	GameSizer = 1.0;

	if( !S_InitialiseSystem() )
		return FALSE;

	LPCTSTR scriptFileName = "data\\tombPC.dat";
#ifdef FEATURE_GOLD
	if( IsGold() ) {
		scriptFileName = "data\\tombPCg.dat";
	}
#endif // FEATURE_GOLD

	if ( !GF_LoadScriptFile(scriptFileName) ) {
		S_ExitSystem("GameMain: could not load script file");
		return FALSE; // the app is terminated here
	}

	SOUND_Init();
	InitialiseStartInfo();
	// NOTE: S_FrontEndCheck() called before S_LoadSettings() in the original game
	S_LoadSettings();
	S_FrontEndCheck();
	HiRes = -1;

	// NOTE: this HWR init was absent in the original code, but must be done here
	if( SavedAppSettings.RenderMode == RM_Hardware ) {
		HWR_InitState();
	}

	GameMemoryPointer = (BYTE *)GlobalAlloc(GMEM_FIXED, GameMemorySize);
	if( GameMemoryPointer == NULL ) {
		lstrcpy(StringToShow, "GameMain: could not allocate malloc_buffer");
		return FALSE;
	}

	HiRes = 0;
	TempVideoAdjust(1, 1.0);
	S_UpdateInput();
	IsVidModeLock = true;
#ifdef FEATURE_BACKGROUND_IMPROVED
	int res = -1;
	if( *PictureSuffix ) {
		char fname[256];
		snprintf(fname, sizeof(fname), "data\\legal%s.pcx", PictureSuffix);
		res = BGND2_LoadPicture(fname, FALSE, FALSE);
	}
	if( res ) {
		res = BGND2_LoadPicture("data\\legal.pcx", FALSE, FALSE);
	}
	if( !res ) {
		BGND2_ShowPicture(30, 90, 10, 2, TRUE);
	}
#else // FEATURE_BACKGROUND_IMPROVED
#ifdef FEATURE_GOLD
	S_DisplayPicture(IsGold()?"data\\legalg.pcx":"data\\legal.pcx", FALSE);
#else // !FEATURE_GOLD
	S_DisplayPicture("data\\legal.pcx", FALSE);
#endif // !FEATURE_GOLD
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
					gfOption = GF_DoLevelSequence(GF_GameFlow.singleLevel, GFL_NORMAL);
				} else {
					if( gfParameter > GF_GameFlow.num_Levels ) {
						wsprintf(StringToShow, "GameMain: STARTGAME with invalid level number (%d)", gfParameter);
						return FALSE;
					}
					gfOption = GF_DoLevelSequence(gfParameter, GFL_NORMAL);
				}
				break;

			case GF_START_SAVEDGAME :
				S_LoadGame(&SaveGame, sizeof(SAVEGAME_INFO), gfParameter);
				if( SaveGame.currentLevel > GF_GameFlow.num_Levels ) {
					wsprintf(StringToShow, "GameMain: STARTSAVEDGAME with invalid level number (%d)", SaveGame.currentLevel);
					return FALSE;
				}
				gfOption = GF_DoLevelSequence(SaveGame.currentLevel, GFL_SAVED);
				break;

			case GF_START_CINE :
				StartCinematic(gfParameter);
				gfOption = GF_EXIT_TO_TITLE;
				break;

			case GF_START_DEMO :
				// NOTE: there is no such bonus flag reset in the original game
				bonusFlag = SaveGame.bonusFlag; // backup bonusFlag
				SaveGame.bonusFlag = 0; // remove bonusFlag while Demo is playing
				gfOption = DoDemoSequence(-1);
				SaveGame.bonusFlag = bonusFlag; // restore bonusFlag
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
					GF_StartGame = true;
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

#ifdef FEATURE_BACKGROUND_IMPROVED
	int res = -1;
	if( *PictureSuffix ) {
		char fname[256];
		snprintf(fname, sizeof(fname), "data\\title%s.pcx", PictureSuffix);
		res = BGND2_LoadPicture(fname, TRUE, FALSE);
	}
	if( res ) {
		res = BGND2_LoadPicture("data\\title.pcx", TRUE, FALSE);
	}
	if( !res ) {
		// NOTE: title menu fade-in was absent in the original game
		BGND2_ShowPicture(15, 0, 0, 0, FALSE);
	}
#elif defined(FEATURE_GOLD)
	S_DisplayPicture(IsGold()?"data\\titleg.pcx":"data\\title.pcx", TRUE);
#else // FEATURE_BACKGROUND_IMPROVED
	S_DisplayPicture("data\\title.pcx", TRUE);
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
			isFlare = ( Lara.gun_type == LGT_Flare );
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
				else if( !isFlare || Lara.gun_type != LGT_Flare ) {
					// Explode Lara!
					ExplodingDeath(Lara.item_number, 0xFFFFFFFF, 1);
					LaraItem->hitPoints = 0;
					LaraItem->flags |= IFL_INVISIBLE;
#ifdef FEATURE_INPUT_IMPROVED
					JoyRumbleExplode(LaraItem->pos.x, LaraItem->pos.y, LaraItem->pos.z, 0x1400, true);
#endif // FEATURE_INPUT_IMPROVED
				} // Check jump forward
				else if( as == AS_FORWARDJUMP ) {
					// Complete level
					IsLevelComplete = TRUE;
				} // Check jump backward
				else if( as == AS_BACKJUMP ) {
					// Give weapon and ammo
					// NOTE: additional weapon availability checks not presented in the original game
					if( Objects[ID_SHOTGUN_OPTION].loaded ) {
						if( !Inv_RequestItem(ID_SHOTGUN_ITEM) ) {
							Inv_AddItem(ID_SHOTGUN_ITEM);
						}
						Lara.shotgun_ammo = SaveGame.bonusFlag ? 10001 : 500;
					}
					if( Objects[ID_MAGNUM_OPTION].loaded ) {
						if( !Inv_RequestItem(ID_MAGNUM_ITEM) ) {
							Inv_AddItem(ID_MAGNUM_ITEM);
						}
						Lara.magnum_ammo = SaveGame.bonusFlag ? 10001 : 500;
					}
					if( Objects[ID_UZI_OPTION].loaded ) {
						if( !Inv_RequestItem(ID_UZI_ITEM) ) {
							Inv_AddItem(ID_UZI_ITEM);
						}
						Lara.uzi_ammo = SaveGame.bonusFlag ? 10001 : 5000;
					}
					if( Objects[ID_HARPOON_OPTION].loaded ) {
						if( !Inv_RequestItem(ID_HARPOON_ITEM) ) {
							Inv_AddItem(ID_HARPOON_ITEM);
						}
						Lara.harpoon_ammo = SaveGame.bonusFlag ? 10001 : 5000;
					}
					if( Objects[ID_M16_OPTION].loaded ) {
						if( !Inv_RequestItem(ID_M16_ITEM) ) {
							Inv_AddItem(ID_M16_ITEM);
						}
						Lara.m16_ammo = SaveGame.bonusFlag ? 10001 : 5000;
					}
					if( Objects[ID_GRENADE_OPTION].loaded ) {
						if( !Inv_RequestItem(ID_GRENADE_ITEM) ) {
							Inv_AddItem(ID_GRENADE_ITEM);
						}
						Lara.grenade_ammo = SaveGame.bonusFlag ? 10001 : 5000;
					}

					// Give medipacks and flares
					for( int i=0; i<50; ++i ) {
						// NOTE: there are no limits in the original code, but it works wrong without limits
						if( Objects[ID_SMALL_MEDIPACK_OPTION].loaded && Inv_RequestItem(ID_SMALL_MEDIPACK_ITEM) < 240 ) {
							Inv_AddItem(ID_SMALL_MEDIPACK_ITEM);
						}
						if( Objects[ID_LARGE_MEDIPACK_OPTION].loaded && Inv_RequestItem(ID_LARGE_MEDIPACK_ITEM) < 240 ) {
							Inv_AddItem(ID_LARGE_MEDIPACK_ITEM);
						}
						if( Objects[ID_FLARES_OPTION].loaded && Inv_RequestItem(ID_FLARE_ITEM) < 240 ) {
							Inv_AddItem(ID_FLARE_ITEM);
						}
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

#ifndef FEATURE_NOLEGACY_OPTIONS
	SetRegistryFloatValue(REG_GAME_SIZER, GameSizer);
#endif // FEATURE_NOLEGACY_OPTIONS
#ifdef FEATURE_HUD_IMPROVED
	SetRegistryBinaryValue(REG_GAME_JOY_LAYOUT, Layout[CTRL_Joystick].key, sizeof(CONTROL_LAYOUT));
	SetRegistryBinaryValue(REG_GAME_KBD_LAYOUT, Layout[CTRL_Custom].key, sizeof(CONTROL_LAYOUT));
	SetRegistryBoolValue(REG_JOYSTICK_HINTS, JoystickHintsEnabled);
#else // FEATURE_HUD_IMPROVED
	SetRegistryBinaryValue(REG_GAME_LAYOUT, (LPBYTE)Layout[CTRL_Custom].key, sizeof(CONTROL_LAYOUT));
#endif // FEATURE_HUD_IMPROVED
#ifdef FEATURE_INPUT_IMPROVED
	SetRegistryBoolValue(REG_JOYSTICK_VIBRATION, JoystickVibrationEnabled);
#endif // FEATURE_INPUT_IMPROVED
	CloseGameRegistryKey();

#ifdef FEATURE_VIEW_IMPROVED
	OpenGameRegistryKey(REG_VIEW_KEY);
	SetRegistryFloatValue(REG_DRAW_DISTANCE, ViewDistanceFactor);
	SetRegistryFloatValue(REG_FOG_BEGIN, FogBeginFactor);
	SetRegistryFloatValue(REG_FOG_END, FogEndFactor);
	SetRegistryFloatValue(REG_UW_FOG_BEGIN, WaterFogBeginFactor);
	SetRegistryFloatValue(REG_UW_FOG_END, WaterFogEndFactor);
	CloseGameRegistryKey();
#endif // FEATURE_VIEW_IMPROVED
}

void __cdecl S_LoadSettings() {
	DWORD soundVol = 0;
	DWORD musicVol = 0;

	OpenGameRegistryKey(REG_GAME_KEY);
	GetRegistryDwordValue(REG_MUSIC_VOLUME, &musicVol, 10); // NOTE: There was bug in the original code. 165 instead of 10.
	GetRegistryDwordValue(REG_SOUND_VOLUME, &soundVol, 10);
	GetRegistryDwordValue(REG_DETAIL_LEVEL, &DetailLevel, 1);

#ifdef FEATURE_NOLEGACY_OPTIONS
	GameSizer = 1.0;
#else // FEATURE_NOLEGACY_OPTIONS
	GetRegistryFloatValue(REG_GAME_SIZER, &GameSizer, 1.0);
#endif // FEATURE_NOLEGACY_OPTIONS

#ifdef FEATURE_HUD_IMPROVED
	GetRegistryBinaryValue(REG_GAME_JOY_LAYOUT, Layout[CTRL_Joystick].key, sizeof(CONTROL_LAYOUT), NULL);
	if( !GetRegistryBinaryValue(REG_GAME_KBD_LAYOUT, Layout[CTRL_Custom].key, sizeof(CONTROL_LAYOUT), NULL) ) {
		UINT16 legacy[14];
		if( GetRegistryBinaryValue(REG_GAME_LAYOUT, (LPBYTE)legacy, sizeof(legacy), NULL) ) {
			// migrate the legacy keyboard Layout to the new one
			UINT16 remap[14] = {0,1,2,3,4,5, 7,8,9, 11,12,13, 10,14};
			for( int i=0; i<14; ++i ) {
				if( legacy[i] < 0x100 ) {
					Layout[CTRL_Custom].key[remap[i]] = legacy[i];
				}
			}
		}
	}
	GetRegistryDwordValue(REG_DEMOTEXT_MODE, &DemoTextMode, 0);
	GetRegistryDwordValue(REG_JOYSTICK_BTN_STYLE, &JoystickButtonStyle, 0);
	GetRegistryDwordValue(REG_SAVEGAME_SLOTS, &SavegameSlots, 0);
	GetRegistryDwordValue(REG_INVTEXTBOX_MODE, &InvTextBoxMode, 1);
	GetRegistryDwordValue(REG_HEALTHBAR_MODE, &HealthBarMode, 2);
	GetRegistryBoolValue(REG_PSXBARPOS_ENABLE, &PsxBarPosEnabled, true);
	GetRegistryBoolValue(REG_JOYSTICK_HINTS, &JoystickHintsEnabled, true);
	GetRegistryFloatValue(REG_GAME_GUI_SCALE, &GameGUI_Scale, 1.0);
	GetRegistryFloatValue(REG_INV_GUI_SCALE, &InvGUI_Scale, 1.0);
	if( JoystickButtonStyle > 3 ) {
		JoystickButtonStyle = 0;
	}
	CLAMP(SavegameSlots, 16, 24);
	CLAMP(GameGUI_Scale, 0.5, 2.0);
	CLAMP(InvGUI_Scale, 0.5, 2.0);
#else // FEATURE_HUD_IMPROVED
	GetRegistryBinaryValue(REG_GAME_LAYOUT, (LPBYTE)Layout[CTRL_Custom].key, sizeof(CONTROL_LAYOUT), NULL);
#endif // FEATURE_HUD_IMPROVED

#ifdef FEATURE_INPUT_IMPROVED
	GetRegistryBoolValue(REG_WALK_TO_SIDESTEP, &WalkToSidestep, false);
	GetRegistryBoolValue(REG_JOYSTICK_VIBRATION, &JoystickVibrationEnabled, true);
	GetRegistryBoolValue(REG_JOYSTICK_LED_COLOR, &JoystickLedColorEnabled, true);
#endif // FEATURE_INPUT_IMPROVED

#ifdef FEATURE_BACKGROUND_IMPROVED
	GetRegistryDwordValue(REG_INVBGND_MODE, &InvBackgroundMode, 2);
	GetRegistryDwordValue(REG_STATSBGND_MODE, &StatsBackgroundMode, 0);
	GetRegistryDwordValue(REG_PAUSEBGND_MODE, &PauseBackgroundMode, 1);
	GetRegistryDwordValue(REG_PICTURE_STRETCH, &PictureStretchLimit, 10);
	GetRegistryBoolValue(REG_REMASTER_PIX_ENABLE, &RemasteredPixEnabled, true);
	GetRegistryBoolValue(REG_LOADING_SCREENS, &LoadingScreensEnabled, true);
	GetRegistryStringValue(REG_PICTURE_SUFFIX, PictureSuffix, sizeof(PictureSuffix), "");
#endif // FEATURE_BACKGROUND_IMPROVED

#ifdef FEATURE_VIDEOFX_IMPROVED
	GetRegistryDwordValue(REG_SHADOW_MODE, &ShadowMode, 1);
	GetRegistryDwordValue(REG_ALPHABLEND_MODE, &AlphaBlendMode, 2);
	GetRegistryDwordValue(REG_REFLECTION_MODE, &ReflectionMode, 2);
	GetRegistryDwordValue(REG_PICKUPITEM_MODE, &PickupItemMode, 1);
	GetRegistryBoolValue(REG_CUSTOM_WATER_COLOR, &CustomWaterColorEnabled, true);
	CLAMPG(AlphaBlendMode, 2);
	CLAMPG(ReflectionMode, 3);
#endif // FEATURE_VIDEOFX_IMPROVED

#ifdef FEATURE_SCREENSHOT_IMPROVED
	GetRegistryDwordValue(REG_SCREENSHOT_FORMAT, &ScreenshotFormat, 1);
	GetRegistryStringValue(REG_SCREENSHOT_PATH, ScreenshotPath, sizeof(ScreenshotPath), ".\\screenshots");
#endif // FEATURE_SCREENSHOT_IMPROVED

#ifdef FEATURE_ASSAULT_SAVE
	GetRegistryBinaryValue(REG_GAME_ASSAULT, (LPBYTE)&Assault, sizeof(Assault), NULL);
	if( Assault.bestTime[0] > 0 ) {
		AssaultBestTime = Assault.bestTime[0];
	}
#endif // FEATURE_ASSAULT_SAVE

#ifdef FEATURE_AUDIO_IMPROVED
	GetRegistryFloatValue(REG_INV_MUSIC_MUTE, &InventoryMusicMute, 0.8);
	GetRegistryFloatValue(REG_UW_MUSIC_MUTE, &UnderwaterMusicMute, 0.8);
	CLAMP(InventoryMusicMute, 0.0, 1.0);
	CLAMP(UnderwaterMusicMute, 0.0, 1.0);
#endif // FEATURE_AUDIO_IMPROVED

#ifdef FEATURE_VIEW_IMPROVED
	GetRegistryBoolValue(REG_PSXFOV_ENABLE, &PsxFovEnabled, false);
#endif // FEATURE_VIEW_IMPROVED

#ifdef FEATURE_MOD_CONFIG
	GetRegistryBoolValue(REG_BAREFOOT_SFX_ENABLE, &BarefootSfxEnabled, true);
#endif // FEATURE_MOD_CONFIG

#ifdef FEATURE_GOLD
	if( IsGold() ) {
		// This RJF check is presented in "The Golden Mask" only
		DWORD rjf = 0;
		GetRegistryDwordValue("RJF", &rjf, 0);
		if( rjf == 150868 ) {
			GF_GameFlow.flags |= GFF_SelectAnyLevel;
		} else {
			DeleteRegistryValue("RJF");
		}
	}
#endif // FEATURE_GOLD

	CloseGameRegistryKey();

	// NOTE: There was no such call in the original code, which produces control configuration bugs
	DefaultConflict();

	// NOTE: There was no such volume range check in the original game
	SoundVolume = (soundVol > 10) ? 10 : soundVol;
	MusicVolume = (musicVol > 10) ? 10 : musicVol;
	S_SoundSetMasterVolume(6 * SoundVolume + 4);	// 4,  10,  16,  22,  28,  34,  40,  46,  52,  58,  64
	S_CDVolume(MusicVolume ? MusicVolume*25+5 : 0);	// 0,  30,  55,  80, 105, 130, 155, 180, 205, 230, 255

#ifdef FEATURE_GAMEPLAY_FIXES
	OpenGameRegistryKey(REG_BUGS_KEY);
	GetRegistryBoolValue(REG_RUNNING_M16_FIX, &IsRunningM16fix, false);
	GetRegistryBoolValue(REG_LOWCEILING_JUMP_FIX, &IsLowCeilingJumpFix, true);
	CloseGameRegistryKey();
#endif // FEATURE_GAMEPLAY_FIXES

#ifdef FEATURE_VIEW_IMPROVED
	OpenGameRegistryKey(REG_VIEW_KEY);
	GetRegistryFloatValue(REG_DRAW_DISTANCE, &ViewDistanceFactor, 6.0);
	GetRegistryFloatValue(REG_FOG_BEGIN, &FogBeginFactor, 1.0);
	GetRegistryFloatValue(REG_FOG_END, &FogEndFactor, 6.0);
	GetRegistryFloatValue(REG_UW_FOG_BEGIN, &WaterFogBeginFactor, 0.6);
	GetRegistryFloatValue(REG_UW_FOG_END, &WaterFogEndFactor, 1.0);
	CloseGameRegistryKey();

	CLAMP(ViewDistanceFactor, 1.0, 6.0);
	CLAMP(FogEndFactor, 0.0, ViewDistanceFactor);
	CLAMP(FogBeginFactor, 0.0, FogEndFactor);
	CLAMP(WaterFogEndFactor, 0.0, FogEndFactor);
	CLAMP(WaterFogBeginFactor, 0.0, FogBeginFactor);

	setup_screen_size();
#endif // FEATURE_VIEW_IMPROVED
}

// NOTE: this function is presented in the "Golden Mask" only
void __cdecl EnableLevelSelect() {
#ifdef FEATURE_GOLD
	if( IsGold() ) {
		OpenGameRegistryKey(REG_GAME_KEY);
		// NOTE: It seems that RJF=150868 refers to
		// Richard J. Flower ("The Golden Mask" programmer)
		// and probably his birthday on August 15th, 1968.
		SetRegistryDwordValue("RJF", 150868);
		GF_GameFlow.flags |= GFF_SelectAnyLevel;
		CloseGameRegistryKey();
	}
#endif // FEATURE_GOLD
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
