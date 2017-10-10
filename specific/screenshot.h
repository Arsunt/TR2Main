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

#ifndef SCREENSHOT_H_INCLUDED
#define SCREENSHOT_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl ScreenShotPCX(); // 0x0044E9A0
DWORD __cdecl CompPCX(BYTE *bitmap, DWORD width, DWORD height, RGB *palette, BYTE **pcxData); // 0x0044EAB0
DWORD __cdecl EncodeLinePCX(BYTE *src, DWORD width, BYTE *dst); // 0x0044EB60
DWORD __cdecl EncodePutPCX(BYTE value, BYTE num, BYTE *buffer); // 0x0044EC40
void __cdecl ScreenShot(LPDIRECTDRAWSURFACE3 screen); // 0x0044EC80

#endif // SCREENSHOT_H_INCLUDED
