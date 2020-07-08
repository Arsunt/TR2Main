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

#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
DWORD __cdecl SyncTicks(DWORD skip); // NOTE: this function is not presented in the original game
void __cdecl Ticks(); // 0x00456680
bool __cdecl TIME_Init(); // 0x004566C0
DWORD __cdecl Sync(); // 0x00456720
LPVOID __cdecl UT_LoadResource(LPCTSTR lpName, LPCTSTR lpType); // 0x00456780
void __cdecl UT_InitAccurateTimer(); // 0x004567C0
double __cdecl UT_Microseconds(); // 0x00456820
BOOL __cdecl UT_CenterWindow(HWND hWnd); // 0x00456870
LPTSTR __cdecl UT_FindArg(LPCTSTR str); // 0x004568E0
int __cdecl UT_MessageBox(LPCTSTR lpText, HWND hWnd); // 0x00456910
int __cdecl UT_ErrorBox(UINT uID, HWND hWnd); // 0x00456930
BOOL __cdecl CD_NoteAlert(LPCTSTR lpTemplateName, HWND hWndParent); // 0x00456980
INT_PTR CALLBACK DialogBoxProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam); // 0x004569B0
void __cdecl UT_MemBlt(BYTE *dstBuf, DWORD dstX, DWORD dstY, DWORD width, DWORD height, DWORD dstPitch,
					   BYTE *srcBuf, DWORD srcX, DWORD srcY, DWORD srcPitch); // 0x004574A0
#endif // UTILS_H_INCLUDED
