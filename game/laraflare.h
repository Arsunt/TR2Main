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

#ifndef LARA_FLARE_H_INCLUDED
#define LARA_FLARE_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
int __cdecl DoFlareLight(PHD_VECTOR *pos, int flareAge); // 0x0042F840
void __cdecl DoFlareInHand(int flareAge); // 0x0042F8E0
void __cdecl DrawFlareInAir(ITEM_INFO *item);
void __cdecl CreateFlare(BOOL isFlying); // 0x0042FAC0
void __cdecl set_flare_arm(int frame); // 0x0042FCA0
void __cdecl draw_flare(); // 0x0042FCF0
void __cdecl undraw_flare(); // 0x0042FE60
void __cdecl draw_flare_meshes(); // 0x00430090
void __cdecl undraw_flare_meshes(); // 0x004300B0
void __cdecl ready_flare(); // 0x004300D0
void __cdecl FlareControl(__int16 itemID); // 0x00430110

#endif // LARA_FLARE_H_INCLUDED
