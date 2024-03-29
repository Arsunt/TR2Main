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

#ifndef GDI_UTILS_H_INCLUDED
#define GDI_UTILS_H_INCLUDED

#include "global/types.h"

typedef enum {
	GDI_BMP,
	GDI_JPG,
	GDI_PNG,
} GDI_FILEFMT;

/*
 * Function list
 */
HBITMAP CreateBitmapFromDC(HDC dc, RECT *rect, LPVOID *lpBits, PALETTEENTRY *pal);

bool __cdecl GDI_Init();

void __cdecl GDI_Cleanup();

int GDI_SaveImageFile(LPCSTR filename, GDI_FILEFMT format, DWORD quality, HBITMAP hbmBitmap);

int GDI_LoadImageBitmap(HBITMAP hbmBitmap, BYTE **bmPtr, DWORD *width, DWORD *height, DWORD bpp);

int GDI_LoadImageFile(LPCSTR filename, BYTE **bmPtr, DWORD *width, DWORD *height, DWORD bpp);

#endif // GDI_UTILS_H_INCLUDED
