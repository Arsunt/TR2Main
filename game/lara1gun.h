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

#ifndef LARA1GUN_H_INCLUDED
#define LARA1GUN_H_INCLUDED

#include "global/types.h"

/*
 * Function list
 */
//	0x0042BC90:		draw_shotgun_meshes
//	0x0042BCD0:		undraw_shotgun_meshes
//	0x0042BD00:		ready_shotgun

void __cdecl RifleHandler(int weaponType); // 0x0042BD70
void __cdecl FireShotgun(); // 0x0042BE70
void __cdecl FireM16(BOOL isRunning); // 0x0042BF70
void __cdecl FireHarpoon(); // 0x0042BFF0

//	0x0042C180:		ControlHarpoonBolt

void __cdecl FireRocket(); // 0x0042C4D0
void __cdecl ControlRocket(__int16 itemID); // 0x0042C5C0

#define draw_shotgun ((void(__cdecl*)(int)) 0x0042C9D0)
#define undraw_shotgun ((void(__cdecl*)(int)) 0x0042CB40)

void __cdecl AnimateShotgun(int gunType); // 0x0042CC50

#endif // LARA1GUN_H_INCLUDED
