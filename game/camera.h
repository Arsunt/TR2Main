/*
 * Copyright (c) 2017-2021 Michael Chaban. All rights reserved.
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

#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "global/types.h"

typedef void(__cdecl *CB_SMARTCAM)(int*, int*, int*, int, int, int, int, int, int, int);

/*
 * Function list
 */
void __cdecl InitialiseCamera(); // 0x00410580
void __cdecl MoveCamera(GAME_VECTOR *destination, int speed); // 0x00410630

#define ClipCamera ((void(__cdecl*)(int*, int*, int*, int, int, int, int, int, int, int)) 0x004109B0)
#define ShiftCamera ((void(__cdecl*)(int*, int*, int*, int, int, int, int, int, int, int)) 0x00410A90)
#define GoodPosition ((FLOOR_INFO*(__cdecl*)(int, int, int, __int16)) 0x00410BF0)
#define SmartShift ((void(__cdecl*)(GAME_VECTOR*, CB_SMARTCAM)) 0x00410C40)
#define ChaseCamera ((void(__cdecl*)(ITEM_INFO*)) 0x004113D0)
#define ShiftClamp ((int(__cdecl*)(GAME_VECTOR*, int)) 0x004114C0)
#define CombatCamera ((void(__cdecl*)(ITEM_INFO*)) 0x00411660)

void __cdecl LookCamera(ITEM_INFO *item); // 0x004117F0
void __cdecl FixedCamera(); // 0x004119E0
void __cdecl CalculateCamera(); // 0x00411A80

#endif // CAMERA_H_INCLUDED
