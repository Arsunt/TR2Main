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

#ifndef REGISTRY_H_INCLUDED
#define REGISTRY_H_INCLUDED

#include "global/types.h"

// Registry path
#define REG_TR2_PATH			"Software\\Core Design\\Tomb Raider II"

// Registry keys
#define REG_SYSTEM_KEY			"System"
#define REG_GAME_KEY			"Game"
#define REG_VIEW_KEY			"View"

// DWORD value names
#define REG_DISPLAY_ADAPTER		"PreferredDisplayAdapterGUID"
#define REG_SOUND_ADAPTER		"PreferredSoundAdapterGUID"
#define REG_JOYSTICK			"PreferredJoystickGUID"
#define REG_RENDERER			"RenderMode"
#define REG_FS_WIDTH			"FullScreenWidth"
#define REG_FS_HEIGHT			"FullScreenHeight"
#define REG_FS_BPP				"FullScreenBPP"
#define REG_WIN_WIDTH			"WindowWidth"
#define REG_WIN_HEIGHT			"WindowHeight"
#define REG_WIN_ASPECT			"AspectMode"
#define REG_ADJUST_MODE			"TexelAdjustMode"
#define REG_ADJUST_NEAREST		"NearestAdjustment"
#define REG_ADJUST_LINEAR		"LinearAdjustment"
#define REG_MUSIC_VOLUME		"MusicVolume"
#define REG_SOUND_VOLUME		"SoundFXVolume"
#define REG_DETAIL_LEVEL		"DetailLevel"
#define REG_INVBGND_MODE		"InvBackgroundMode"
#define REG_STATSBGND_MODE		"StatsBackgroundMode"
#define REG_PICTURE_STRETCH		"PictureStretchLimit"
#define REG_LOADING_SCREENS		"LoadingScreens"
#define REG_CUSTOM_WATER_COLOR	"CustomWaterColor"
#define REG_SHADOW_MODE			"ShadowMode"
#define REG_REFLECTION_MODE		"ReflectionMode"
#define REG_REFLECTION_BLUR		"ReflectionBlur"
#define REG_ALPHABLEND_MODE		"AlphaBlendMode"
#define REG_INVTEXTBOX_MODE		"InvTextBoxMode"
#define REG_HEALTHBAR_MODE		"HealthBarMode"
#define REG_PICKUPITEM_MODE		"PickupItemMode"
#define REG_SAVEGAME_SLOTS		"SavegameSlots"
#define REG_SCREENSHOT_FORMAT	"ScreenshotFormat"
#define REG_JOYSTICK_MOVEMENT	"JoystickMovement"

// BOOL value names
#define REG_PERSPECTIVE			"PerspectiveCorrect"
#define REG_DITHER				"Dither"
#define REG_ZBUFFER				"ZBuffer"
#define REG_BILINEAR			"BilinearFiltering"
#define REG_TRIPLEBUFFER		"TripleBuffering"
#define REG_FULLSCREEN			"FullScreen"
#define REG_SOUND_ENABLE		"SoundEnabled"
#define REG_LARA_MIC			"LaraMic"
#define REG_JOY_ENABLE			"JoystickEnabled"
#define REG_16BIT_DISABLE		"Disable16BitTextures"
#define REG_SORT_DISABLE		"DontSortPrimitives"
#define REG_FLIP_BROKEN			"FlipBroken"
#define REG_FMV_DISABLE			"DisableFMV"
#define REG_PSXBARPOS_ENABLE	"EnablePsxBarPos"
#define REG_PSXFOV_ENABLE		"EnablePsxFov"
#define REG_BAREFOOT_SFX_ENABLE	"BarefootSFX"
#define REG_REMASTER_PIX_ENABLE	"RemasteredPictures"
#define REG_WALK_TO_SIDESTEP	"WalkToSidestep"

// FLOAT value names
#define REG_GAME_SIZER		"Sizer"
#define REG_INV_MUSIC_MUTE	"InvMusicMute"
#define REG_UW_MUSIC_MUTE	"UwMusicMute"
#define REG_DRAW_DISTANCE	"DrawDistance"
#define REG_FOG_BEGIN		"FogBegin"
#define REG_FOG_END			"FogEnd"
#define REG_UW_FOG_BEGIN	"UwFogBegin"
#define REG_UW_FOG_END		"UwFogEnd"
#define REG_GAME_GUI_SCALE	"GameGUIScale"
#define REG_INV_GUI_SCALE	"InvGUIScale"

// BINARY value names
#define REG_GAME_LAYOUT		"Layout"
#define REG_GAME_ASSAULT	"Assault"

// STRING value names
#define REG_SCREENSHOT_PATH	"ScreenshotPath"
#define REG_PICTURE_SUFFIX	"PictureSuffix"

// GUID string size
#define GUID_STRING_SIZE (sizeof("{00112233-4455-6677-8899AABBCCDDEEFF}"))

/*
 * Function list
 */
LPCTSTR __cdecl GuidBinaryToString(GUID *guid); // 0x00456A20
bool __cdecl GuidStringToBinary(LPCTSTR lpString, GUID *guid); // 0x00456A80
BOOL __cdecl OpenRegistryKey(LPCTSTR lpSubKey); // 0x00456B30
bool __cdecl IsNewRegistryKeyCreated(); // 0x00456B60
LONG __cdecl CloseRegistryKey(); // 0x00456B70
LONG __cdecl SetRegistryDwordValue(LPCTSTR lpValueName, DWORD value); // 0x00456B80
LONG __cdecl SetRegistryBoolValue(LPCTSTR lpValueName, bool value); // 0x00456BA0
LONG __cdecl SetRegistryFloatValue(LPCTSTR lpValueName, double value); // 0x00456BD0
LONG __cdecl SetRegistryBinaryValue(LPCTSTR lpValueName, LPBYTE value, DWORD valueSize); // 0x00456C10
LONG __cdecl SetRegistryStringValue(LPCTSTR lpValueName, LPCTSTR value, int length); // 0x00456C50
LONG __cdecl DeleteRegistryValue(LPCTSTR lpValueName); // 0x00456CA0
bool __cdecl GetRegistryDwordValue(LPCTSTR lpValueName, DWORD *pValue, DWORD defaultValue); // 0x00456CC0
bool __cdecl GetRegistryBoolValue(LPCTSTR lpValueName, bool *pValue, bool defaultValue); // 0x00456D20
bool __cdecl GetRegistryFloatValue(LPCTSTR lpValueName, double *value, double defaultValue); // 0x00456DA0
bool __cdecl GetRegistryBinaryValue(LPCTSTR lpValueName, LPBYTE value, DWORD valueSize, LPBYTE defaultValue); // 0x00456E00
bool __cdecl GetRegistryStringValue(LPCTSTR lpValueName, LPTSTR value, DWORD maxSize, LPCTSTR defaultValue); // 0x00456E80
bool __cdecl GetRegistryGuidValue(LPCTSTR lpValueName, GUID *value, GUID *defaultValue); // 0x00456F20

#endif // REGISTRY_H_INCLUDED
