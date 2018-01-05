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

#ifndef SCALESPR_H_INCLUDED
#define SCALESPR_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl S_DrawSprite(DWORD flags, int x, int y, int z, __int16 spriteIdx, __int16 shade, __int16 scale); // 0x0040C030
void __cdecl S_DrawPickup(int sx, int sy, int scale, __int16 spriteIdx, __int16 shade); // 0x0040C300
__int16 *__cdecl ins_room_sprite(__int16 *ptrObj, int vtxCount); // 0x0040C390
void __cdecl S_DrawScreenSprite2d(int sx, int sy, int sz, int scaleH, int scaleV, __int16 spriteIdx, __int16 shade); // 0x0040C4F0
void __cdecl S_DrawScreenSprite(int sx, int sy, int sz, int scaleH, int scaleV, __int16 spriteIdx, __int16 shade); // 0x0040C590
void __cdecl draw_scaled_spriteC(__int16 *ptrObj); // 0x0040C630

#endif // SCALESPR_H_INCLUDED
