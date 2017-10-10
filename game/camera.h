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

#ifndef CAMERA_H_INCLUDED
#define CAMERA_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
#define InitialiseCamera ((void(__cdecl*)(void)) 0x00410580)

// 0x00410630:		MoveCamera
// 0x004109B0:		ClipCamera
// 0x00410A90:		ShiftCamera
// 0x00410BF0:		BadPosition
// 0x00410C40:		SmartShift
// 0x004113D0:		ChaseCamera
// 0x004114C0:		ShiftClamp
// 0x00411660:		CombatCamera
// 0x004117F0:		LookCamera
// 0x004119E0:		FixedCamera
// 0x00411A80:		CalculateCamera

#endif // CAMERA_H_INCLUDED
