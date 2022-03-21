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

#ifndef LARA_SWIM_H_INCLUDED
#define LARA_SWIM_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
void __cdecl LaraUnderWater(ITEM_INFO *item, COLL_INFO *coll); // 0x00432000

void __cdecl SwimTurn(ITEM_INFO *item); // 0x00432230
void __cdecl lara_as_swim(ITEM_INFO *item, COLL_INFO *coll); // 0x004322C0

//	0x00432330:		lara_as_glide
//	0x004323B0:		lara_as_tread
//	0x00432440:		lara_as_dive
//	0x00432460:		lara_as_uwdeath
//	0x004324C0:		lara_as_waterroll
//	0x004324D0:		lara_col_swim
//	----------:		lara_col_glide
//	----------:		lara_col_tread
//	----------:		lara_col_dive
//	0x004324F0:		lara_col_uwdeath
//	----------:		lara_col_waterroll

#define GetWaterDepth ((int(__cdecl*)(int, int, int, __int16)) 0x00432550)

//	0x004326F0:		LaraTestWaterDepth
//	0x004327C0:		LaraSwimCollision
#define LaraWaterCurrent ((void(__cdecl*)(COLL_INFO*)) 0x00432920)

#endif // LARA_SWIM_H_INCLUDED
