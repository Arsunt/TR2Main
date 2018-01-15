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

/**
 * @file
 * @brief TR2Main interface
 *
 * This file implements TR2Main.DLL exported functions and interfaces
 */

/**
 * @defgroup TR2MAIN TR2Main
 * @brief TR2Main interface
 *
 * This module contains TR2Main.DLL exported functions and interfaces
 *
 * @{
 */

#include <windows.h>

/** @cond Doxygen_Suppress */
#ifdef BUILD_DLL
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT __declspec(dllimport)
#endif
/** @endcond */

// 3d system
extern void Inject_3Dgen();
extern void Inject_3Dout();
extern void Inject_3Dinsert();
extern void Inject_PhdMath();
extern void Inject_ScaleSpr();
// game
extern void Inject_Draw();
extern void Inject_Gameflow();
extern void Inject_Health();
extern void Inject_Inventory();
extern void Inject_InvFunc();
extern void Inject_InvText();
extern void Inject_Items();
extern void Inject_Lara();
extern void Inject_LaraMisc();
extern void Inject_SaveGame();
extern void Inject_Setup();
extern void Inject_Sound();
extern void Inject_Text();
//specific
extern void Inject_Background();
extern void Inject_Display();
extern void Inject_File();
extern void Inject_Fmv();
extern void Inject_Frontend();
extern void Inject_HWR();
extern void Inject_Init();
extern void Inject_Init3d();
extern void Inject_InitDisplay();
extern void Inject_InitInput();
extern void Inject_InitSound();
extern void Inject_Input();
extern void Inject_Option();
extern void Inject_Output();
extern void Inject_Registry();
extern void Inject_Screenshot();
extern void Inject_SetupDlg();
extern void Inject_SetupWnd();
extern void Inject_SMain();
extern void Inject_SndPC();
extern void Inject_Texture();
extern void Inject_Utils();
extern void Inject_WinMain();
extern void Inject_WinVid();

static void Inject() {
// 3d system
	Inject_3Dgen();
	Inject_3Dout();
	Inject_3Dinsert();
	Inject_PhdMath();
	Inject_ScaleSpr();
// game
	Inject_Draw();
	Inject_Gameflow();
	Inject_Health();
	Inject_Inventory();
	Inject_InvFunc();
	Inject_InvText();
	Inject_Items();
	Inject_Lara();
	Inject_LaraMisc();
	Inject_SaveGame();
	Inject_Setup();
	Inject_Sound();
	Inject_Text();
//specific
	Inject_Background();
	Inject_Display();
	Inject_File();
	Inject_Fmv();
	Inject_Frontend();
	Inject_HWR();
	Inject_Init();
	Inject_Init3d();
	Inject_InitDisplay();
	Inject_InitInput();
	Inject_InitSound();
	Inject_Input();
	Inject_Option();
	Inject_Output();
	Inject_Registry();
	Inject_Screenshot();
	Inject_SetupDlg();
	Inject_SetupWnd();
	Inject_SMain();
	Inject_SndPC();
	Inject_Texture();
	Inject_Utils();
	Inject_WinMain();
	Inject_WinVid();
}

extern "C" DLL_EXPORT int DummyFunction() {
	return 0;
}

/**
 * An optional entry point into a dynamic-link library (DLL)
 * @param[in] hinstDLL A handle to the DLL module
 * @param[in] fdwReason The reason code that indicates why the DLL
 * entry-point function is being called
 * @param[in] lpvReserved used if fdwReason is DLL_PROCESS_ATTACH or DLL_PROCESS_DETACH
 * @return TRUE if it succeeds or FALSE if it fails
 * @note See the MSDN for more information
 */
extern "C" BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	switch( fdwReason ) {
		case DLL_PROCESS_ATTACH :
			// attach to process
			Inject();
			break;

		case DLL_PROCESS_DETACH :
			// detach from process
			break;

		case DLL_THREAD_ATTACH :
			// attach to thread
			break;

		case DLL_THREAD_DETACH :
			// detach from thread
			break;
	}
	return TRUE; // successful
}

/** @} */
