/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
 * Original game is created by Core Design Ltd. in 1997.
 * Lara Croft and Tomb Raider are trademarks of Embracer Group AB.
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

#ifndef SETUPDLG_H_INCLUDED
#define SETUPDLG_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
bool __cdecl OpenGameRegistryKey(LPCTSTR key); // 0x00452480
LONG __cdecl CloseGameRegistryKey(); // 0x004524D0
bool __cdecl SE_WriteAppSettings(APP_SETTINGS *settings); // 0x004524E0
int __cdecl SE_ReadAppSettings(APP_SETTINGS *settings); // 0x00452760
bool __cdecl SE_GraphicsTestStart(); // 0x00452AC0
void __cdecl SE_GraphicsTestFinish(); // 0x00452B90
int __cdecl SE_GraphicsTestExecute(); // 0x00452BB0
int __cdecl SE_GraphicsTest(); // 0x00452BC0
void __cdecl SE_DefaultGraphicsSettings(); // 0x00452C20
bool __cdecl SE_SoundTestStart(); // 0x00452D70
void __cdecl SE_SoundTestFinish(); // 0x00452E30
int __cdecl SE_SoundTestExecute(); // 0x00452E40
int __cdecl SE_SoundTest(); // 0x00452E80
int CALLBACK SE_PropSheetCallback(HWND hwndDlg, UINT uMsg, LPARAM lParam); // 0x00452EE0
LRESULT CALLBACK SE_NewPropSheetWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // 0x00452F20
bool __cdecl SE_ShowSetupDialog(HWND hParent, bool isDefault); // 0x00452F80
INT_PTR CALLBACK SE_GraphicsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); // 0x00453150
void __cdecl SE_GraphicsDlgFullScreenModesUpdate(HWND hwndDlg); // 0x00453560
void __cdecl SE_GraphicsAdapterSet(HWND hwndDlg, DISPLAY_ADAPTER_NODE *adapter); // 0x00453750
void __cdecl SE_GraphicsDlgUpdate(HWND hwndDlg); // 0x00453770
void __cdecl SE_GraphicsDlgInit(HWND hwndDlg); // 0x00453EB0
INT_PTR CALLBACK SE_SoundDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); // 0x00454030
void __cdecl SE_SoundAdapterSet(HWND hwndDlg, SOUND_ADAPTER_NODE *adapter); // 0x004541C0
void __cdecl SE_SoundDlgUpdate(HWND hwndDlg); // 0x004541D0
void __cdecl SE_SoundDlgInit(HWND hwndDlg); // 0x004542F0
INT_PTR CALLBACK SE_ControlsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); // 0x004543B0
void __cdecl SE_ControlsJoystickSet(HWND hwndDlg, JOYSTICK_NODE *joystick); // 0x004544C0
void __cdecl SE_ControlsDlgUpdate(HWND hwndDlg); // 0x004544D0
void __cdecl SE_ControlsDlgInit(HWND hwndDlg); // 0x00454540
INT_PTR CALLBACK SE_OptionsDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); // 0x00454600
void __cdecl SE_OptionsDlgUpdate(HWND hwndDlg); // 0x00454690
void __cdecl SE_OptionsStrCat(LPTSTR *dstString, bool isEnabled, bool *isNext, const LPCTSTR srcString); // 0x004548D0
INT_PTR CALLBACK SE_AdvancedDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam); // 0x00454920
void __cdecl SE_AdvancedDlgUpdate(HWND hwndDlg); // 0x00454A30
void __cdecl SE_AdvancedDlgInit(HWND hwndDlg); // 0x00454AE0
HWND __cdecl SE_FindSetupDialog(); // 0x00454AF0

#endif // SETUPDLG_H_INCLUDED
