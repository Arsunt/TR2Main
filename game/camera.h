/*
 * Copyright (c) 2017-2021 Michael Chaban. All rights reserved.
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

#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "global/types.h"

typedef void(__cdecl *CB_SMARTCAM)(int*, int*, int*, int, int, int, int, int, int, int);

/*
 * Function list
 */
void __cdecl InitialiseCamera(); // 0x00410580
void __cdecl MoveCamera(GAME_VECTOR *destination, int speed); // 0x00410630
void __cdecl ClipCamera(int *x, int *y, int *z, int tx, int ty, int tz, int left, int top, int right, int bottom); // 0x004109B0
void __cdecl ShiftCamera(int *x, int *y, int *z, int tx, int ty, int tz, int left, int top, int right, int bottom); // 0x00410A90
FLOOR_INFO* __cdecl GoodPosition(int x, int y, int z, __int16 roomID); // 0x00410BF0
void __cdecl SmartShift(GAME_VECTOR *goal, CB_SMARTCAM shift); // 0x00410C40
void __cdecl ChaseCamera(ITEM_INFO *item); // 0x004113D0
int __cdecl ShiftClamp(GAME_VECTOR *pos, int clamp); // 0x004114C0
void __cdecl CombatCamera(ITEM_INFO *item); // 0x00411660
void __cdecl LookCamera(ITEM_INFO *item); // 0x004117F0
void __cdecl FixedCamera(); // 0x004119E0
void __cdecl CalculateCamera(); // 0x00411A80

#endif // CAMERA_H_INCLUDED
