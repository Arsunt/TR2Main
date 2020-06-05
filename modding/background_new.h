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

#ifndef BACKGROUND_NEW_H_INCLUDED
#define BACKGROUND_NEW_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void PSX_Background(HWR_TEXHANDLE texSource, int tu, int tv, int t_width, int t_height, int halfRowCount,
					__int16 amplitude, __int16 deformWavePhase, __int16 shortWavePhase, __int16 longWavePhase);

int __cdecl BGND2_FadeTo(int target, int delta);

int __cdecl BGND2_CapturePicture();

int __cdecl BGND2_LoadPicture(LPCTSTR fileName, BOOL isTitle, BOOL isReload);

int __cdecl BGND2_ShowPicture(DWORD fadeIn, DWORD waitIn, DWORD fadeOut, DWORD waitOut, BOOL inputCheck);

void __cdecl BGND2_DrawTextures(RECT *rect, D3DCOLOR color);

int __cdecl BGND2_CalculatePictureRect(RECT *rect);

#endif // BACKGROUND_NEW_H_INCLUDED
