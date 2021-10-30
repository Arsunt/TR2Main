/*
 * Copyright (c) 2017-2020 Michael Chaban. All rights reserved.
 * Original game is created by Core Design Ltd. in 1997.
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

#ifndef WINVID_H_INCLUDED
#define WINVID_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
#if (DIRECT3D_VERSION < 0x900)
bool __cdecl DDrawCreate(LPGUID lpGUID); // 0x00444C80
void __cdecl DDrawRelease(); // 0x00444CE0
#endif // (DIRECT3D_VERSION < 0x900)
void __cdecl GameWindowCalculateSizeFromClient(int *width, int *height); // 0x00444D20
void __cdecl GameWindowCalculateSizeFromClientByZero(int *width, int *height); // 0x00444DA0
void __cdecl WinVidSetMinWindowSize(int width, int height); // 0x00444E10
void __cdecl WinVidClearMinWindowSize(); // 0x00444E60
void __cdecl WinVidSetMaxWindowSize(int width, int height); // 0x00444E70
void __cdecl WinVidClearMaxWindowSize(); // 0x00444EC0
int __cdecl CalculateWindowWidth(int width, int height); // 0x00444ED0
int __cdecl CalculateWindowHeight(int width, int height); // 0x00444F20
bool __cdecl WinVidGetMinMaxInfo(LPMINMAXINFO info); // 0x00444F50
HWND __cdecl WinVidFindGameWindow(); // 0x00445060
bool __cdecl WinVidSpinMessageLoop(bool needWait); // 0x00445080
void __cdecl WinVidShowGameWindow(int nCmdShow); // 0x00445170
void __cdecl WinVidHideGameWindow(); // 0x004451C0
void __cdecl WinVidSetGameWindowSize(int width, int height); // 0x00445200
#if (DIRECT3D_VERSION < 0x900)
bool __cdecl ShowDDrawGameWindow(bool active); // 0x00445240
bool __cdecl HideDDrawGameWindow(); // 0x004452F0
HRESULT __cdecl DDrawSurfaceCreate(LPDDSDESC dsp, LPDDS *surface); // 0x00445380
HRESULT __cdecl DDrawSurfaceRestoreLost(LPDDS surface1, LPDDS surface2, bool blank); // 0x004453D0
bool __cdecl WinVidClearBuffer(LPDDS surface, LPRECT rect, DWORD fillColor); // 0x00445420
HRESULT __cdecl WinVidBufferLock(LPDDS surface, LPDDSDESC desc, DWORD flags); // 0x00445470
HRESULT __cdecl WinVidBufferUnlock(LPDDS surface, LPDDSDESC desc); // 0x004454B0
bool __cdecl WinVidCopyBitmapToBuffer(LPDDS surface, BYTE *bitmap); // 0x004454E0
DWORD __cdecl GetRenderBitDepth(DWORD dwRGBBitCount); // 0x00445570
void __thiscall WinVidGetColorBitMasks(COLOR_BIT_MASKS *bm, LPDDPIXELFORMAT pixelFormat); // 0x00445600
void __cdecl BitMaskGetNumberOfBits(DWORD bitMask, DWORD *bitDepth, DWORD *bitOffset); // 0x00445680
DWORD __cdecl CalculateCompatibleColor(COLOR_BIT_MASKS *mask, int red, int green, int blue, int alpha); // 0x004456D0
#endif // (DIRECT3D_VERSION < 0x900)
bool __cdecl WinVidGetDisplayMode(DISPLAY_MODE *dispMode); // 0x00445740
bool __cdecl WinVidGoFullScreen(DISPLAY_MODE *dispMode); // 0x004457D0
bool __cdecl WinVidGoWindowed(int width, int height, DISPLAY_MODE *dispMode); // 0x00445860
void __cdecl WinVidSetDisplayAdapter(DISPLAY_ADAPTER *dispAdapter); // 0x00445970
bool __thiscall CompareVideoModes(DISPLAY_MODE *mode1, DISPLAY_MODE *mode2); // 0x00445A50
bool __cdecl WinVidGetDisplayModes(); // 0x00445AA0
#if (DIRECT3D_VERSION < 0x900)
HRESULT WINAPI EnumDisplayModesCallback(LPDDSDESC lpDDSurfaceDesc, LPVOID lpContext); // 0x00445B00
#endif // (DIRECT3D_VERSION < 0x900)
bool __cdecl WinVidInit(); // 0x00445EC0
bool __cdecl WinVidGetDisplayAdapters(); // 0x00445F00
void __thiscall FlaggedStringDelete(STRING_FLAGGED *item); // 0x00445FB0
bool __cdecl EnumerateDisplayAdapters(DISPLAY_ADAPTER_LIST *displayAdapterList); // 0x00445FD0
#if (DIRECT3D_VERSION < 0x900)
BOOL WINAPI EnumDisplayAdaptersCallback(GUID FAR *lpGUID, LPTSTR lpDriverDescription, LPTSTR lpDriverName, LPVOID lpContext); // 0x00445FF0
#endif // (DIRECT3D_VERSION < 0x900)
void __thiscall FlaggedStringsCreate(DISPLAY_ADAPTER *adapter); // 0x004461B0
bool __cdecl WinVidRegisterGameWindowClass(); // 0x004461F0
LRESULT CALLBACK WinVidGameWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam); // 0x00446260
void __cdecl WinVidResizeGameWindow(HWND hWnd, int edge, LPRECT rect); // 0x00446870
bool __cdecl WinVidCheckGameWindowPalette(HWND hWnd); // 0x00446A50
bool __cdecl WinVidCreateGameWindow(); // 0x00446B10
void __cdecl WinVidFreeWindow(); // 0x00446BE0
void __cdecl WinVidExitMessage(); // 0x00446C10
DISPLAY_ADAPTER_NODE *__cdecl WinVidGetDisplayAdapter(GUID *lpGuid); // 0x00446C60
void __cdecl WinVidStart(); // 0x00446CB0
void __cdecl WinVidFinish(); // 0x00447030
void __thiscall DisplayModeListInit(DISPLAY_MODE_LIST *pList); // 0x00447050
void __thiscall DisplayModeListDelete(DISPLAY_MODE_LIST *pList); // 0x00447060
DISPLAY_MODE *__thiscall InsertDisplayMode(DISPLAY_MODE_LIST *modeList, DISPLAY_MODE_NODE *before); // 0x004470A0
DISPLAY_MODE *__thiscall InsertDisplayModeInListHead(DISPLAY_MODE_LIST *modeList); // 0x004470C0
DISPLAY_MODE *__thiscall InsertDisplayModeInListTail(DISPLAY_MODE_LIST *modeList); // 0x00447110

#endif // WINVID_H_INCLUDED
