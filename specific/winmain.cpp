/*
 * Copyright (c) 2017 Michael Chaban. All rights reserved.
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
#include "specific/winmain.h"
#include "specific/background.h"
#include "specific/hwr.h"
#include "specific/init.h"
#include "specific/init_3d.h"
#include "specific/init_display.h"
#include "specific/init_input.h"
#include "specific/init_sound.h"
#include "specific/setupdlg.h"
#include "specific/smain.h"
#include "specific/sndpc.h"
#include "specific/texture.h"
#include "specific/utils.h"
#include "specific/winvid.h"
#include "global/resource.h"
#include "global/vars.h"

#if defined(_MSC_VER)
#include <se.h>

static void SEH_TR(unsigned int error, EXCEPTION_POINTERS* pExp) {
	throw error;
}
#endif // _MSC_VER

int __cdecl RenderErrorBox(int errorCode) {
	char errorText[128];
	LPCTSTR errorMessage = DecodeErrorMessage(errorCode);
	wsprintf(errorText, "Render init failed with \"%s\"", errorMessage);
	return UT_MessageBox(errorText, 0);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nShowCmd) {
	HWND gameWindow, setupDialog;
	int rc, initStatus, appSettingsStatus;
	bool isSetupRequested;

	GameModule = hInstance;
	CommandLinePtr = lpCmdLine;

	// If another game instance is already run, show its window
	gameWindow = WinVidFindGameWindow();
	if( gameWindow != NULL ) {
		setupDialog = SE_FindSetupDialog();
		SetForegroundWindow(setupDialog ? setupDialog : gameWindow);
		exit(0); // NOTE: there may be bugs on some systems if we just return here
	}

//	TODO: remove log stuff at release
//	fflush(stderr);
//	freopen("../TR2Main.log", "w", stderr);

#if defined(_MSC_VER)
	_set_se_translator(SEH_TR);
#endif // _MSC_VER

	try {
		isSetupRequested = ( UT_FindArg("setup") != NULL );

		initStatus = Init(isSetupRequested);
		if( initStatus == 0 )
			UT_ErrorBox(IDS_DX5_REQUIRED, NULL); // "Tomb Raider II requires Microsoft DirectX 5 to be installed."
		if( initStatus != 1 )
			goto EXIT;

		appSettingsStatus = SE_ReadAppSettings(&SavedAppSettings);

		if( appSettingsStatus == 0 )
			goto EXIT;

		if( isSetupRequested || appSettingsStatus == 2) {
			if( !SE_ShowSetupDialog(NULL, appSettingsStatus == 2) )
				goto EXIT;
			SE_WriteAppSettings(&SavedAppSettings);
			if( isSetupRequested )
				goto EXIT;
		}

		while( 0 != (rc = WinGameStart()) ) {
			WinGameFinish();
			RenderErrorBox(rc);

			if( !SE_ShowSetupDialog(NULL, false) )
				goto EXIT;
			SE_WriteAppSettings(&SavedAppSettings);
		}

		StopInventory = false;
		IsGameToExit = false;
		GameMain();

		WinGameFinish();
		SE_WriteAppSettings(&SavedAppSettings);

	} catch(...) {
		WinGameFinish();
		WinCleanup();
		abort();
	}
EXIT :
	WinCleanup();
	exit(AppResultCode); // NOTE: there may be bugs on some systems if we just return here
}

int __cdecl Init(bool skipCDInit) {
	if( !skipCDInit && !CD_Init() )
		return 2;

	UT_InitAccurateTimer();

	if( WinVidInit() &&
		Direct3DInit() &&
		RenderInit() &&
		InitTextures() &&
		WinSndInit() &&
		WinInputInit() &&
		TIME_Init() &&
		HWR_Init() &&
		BGND_Init() )
	{
		return 1;
	}
	return 0;
}

void __cdecl WinCleanup() {
	WinVidFreeWindow();
	CD_Cleanup();
}

int __cdecl WinGameStart() {
	try {
		WinVidStart();
		RenderStart(true);
		WinSndStart(NULL);
		WinInStart();
	} catch(int error) {
		return error;
	}
	return 0;
}

void __cdecl WinGameFinish() {
	WinInFinish();
	WinSndFinish();
	RenderFinish(true);
	WinVidFinish();
	WinVidHideGameWindow();
	if( *StringToShow )
		MessageBox(NULL, StringToShow, NULL, MB_ICONWARNING);
}

void __cdecl S_ExitSystem(LPCTSTR message) {
	ShutdownGame();
	lstrcpy(StringToShow, message);
	WinGameFinish();
	WinCleanup();
	exit(1); // the app is terminated here
}

/*
 * Inject function
 */
void Inject_WinMain() {
	INJECT(0x0044E5A0, RenderErrorBox);
	INJECT(0x0044E5E0, WinMain);
	INJECT(0x0044E7B0, Init);
	INJECT(0x0044E830, WinCleanup);
	INJECT(0x0044E860, WinGameStart);
	INJECT(0x0044E8E0, WinGameFinish);
	INJECT(0x0044E950, S_ExitSystem);
}
