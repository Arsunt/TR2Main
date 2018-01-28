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
#include "specific/fmv.h"
#include "specific/file.h"
#include "specific/init_display.h"
#include "specific/input.h"
#include "specific/sndpc.h"
#include "global/vars.h"

// GetProcAddress Macro
#define GET_DLL_PROC(dll, proc) { \
	*(FARPROC *)&(proc) = GetProcAddress((dll), #proc); \
	if( proc == NULL ) throw #proc; \
}

// FMV Player DLL name
#define FMV_PLAYER_DLL_NAME "winplay.dll"

// FMV Player
static HMODULE hPlayer = NULL;

// Imports from winplay.dll
static int (__cdecl *Movie_GetCurrentFrame)(LPVOID);
static int (__cdecl *Movie_GetFormat)(LPVOID);
static int (__cdecl *Movie_GetSoundChannels)(LPVOID);
static int (__cdecl *Movie_GetSoundPrecision)(LPVOID);
static int (__cdecl *Movie_GetSoundRate)(LPVOID);
static int (__cdecl *Movie_GetTotalFrames)(LPVOID);
static int (__cdecl *Movie_GetXSize)(LPVOID);
static int (__cdecl *Movie_GetYSize)(LPVOID);
static int (__cdecl *Movie_SetSyncAdjust)(LPVOID, LPVOID, DWORD);
static int (__cdecl *Player_BlankScreen)(DWORD, DWORD, DWORD, DWORD);
static int (__cdecl *Player_GetDSErrorCode)();
static int (__cdecl *Player_InitMovie)(LPVOID, DWORD, DWORD, LPCTSTR, DWORD);
static int (__cdecl *Player_InitMoviePlayback)(LPVOID, LPVOID, LPVOID);
static int (__cdecl *Player_InitPlaybackMode)(HWND, LPVOID, DWORD, DWORD);
static int (__cdecl *Player_InitSound)(LPVOID, DWORD, DWORD, BOOL, DWORD, DWORD, DWORD, DWORD, DWORD);
static int (__cdecl *Player_InitSoundSystem)(HWND);
static int (__cdecl *Player_InitVideo)(LPVOID, LPVOID, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD);
static int (__cdecl *Player_PassInDirectDrawObject)(LPDIRECTDRAW2);
static int (__cdecl *Player_PlayFrame)(LPVOID, LPVOID, LPVOID, DWORD, LPRECT, DWORD, DWORD, DWORD);
static int (__cdecl *Player_ReturnPlaybackMode)(BOOL);
static int (__cdecl *Player_ShutDownMovie)(LPVOID);
static int (__cdecl *Player_ShutDownSound)(LPVOID);
static int (__cdecl *Player_ShutDownSoundSystem)();
static int (__cdecl *Player_ShutDownVideo)(LPVOID);
static int (__cdecl *Player_StartTimer)(LPVOID);
static int (__cdecl *Player_StopTimer)(LPVOID);


static bool FmvPlayerInit() {
	static int initStatus = 0;

	if( initStatus > 0 )
		return true;
	else if( initStatus < 0 )
		return false;

	hPlayer = GetModuleHandle(FMV_PLAYER_DLL_NAME);
	if( hPlayer == NULL ) {
		// failed to load DLL
		initStatus = -1;
		return false;
	}

	try {
		GET_DLL_PROC(hPlayer, Movie_GetCurrentFrame);
		GET_DLL_PROC(hPlayer, Movie_GetFormat);
		GET_DLL_PROC(hPlayer, Movie_GetSoundChannels);
		GET_DLL_PROC(hPlayer, Movie_GetSoundPrecision);
		GET_DLL_PROC(hPlayer, Movie_GetSoundRate);
		GET_DLL_PROC(hPlayer, Movie_GetTotalFrames);
		GET_DLL_PROC(hPlayer, Movie_GetXSize);
		GET_DLL_PROC(hPlayer, Movie_GetYSize);
		GET_DLL_PROC(hPlayer, Movie_SetSyncAdjust);
		GET_DLL_PROC(hPlayer, Player_BlankScreen);
		GET_DLL_PROC(hPlayer, Player_GetDSErrorCode);
		GET_DLL_PROC(hPlayer, Player_InitMovie);
		GET_DLL_PROC(hPlayer, Player_InitMoviePlayback);
		GET_DLL_PROC(hPlayer, Player_InitPlaybackMode);
		GET_DLL_PROC(hPlayer, Player_InitSound);
		GET_DLL_PROC(hPlayer, Player_InitSoundSystem);
		GET_DLL_PROC(hPlayer, Player_InitVideo);
		GET_DLL_PROC(hPlayer, Player_PassInDirectDrawObject);
		GET_DLL_PROC(hPlayer, Player_PlayFrame);
		GET_DLL_PROC(hPlayer, Player_ReturnPlaybackMode);
		GET_DLL_PROC(hPlayer, Player_ShutDownMovie);
		GET_DLL_PROC(hPlayer, Player_ShutDownSound);
		GET_DLL_PROC(hPlayer, Player_ShutDownSoundSystem);
		GET_DLL_PROC(hPlayer, Player_ShutDownVideo);
		GET_DLL_PROC(hPlayer, Player_StartTimer);
		GET_DLL_PROC(hPlayer, Player_StopTimer);
	} catch (LPCTSTR procName) {
		// failed to load one of the procs
		initStatus = -1;
		return false;
	}

	initStatus = 1;
    return true;
}

