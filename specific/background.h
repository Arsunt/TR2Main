/*
 * Copyright (c) 2017-2018 Michael Chaban. All rights reserved.
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

#ifndef BACKGROUND_H_INCLUDED
#define BACKGROUND_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl BGND_Make640x480(BYTE *bitmap, RGB *palette); // 0x00443A40
int __cdecl BGND_AddTexture(int tileIndex, BYTE *bitmap, int palIndex, RGB *bmpPal); // 0x00443C00
void __cdecl BGND_GetPageHandles(); // 0x00443CC0
void __cdecl BGND_DrawInGameBlack(); // 0x00443D00
void __cdecl DrawQuad(float sx, float sy, float width, float height, D3DCOLOR color); // 0x00443D60
void __cdecl BGND_DrawInGameBackground(); // 0x00443E40
void __cdecl DrawTextureTile(int sx, int sy, int width, int height, D3DTEXTUREHANDLE texSource,
							 int tu, int tv, int t_width, int t_height,
							 D3DCOLOR color0, D3DCOLOR color1, D3DCOLOR color2, D3DCOLOR color3); // 0x00444060
D3DCOLOR __cdecl BGND_CenterLighting(int x, int y, int width, int height); // 0x004442C0
void __cdecl BGND_Free(); // 0x00444570
bool __cdecl BGND_Init(); // 0x004445C0

#endif // BACKGROUND_H_INCLUDED
