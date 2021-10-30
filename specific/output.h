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

#ifndef OUTPUT_H_INCLUDED
#define OUTPUT_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
int __cdecl GetRenderScale(int unit);
int __cdecl GetRenderHeightDownscaled();
int __cdecl GetRenderWidthDownscaled();
int __cdecl GetRenderHeight(); // 0x00450BA0
int __cdecl GetRenderWidth(); // 0x00450BB0
void __cdecl S_InitialisePolyList(BOOL clearBackBuffer); // 0x00450BC0
DWORD __cdecl S_DumpScreen(); // 0x00450CB0
void __cdecl S_ClearScreen(); // 0x00450CF0
void __cdecl S_InitialiseScreen(GF_LEVEL_TYPE levelType); // 0x00450D00
void __cdecl S_OutputPolyList(); // 0x00450D40
int __cdecl S_GetObjectBounds(__int16 *bPtr); // 0x00450D80
void __cdecl S_InsertBackPolygon(int x0, int y0, int x1, int y1); // 0x00450FF0
void __cdecl S_PrintShadow(__int16 radius, __int16 *bPtr, ITEM_INFO *item); // 0x00451040
void __cdecl S_CalculateLight(int x, int y, int z, __int16 roomNumber); // 0x00451240
void __cdecl S_CalculateStaticLight(__int16 adder); // 0x00451540
void __cdecl S_CalculateStaticMeshLight(int x, int y, int z, int shade1, int shade2, ROOM_INFO *room); // 0x00451580
void __cdecl S_LightRoom(ROOM_INFO *room); // 0x004516B0
void __cdecl S_DrawHealthBar(int percent); // 0x004518C0
void __cdecl S_DrawAirBar(int percent); // 0x00451A90
void __cdecl AnimateTextures(int nTicks); // 0x00451C90
void __cdecl S_SetupBelowWater(BOOL underwater); // 0x00451D50
void __cdecl S_SetupAboveWater(BOOL underwater); // 0x00451DB0
void __cdecl S_AnimateTextures(int nTicks); // 0x00451DE0
void __cdecl S_DisplayPicture(LPCTSTR fileName, BOOL reallocGame); // 0x00451EA0
void __cdecl S_SyncPictureBufferPalette(); // 0x00451FB0
void __cdecl S_DontDisplayPicture(); // 0x00452030
void __cdecl ScreenDump(); // 0x00452040
void __cdecl ScreenPartialDump(); // 0x00452050
void __cdecl FadeToPal(int fadeValue, RGB888 *palette); // 0x00452060
void __cdecl ScreenClear(bool isPhdWinSize); // 0x00452230
void __cdecl S_CopyScreenToBuffer(); // 0x00452260
void __cdecl S_CopyBufferToScreen(); // 0x00452310
BOOL __cdecl DecompPCX(LPCBYTE pcx, DWORD pcxSize, LPBYTE pic, RGB888 *pal); // 0x00452360

// NOTE: this function is not presented in the original game
int GetPcxResolution(LPCBYTE pcx, DWORD pcxSize, DWORD *width, DWORD *height);

#endif // OUTPUT_H_INCLUDED
