/*
 * Copyright (c) 2017-2021 Michael Chaban. All rights reserved.
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
#include "specific/input.h"
#include "game/invfunc.h"
#include "game/laramisc.h"
#include "specific/display.h"
#include "specific/init_display.h"
#include "specific/init_input.h"
#include "specific/screenshot.h"
#include "specific/winvid.h"
#include "global/vars.h"

#ifdef FEATURE_INPUT_IMPROVED
bool WalkToSidestep = false;
#endif // FEATURE_INPUT_IMPROVED

// Macros
#define KEY_DOWN(a)		((DIKeys[(a)]&0x80)!=0)
#define TOGGLE(a)		{(a)=!(a);}

// NOTE: not presented in the original game
static BOOL JoyKey(KEYMAP keyMap) {
#ifdef FEATURE_HUD_IMPROVED
	if( keyMap < 4 ) return FALSE; // ignore direction keys
	UINT16 key = Layout[CTRL_Joystick].key[keyMap];
	return CHK_ANY(JoyKeys, (1 << key));
#else // FEATURE_HUD_IMPROVED
	UINT16 key = Layout[CTRL_Custom].key[keyMap];
	return ( key >= 0x100 && CHK_ANY(JoyKeys, (1 << key)) );
#endif // FEATURE_HUD_IMPROVED
}

// NOTE: not presented in the original game
static BOOL KbdKey(KEYMAP keyMap, bool isCustom) {
	UINT16 key = Layout[isCustom ? CTRL_Custom : CTRL_Default].key[keyMap];
	if( key < 0x100 ) {
		if KEY_DOWN(key) {
			return TRUE;
		}
		if( key == DIK_LCONTROL )	return KEY_DOWN(DIK_RCONTROL);
		if( key == DIK_RCONTROL )	return KEY_DOWN(DIK_LCONTROL);
		if( key == DIK_LSHIFT )		return KEY_DOWN(DIK_RSHIFT);
		if( key == DIK_RSHIFT )		return KEY_DOWN(DIK_LSHIFT);
		if( key == DIK_LMENU )		return KEY_DOWN(DIK_RMENU);
		if( key == DIK_RMENU )		return KEY_DOWN(DIK_LMENU);
	}
	return FALSE;
}

// NOTE: the original function is splitted into JoyKey() and KbdKey()
BOOL __cdecl Key(KEYMAP keyMap) {
	return JoyKey(keyMap) || KbdKey(keyMap, true) || (!ConflictLayout[keyMap] && KbdKey(keyMap, false));
}

bool __cdecl S_UpdateInput() {
	// NOTE: some of these isF*KeyPressed are presented in the original code.
	// But some has been added by me (marked as + below)
	// This prevents from multiple actions by one key press
	static bool isScreenShotKeyPressed = false; // +
	static bool isF1KeyPressed = false; // +
	static bool isF2KeyPressed = false; // +
	static bool isF3KeyPressed = false;
	static bool isF4KeyPressed = false;
	static bool isF7KeyPressed = false;
	static bool isF8KeyPressed = false;
	static bool isF11KeyPressed = false;
	static bool isF12KeyPressed = false; // +
	static BYTE mediPackCooldown = 0;
	bool isShiftKeyPressed;
	DISPLAY_MODE_NODE *mode;
	DISPLAY_MODE_LIST *modeList;
	DISPLAY_MODE targetMode;
	APP_SETTINGS newSettings;
	int joyXPos = 0;
	int joyYPos = 0;
	DWORD input = 0;

	WinVidSpinMessageLoop(false);
	WinInReadKeyboard(DIKeys);
	JoyKeys = WinInReadJoystick(&joyXPos, &joyYPos);

	// Joystick Y
	if( joyYPos < -8 ) {
		input |= IN_FORWARD;
	}
	else if( joyYPos > 8 ) {
		input |= IN_BACK;
	}

	// Joystick X
	if( joyXPos < -8 ) {
		input |= IN_LEFT;
	}
	else if( joyXPos > 8 ) {
		input |= IN_RIGHT;
	}

	// Key maps
	if( Key(KM_Forward) ) {
		input |= IN_FORWARD;
	}
	if( Key(KM_Back) ) {
		input |= IN_BACK;
	}
	if( Key(KM_Left) ) {
		input |= IN_LEFT;
	}
	if( Key(KM_Right) ) {
		input |= IN_RIGHT;
	}
	if( Key(KM_StepLeft) ) {
		input |= IN_STEPL;
	}
	if( Key(KM_StepRight) ) {
		input |= IN_STEPR;
	}
	if( Key(KM_Slow) ) {
		input |= IN_SLOW;
	}
	if( Key(KM_Jump) ) {
		input |= IN_JUMP;
	}
	if( Key(KM_Action) ) {
		input |= IN_ACTION;
	}
	if( Key(KM_WeaponDraw) ) {
		input |= IN_DRAW;
	}
	if( Key(KM_Flare ) ) {
		input |= IN_FLARE;
	}
	if( Key(KM_Look) ) {
		input |= IN_LOOK;
	}
	if( Key(KM_Roll) ) {
		input |= IN_ROLL;
	}
	if( Key(KM_Option) && Camera.type != CAM_Cinematic ) {
		input |= IN_OPTION;
	}

	// Key combinations and alternatives
#ifdef FEATURE_HUD_IMPROVED
	if( Key(KM_Step) ) {
		if( CHK_ANY(input, IN_LEFT) ) {
			input &= ~(IN_LEFT|IN_FORWARD|IN_BACK);
			input |= IN_STEPL;
		} else if( CHK_ANY(input, IN_RIGHT) ) {
			input &= ~(IN_RIGHT|IN_FORWARD|IN_BACK);
			input |= IN_STEPR;
		} else {
			input |= IN_SLOW;
		}
	}
#endif // FEATURE_HUD_IMPROVED
#ifdef FEATURE_INPUT_IMPROVED
	if( WalkToSidestep && CHK_ANY(input, IN_SLOW) && !CHK_ANY(input, IN_FORWARD|IN_BACK|IN_STEPL|IN_STEPR) ) {
		if( CHK_ANY(input, IN_LEFT) ) {
			input &= ~IN_LEFT;
			input |= IN_STEPL;
		} else if( CHK_ANY(input, IN_RIGHT) ) {
			input &= ~IN_RIGHT;
			input |= IN_STEPR;
		}
	}
#endif // FEATURE_INPUT_IMPROVED
	if( CHK_ALL(input, IN_FORWARD|IN_BACK) ) {
		input |= IN_ROLL;
	}
	if( KEY_DOWN(DIK_RETURN) || CHK_ANY(input, IN_ACTION) ) {
		input |= IN_SELECT;
	}
	// NOTE: there is no KM_WeaponDraw for Deselect in the original game
	if( KEY_DOWN(DIK_ESCAPE) || JoyKey(KM_WeaponDraw) ) {
		input |= IN_DESELECT;
	}
	if( CHK_ALL(input, IN_STEPL|IN_STEPR) ) {
		input &= ~(IN_STEPL|IN_STEPR);
	}

	// If FMV is playing just save input status and exit here
	if( IsFmvPlaying )
		goto EXIT;

	// NOTE: this check is absent in the original game
	// it fixes a bug, when the player could interfere with the demo level
	if( !IsDemoLevelType ) {
		// Weapon requests
		if( KEY_DOWN(DIK_1) && Inv_RequestItem(ID_PISTOL_OPTION) ) {
			Lara.request_gun_type = LGT_Pistols;
		}
		else if(KEY_DOWN(DIK_2) && Inv_RequestItem(ID_SHOTGUN_OPTION) ) {
			Lara.request_gun_type = LGT_Shotgun;
		}
		else if( KEY_DOWN(DIK_3) && Inv_RequestItem(ID_MAGNUM_OPTION) ) {
			Lara.request_gun_type = LGT_Magnums;
		}
		else if( KEY_DOWN(DIK_4) && Inv_RequestItem(ID_UZI_OPTION) ) {
			Lara.request_gun_type = LGT_Uzis;
		}
		else if( KEY_DOWN(DIK_5) && Inv_RequestItem(ID_HARPOON_OPTION) ) {
			Lara.request_gun_type = LGT_Harpoon;
		}
		else if( KEY_DOWN(DIK_6) && Inv_RequestItem(ID_M16_OPTION) ) {
			Lara.request_gun_type = LGT_M16;
		}
		else if( KEY_DOWN(DIK_7) && Inv_RequestItem(ID_GRENADE_OPTION) ) {
			Lara.request_gun_type = LGT_Grenade;
		}
		else if( KEY_DOWN(DIK_0) && Inv_RequestItem(ID_FLARES_OPTION) ) {
			Lara.request_gun_type = LGT_Flare;
		}

		// MediPack requests
		if( mediPackCooldown > 0 ) {
			--mediPackCooldown; // MediPack shortcuts have half second cooldown
		} else {
			if( KEY_DOWN(DIK_8) && Inv_RequestItem(ID_SMALL_MEDIPACK_OPTION) ) {
				UseItem(ID_SMALL_MEDIPACK_OPTION);
				mediPackCooldown = 15;
			}
			else if( KEY_DOWN(DIK_9) && Inv_RequestItem(ID_LARGE_MEDIPACK_OPTION) ) {
				UseItem(ID_LARGE_MEDIPACK_OPTION);
				mediPackCooldown = 15;
			}
		}
#ifdef FEATURE_CHEAT
		// Cheats
		static bool isStuffCheatKeyPressed = false;
		if( KEY_DOWN(DIK_I) ) {
			if( !isStuffCheatKeyPressed ) {
				isStuffCheatKeyPressed = true;
				input |= IN_STUFFCHEAT;
			}
		} else {
			isStuffCheatKeyPressed = false;
		}

		if( KEY_DOWN(DIK_O) ) {
			input |= IN_DOZYCHEAT;
		}
#endif // FEATURE_CHEAT
	}

#ifdef FEATURE_BACKGROUND_IMPROVED
	static bool isPauseKeyPressed = false;
#ifdef FEATURE_HUD_IMPROVED
	if( Key(KM_Pause) )
#else // FEATURE_HUD_IMPROVED
	if( KEY_DOWN(DIK_P) )
#endif // FEATURE_HUD_IMPROVED
	{
		if( !isPauseKeyPressed ) {
			isPauseKeyPressed = true;
			input |= IN_PAUSE;
		}
	} else {
		isPauseKeyPressed = false;
	}
#endif // FEATURE_BACKGROUND_IMPROVED

	// Screenshot
#ifdef FEATURE_SCREENSHOT_IMPROVED
	if( KEY_DOWN(DIK_BACK) ) { // BackSpace Key instead of S
#else // !FEATURE_SCREENSHOT_IMPROVED
	if( KEY_DOWN(DIK_S) ) {
#endif // FEATURE_SCREENSHOT_IMPROVED
		if( !isScreenShotKeyPressed ) {
			isScreenShotKeyPressed = true;
#if (DIRECT3D_VERSION >= 0x900)
			ScreenShot(NULL);
#else // (DIRECT3D_VERSION >= 0x900)
			ScreenShot(PrimaryBufferSurface);
#endif // (DIRECT3D_VERSION >= 0x900)
		}
	} else {
		isScreenShotKeyPressed = false;
	}

	// Save/Load Game
	if( !CHK_ANY(GF_GameFlow.flags, GFF_LoadSaveDisabled) ) {
		if( KEY_DOWN(DIK_F5) )
			input |= IN_SAVE;
		else if( KEY_DOWN(DIK_F6) )
			input |= IN_LOAD;
	}

	// Shift Key check
	isShiftKeyPressed = KEY_DOWN(DIK_LSHIFT) || KEY_DOWN(DIK_RSHIFT);

	// Graphics option toggles
	if( SavedAppSettings.RenderMode == RM_Software ) {

		// Software Renderer F7 key
		if( KEY_DOWN(DIK_F7) ) {
			if( !isF7KeyPressed ) {
				isF7KeyPressed = true;
#ifndef FEATURE_NOLEGACY_OPTIONS
				if( isShiftKeyPressed ) {
					// Triple Buffer (Shift + F7)
					if( SavedAppSettings.FullScreen ) {
						newSettings = SavedAppSettings;
						TOGGLE(newSettings.TripleBuffering);
						GameApplySettings(&newSettings);
					}
				} else {
					// Perspective Correction (F7). For SW this means Detail Level: single or double perspective distance
					TOGGLE(SavedAppSettings.PerspectiveCorrect);
					if( SavedAppSettings.PerspectiveCorrect ) {
						DetailLevel = 2;
						PerspectiveDistance = SW_DETAIL_HIGH;
					} else {
						DetailLevel = 1;
						PerspectiveDistance = SW_DETAIL_MEDIUM;
					}
				}
#endif // FEATURE_NOLEGACY_OPTIONS
			}
		} else {
			isF7KeyPressed = false;
		}
	} else {

		// Hardware Renderer F7 key
		if( KEY_DOWN(DIK_F7) ) {
			if( !isF7KeyPressed ) {
				isF7KeyPressed = true;
#ifndef FEATURE_NOLEGACY_OPTIONS
				if( isShiftKeyPressed ) {
					// Triple Buffer (Shift + F7)
					if( SavedAppSettings.FullScreen ) {
						newSettings = SavedAppSettings;
						TOGGLE(newSettings.TripleBuffering);
						GameApplySettings(&newSettings);
					}
				} else
#endif // FEATURE_NOLEGACY_OPTIONS
				{
					// ZBuffer (F7)
					newSettings = SavedAppSettings;
					TOGGLE(newSettings.ZBuffer);
					GameApplySettings(&newSettings);
				}
			}
		} else {
			isF7KeyPressed = false;
		}

		// Hardware Renderer F8 key
		if( KEY_DOWN(DIK_F8) ) {
			if( !isF8KeyPressed ) {
				isF8KeyPressed = true;
#ifndef FEATURE_NOLEGACY_OPTIONS
				if( isShiftKeyPressed ) {
					// Perspective Correction (Shift + F8)
					newSettings = SavedAppSettings;
					TOGGLE(newSettings.PerspectiveCorrect);
					GameApplySettings(&newSettings);
				} else
#endif // FEATURE_NOLEGACY_OPTIONS
				{
					// Bilinear Filtering (F8)
					newSettings = SavedAppSettings;
					TOGGLE(newSettings.BilinearFiltering);
					GameApplySettings(&newSettings);
				}
			}
		} else {
			isF8KeyPressed = false;
		}

		// Hardware Renderer F11 key
		if( KEY_DOWN(DIK_F11) ) {
			if( !isF11KeyPressed ) {
				isF11KeyPressed = true;
#ifndef FEATURE_NOLEGACY_OPTIONS
				// Dithering (F11)
				newSettings = SavedAppSettings;
				TOGGLE(newSettings.Dither);
				GameApplySettings(&newSettings);
#endif // FEATURE_NOLEGACY_OPTIONS
			}
		} else {
			isF11KeyPressed = false;
		}
	}

	// Check F12 key if view mode is not locked
	if( !IsVidModeLock ) {
		if( KEY_DOWN(DIK_F12) ) {
			if( !isF12KeyPressed ) {
				isF12KeyPressed = true;
				newSettings = SavedAppSettings;

				if( !isShiftKeyPressed ) {
					// FullScreen/Windowed Toggle (F12)
					TOGGLE(newSettings.FullScreen);

					if( SavedAppSettings.FullScreen ) {

						// FullScreen to Windowed
#ifdef FEATURE_NOLEGACY_OPTIONS
						CLAMPL(newSettings.WindowWidth, 320);
						CLAMPL(newSettings.WindowHeight, 240);
						newSettings.WindowWidth = CalculateWindowWidth(newSettings.WindowWidth, newSettings.WindowHeight);
#else // FEATURE_NOLEGACY_OPTIONS
						int winWidth  = MAX(PhdWinWidth, 320);
						int winHeight = MAX(PhdWinHeight, 240);

						newSettings.WindowHeight = winHeight;
						newSettings.WindowWidth = CalculateWindowWidth(winWidth, winHeight);
						newSettings.TripleBuffering = false;
#endif // FEATURE_NOLEGACY_OPTIONS
						GameApplySettings(&newSettings);

						// Reset inner screen size for windowed mode
						GameSizer = 1.0;
						ScreenSizer = 1.0;
						setup_screen_size();
					} else {

						// Windowed to FullScreen
						if( SavedAppSettings.RenderMode == RM_Hardware )
							modeList = &CurrentDisplayAdapter.hwDispModeList;
						else
							modeList = &CurrentDisplayAdapter.swDispModeList;

						if( modeList->dwCount > 0 ) {
							targetMode.width  = GameVidWidth;
							targetMode.height = GameVidHeight;
							targetMode.bpp = GameVidBPP;
							targetMode.vga = VGA_NoVga;
#ifdef FEATURE_NOLEGACY_OPTIONS
							if( SavedAppSettings.VideoMode ) {
								targetMode.width = SavedAppSettings.VideoMode->body.width;
								targetMode.height = SavedAppSettings.VideoMode->body.height;
							}
							if( modeList->head ) {
								targetMode.bpp = modeList->head->body.bpp;
								targetMode.vga = modeList->head->body.vga;
							}
#endif // FEATURE_NOLEGACY_OPTIONS

							for( mode = modeList->head; mode != NULL; mode = mode->next ) {
								if( !CompareVideoModes(&mode->body, &targetMode) )
									break;
							}
							if( mode == NULL ) {
								mode = modeList->tail;
							}

							newSettings.VideoMode = mode;
							GameApplySettings(&newSettings);
						}
					}
				}
				// Check Shift+F12 key if Inventory is not active now
				else if( !IsInventoryActive ) {
					// Renderer Toggle (Shift + F12)

					if( SavedAppSettings.RenderMode == RM_Software ) {
						// Software to Hardware
						newSettings.RenderMode = RM_Hardware;
						modeList = &CurrentDisplayAdapter.hwDispModeList;
						targetMode.bpp = 16;
					} else {
						// Hardware to Software
						newSettings.RenderMode = RM_Software;
						modeList = &CurrentDisplayAdapter.swDispModeList;
						targetMode.bpp = 8;
					}

					if( modeList->dwCount > 0 ) {
						targetMode.width  = GameVidWidth;
						targetMode.height = GameVidHeight;
						targetMode.vga = VGA_NoVga;
#ifdef FEATURE_NOLEGACY_OPTIONS
						if( SavedAppSettings.VideoMode ) {
							targetMode.width = SavedAppSettings.VideoMode->body.width;
							targetMode.height = SavedAppSettings.VideoMode->body.height;
						}
						if( modeList->head ) {
							targetMode.bpp = modeList->head->body.bpp;
							targetMode.vga = modeList->head->body.vga;
						}
#endif // FEATURE_NOLEGACY_OPTIONS

						for( mode = modeList->head; mode != NULL; mode = mode->next ) {
							if( !CompareVideoModes(&mode->body, &targetMode) )
								break;
						}
						if( mode == NULL ) {
							mode = modeList->tail;
						}

						newSettings.VideoMode = mode;
#if (DIRECT3D_VERSION < 0x900)
						newSettings.FullScreen = true;
#endif // (DIRECT3D_VERSION < 0x900)
						GameApplySettings(&newSettings);
					}
				}
			}
		} else {
			isF12KeyPressed = false;
		}
	}

#ifdef FEATURE_NOLEGACY_OPTIONS
	if( SavedAppSettings.RenderMode == RM_Software ) {
		// Decrease Software Renderer Detail Level
		if( KEY_DOWN(DIK_F3) ) {
			if( !isF3KeyPressed && DetailLevel > 0 ) {
				isF3KeyPressed = true;
				switch( --DetailLevel ) {
					case 0: PerspectiveDistance = SW_DETAIL_LOW; break;
					case 1: PerspectiveDistance = SW_DETAIL_MEDIUM; break;
					case 2: PerspectiveDistance = SW_DETAIL_HIGH; break;
				}
			}
		} else {
			isF3KeyPressed = false;
		}

		// Increase Software Renderer Detail Level
		if( KEY_DOWN(DIK_F4) ) {
			if( !isF4KeyPressed && DetailLevel < 2 ) {
				isF4KeyPressed = true;
				switch( ++DetailLevel ) {
					case 0: PerspectiveDistance = SW_DETAIL_LOW; break;
					case 1: PerspectiveDistance = SW_DETAIL_MEDIUM; break;
					case 2: PerspectiveDistance = SW_DETAIL_HIGH; break;
				}
			}
		} else {
			isF4KeyPressed = false;
		}
	}
#endif // FEATURE_NOLEGACY_OPTIONS

	// Check if we cannot change full screen video parameters here
	if( IsVidSizeLock ||
		Camera.type == CAM_Cinematic ||
		!SavedAppSettings.FullScreen ||
		CHK_ANY(GF_GameFlow.flags, GFF_ScreenSizingDisabled) )
	{
		goto EXIT;
	}

	// Decrease resolution/depth (F1)
	if( KEY_DOWN(DIK_F1) ) {
		if( !isF1KeyPressed ) {
			isF1KeyPressed = true;
			mode = SavedAppSettings.VideoMode;

			// Get previous videomode (enough for Software Renderer)
			if( mode != NULL ) {
				mode = mode->previous;
			}
			// Additional checks for Hardware Renderer
			if( SavedAppSettings.RenderMode == RM_Hardware ) {
				for( ; mode != NULL; mode = mode->previous ) {
#ifndef FEATURE_NOLEGACY_OPTIONS
					// Decrease depth (Shift + F1)
					if( isShiftKeyPressed ) {
						if( mode->body.width  == SavedAppSettings.VideoMode->body.width &&
							mode->body.height == SavedAppSettings.VideoMode->body.height &&
							mode->body.vga == SavedAppSettings.VideoMode->body.vga &&
							mode->body.bpp <  SavedAppSettings.VideoMode->body.bpp )
						{
							break;
						}
					} else
#endif // FEATURE_NOLEGACY_OPTIONS
					// Decrease resolution (F1)
					{
						if( mode->body.vga == SavedAppSettings.VideoMode->body.vga &&
							mode->body.bpp == SavedAppSettings.VideoMode->body.bpp )
						{
							break;
						}
					}
				}
			}
			if( mode != NULL ) {
				newSettings = SavedAppSettings;
				newSettings.VideoMode = mode;
				GameApplySettings(&newSettings);
			}
		}
	} else {
		isF1KeyPressed = false;
	}

	// Increase resolution/depth (F2)
	if( KEY_DOWN(DIK_F2) ) {
		if( !isF2KeyPressed ) {
			isF2KeyPressed = true;
			mode = SavedAppSettings.VideoMode;

			// Get next videomode (enough for Software Renderer)
			if( mode != NULL ) {
				mode = mode->next;
			}
			// Additional checks for Hardware Renderer
			if( SavedAppSettings.RenderMode == RM_Hardware ) {
				for( ; mode != NULL; mode = mode->next ) {
#ifndef FEATURE_NOLEGACY_OPTIONS
					// Increase depth (Shift + F2)
					if( isShiftKeyPressed ) {
						if( mode->body.width  == SavedAppSettings.VideoMode->body.width &&
							mode->body.height == SavedAppSettings.VideoMode->body.height &&
							mode->body.vga == SavedAppSettings.VideoMode->body.vga &&
							mode->body.bpp >  SavedAppSettings.VideoMode->body.bpp )
						{
							break;
						}
					} else
#endif // FEATURE_NOLEGACY_OPTIONS
					// Increase resolution (F2)
					{
						if( mode->body.vga == SavedAppSettings.VideoMode->body.vga &&
							mode->body.bpp == SavedAppSettings.VideoMode->body.bpp )
						{
							break;
						}
					}
				}
			}
			if( mode != NULL ) {
				newSettings = SavedAppSettings;
				newSettings.VideoMode = mode;
				GameApplySettings(&newSettings);
			}
		}
	}else {
		isF2KeyPressed = false;
	}

#ifndef FEATURE_NOLEGACY_OPTIONS
	// Decrease inner screen size (F3)
	if( KEY_DOWN(DIK_F3) ) {
		if( !isF3KeyPressed ) {
			isF3KeyPressed = true;
			DecreaseScreenSize();
		}
	} else {
		isF3KeyPressed = false;
	}

	// Increase inner screen size (F4)
	if( KEY_DOWN(DIK_F4) ) {
		if( !isF4KeyPressed ) {
			isF4KeyPressed = true;
			IncreaseScreenSize();
		}
	} else {
		isF4KeyPressed = false;
	}
#endif // !FEATURE_NOLEGACY_OPTIONS

EXIT :
	InputStatus = input;
	return IsGameToExit;
}

/*
 * Inject function
 */
void Inject_Input() {
	INJECT(0x0044D9B0, Key);
	INJECT(0x0044DAD0, S_UpdateInput);
}