bool __cdecl PlayFMV(LPCTSTR fileName) {
	LPCTSTR fullPath;

	if( SavedAppSettings.DisableFMV || !FmvPlayerInit() )
		return IsGameToExit;

	S_CDStop();
	ShowCursor(FALSE);
	RenderFinish(true);
	IsFmvPlaying = TRUE;

	fullPath = GetFullPath(fileName);
	WinPlayFMV(fullPath, true);
	WinStopFMV(true);

	IsFmvPlaying = FALSE;
	if( !IsGameToExit )
		FmvBackToGame();
	ShowCursor(TRUE);

	return IsGameToExit;
}

void __cdecl WinPlayFMV(LPCTSTR fileName, bool isPlayback) {
	int xSize, ySize, xOffset, yOffset;
	int soundPrecision, soundRate, soundChannels, soundFormat;
	bool isUncompressed;
	RECT rect = {0, 0, 640, 480};

	if( 0 != Player_PassInDirectDrawObject(_DirectDraw2) ||
		0 != Player_InitMovie(&MovieContext, 0, 0, fileName, 0x200000) ||
		130 != Movie_GetFormat(MovieContext) )
	{
		return;
	}

	xSize = Movie_GetXSize(MovieContext); // RPL movie width is always 320
	ySize = Movie_GetYSize(MovieContext); // maximum possible value for RPL movie height is 240
	xOffset = 320 - xSize;
	yOffset = 240 - ySize;

	if( 0 != Player_InitVideo(&FmvContext, MovieContext, xSize, ySize, xOffset, yOffset, 0, 0, 640, 480, 0, 1, 13) ||
		(isPlayback && 0 != Player_InitPlaybackMode(HGameWindow, FmvContext, 1, 0)) )
	{
		return;
	}

	Player_BlankScreen(rect.left, rect.top, rect.right, rect.bottom);

	if( 0 != Player_InitSoundSystem(HGameWindow) ||
		Player_GetDSErrorCode() < 0 )
	{
		return;
	}

	soundPrecision = Movie_GetSoundPrecision(MovieContext);
	soundRate = Movie_GetSoundRate(MovieContext);
	soundChannels = Movie_GetSoundChannels(MovieContext);
	isUncompressed = ( soundPrecision != 4 );
	soundFormat = isUncompressed ? 1 : 4;
	if( 0 != Player_InitSound(&FmvSoundContext, 16384, soundFormat, isUncompressed, 4096, soundChannels, soundRate, soundPrecision, 2) )
	{
		return;
	}

	Movie_SetSyncAdjust(MovieContext, FmvSoundContext, 4);

	if( 0 != Player_InitMoviePlayback(MovieContext, FmvContext, FmvSoundContext) )
	{
		return;
	}

	S_UpdateInput(); // NOTE: should use WinVidSpinMessageLoop(false) instead
	Player_StartTimer(MovieContext);
	Player_BlankScreen(rect.left, rect.top, rect.right, rect.bottom);
	S_UpdateInput(); // NOTE: should use WinVidSpinMessageLoop(false) instead

	while( Movie_GetCurrentFrame(MovieContext) < Movie_GetTotalFrames(MovieContext) ) {
		if( 0 != Player_PlayFrame(MovieContext, FmvContext, FmvSoundContext, 0, &rect, 0, 0, 0) )
			return;
		if( S_UpdateInput() || CHK_ANY(InputStatus, IN_OPTION) )
			break;
	}
}

void __cdecl WinStopFMV(bool isPlayback)
{
	Player_StopTimer(MovieContext);
	Player_ShutDownSound(&FmvSoundContext);
	Player_ShutDownVideo(&FmvContext);
	Player_ShutDownMovie(&MovieContext);
	Player_ShutDownSoundSystem();
	if( isPlayback )
		Player_ReturnPlaybackMode(isPlayback);
}

bool __cdecl IntroFMV(LPCTSTR fileName1, LPCTSTR fileName2) {
	LPCTSTR fullPath;

	if( SavedAppSettings.DisableFMV || !FmvPlayerInit() )
		return IsGameToExit;

	ShowCursor(FALSE);
	RenderFinish(true);
	IsFmvPlaying = TRUE;

	fullPath = GetFullPath(fileName1);
	WinPlayFMV(fullPath, true);
	WinStopFMV(true);

	fullPath = GetFullPath(fileName2);
	WinPlayFMV(fullPath, true);
	WinStopFMV(true);

	IsFmvPlaying = FALSE;
	if( !IsGameToExit )
		FmvBackToGame();
	ShowCursor(TRUE);

	return IsGameToExit;
}

/*
 * Inject function
 */
void Inject_Fmv() {
	INJECT(0x0044BE50, PlayFMV);
	INJECT(0x0044BED0, WinPlayFMV);
	INJECT(0x0044C1B0, WinStopFMV);
	INJECT(0x0044C200, IntroFMV);
}
