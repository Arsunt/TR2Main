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

#ifndef SETUPWND_H_INCLUDED
#define SETUPWND_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __thiscall SE_ReleaseBitmapResource(BITMAP_RESOURCE *bmpRsrc); // 0x00456FC0
void __thiscall SE_LoadBitmapResource(BITMAP_RESOURCE *bmpRsrc, LPCTSTR lpName); // 0x00457000
void __thiscall SE_DrawBitmap(BITMAP_RESOURCE *bmpRsrc, HDC hdc, int x, int y); // 0x004570D0
void __thiscall SE_UpdateBitmapPalette(BITMAP_RESOURCE *bmpRsrc, HWND hWnd, HWND hSender); // 0x00457140
void __thiscall SE_ChangeBitmapPalette(BITMAP_RESOURCE *bmpRsrc, HWND hWnd); // 0x00457160
bool __cdecl SE_RegisterSetupWindowClass(); // 0x004571C0
LRESULT CALLBACK SE_SetupWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam); // 0x00457230
void __cdecl SE_PassMessageToImage(HWND hWnd, UINT uMsg, WPARAM wParam); // 0x00457470

#endif // SETUPWND_H_INCLUDED
